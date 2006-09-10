/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2006 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
 *  or direct your browser at http://www.gnu.org.
 */

#include "dvdisaster.h"

/***
 *** Create our local working context
 ***/

RawBuffer *CreateRawBuffer(void)
{  RawBuffer *rb;
   int i;

   rb = g_malloc0(sizeof(RawBuffer));
   rb->rawBuf    = g_malloc(Closure->rawAttempts * sizeof(unsigned char*));
   rb->rawState  = g_malloc(Closure->rawAttempts * sizeof(int));
   rb->cdFrame   = g_malloc(2352);
   rb->byteState = g_malloc(2352);

   for(i=0; i<Closure->rawAttempts; i++)
   {  rb->rawBuf[i] = g_malloc(2352);
   }
   return rb;
}

void FreeRawBuffer(RawBuffer *rb)
{  int i;

   for(i=0; i<Closure->rawAttempts; i++)
     g_free(rb->rawBuf[i]);

   g_free(rb->rawBuf);
   g_free(rb->rawState);
   g_free(rb->cdFrame);
   g_free(rb->byteState);
   g_free(rb);
}

/***
 *** CD level CRC and ECC calculations
 ***/

#if 0  /* currently unused */
/* Convert LBA sector number into MSF format */

static void lba_to_msf(int lba, unsigned char *minute, unsigned char *second, unsigned char
*frame)
{
  *frame = lba % 75;
  lba /= 75;
  lba += 2;             /* address + 150 frames */
  *second = lba % 60;
  *minute = lba / 60;
}

/* Convert byte into BCD notation */

static int int_to_bcd(int value)
{
  return ((value / 10) << 4) | (value % 10);
}

/* Initialize Sync, MSF and data mode bytes */

static void initialize_cd_frame(unsigned char *cd_frame, int sector)
{  unsigned char minute, second, frame;
   int i;

   memset(cd_frame, 0, 2352);  /* defensive programming */

   /* 12 sync bytes 0x00, 0xff, 0xff, ..., 0xff, 0xff, 0x00 */

   cd_frame[0] = 0x00;
   for(i=1; i<=10; i++)
     cd_frame[i] = 0xff;
   cd_frame[11] = 0x00;

   /* MSF of sector address, BCD encoded */

   lba_to_msf(sector, &minute, &second, &frame);
   cd_frame[12] = int_to_bcd(minute);
   cd_frame[13] = int_to_bcd(second);
   cd_frame[14] = int_to_bcd(frame);

   /* Data mode */
   
   cd_frame[15] = 0x01;
}
#endif

/*
 * Test raw sector against its 32bit CRC.
 * Returns TRUE if frame is good.
 */

static int check_edc(unsigned char *cd_frame)
{ unsigned int expected_crc, real_crc;

  /* Get CRC from CD frame (byte position 2064) */

   expected_crc =  cd_frame[0x810] << 24
                 | cd_frame[0x811] << 16
                 | cd_frame[0x812] <<  8
                 | cd_frame[0x813];

   expected_crc = SwapBytes32(expected_crc);  /* CRC on disc is big endian */

   real_crc = EDCCrc32(cd_frame, 2064);

   printf("CRC   : %x\n", expected_crc);
   printf("My CRC: %x\n", real_crc);

   return expected_crc == real_crc;
}

/***
 *** The majority decision algorithm
 ***/   

