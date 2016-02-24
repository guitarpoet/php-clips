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
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*            Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Loading a binary instance file from a run-time */
/*            program caused a bus error. DR0866             */
/*                                                           */
/*            Removed LOGICAL_DEPENDENCIES compilation flag. */
/*                                                           */
/*            Converted INSTANCE_PATTERN_MATCHING to         */
/*            DEFRULE_CONSTRUCT.                             */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_inscom

#pragma once

#define _H_inscom

#include "insfun.h"
#include "object.h"

#define INSTANCE_DATA 29

struct instanceData
  { 
   INSTANCE_TYPE DummyInstance;
   INSTANCE_TYPE **InstanceTable;
   bool MaintainGarbageInstances;
   bool MkInsMsgPass;
   bool ChangesToInstances;
   IGARBAGE *InstanceGarbageList;
   struct patternEntityRecord InstanceInfo;
   INSTANCE_TYPE *InstanceList;  
   unsigned long GlobalNumberOfInstances;
   INSTANCE_TYPE *CurrentInstance;
   INSTANCE_TYPE *InstanceListBottom;
   bool ObjectModDupMsgValid;
  };

#define InstanceData(theEnv) ((struct instanceData *) GetEnvironmentData(theEnv,INSTANCE_DATA))

   void                           SetupInstances(void *);
   bool                           EnvDeleteInstance(void *,void *);
   bool                           EnvUnmakeInstance(void *,void *);
#if DEBUGGING_FUNCTIONS
   void                           InstancesCommand(UDFContext *,CLIPSValue *);
   void                           PPInstanceCommand(UDFContext *,CLIPSValue *);
   void                           EnvInstances(void *,const char *,void *,const char *,bool);
#endif
   void                          *EnvMakeInstance(void *,const char *);
   void                          *EnvCreateRawInstance(void *,void *,const char *);
   void                          *EnvFindInstance(void *,void *,const char *,bool);
   bool                           EnvValidInstanceAddress(void *,void *);
   void                           EnvDirectGetSlot(void *,void *,const char *,DATA_OBJECT *);
   bool                           EnvDirectPutSlot(void *,void *,const char *,DATA_OBJECT *);
   const char                    *EnvGetInstanceName(void *,void *);
   void                          *EnvGetInstanceClass(void *,void *);
   unsigned long GetGlobalNumberOfInstances(void *);
   void                          *EnvGetNextInstance(void *,void *);
   void                          *GetNextInstanceInScope(void *,void *);
   void                          *EnvGetNextInstanceInClass(void *,void *,void *);
   void                          *EnvGetNextInstanceInClassAndSubclasses(void *,void **,void *,DATA_OBJECT *);
   void                           EnvGetInstancePPForm(void *,char *,size_t,void *);
   void                           ClassCommand(UDFContext *,CLIPSValue *);
   void                           DeleteInstanceCommand(UDFContext *,CLIPSValue *);
   void                           UnmakeInstanceCommand(UDFContext *,CLIPSValue *);
   void                           SymbolToInstanceName(UDFContext *,CLIPSValue *);
   void                           InstanceNameToSymbol(UDFContext *,CLIPSValue *);
   void                           InstanceAddressCommand(UDFContext *,CLIPSValue *);
   void                           InstanceNameCommand(UDFContext *,CLIPSValue *);
   void                           InstanceAddressPCommand(UDFContext *,CLIPSValue *);
   void                           InstanceNamePCommand(UDFContext *,CLIPSValue *);
   void                           InstancePCommand(UDFContext *,CLIPSValue *);
   void                           InstanceExistPCommand(UDFContext *,CLIPSValue *);
   void                           CreateInstanceHandler(UDFContext *,CLIPSValue *);

#endif /* _H_inscom */





