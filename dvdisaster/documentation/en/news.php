<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2007 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

if($news_flash == 0) 
{  require("../include/dvdisaster.php");
   begin_page();
}

$news_counter = 0;

# The news page is different from the other pages;
# you must only use the news_*() functions below to create content.
# Do not insert plain HTML!

news_headline("dvdisaster News");

/*
news_item("xx.xx.2007", "dvdisaster 0.71.24 released", "
   dvdisaster 0.71.24 has just been released. This release marks the start of
   an online documentation rewrite, including a Russian translation made
   by Igor Gorbounov. Also new are the raw CD sector editor and some improvements
   in the raw reading heuristics. 
"); # end of news_item
*/

news_item("28.10.2007", "New documentation started", "
   The dvdisaster documentation is currently being reworked for the upcoming
   V0.72 release. Please be patient; the new documentation will hopefully be more
   useful than the old one, but we will need a few weeks to fill in all parts.
"); # end of news_item

if($news_flash == 0) 
   end_page();
?>