   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*         DEFGLOBAL BASIC COMMANDS HEADER FILE        */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*            Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*            Changed name of variable log to logName        */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Moved WatchGlobals global to defglobalData.    */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_globlbsc

#pragma once

#define _H_globlbsc

#include "evaluatn.h"

   void                           DefglobalBasicCommands(void *);
   void                           UndefglobalCommand(UDFContext *,CLIPSValue *);
   bool                           EnvUndefglobal(void *,void *);
   void                           GetDefglobalListFunction(UDFContext *,CLIPSValue *);
   void                           EnvGetDefglobalList(void *,DATA_OBJECT_PTR,void *);
   void                           DefglobalModuleFunction(UDFContext *,CLIPSValue *);
   void                           PPDefglobalCommand(UDFContext *,CLIPSValue *);
   int                            PPDefglobal(void *,const char *,const char *);
   void                           ListDefglobalsCommand(UDFContext *,CLIPSValue *);
#if DEBUGGING_FUNCTIONS
   bool                           EnvGetDefglobalWatch(void *,void *);
   void                           EnvListDefglobals(void *,const char *,void *);
   void                           EnvSetDefglobalWatch(void *,bool,void *);
#endif
   void                           ResetDefglobals(void *);

#endif /* _H_globlbsc */


