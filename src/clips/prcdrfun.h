   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*          PROCEDURAL FUNCTIONS HEADER FILE           */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*      Brian L. Dantes                                      */
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
/*      6.30: Local variables set with the bind function     */
/*            persist until a reset/clear command is issued. */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*************************************************************/

#ifndef _H_prcdrfun

#pragma once

#define _H_prcdrfun

#include "evaluatn.h"

typedef struct loopCounterStack
  {
   long long loopCounter;
   struct loopCounterStack *nxt;
  } LOOP_COUNTER_STACK;

#define PRCDRFUN_DATA 13

struct procedureFunctionData
  { 
   bool ReturnFlag;
   bool BreakFlag;
   LOOP_COUNTER_STACK *LoopCounterStack;
   struct dataObject *BindList;
  };

#define ProcedureFunctionData(theEnv) ((struct procedureFunctionData *) GetEnvironmentData(theEnv,PRCDRFUN_DATA))

   void                           ProceduralFunctionDefinitions(void *);
   void                           WhileFunction(UDFContext *,CLIPSValue *);
   void                           LoopForCountFunction(UDFContext *,CLIPSValue *);
   void                           GetLoopCount(UDFContext *,CLIPSValue *);
   void                           IfFunction(UDFContext *,CLIPSValue *);
   void                           BindFunction(UDFContext *,CLIPSValue *);
   void                           PrognFunction(UDFContext *,CLIPSValue *);
   void                           ReturnFunction(UDFContext *,CLIPSValue *);
   void                           BreakFunction(UDFContext *,CLIPSValue *);
   void                           SwitchFunction(UDFContext *,CLIPSValue *);
   bool                           GetBoundVariable(void *,struct dataObject *,struct symbolHashNode *);
   void                           FlushBindList(void *);

#endif /* _H_prcdrfun */






