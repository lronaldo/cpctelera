/*
 * Simulator of microcontrollers (globals.cc)
 *
 * Copyright (C) 1997,16 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

//#include "ddconfig.h"

#include <stdio.h>

#include "stypes.h"


// No other global variable should exists!
class cl_app *application;


/* Delimiters to split up command line */
char delimiters[]= " ,;\t\r\n";

struct id_element mem_ids[]= {
  { MEM_ROM  , "ROM  " },
  { MEM_XRAM , "XRAM " },
  { MEM_IRAM , "IRAM " },
  { MEM_SFR  , "SFR  " },
  { MEM_IXRAM, "IXRAM" },
  { MEM_DUMMY, "DUMMY" },
  { 0, 0 }
};

struct id_element mem_classes[]= {
  { MEM_ROM  , "rom" },
  { MEM_XRAM , "xram" },
  { MEM_IRAM , "iram" },
  { MEM_SFR  , "sfr" },
  { MEM_IXRAM, "ixram" },
  { MEM_DUMMY, "dummy" },
  { 0, 0 }
};

struct id_element cpu_states[]= {
  { stGO,	"OK" },
  { stIDLE,	"Idle" },
  { stPD,	"PowerDown" },
  { 0, 0 }
};

struct id_element error_type_names[]= {
  { err_unknown,	"unclassified" },
  { err_error,		"error" },
  { err_warning,	"warning" }
};


const char *warranty= 
"                            NO WARRANTY\n"
"\n"
"  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
"FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
"OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
"PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
"OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
"MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
"TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
"PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
"REPAIR OR CORRECTION.\n"
"\n"
"  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
"WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
"REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
"INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
"OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
"TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
"YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
"PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
"POSSIBILITY OF SUCH DAMAGES.\n"
;


