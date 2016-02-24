   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*          DEFTEMPLATE FUNCTION HEADER FILE           */
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
/*      6.24: Added deftemplate-slot-names,                  */
/*            deftemplate-slot-default-value,                */
/*            deftemplate-slot-cardinality,                  */
/*            deftemplate-slot-allowed-values,               */
/*            deftemplate-slot-range,                        */
/*            deftemplate-slot-types,                        */
/*            deftemplate-slot-multip,                       */
/*            deftemplate-slot-singlep,                      */
/*            deftemplate-slot-existp, and                   */
/*            deftemplate-slot-defaultp functions.           */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Support for deftemplate slot facets.           */
/*                                                           */
/*            Added deftemplate-slot-facet-existp and        */
/*            deftemplate-slot-facet-value functions.        */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Used gensprintf instead of sprintf.            */
/*                                                           */
/*            Support for modify callback function.          */
/*                                                           */
/*            Added additional argument to function          */
/*            CheckDeftemplateAndSlotArguments to specify    */
/*            the expected number of arguments.              */
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
/*      6.40: Fact ?var:slot references in defrule actions.  */
/*                                                           */
/*************************************************************/

#ifndef _H_tmpltfun

#pragma once

#define _H_tmpltfun

#include "expressn.h"
#include "factmngr.h"
#include "scanner.h"
#include "symbol.h"
#include "tmpltdef.h"

   bool                           UpdateModifyDuplicate(void *,struct expr *,const char *,void *);
   struct expr                   *ModifyParse(void *,struct expr *,const char *);
   struct expr                   *DuplicateParse(void *,struct expr *,const char *);
   void                           DeftemplateFunctions( void *);
   void                           ModifyCommand(UDFContext *,CLIPSValue *);
   void                           DuplicateCommand(UDFContext *,CLIPSValue *);
   void                           DeftemplateSlotNamesFunction(UDFContext *,CLIPSValue *);
   void                           EnvDeftemplateSlotNames(void *,void *,DATA_OBJECT *);
   void                           DeftemplateSlotDefaultValueFunction(UDFContext *,CLIPSValue *);
   bool                           EnvDeftemplateSlotDefaultValue(void *,void *,const char *,DATA_OBJECT *);
   void                           DeftemplateSlotCardinalityFunction(UDFContext *,CLIPSValue *);
   void                           EnvDeftemplateSlotCardinality(void *,void *,const char *,DATA_OBJECT *);
   void                           DeftemplateSlotAllowedValuesFunction(UDFContext *,CLIPSValue *);
   void                           EnvDeftemplateSlotAllowedValues(void *,void *,const char *,DATA_OBJECT *);
   void                           DeftemplateSlotRangeFunction(UDFContext *,CLIPSValue *);
   void                           EnvDeftemplateSlotRange(void *,void *,const char *,DATA_OBJECT *);
   void                           DeftemplateSlotTypesFunction(UDFContext *,CLIPSValue *);
   void                           EnvDeftemplateSlotTypes(void *,void *,const char *,DATA_OBJECT *);
   void                           DeftemplateSlotMultiPFunction(UDFContext *,CLIPSValue *);
   bool                           EnvDeftemplateSlotMultiP(void *,void *,const char *);
   void                           DeftemplateSlotSinglePFunction(UDFContext *,CLIPSValue *);
   bool                           EnvDeftemplateSlotSingleP(void *,void *,const char *);
   void                           DeftemplateSlotExistPFunction(UDFContext *,CLIPSValue *);
   bool                           EnvDeftemplateSlotExistP(void *,void *,const char *);
   void                           DeftemplateSlotDefaultPFunction(UDFContext *,CLIPSValue *);
   int                            EnvDeftemplateSlotDefaultP(void *,void *,const char *);
   void                           DeftemplateSlotFacetExistPFunction(UDFContext *,CLIPSValue *);
   bool                           EnvDeftemplateSlotFacetExistP(void *,void *,const char *,const char *);
   void                           DeftemplateSlotFacetValueFunction(UDFContext *,CLIPSValue *);
   bool                           EnvDeftemplateSlotFacetValue(void *,void *,const char *,const char *,DATA_OBJECT *);
   SYMBOL_HN                     *FindTemplateForFactAddress(SYMBOL_HN *,struct lhsParseNode *);

#endif /* _H_tmpltfun */




