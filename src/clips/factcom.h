   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*               FACT COMMANDS HEADER FILE             */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*      6.24: Added environment parameter to GenClose.       */
/*            Added environment parameter to GenOpen.        */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Support for long long integers.                */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*            Added code to prevent a clear command from     */
/*            being executed during fact assertions via      */
/*            Increment/DecrementClearReadyLocks API.        */
/*                                                           */
/*************************************************************/

#ifndef _H_factcom

#pragma once

#define _H_factcom

#include "evaluatn.h"

   void                           FactCommandDefinitions(void *);
   void                           AssertCommand(UDFContext *,CLIPSValue *);
   void                           RetractCommand(UDFContext *,CLIPSValue *);
   void                           AssertStringFunction(UDFContext *,CLIPSValue *);
   void                           FactsCommand(UDFContext *,CLIPSValue *);
   void                           EnvFacts(void *,const char *,void *,long long,long long,long long);
   void                           SetFactDuplicationCommand(UDFContext *,CLIPSValue *);
   void                           GetFactDuplicationCommand(UDFContext *,CLIPSValue *);
   void                           SaveFactsCommand(UDFContext *,CLIPSValue *);
   void                           LoadFactsCommand(UDFContext *,CLIPSValue *);
   bool                           EnvSaveFacts(void *,const char *,int);
   bool                           EnvSaveFactsDriver(void *,const char *,int,struct expr *);
   bool                           EnvLoadFacts(void *,const char *);
   bool                           EnvLoadFactsFromString(void *,const char *,long);
   void                           FactIndexFunction(UDFContext *,CLIPSValue *);

#endif /* _H_factcom */


