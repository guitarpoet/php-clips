   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*             ARGUMENT ACCESS HEADER FILE             */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides access routines for accessing arguments */
/*   passed to user or system functions defined using the    */
/*   DefineFunction protocol.                                */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added IllegalLogicalNameMessage function.      */
/*                                                           */
/*      6.30: Support for long long integers.                */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_argacces

#pragma once

#define _H_argacces

#include "expressn.h"
#include "evaluatn.h"
#include "moduldef.h"

   int                            EnvRtnArgCount(void *);
   int                            EnvArgCountCheck(void *,const char *,int,int);
   int                            EnvArgRangeCheck(void *,const char *,int,int);
   const char                    *EnvRtnLexeme(void *,int);
   double                         EnvRtnDouble(void *,int);
   long long                      EnvRtnLong(void *,int);
   struct dataObject             *EnvRtnUnknown(void *,int,struct dataObject *);
   bool                           EnvArgTypeCheck(void *,const char *,int,int,struct dataObject *);
   bool                           GetNumericArgument(void *,struct expr *,const char *,struct dataObject *,bool,int);
   const char                    *GetLogicalName(UDFContext *,const char *);
   const char                    *GetFileName(UDFContext *);
   const char                    *GetConstructName(UDFContext *,const char *,const char *);
   void                           ExpectedCountError(void *,const char *,int,int);
   void                           OpenErrorMessage(void *,const char *,const char *);
   bool                           CheckFunctionArgCount(void *,struct FunctionDefinition *,int);
   void                           ExpectedTypeError0(void *,const char *,int);
   void                           ExpectedTypeError1(void *,const char *,int,const char *);
   void                           ExpectedTypeError2(void *,const char *,int);
   struct defmodule              *GetModuleName(void *,const char *,int,bool *);
   void                          *GetFactOrInstanceArgument(void *,int,DATA_OBJECT *,const char *);
   void                           IllegalLogicalNameMessage(void *,const char *);

#endif






