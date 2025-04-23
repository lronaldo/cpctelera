/*-------------------------------------------------------------------------
  cmd.h - header  file for debugger command execution
        Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1999)

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

#ifndef SDCDB_CMD_H
#define SDCDB_CMD_H
/* forward definition for command functions */
extern int cmdSetTmpUserBp (char *, context *);
extern int cmdSetUserBp (char *, context *);
extern int cmdClrUserBp (char *, context *);
extern int cmdHelp      (char *, context *);
extern int cmdJump      (char *, context *);
extern int cmdListSrc   (char *, context *);
extern int cmdListAsm   (char *, context *);
extern int cmdSetOption (char *, context *);
extern int cmdCondition (char *, context *);
extern int cmdIgnore    (char *, context *);
extern int cmdContinue  (char *, context *);
extern int cmdDelUserBp (char *, context *);
extern int cmdStep      (char *, context *);
extern int cmdRun       (char *, context *);
extern int cmdNext      (char *, context *);
extern int cmdPrint     (char *, context *);
extern int cmdFrame     (char *, context *);
extern int cmdSimulator (char *, context *);
extern int cmdQuit      (char *, context *);
extern int cmdPrintType (char *, context *);
extern int cmdFile      (char *, context *);
extern int cmdInfo      (char *, context *);
extern int cmdShow      (char *, context *);
extern int cmdFinish    (char *, context *);
extern int cmdCommands  (char *, context *);
extern int cmdStepi     (char *, context *);
extern int cmdNexti     (char *, context *);
extern int cmdUp        (char *, context *);
extern int cmdDown      (char *, context *);
extern int cmdWhere     (char *, context *);
extern int cmdOutput    (char *, context *);
extern int cmdDisasm1   (char *, context *);
extern int cmdDisasmF   (char *, context *);
extern int cmdDisplay   (char *, context *);
extern int cmdUnDisplay (char *, context *);
extern int cmdSource    (char *, context *);
extern void displayAll  (context *);

extern int cmdListModules (char *s, context *cctxt);
extern int cmdListFunctions (char *s, context *cctxt);
extern int cmdListSymbols (char *s, context *cctxt);

extern void setMainContext( void);
extern function *needExtraMainFunction(void);
int conditionIsTrue( char *s, context *cctxt);

#endif
