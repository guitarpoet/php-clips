   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*            DEFGLOBAL COMMANDS HEADER FILE           */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
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
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_globlcom

#pragma once

#define _H_globlcom

   void                           DefglobalCommandDefinitions(void *);
   void                           SetResetGlobalsCommand(UDFContext *,CLIPSValue *);
   bool                           EnvSetResetGlobals(void *,bool);
   void                           GetResetGlobalsCommand(UDFContext *,CLIPSValue *);
   bool                           EnvGetResetGlobals(void *);
   void                           ShowDefglobalsCommand(UDFContext *,CLIPSValue *);
   void                           EnvShowDefglobals(void *,const char *,void *);

#endif /* _H_globlcom */

