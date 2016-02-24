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
/*                                                            */
/*      6.24: Added allowed-classes slot facet.              */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Borland C (IBM_TBC) and Metrowerks CodeWarrior */
/*            (MAC_MCW, IBM_MCW) are no longer supported.    */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_classinf

#pragma once

#define _H_classinf

#include "evaluatn.h"

   void                           ClassAbstractPCommand(UDFContext *,CLIPSValue *);
#if DEFRULE_CONSTRUCT
   void                           ClassReactivePCommand(UDFContext *,CLIPSValue *);
#endif
   void                          *ClassInfoFnxArgs(UDFContext *,const char *,bool *);
   void                           ClassSlotsCommand(UDFContext *,CLIPSValue *);
   void                           ClassSuperclassesCommand(UDFContext *,CLIPSValue *);
   void                           ClassSubclassesCommand(UDFContext *,CLIPSValue *);
   void                           GetDefmessageHandlersListCmd(UDFContext *,CLIPSValue *);
   void                           SlotFacetsCommand(UDFContext *,CLIPSValue *);
   void                           SlotSourcesCommand(UDFContext *,CLIPSValue *);
   void                           SlotTypesCommand(UDFContext *,CLIPSValue *);
   void                           SlotAllowedValuesCommand(UDFContext *,CLIPSValue *);
   void                           SlotAllowedClassesCommand(UDFContext *,CLIPSValue *);
   void                           SlotRangeCommand(UDFContext *,CLIPSValue *);
   void                           SlotCardinalityCommand(UDFContext *,CLIPSValue *);
   bool                           EnvClassAbstractP(void *,void *);
#if DEFRULE_CONSTRUCT
   bool                           EnvClassReactiveP(void *,void *);
#endif
   void                           EnvClassSlots(void *,void *,DATA_OBJECT *,bool);
   void                           EnvGetDefmessageHandlerList(void *,void *,DATA_OBJECT *,bool);
   void                           EnvClassSuperclasses(void *,void *,DATA_OBJECT *,bool);
   void                           EnvClassSubclasses(void *,void *,DATA_OBJECT *,bool);
   void                           ClassSubclassAddresses(void *,void *,DATA_OBJECT *,bool);
   void                           EnvSlotFacets(void *,void *,const char *,DATA_OBJECT *);
   void                           EnvSlotSources(void *,void *,const char *,DATA_OBJECT *);
   void                           EnvSlotTypes(void *,void *,const char *,DATA_OBJECT *);
   void                           EnvSlotAllowedValues(void *,void *,const char *,DATA_OBJECT *);
   void                           EnvSlotAllowedClasses(void *,void *,const char *,DATA_OBJECT *);
   void                           EnvSlotRange(void *,void *,const char *,DATA_OBJECT *);
   void                           EnvSlotCardinality(void *,void *,const char *,DATA_OBJECT *);

#endif /* _H_classinf */





