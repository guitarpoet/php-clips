   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*           MULTIFIELD FUNCTIONS HEADER FILE          */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary Riley and Brian Dantes                          */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*            Changed name of variable exp to theExp         */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Moved ImplodeMultifield to multifld.c.         */
/*                                                           */
/*      6.30: Changed integer type/precision.                */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Fixed memory leaks when error occurred.        */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Fixed linkage issue when DEFMODULE_CONSTRUCT   */
/*            compiler flag is set to 0.                     */
/*                                                           */
/*************************************************************/

#ifndef _H_multifun

#pragma once

#define _H_multifun

#include "evaluatn.h"

   void                    MultifieldFunctionDefinitions(void *);
#if MULTIFIELD_FUNCTIONS
   void                    DeleteFunction(UDFContext *,CLIPSValue *);
   void                    MVDeleteFunction(void *,DATA_OBJECT_PTR);
   void                    ReplaceFunction(UDFContext *,CLIPSValue *);
   void                    MVReplaceFunction(void *,DATA_OBJECT_PTR);
   void                    DeleteMemberFunction(UDFContext *,CLIPSValue *);
   void                    ReplaceMemberFunction(UDFContext *,CLIPSValue *);
   void                    InsertFunction(UDFContext *,CLIPSValue *);
   void                    ExplodeFunction(UDFContext *,CLIPSValue *);
   void                    ImplodeFunction(UDFContext *,CLIPSValue *);
   void                    SubseqFunction(UDFContext *,CLIPSValue *);
   void                    MVSubseqFunction(void *,DATA_OBJECT_PTR);
   void                    FirstFunction(UDFContext *,CLIPSValue *);
   void                    RestFunction(UDFContext *,CLIPSValue *);
   void                    NthFunction(UDFContext *,CLIPSValue *);
   void                    SubsetpFunction(UDFContext *,CLIPSValue *);
   void                    MemberFunction(UDFContext *,CLIPSValue *);
   void                    MultifieldPrognFunction(UDFContext *,CLIPSValue *);
   void                    ForeachFunction(UDFContext *,CLIPSValue *);
   void                    GetMvPrognField(UDFContext *,CLIPSValue *);
   void                    GetMvPrognIndex(UDFContext *,CLIPSValue *);
   bool                    FindDOsInSegment(DATA_OBJECT_PTR,int,DATA_OBJECT_PTR,
                                                   long *,long *,long *,int);
#endif
   bool                    ReplaceMultiValueField(void *,struct dataObject *,
                                                         struct dataObject *,
                                                         long,long,
                                                         struct dataObject *,const char *);
   bool                    InsertMultiValueField(void *,struct dataObject *,
                                                        struct dataObject *,
                                                        long,struct dataObject *,const char *);
   bool                    DeleteMultiValueField(void *,struct dataObject *,struct dataObject *,
                                                        long,long,const char *);

#endif /* _H_multifun */