const char *copying=
"		    GNU GENERAL PUBLIC LICENSE\n"
"		       Version 2, June 1991\n"
"\n"
" Copyright (C) 1989, 1991 Free Software Foundation, Inc.\n"
" 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA\n"
" Everyone is permitted to copy and distribute verbatim copies\n"
" of this license document, but changing it is not allowed.\n"
"\n"
"			    Preamble\n"
"\n"
"  The licenses for most software are designed to take away your\n"
"freedom to share and change it.  By contrast, the GNU General Public\n"
"License is intended to guarantee your freedom to share and change free\n"
"software--to make sure the software is free for all its users.  This\n"
"General Public License applies to most of the Free Software\n"
"Foundation's software and to any other program whose authors commit to\n"
"using it.  (Some other Free Software Foundation software is covered by\n"
"the GNU Library General Public License instead.)  You can apply it to\n"
"your programs, too.\n"
"\n"
"  When we speak of free software, we are referring to freedom, not\n"
"price.  Our General Public Licenses are designed to make sure that you\n"
"have the freedom to distribute copies of free software (and charge for\n"
"this service if you wish), that you receive source code or can get it\n"
"if you want it, that you can change the software or use pieces of it\n"
"in new free programs; and that you know you can do these things.\n"
"\n"
"  To protect your rights, we need to make restrictions that forbid\n"
"anyone to deny you these rights or to ask you to surrender the rights.\n"
"These restrictions translate to certain responsibilities for you if you\n"
"distribute copies of the software, or if you modify it.\n"
"\n"
"  For example, if you distribute copies of such a program, whether\n"
"gratis or for a fee, you must give the recipients all the rights that\n"
"you have.  You must make sure that they, too, receive or can get the\n"
"source code.  And you must show them these terms so they know their\n"
"rights.\n"
"\n"
"  We protect your rights with two steps: (1) copyright the software, and\n"
"(2) offer you this license which gives you legal permission to copy,\n"
"distribute and/or modify the software.\n"
"\n"
"  Also, for each author's protection and ours, we want to make certain\n"
"that everyone understands that there is no warranty for this free\n"
"software.  If the software is modified by someone else and passed on, we\n"
"want its recipients to know that what they have is not the original, so\n"
"that any problems introduced by others will not reflect on the original\n"
"authors' reputations.\n"
"\n"
"  Finally, any free program is threatened constantly by software\n"
"patents.  We wish to avoid the danger that redistributors of a free\n"
"program will individually obtain patent licenses, in effect making the\n"
"program proprietary.  To prevent this, we have made it clear that any\n"
"patent must be licensed for everyone's free use or not licensed at all.\n"
"\n"
"  The precise terms and conditions for copying, distribution and\n"
"modification follow.\n"
"\n"
"		    GNU GENERAL PUBLIC LICENSE\n"
"   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n"
"\n"
"  0. This License applies to any program or other work which contains\n"
"a notice placed by the copyright holder saying it may be distributed\n"
"under the terms of this General Public License.  The \"Program\", below,\n"
"refers to any such program or work, and a \"work based on the Program\"\n"
"means either the Program or any derivative work under copyright law:\n"
"that is to say, a work containing the Program or a portion of it,\n"
"either verbatim or with modifications and/or translated into another\n"
"language.  (Hereinafter, translation is included without limitation in\n"
"the term \"modification\".)  Each licensee is addressed as \"you\".\n"
"\n"
"Activities other than copying, distribution and modification are not\n"
"covered by this License; they are outside its scope.  The act of\n"
"running the Program is not restricted, and the output from the Program\n"
"is covered only if its contents constitute a work based on the\n"
"Program (independent of having been made by running the Program).\n"
"Whether that is true depends on what the Program does.\n"
"\n"
"  1. You may copy and distribute verbatim copies of the Program's\n"
"source code as you receive it, in any medium, provided that you\n"
"conspicuously and appropriately publish on each copy an appropriate\n"
"copyright notice and disclaimer of warranty; keep intact all the\n"
"notices that refer to this License and to the absence of any warranty;\n"
"and give any other recipients of the Program a copy of this License\n"
"along with the Program.\n"
"\n"
"You may charge a fee for the physical act of transferring a copy, and\n"
"you may at your option offer warranty protection in exchange for a fee.\n"
"\n"
"  2. You may modify your copy or copies of the Program or any portion\n"
"of it, thus forming a work based on the Program, and copy and\n"
"distribute such modifications or work under the terms of Section 1\n"
"above, provided that you also meet all of these conditions:\n"
"\n"
"    a) You must cause the modified files to carry prominent notices\n"
"    stating that you changed the files and the date of any change.\n"
"\n"
"    b) You must cause any work that you distribute or publish, that in\n"
"    whole or in part contains or is derived from the Program or any\n"
"    part thereof, to be licensed as a whole at no charge to all third\n"
"    parties under the terms of this License.\n"
"\n"
"    c) If the modified program normally reads commands interactively\n"
"    when run, you must cause it, when started running for such\n"
"    interactive use in the most ordinary way, to print or display an\n"
"    announcement including an appropriate copyright notice and a\n"
"    notice that there is no warranty (or else, saying that you provide\n"
"    a warranty) and that users may redistribute the program under\n"
"    these conditions, and telling the user how to view a copy of this\n"
"    License.  (Exception: if the Program itself is interactive but\n"
"    does not normally print such an announcement, your work based on\n"
"    the Program is not required to print an announcement.)\n"
"\n"
"These requirements apply to the modified work as a whole.  If\n"
"identifiable sections of that work are not derived from the Program,\n"
"and can be reasonably considered independent and separate works in\n"
"themselves, then this License, and its terms, do not apply to those\n"
"sections when you distribute them as separate works.  But when you\n"
"distribute the same sections as part of a whole which is a work based\n"
"on the Program, the distribution of the whole must be on the terms of\n"
"this License, whose permissions for other licensees extend to the\n"
"entire whole, and thus to each and every part regardless of who wrote it.\n"
"\n"
"Thus, it is not the intent of this section to claim rights or contest\n"
"your rights to work written entirely by you; rather, the intent is to\n"
"exercise the right to control the distribution of derivative or\n"
"collective works based on the Program.\n"
"\n"
"In addition, mere aggregation of another work not based on the Program\n"
"with the Program (or with a work based on the Program) on a volume of\n"
"a storage or distribution medium does not bring the other work under\n"
"the scope of this License.\n"
"\n"
"  3. You may copy and distribute the Program (or a work based on it,\n"
"under Section 2) in object code or executable form under the terms of\n"
"Sections 1 and 2 above provided that you also do one of the following:\n"
"\n"
"    a) Accompany it with the complete corresponding machine-readable\n"
"    source code, which must be distributed under the terms of Sections\n"
"    1 and 2 above on a medium customarily used for software interchange; or,\n"
"\n"
"    b) Accompany it with a written offer, valid for at least three\n"
"    years, to give any third party, for a charge no more than your\n"
"    cost of physically performing source distribution, a complete\n"
"    machine-readable copy of the corresponding source code, to be\n"
"    distributed under the terms of Sections 1 and 2 above on a medium\n"
"    customarily used for software interchange; or,\n"
"\n"
"    c) Accompany it with the information you received as to the offer\n"
"    to distribute corresponding source code.  (This alternative is\n"
"    allowed only for noncommercial distribution and only if you\n"
"    received the program in object code or executable form with such\n"
"    an offer, in accord with Subsection b above.)\n"
"\n"
"The source code for a work means the preferred form of the work for\n"
"making modifications to it.  For an executable work, complete source\n"
"code means all the source code for all modules it contains, plus any\n"
"associated interface definition files, plus the scripts used to\n"
"control compilation and installation of the executable.  However, as a\n"
"special exception, the source code distributed need not include\n"
"anything that is normally distributed (in either source or binary\n"
"form) with the major components (compiler, kernel, and so on) of the\n"
"operating system on which the executable runs, unless that component\n"
"itself accompanies the executable.\n"
"\n"
"If distribution of executable or object code is made by offering\n"
"access to copy from a designated place, then offering equivalent\n"
"access to copy the source code from the same place counts as\n"
"distribution of the source code, even though third parties are not\n"
"compelled to copy the source along with the object code.\n"
"\n"
"  4. You may not copy, modify, sublicense, or distribute the Program\n"
"except as expressly provided under this License.  Any attempt\n"
"otherwise to copy, modify, sublicense or distribute the Program is\n"
"void, and will automatically terminate your rights under this License.\n"
"However, parties who have received copies, or rights, from you under\n"
"this License will not have their licenses terminated so long as such\n"
"parties remain in full compliance.\n"
"\n"
"  5. You are not required to accept this License, since you have not\n"
"signed it.  However, nothing else grants you permission to modify or\n"
"distribute the Program or its derivative works.  These actions are\n"
"prohibited by law if you do not accept this License.  Therefore, by\n"
"odifying or distributing the Program (or any work based on the\n"
"Program), you indicate your acceptance of this License to do so, and\n"
"all its terms and conditions for copying, distributing or modifying\n"
"the Program or works based on it.\n"
"\n"
"  6. Each time you redistribute the Program (or any work based on the\n"
"Program), the recipient automatically receives a license from the\n"
"original licensor to copy, distribute or modify the Program subject to\n"
"these terms and conditions.  You may not impose any further\n"
"restrictions on the recipients' exercise of the rights granted herein.\n"
"You are not responsible for enforcing compliance by third parties to\n"
"this License.\n"
"\n"
"  7. If, as a consequence of a court judgment or allegation of patent\n"
"infringement or for any other reason (not limited to patent issues),\n"
"conditions are imposed on you (whether by court order, agreement or\n"
"otherwise) that contradict the conditions of this License, they do not\n"
"excuse you from the conditions of this License.  If you cannot\n"
"distribute so as to satisfy simultaneously your obligations under this\n"
"License and any other pertinent obligations, then as a consequence you\n"
"may not distribute the Program at all.  For example, if a patent\n"
"license would not permit royalty-free redistribution of the Program by\n"
"all those who receive copies directly or indirectly through you, then\n"
"the only way you could satisfy both it and this License would be to\n"
"refrain entirely from distribution of the Program.\n"
"\n"
"If any portion of this section is held invalid or unenforceable under\n"
"any particular circumstance, the balance of the section is intended to\n"
"apply and the section as a whole is intended to apply in other\n"
"circumstances.\n"
"\n"
"It is not the purpose of this section to induce you to infringe any\n"
"patents or other property right claims or to contest validity of any\n"
"such claims; this section has the sole purpose of protecting the\n"
"integrity of the free software distribution system, which is\n"
"implemented by public license practices.  Many people have made\n"
"generous contributions to the wide range of software distributed\n"
"through that system in reliance on consistent application of that\n"
"system; it is up to the author/donor to decide if he or she is willing\n"
"to distribute software through any other system and a licensee cannot\n"
"impose that choice.\n"
"\n"
"This section is intended to make thoroughly clear what is believed to\n"
"be a consequence of the rest of this License.\n"
"\n"
"  8. If the distribution and/or use of the Program is restricted in\n"
"certain countries either by patents or by copyrighted interfaces, the\n"
"original copyright holder who places the Program under this License\n"
"may add an explicit geographical distribution limitation excluding\n"
"those countries, so that distribution is permitted only in or among\n"
"countries not thus excluded.  In such case, this License incorporates\n"
"the limitation as if written in the body of this License.\n"
"\n"
"  9. The Free Software Foundation may publish revised and/or new versions\n"
"of the General Public License from time to time.  Such new versions will\n"
"be similar in spirit to the present version, but may differ in detail to\n"
"address new problems or concerns.\n"
"\n"
"Each version is given a distinguishing version number.  If the Program\n"
"specifies a version number of this License which applies to it and \"any\n"
"later version\", you have the option of following the terms and conditions\n"
"either of that version or of any later version published by the Free\n"
"Software Foundation.  If the Program does not specify a version number of\n"
"this License, you may choose any version ever published by the Free Software\n"
"Foundation.\n"
"\n"
"  10. If you wish to incorporate parts of the Program into other free\n"
"programs whose distribution conditions are different, write to the author\n"
"to ask for permission.  For software which is copyrighted by the Free\n"
"Software Foundation, write to the Free Software Foundation; we sometimes\n"
"make exceptions for this.  Our decision will be guided by the two goals\n"
"of preserving the free status of all derivatives of our free software and\n"
"of promoting the sharing and reuse of software generally.\n"
;


