   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*         DEFMODULE BASIC COMMANDS HEADER FILE        */
   /*******************************************************/

/*************************************************************/
/* Purpose: Implements core commands for the deffacts        */
/*   construct such as clear, reset, save, undeffacts,       */
/*   ppdeffacts, list-deffacts, and get-deffacts-list.       */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
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

#ifndef _H_modulbsc

#pragma once

#define _H_modulbsc

#include "evaluatn.h"

   void                           DefmoduleBasicCommands(void *);
   void                           EnvGetDefmoduleListFunction(UDFContext *,CLIPSValue *);
   void                           EnvGetDefmoduleList(void *,DATA_OBJECT_PTR);
   void                           PPDefmoduleCommand(UDFContext *,CLIPSValue *);
   bool                           PPDefmodule(void *,const char *,const char *);
   void                           ListDefmodulesCommand(UDFContext *,CLIPSValue *);
   void                           EnvListDefmodules(void *,const char *);

#endif /* _H_modulbsc */

