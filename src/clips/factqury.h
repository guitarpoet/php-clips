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
/*      Gary D. Riley                                        */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Added fact-set queries.                        */
/*                                                           */
/*      6.24: Corrected errors when compiling as a C++ file. */
/*            DR0868                                         */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Changed garbage collection algorithm.          */
/*                                                           */
/*            Fixes for run-time use of query functions.     */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_factqury

#pragma once

#define _H_factqury

#if FACT_SET_QUERIES

#include "factmngr.h"

typedef struct query_template
  {
   struct deftemplate *templatePtr;
   struct query_template *chain, *nxt;
  } QUERY_TEMPLATE;

typedef struct query_soln
  {
   struct fact **soln;
   struct query_soln *nxt;
  } QUERY_SOLN;

typedef struct query_core
  {
   struct fact **solns;
   EXPRESSION *query,*action;
   QUERY_SOLN *soln_set,*soln_bottom;
   unsigned soln_size,soln_cnt;
   DATA_OBJECT *result;
  } QUERY_CORE;

typedef struct query_stack
  {
   QUERY_CORE *core;
   struct query_stack *nxt;
  } QUERY_STACK;

#define FACT_QUERY_DATA 63

struct factQueryData
  { 
   SYMBOL_HN *QUERY_DELIMETER_SYMBOL;
   QUERY_CORE *QueryCore;
   QUERY_STACK *QueryCoreStack;
   bool AbortQuery;
  };

#define FactQueryData(theEnv) ((struct factQueryData *) GetEnvironmentData(theEnv,FACT_QUERY_DATA))

#define QUERY_DELIMETER_STRING     "(QDS)"

   void                           SetupFactQuery(void *);
   void                           GetQueryFact(UDFContext *,CLIPSValue *);
   void                           GetQueryFactSlot(UDFContext *,CLIPSValue *);
   void                           AnyFacts(UDFContext *,CLIPSValue *);
   void                           QueryFindFact(UDFContext *,CLIPSValue *);
   void                           QueryFindAllFacts(UDFContext *,CLIPSValue *);
   void                           QueryDoForFact(UDFContext *,CLIPSValue *);
   void                           QueryDoForAllFacts(UDFContext *,CLIPSValue *);
   void                           DelayedQueryDoForAllFacts(UDFContext *,CLIPSValue *);

#endif /* FACT_SET_QUERIES */

#endif /* _H_factqury */
