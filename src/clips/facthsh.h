   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                 FACT HASHING MODULE                 */
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
/*      6.24: Removed LOGICAL_DEPENDENCIES compilation flag. */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Fact hash table is resizable.                  */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Added FactWillBeAsserted.                      */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*      6.40: Modify command preserves fact id and address.  */
/*                                                           */
/*************************************************************/

#ifndef _H_facthsh

#pragma once

#define _H_facthsh

struct factHashEntry;

#include "factmngr.h"

struct factHashEntry
  {
   struct fact *theFact;
   struct factHashEntry *next;
  };

#define SIZE_FACT_HASH 16231

   void                           AddHashedFact(void *,struct fact *,unsigned long);
   bool                           RemoveHashedFact(void *,struct fact *);
   unsigned long                  HandleFactDuplication(void *,void *,bool *,long long);
   bool                           EnvGetFactDuplication(void *);
   bool                           EnvSetFactDuplication(void *,bool);
   void                           InitializeFactHashTable(void *);
   void                           ShowFactHashTable(UDFContext *,CLIPSValue *);
   unsigned long                  HashFact(struct fact *);
   bool                           FactWillBeAsserted(void *,void *);

#endif /* _H_facthsh */


