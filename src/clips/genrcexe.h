   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                                                     */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Removed IMPERATIVE_METHODS compilation flag.   */
/*                                                           */
/*      6.30: Changed garbage collection algorithm.          */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_genrcexe

#pragma once

#define _H_genrcexe

#if DEFGENERIC_CONSTRUCT

#include "evaluatn.h"
#include "expressn.h"
#include "genrcfun.h"

   void                           GenericDispatch(void *,DEFGENERIC *,DEFMETHOD *,DEFMETHOD *,EXPRESSION *,DATA_OBJECT *);
   void                           UnboundMethodErr(void *);
   bool                           IsMethodApplicable(void *,DEFMETHOD *);

   bool                           NextMethodP(void *);
   void                           NextMethodPCommand(UDFContext *,CLIPSValue *);

   void                           CallNextMethod(UDFContext *,CLIPSValue *);
   void                           CallSpecificMethod(UDFContext *,CLIPSValue *);
   void                           OverrideNextMethod(UDFContext *,CLIPSValue *);

   void                           GetGenericCurrentArgument(UDFContext *,CLIPSValue *);

#endif /* DEFGENERIC_CONSTRUCT */

#endif /* _H_genrcexe */