/*
 * Information about different type of CPUs
 */

struct cpu_entry *cpus= 0;

struct cpu_entry cpus_51[]=
  {
    {"C52"   , CPU_52, CPU_CMOS			, "mcs52"	, "cmos"},

    {"51"    , CPU_51, CPU_HMOS			, "mcs51"	, "hmos"},
    {"8051"  , CPU_51, CPU_HMOS			, "mcs51"	, "hmos"},
    {"8751"  , CPU_51, CPU_HMOS			, "mcs51"	, "hmos"},
    {"C51"   , CPU_51, CPU_CMOS			, "mcs51"	, "cmos"},
    {"80C51" , CPU_51, CPU_CMOS			, "mcs51"	, "cmos"},
    {"87C51" , CPU_51, CPU_CMOS			, "mcs51"	, "cmos"},
    {"31"    , CPU_31, CPU_HMOS			, "mcs51"	, "hmos"},
    {"8031"  , CPU_31, CPU_HMOS			, "mcs51"	, "hmos"},
    {"C31"   , CPU_31, CPU_CMOS			, "mcs51"	, "cmos"},
    {"80C31" , CPU_31, CPU_CMOS			, "mcs51"	, "cmos"},
    
    {"52"    , CPU_52, CPU_HMOS			, "mcs52"	, "hmos"},
    {"8052"  , CPU_52, CPU_HMOS			, "mcs51"	, "hmos"},
    {"8752"  , CPU_52, CPU_HMOS			, "mcs51"	, "hmos"},
    {"80C52" , CPU_52, CPU_CMOS			, "mcs51"	, "cmos"},
    {"87C52" , CPU_52, CPU_CMOS			, "mcs51"	, "cmos"},
    {"32"    , CPU_32, CPU_HMOS			, "mcs51"	, "hmos"},
    {"8032"  , CPU_32, CPU_HMOS			, "mcs51"	, "hmos"},
    {"C32"   , CPU_32, CPU_CMOS			, "mcs51"	, "cmos"},
    {"80C32" , CPU_32, CPU_CMOS			, "mcs51"	, "cmos"},
    {"F380"  , CPU_F380, CPU_CMOS		, "Silabs F380"	, "cmos"},
    
    
    {"51R"   , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    {"51RA"  , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    {"51RB"  , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    {"51RC"  , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    {"C51R"  , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    {"C51RA" , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    {"C51RB" , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    {"C51RC" , CPU_51R, CPU_CMOS		, "Intel mcs51R", "cmos"},
    
    {"89C51R", CPU_89C51R, CPU_CMOS		, "89C51R"	, "cmos"},
    
    {"C521"  , CPU_C521  , CPU_CMOS		, "AMD 80C521"	, "cmos"},
    {"521"   , CPU_C521  , CPU_CMOS		, "AMD 80C521"	, "cmos"},
    
    {"251"   , CPU_251, CPU_CMOS		, "Intel mcs251", "cmos"},
    {"C251"  , CPU_251, CPU_CMOS		, "Intel mcs251", "cmos"},
    
    {"517"   , CPU_517, CPU_CMOS		, "Siemens C517", "cmos"},
    {"C517"  , CPU_517, CPU_CMOS		, "Siemens C517", "cmos"},
    
    {"88X"   , CPU_XC88X, CPU_CMOS		, "Infineon XC88[68]", "cmos"},
    {"886"   , CPU_XC88X, CPU_CMOS		, "Infineon XC88[68]", "cmos"},
    {"888"   , CPU_XC88X, CPU_CMOS		, "Infineon XC88[68]", "cmos"},
    {"XC88X" , CPU_XC88X, CPU_CMOS		, "Infineon XC88[68]", "cmos"},
    {"XC886" , CPU_XC88X, CPU_CMOS		, "Infineon XC88[68]", "cmos"},
    {"XC888" , CPU_XC88X, CPU_CMOS		, "Infineon XC88[68]", "cmos"},

    {"DS320" , CPU_DS320, CPU_CMOS    , "Dallas DS80C320" , "cmos"},
    // F type should go to subtype...
    {"DS390" , CPU_DS390, CPU_CMOS		, "Dallas DS80C390" , "cmos"},
    {"DS390F", CPU_DS390F, CPU_CMOS		, "Dallas DS80C390F", "cmos"},
    
    {NULL, CPU_NONE, 0, "", ""}
  };

struct cpu_entry cpus_z80[]=
  {
    {"Z80"    , CPU_Z80,     0			, ""		, ""},

    {"Z180"   , CPU_Z180,    0			, ""		, ""},
    {"R2K"    , CPU_R2K,     0			, ""		, ""},
    {"R3KA"   , CPU_R3KA,    0			, ""		, ""},
    {"LR35902", CPU_LR35902, 0			, ""		, ""},

    {"EZ80"   , CPU_EZ80,    0			, ""		, ""},
    
    {NULL, CPU_NONE, 0, "", ""}
  };

struct cpu_entry cpus_hc08[]=
  {
    {"HC08"   , CPU_HC08, 0			, ""		, ""},
    
    {"HCS08"  , CPU_HCS08,0			, ""		, ""},
    
    {NULL, CPU_NONE, 0, "", ""}
  };

struct cpu_entry cpus_stm8[]=
  {
    {"S"		, CPU_STM8S, DEV_STM8S208	, "S"		, "208"},
    
    {"STM8S"  		, CPU_STM8S, DEV_STM8S208	, "S"		, "208"},

    {"S903"		, CPU_STM8S, DEV_STM8S903	, "S"		, "903"},
    {"STM8S903"		, CPU_STM8S, DEV_STM8S903	, "S"		, "903"},
    {"S003"		, CPU_STM8S, DEV_STM8S003	, "S"		, "003"},
    {"STM8S003"		, CPU_STM8S, DEV_STM8S003	, "S"		, "003"},
    {"S005"		, CPU_STM8S, DEV_STM8S005	, "S"		, "005"},
    {"STM8S005"		, CPU_STM8S, DEV_STM8S005	, "S"		, "005"},
    {"S007"		, CPU_STM8S, DEV_STM8S007	, "S"		, "007"},
    {"STM8S007"		, CPU_STM8S, DEV_STM8S007	, "S"		, "007"},
    {"S103"		, CPU_STM8S, DEV_STM8S103	, "S"		, "103"},
    {"STM8S103"		, CPU_STM8S, DEV_STM8S103	, "S"		, "103"},
    {"S105"		, CPU_STM8S, DEV_STM8S105	, "S"		, "105"},
    {"STM8S105"		, CPU_STM8S, DEV_STM8S105	, "S"		, "105"},
    {"SDISC"		, CPU_STM8S, DEV_STM8S105	, "S"		, "105"},
    {"S207"		, CPU_STM8S, DEV_STM8S207	, "S"		, "207"},
    {"STM8S207"		, CPU_STM8S, DEV_STM8S207	, "S"		, "207"},
    {"S208"		, CPU_STM8S, DEV_STM8S208	, "S"		, "208"},
    {"STM8S208"		, CPU_STM8S, DEV_STM8S208	, "S"		, "208"},

    {"AF52"		, CPU_STM8S, DEV_STM8AF52	, "AF"		, "52"},
    {"STM8AF52"		, CPU_STM8S, DEV_STM8AF52	, "AF"		, "52"},
    {"AF62_12"		, CPU_STM8S, DEV_STM8AF62_12	, "AF"		, "62[12]3, 622[36]A"},
    {"AF621"		, CPU_STM8S, DEV_STM8AF62_12	, "AF"		, "62[12]3, 622[36]A"},
    {"AF622"		, CPU_STM8S, DEV_STM8AF62_12	, "AF"		, "62[12]3, 622[36]A"},
    {"STM8AF62_12"	, CPU_STM8S, DEV_STM8AF62_12	, "AF"		, "62[12]3, 622[36]A"},
    {"AF62"		, CPU_STM8S, DEV_STM8AF62_46	, "A[FP]"	, "624[68], 626[68]"},
    {"AF62_46"		, CPU_STM8S, DEV_STM8AF62_46	, "A[FP]"	, "624[68], 626[68]"},
    {"AF624"		, CPU_STM8S, DEV_STM8AF62_46	, "A[FP]"	, "624[68], 626[68]"},
    {"AF626"		, CPU_STM8S, DEV_STM8AF62_46	, "A[FP]"	, "624[68], 626[68]"},
    {"STM8AF62_46"	, CPU_STM8S, DEV_STM8AF62_46	, "A[FP]"	, "624[68], 626[68]"},
    
    {"L"      		, CPU_STM8L, DEV_STM8L162	, "L"		, "162[RM]8"},
    {"STM8L"  		, CPU_STM8L, DEV_STM8L162	, "L"		, "162[RM]8"},

    {"AL3xE"		, CPU_STM8L, DEV_STM8AL3xE	, "AL"		, "31E8x, 3LE8x"},
    {"AL31E"		, CPU_STM8L, DEV_STM8AL3xE	, "AL"		, "31E8x, 3LE8x"},
    {"AL3LE"		, CPU_STM8L, DEV_STM8AL3xE	, "AL"		, "31E8x, 3LE8x"},
    {"STM8AL3xE"	, CPU_STM8L, DEV_STM8AL3xE	, "AL"		, "31E8x, 3LE8x"},
    {"AL3x8"		, CPU_STM8L, DEV_STM8AL3x8	, "AL"		, "318x, 3L8x"},
    {"AL318"		, CPU_STM8L, DEV_STM8AL3x8	, "AL"		, "318x, 3L8x"},
    {"AL3L8"		, CPU_STM8L, DEV_STM8AL3x8	, "AL"		, "318x, 3L8x"},
    {"STM8AL3x8"	, CPU_STM8L, DEV_STM8AL3x8	, "AL"		, "318x, 3L8x"},
    {"AL3x346"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL3x3"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL313"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL3L3"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL3x4"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL314"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL3L4"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL3x6"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL316"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"AL3L6"		, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},
    {"STM8AL3x346"	, CPU_STM8L, DEV_STM8AL3x346	, "AL"		, "31[346]x, 3L[46]x"},

    {"L051"		, CPU_STM8L, DEV_STM8L051	, "L"		, "051F3"},
    {"STM8L051"		, CPU_STM8L, DEV_STM8L051	, "L"		, "051F3"},
    {"L052C"		, CPU_STM8L, DEV_STM8L052C	, "L"		, "052C6"},
    {"STM8L052C"	, CPU_STM8L, DEV_STM8L052C	, "L"		, "052C6"},
    {"L052R"		, CPU_STM8L, DEV_STM8L052R	, "L"		, "052R8"},
    {"L052"		, CPU_STM8L, DEV_STM8L052R	, "L"		, "052R8"},
    {"STM8L052R"	, CPU_STM8L, DEV_STM8L052R	, "L"		, "052R8"},
    {"L151x23"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151x2"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151x3"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L1512"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L1513"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151F3"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151G3"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151K3"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151C3"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151F2"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151G2"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151K2"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L151C2"		, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"STM8L151x23"	, CPU_STM8L, DEV_STM8L151x23	, "L"		, "151[CKGF][23]"},
    {"L15x46"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"L151G4"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"L151G6"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"L1514"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"L1524"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"L1516"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"L1526"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"STM8L15x46"	, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"LDISC"		, CPU_STM8L, DEV_STM8L15x46	, "L"		, "151x[46], 152x[46]"},
    {"L15x8"		, CPU_STM8L, DEV_STM8L15x8	, "L"		, "151x[68], 152x[68]"},
    {"STM8L15x8"	, CPU_STM8L, DEV_STM8L15x8	, "L"		, "151x[68], 152x[68]"},
    {"L162"		, CPU_STM8L, DEV_STM8L162	, "L"		, "162[RM]8"},
    {"STM8L162"		, CPU_STM8L, DEV_STM8L162	, "L"		, "162[RM]8"},
    
    {"L101"     	, CPU_STM8L101, DEV_STM8L101	, "L101"	, "L101xx"},
    {"STM8L101" 	, CPU_STM8L101, DEV_STM8L101	, "L101"	, "L101xx"},

    {NULL, CPU_NONE, 0, "", ""}
  };

struct cpu_entry cpus_pdk[]=
  {
    {"PDK13"   , CPU_PDK13, 0			, "PDK13"		, ""},
    {"PMC153"  , CPU_PDK13, 0			, "PDK13"		, ""},
    {"PDK14"   , CPU_PDK14, 0			, "PDK14"		, ""},
    {"PMS132B" , CPU_PDK14, 0			, "PDK14"		, ""},
    {"PDK15"   , CPU_PDK15, 0			, "PDK15"		, ""},
    {"PMS134"  , CPU_PDK15, 0			, "PDK15"		, ""},
    
    {NULL, CPU_NONE, 0, "", ""}
  };

/* End of globals.cc */
