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

#ifndef RS02INCLUDES_H
#define RS02INCLUDES_H

/* Data structs from rs02-window.c */

typedef struct
{
   /*** Widgets for RS02 encoding */

   GtkWidget *encHeadline;
   GtkWidget *encLabel1;
   GtkWidget *encPBar1;
   GtkWidget *encLabel2;
   GtkWidget *encPBar2;
   GtkWidget *encFootline;
   GtkWidget *encFootline2;
   GtkWidget *curveButton;

   /*** Widgets for RS02 fixing */

   GtkWidget *fixHeadline;
   GtkWidget *fixDrawingArea;
   GtkWidget *fixNotebook;
   GtkWidget *fixFootline;
   GtkWidget *fixFootlineBox;
   GtkWidget *fixCorrected;
   GtkWidget *fixProgress;
   GtkWidget *fixUncorrected;
   Curve  *fixCurve;

   /*** Widgets in the Preferences window */

   GtkWidget *radio1,*radio2,*radio3,*radio4;
   GtkWidget *radio4Label;
   GtkWidget *redundancyScale;
   GtkWidget *redundancySpin;

   /*** Some state vars used during fixing */

   gint64 corrected;
   gint64 uncorrected;
   gint64 nSectors;
   int    eccBytes;
   int    dataBytes;
   int    percent, lastPercent;
} RS02Widgets;

/* 
 * These are exported via the Method struct 
 */

void CreateRS02EncWindow(Method*, GtkWidget*);
void CreateRS02FixWindow(Method*, GtkWidget*);
//void CreateRS02PrefsPage(Method*, GtkWidget*);
void ResetRS02EncWindow(Method*);
void ResetRS02FixWindow(Method*);

void ResetRS02CompareWindow(Method*);
void CreateRS02CompareWindow(Method*, GtkWidget*);

/*
 * These are exported (resp. only used) in ecc-rs02.c and rs02*.c
 * and should not be called from somewhere else as we can not
 * rely on the method plug-in being available.
 * If you need similar functions in your own codec,
 * please copy these functions over to the respective plug-in.
 */

/* rs02-compare.c */

#define COMPARE_IMAGE_SEGMENTS 1000

void RS02Compare(Method*);

/* rs02-common.c */

typedef struct _RS02Layout
{  guint64 mediumCapacity;       /* maximum capacity of medium */
   guint64 dataSectors;          /* number of sectors used for image data */
   guint64 crcSectors;           /* number of sectors needed for CRC32 sector checkums */
   guint64 firstEccHeader;       /* location of first ecc header */
   guint64 headers;              /* number of ecc header ("master block") repeats */
   guint64 headerModulo;         /* Modulo for header repeats */
   guint64 protectedSectors;     /* number of sectors protected by Reed-Solomon data */
   guint64 rsSectors;            /* number of sectors needed for Reed-Solomon data */
   guint64 eccSectors;           /* total number of sectors added to image */
   guint64 sectorsPerLayer;      /* sectors per RS layer (the are ndata layers) */
   guint64 firstCrcLayerIndex;   /* first slice containing a CRC32 data block */
   int nroots,ndata;            /* RS encoding specification */
   double redundancy;           /* resulting redundancy */
} RS02Layout;

void RS02ReadSector(ImageInfo*, RS02Layout*, unsigned char*, gint64);
gint64 RS02EccSectorIndex(RS02Layout*, gint64, gint64);
RS02Layout *CalcRS02Layout(gint64, int);
void WriteRS02Headers(LargeFile*, RS02Layout*, EccHeader*);

/* rs02-create.c */

void RS02Create(Method*);

/* rs02-fix.c */

void RS02Fix(Method*);

/* rs02-window.c */


#endif