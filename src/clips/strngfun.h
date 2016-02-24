   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*             STRING FUNCTIONS HEADER FILE            */
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
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*      6.30: Support for long long integers.                */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Used gensprintf instead of sprintf.            */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Added support for UTF-8 strings to str-length, */
/*            str-index, and sub-string functions.           */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_strngfun

#pragma once

#define _H_strngfun

#include "evaluatn.h"

   bool                           EnvBuild(void *,const char *);
   bool                           EnvEval(void *,const char *,DATA_OBJECT_PTR);
   void                           StringFunctionDefinitions(void *);
   void                           StrCatFunction(UDFContext *,CLIPSValue *);
   void                           SymCatFunction(UDFContext *,CLIPSValue *);
   void                           StrLengthFunction(UDFContext *,CLIPSValue *);
   void                           UpcaseFunction(UDFContext *,CLIPSValue *);
   void                           LowcaseFunction(UDFContext *,CLIPSValue *);
   void                           StrCompareFunction(UDFContext *,CLIPSValue *);
   void                           SubStringFunction(UDFContext *,CLIPSValue *);
   void                           StrIndexFunction(UDFContext *,CLIPSValue *);
   void                           EvalFunction(UDFContext *,CLIPSValue *);
   void                           BuildFunction(UDFContext *,CLIPSValue *);
   void                           StringToFieldFunction(UDFContext *,CLIPSValue *);
   void                           StringToField(void *,const char *,DATA_OBJECT *);

#endif /* _H_strngfun */






