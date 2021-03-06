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
/*      6.24: Converted INSTANCE_PATTERN_MATCHING to         */
/*            DEFRULE_CONSTRUCT.                             */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Borland C (IBM_TBC) and Metrowerks CodeWarrior */
/*            (MAC_MCW, IBM_MCW) are no longer supported.    */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Used genstrcpy and genstrcat instead of strcpy */
/*            and strcat.                                    */
/*                                                           */             
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*      6.40: Removed initial-object support.                */
/*                                                           */
/*************************************************************/

#ifndef _H_classfun

#pragma once

#define _H_classfun

#include "object.h"

#define TestTraversalID(traversalRecord,id) TestBitMap(traversalRecord,id)
#define SetTraversalID(traversalRecord,id) SetBitMap(traversalRecord,id)
#define ClearTraversalID(traversalRecord,id) ClearBitMap(traversalRecord,id)

#define CLASS_TABLE_HASH_SIZE     167
#define SLOT_NAME_TABLE_HASH_SIZE 167

#define ISA_ID  0
#define NAME_ID 1

void IncrementDefclassBusyCount(void *,void *);
void DecrementDefclassBusyCount(void *,void *);
bool InstancesPurge(void *theEnv);

#if ! RUN_TIME
void InitializeClasses(void *);
#endif
SLOT_DESC *FindClassSlot(DEFCLASS *,SYMBOL_HN *);
void ClassExistError(void *,const char *,const char *);
void DeleteClassLinks(void *,CLASS_LINK *);
void PrintClassName(void *,const char *,DEFCLASS *,bool);

#if DEBUGGING_FUNCTIONS || ((! BLOAD_ONLY) && (! RUN_TIME))
void PrintPackedClassLinks(void *,const char *,const char *,PACKED_CLASS_LINKS *);
#endif

#if ! RUN_TIME
void PutClassInTable(void *,DEFCLASS *);
void RemoveClassFromTable(void *,DEFCLASS *);
void AddClassLink(void *,PACKED_CLASS_LINKS *,DEFCLASS *,int);
void DeleteSubclassLink(void *,DEFCLASS *,DEFCLASS *);
void DeleteSuperclassLink(void *,DEFCLASS *,DEFCLASS *);
DEFCLASS *NewClass(void *,SYMBOL_HN *);
void DeletePackedClassLinks(void *,PACKED_CLASS_LINKS *,bool);
void AssignClassID(void *,DEFCLASS *);
SLOT_NAME *AddSlotName(void *,SYMBOL_HN *,int,bool);
void DeleteSlotName(void *,SLOT_NAME *);
void RemoveDefclass(void *,void *);
void InstallClass(void *,DEFCLASS *,bool);
#endif
void DestroyDefclass(void *,void *);

#if (! BLOAD_ONLY) && (! RUN_TIME)
bool IsClassBeingUsed(DEFCLASS *);
bool RemoveAllUserClasses(void *);
bool DeleteClassUAG(void *,DEFCLASS *);
void MarkBitMapSubclasses(char *,DEFCLASS *,int);
#endif

short FindSlotNameID(void *,SYMBOL_HN *);
SYMBOL_HN *FindIDSlotName(void *,int);
SLOT_NAME *FindIDSlotNameHash(void *,int);
int GetTraversalID(void *);
void ReleaseTraversalID(void *);
unsigned HashClass(SYMBOL_HN *);

#define DEFCLASS_DATA 21

#define PRIMITIVE_CLASSES 9

struct defclassData
  { 
   struct construct *DefclassConstruct;
   int DefclassModuleIndex;
   ENTITY_RECORD DefclassEntityRecord;
   DEFCLASS *PrimitiveClassMap[PRIMITIVE_CLASSES];
   DEFCLASS **ClassIDMap;
   DEFCLASS **ClassTable;
   unsigned short MaxClassID;
   unsigned short AvailClassID;
   SLOT_NAME **SlotNameTable;
   SYMBOL_HN *ISA_SYMBOL;
   SYMBOL_HN *NAME_SYMBOL;
#if DEBUGGING_FUNCTIONS
   unsigned WatchInstances;
   unsigned WatchSlots;
#endif
   unsigned short CTID;
   struct token ObjectParseToken;
   unsigned short ClassDefaultsMode;
  };

#define DefclassData(theEnv) ((struct defclassData *) GetEnvironmentData(theEnv,DEFCLASS_DATA))

#endif