static int majority_decision(unsigned char *cd_frame, RawBuffer *rb)
{  int byte_votes[256];
   int byte_nominations[256];
   int buffer_score[rb->samplesRead];
   unsigned char tie[rb->sampleLength];
   int smallest = 256;
   int ties_encountered = FALSE;
   int i,j;

   Verbose("Doing majority decision for %d samples\n", rb->samplesRead);

   /*** We need at last three samples for a majority */

   if(rb->samplesRead < 3)
     return FALSE;

   /*** If a read buffer contributed a byte to a winning votes,
	its score get upped by one. Read buffers which have been
	flagged as valid by the drive start with a score of
	rb->sampleLength (1 point for each byte contained). */

   for(i=0; i<rb->samplesRead; i++)
     if(rb->rawState == RAW_SUCCESS)  
           buffer_score[i] = rb->sampleLength;
     else  buffer_score[i] = 0;

   /*** Reset the number of ties */

   memset(tie, 0, rb->sampleLength);

   /*** Count votes for each byte */

   for(i=0; i<rb->sampleLength; i++)
   {  unsigned char chosen_byte = 0;
      int max_vote = -1;

      memset(byte_votes, 0, 256*sizeof(int));

      /* See how often each byte appeared */

      for(j=0; j<rb->samplesRead; j++)
	 byte_votes[rb->rawBuf[j][i]]++;

      /* Determine which byte was found most */

      for(j=0; j<256; j++)
	if(byte_votes[j] > max_vote) 
	{  max_vote = byte_votes[j];
	   chosen_byte = j;
	   tie[i] = FALSE;
	}
        else 
	  if(byte_votes[j] == max_vote)
	    tie[i] = TRUE;

      /* If no tie was detected, use the chosen byte.
	 Otherwise put off decision for later. */

      if(tie[i] == 0)
      {  cd_frame[i] = chosen_byte;

	 for(j=0; j<rb->samplesRead; j++)  /* Raise score for winning buffers */
	   if(rb->rawBuf[j][i] == chosen_byte)
	     buffer_score[j]++;

	 if(smallest > byte_votes[chosen_byte]) /* debugging stats */
	   smallest = byte_votes[chosen_byte];
      }
      else
      {  ties_encountered = TRUE;
      }
   }

   Verbose("smallest majority was %d.\n", smallest);

   /*** Resolve ties.
	If two or more bytes got the same number of votes,
	vote again giving each buffer buffer_score[j] votes 
	instead of one. */

   if(ties_encountered)
   {  Verbose("resolving ties...\n");

      for(i=0; i<rb->sampleLength; i++)
      {  if(tie[i])
	 {  int max_votes = -1;
	    int max_nominations = -1;
	    int chosen_byte = 0;

	    Verbose("Tie for byte %4x:\n", i);

	    /* Vote again by counting the number of nominations
	       of each bytes (= buffers containing it) and the
	       score of its voting buffers. The byte winning in
	       both categories is selected. */

	    memset(byte_votes, 0, 256*sizeof(int));
	    memset(byte_nominations, 0, 256*sizeof(int));

	    for(j=0; j<rb->samplesRead; j++)
	    {  byte_votes[rb->rawBuf[j][i]] += buffer_score[j];
	       byte_nominations[rb->rawBuf[j][i]]++;
	       Verbose(" %2x [%d]\n", rb->rawBuf[j][i], buffer_score[j]);
	    }

	    /* Pick byte with highest score,
	       if a tie happens again, ignore. */

	    for(j=0; j<256; j++)
	    {  if(byte_nominations[j] > max_nominations)
	       {  max_nominations = byte_nominations[j];
		  max_votes = byte_votes[j];
		  chosen_byte = j;
	       }
	       else if(   byte_nominations[j] == max_nominations 
		       && byte_votes[j] > max_votes) 
	       {  max_votes = byte_votes[j];
		  chosen_byte = j;
	       }
	    }

	    cd_frame[i] = chosen_byte;
	    Verbose("picked %2x with %d nominations and %d votes\n", 
		    chosen_byte, max_nominations, max_votes);
	 }
      }
   }
   
   return TRUE;
}

/***
 *** Perform analysis and recovery of raw read data.
 ***
 * RawBuffer contains the following:
 * 
 * rb->rawBuf[]       the samples from the raw read attempts, unscrambled
 * rb->samplesRead    the number of samples in the array above
 * rb->sampleLength   the length of each sample
 *
 * e.g. rb->rawBuf contains samplesRead samples of sampleLength bytes,
 * and the max array dimensions are rb->rawBuf[samplesRead][sampleLength]
 *
 * rb->rawState       the drives comments about the respective reading attempt:
 *                    RAW_SUCCESS    - drive considered read to be good
 *                    RAW_READ_ERROR - drive could not correct the sector,
 *                                     but something was read
 *
 * rb->cdFrame        working buffer for recovering the CD frame, unscrambled
 * rb->byteState      state of bytes in rb->cdFrame as a guidance to error correction:
 *                    FRAME_BYTE_UNKNOWN - state of byte is unknown
 *                    FRAME_BYTE_ERROR   - byte is wrong, treat as erasure
 *                    FRAME_BYTE_GOOD    - we could somehow prove that byte is correct
 * rb->lba            sector we are currently working on
 *
 * Return TRUE if data was recovered successfully, else return FALSE.
 */

int RecoverRaw(unsigned char *out, RawBuffer *rb)
{  
   /*** Try the simple majority algorithm first. */

   if(majority_decision(rb->cdFrame, rb))
   { 
     /* See if the created CD frame is L-EC recoverable. */

#if 0
     if(try_l_ec(rb->cdFrame))   /* not yet implemented */
     {  Verbose("Sector %d sucessfully recovered by L_EC after majority algorithm.\n", rb->lba);

        HexDump(rb->cdFrame, rb->sampleLength, 32);
        memcpy(out, rb->cdFrame+16, 2048);
	return TRUE;
     }

#endif

     /* If the CRC is good, return the user data portion */

     if(check_edc(rb->cdFrame))
     {  Verbose("Sector %d sucessfully recovered by majority algorithm.\n", rb->lba);

        HexDump(rb->cdFrame, rb->sampleLength, 32);
        memcpy(out, rb->cdFrame+16, 2048);
	return TRUE;
     }
     else
     {  Verbose("Sector %d NOT recovered by majority algorithm.\n", rb->lba);
     }
   }

   /*** Try harder. Andrei? ;-) */

   return FALSE;
}
  