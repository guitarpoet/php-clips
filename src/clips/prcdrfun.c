   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/13/16             */
   /*                                                     */
   /*             PROCEDURAL FUNCTIONS MODULE             */
   /*******************************************************/

/*************************************************************/
/* Purpose: Contains the code for several procedural         */
/*   functions including if, while, loop-for-count, bind,    */
/*   progn, return, break, and switch                        */
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
/*      6.40: Added Env prefix to GetHaltExecution and       */
/*            SetHaltExecution functions.                    */
/*                                                           */
/*************************************************************/

#include <stdio.h>

#include "setup.h"

#include "argacces.h"
#include "constrnt.h"
#include "cstrnchk.h"
#include "cstrnops.h"
#include "envrnmnt.h"
#include "exprnpsr.h"
#include "memalloc.h"
#include "multifld.h"
#include "prcdrpsr.h"
#include "router.h"
#include "scanner.h"
#include "utility.h"

#include "prcdrfun.h"

#if DEFGLOBAL_CONSTRUCT
#include "globldef.h"
#endif

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static void                    DeallocateProceduralFunctionData(void *);

/**********************************************/
/* ProceduralFunctionDefinitions: Initializes */
/*   the procedural functions.                */
/**********************************************/
void ProceduralFunctionDefinitions(
  void *theEnv)
  {
   AllocateEnvironmentData(theEnv,PRCDRFUN_DATA,sizeof(struct procedureFunctionData),DeallocateProceduralFunctionData);

#if ! RUN_TIME
   EnvAddUDF(theEnv,"if",    "*",  IfFunction, "IfFunction", 0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"while", "*",  WhileFunction, "WhileFunction", 0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"loop-for-count",   "*",  LoopForCountFunction, "LoopForCountFunction", 0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"(get-loop-count)", "l",  GetLoopCount, "GetLoopCount", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"bind",   "*",  BindFunction, "BindFunction", 0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"progn",  "*",  PrognFunction, "PrognFunction", 0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"return", "*",  ReturnFunction, "ReturnFunction",0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"break",  "v",BreakFunction, "BreakFunction",0,0,NULL,NULL);
   EnvAddUDF(theEnv,"switch", "*",  SwitchFunction, "SwitchFunction",0,UNBOUNDED,NULL,NULL);

   ProceduralFunctionParsers(theEnv);

   FuncSeqOvlFlags(theEnv,"progn",false,false);
   FuncSeqOvlFlags(theEnv,"if",false,false);
   FuncSeqOvlFlags(theEnv,"while",false,false);
   FuncSeqOvlFlags(theEnv,"loop-for-count",false,false);
   FuncSeqOvlFlags(theEnv,"return",false,false);
   FuncSeqOvlFlags(theEnv,"switch",false,false);
#endif

   EnvAddResetFunction(theEnv,"bind",FlushBindList,0);
   EnvAddClearFunction(theEnv,"bind",FlushBindList,0);
  }

/*************************************************************/
/* DeallocateProceduralFunctionData: Deallocates environment */
/*    data for procedural functions.                         */
/*************************************************************/
static void DeallocateProceduralFunctionData(
  void *theEnv)
  {
   DATA_OBJECT_PTR nextPtr, garbagePtr;

   garbagePtr = ProcedureFunctionData(theEnv)->BindList;

   while (garbagePtr != NULL)
     {
      nextPtr = garbagePtr->next;
      rtn_struct(theEnv,dataObject,garbagePtr);
      garbagePtr = nextPtr;
     }
  }

/***************************************/
/* WhileFunction: H/L access routine   */
/*   for the while function.           */
/***************************************/
void WhileFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT theResult;
   struct CLIPSBlock gcBlock;
   Environment *theEnv = UDFContextEnvironment(context);
   
   /*====================================================*/
   /* Evaluate the body of the while loop as long as the */
   /* while condition evaluates to a non-false value.    */
   /*====================================================*/
   
   CLIPSBlockStart(theEnv,&gcBlock);

   EnvRtnUnknown(theEnv,1,&theResult);
   while (((theResult.value != EnvFalseSymbol(theEnv)) ||
           (theResult.type != SYMBOL)) &&
           (EvaluationData(theEnv)->HaltExecution != true))
     {
      if ((ProcedureFunctionData(theEnv)->BreakFlag == true) || (ProcedureFunctionData(theEnv)->ReturnFlag == true))
        break;
        
      EnvRtnUnknown(theEnv,2,&theResult);

      if ((ProcedureFunctionData(theEnv)->BreakFlag == true) || (ProcedureFunctionData(theEnv)->ReturnFlag == true))
        break;

      CleanCurrentGarbageFrame(theEnv,NULL);
      CallPeriodicTasks(theEnv);

      EnvRtnUnknown(theEnv,1,&theResult);
     }

   /*=====================================================*/
   /* Reset the break flag. The return flag is not reset  */
   /* because the while loop is probably contained within */
   /* a deffunction or RHS of a rule which needs to be    */
   /* returned from as well.                              */
   /*=====================================================*/

   ProcedureFunctionData(theEnv)->BreakFlag = false;

   /*====================================================*/
   /* If the return command was issued, then return that */
   /* value, otherwise return the symbol FALSE.          */
   /*====================================================*/

   if (ProcedureFunctionData(theEnv)->ReturnFlag == true)
     {
      returnValue->type = theResult.type;
      returnValue->value = theResult.value;
      returnValue->begin = theResult.begin;
      returnValue->end = theResult.end;
     }
   else
     {
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
     }
   
   CLIPSBlockEnd(theEnv,&gcBlock,returnValue);
   CallPeriodicTasks(theEnv);
  }

/********************************************/
/* LoopForCountFunction: H/L access routine */
/*   for the loop-for-count function.       */
/********************************************/
void LoopForCountFunction(
  UDFContext *context,
  CLIPSValue *loopResult)
  {
   DATA_OBJECT arg_ptr;
   long long iterationEnd;
   LOOP_COUNTER_STACK *tmpCounter;
   struct CLIPSBlock gcBlock;
   Environment *theEnv = UDFContextEnvironment(context);

   tmpCounter = get_struct(theEnv,loopCounterStack);
   tmpCounter->loopCounter = 0L;
   tmpCounter->nxt = ProcedureFunctionData(theEnv)->LoopCounterStack;
   ProcedureFunctionData(theEnv)->LoopCounterStack = tmpCounter;
   if (EnvArgTypeCheck(theEnv,"loop-for-count",1,INTEGER,&arg_ptr) == false)
     {
      loopResult->type = SYMBOL;
      loopResult->value = EnvFalseSymbol(theEnv);
      ProcedureFunctionData(theEnv)->LoopCounterStack = tmpCounter->nxt;
      rtn_struct(theEnv,loopCounterStack,tmpCounter);
      return;
     }
   tmpCounter->loopCounter = DOToLong(arg_ptr);
   if (EnvArgTypeCheck(theEnv,"loop-for-count",2,INTEGER,&arg_ptr) == false)
     {
      loopResult->type = SYMBOL;
      loopResult->value = EnvFalseSymbol(theEnv);
      ProcedureFunctionData(theEnv)->LoopCounterStack = tmpCounter->nxt;
      rtn_struct(theEnv,loopCounterStack,tmpCounter);
      return;
     }
     
   CLIPSBlockStart(theEnv,&gcBlock);
   
   iterationEnd = DOToLong(arg_ptr);
   while ((tmpCounter->loopCounter <= iterationEnd) &&
          (EvaluationData(theEnv)->HaltExecution != true))
     {
      if ((ProcedureFunctionData(theEnv)->BreakFlag == true) || (ProcedureFunctionData(theEnv)->ReturnFlag == true))
        break;

      EnvRtnUnknown(theEnv,3,&arg_ptr);

      if ((ProcedureFunctionData(theEnv)->BreakFlag == true) || (ProcedureFunctionData(theEnv)->ReturnFlag == true))
        break;
        
      CleanCurrentGarbageFrame(theEnv,NULL);
      CallPeriodicTasks(theEnv);
        
      tmpCounter->loopCounter++;
     }
     
   ProcedureFunctionData(theEnv)->BreakFlag = false;
   if (ProcedureFunctionData(theEnv)->ReturnFlag == true)
     {
      loopResult->type = arg_ptr.type;
      loopResult->value = arg_ptr.value;
      loopResult->begin = arg_ptr.begin;
      loopResult->end = arg_ptr.end;
     }
   else
     {
      loopResult->type = SYMBOL;
      loopResult->value = EnvFalseSymbol(theEnv);
     }
   ProcedureFunctionData(theEnv)->LoopCounterStack = tmpCounter->nxt;
   rtn_struct(theEnv,loopCounterStack,tmpCounter);
    
   CLIPSBlockEnd(theEnv,&gcBlock,loopResult);
   CallPeriodicTasks(theEnv);
  }

/*****************/
/* GetLoopCount: */
/*****************/
void GetLoopCount(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   int depth;
   CLIPSValue theArg;
   LOOP_COUNTER_STACK *tmpCounter;
   Environment *theEnv = UDFContextEnvironment(context);
   
   if (! UDFFirstArgument(context,INTEGER_TYPE,&theArg))
     { return; }
   depth = mCVToInteger(&theArg);
   tmpCounter = ProcedureFunctionData(theEnv)->LoopCounterStack;
   while (depth > 0)
     {
      tmpCounter = tmpCounter->nxt;
      depth--;
     }
   
   mCVSetInteger(returnValue,tmpCounter->loopCounter);
  }

/************************************/
/* IfFunction: H/L access routine   */
/*   for the if function.           */
/************************************/
void IfFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   int numArgs;
   struct expr *theExpr;
   Environment *theEnv = UDFContextEnvironment(context);

   /*============================================*/
   /* Check for the correct number of arguments. */
   /*============================================*/

   if ((EvaluationData(theEnv)->CurrentExpression->argList == NULL) ||
       (EvaluationData(theEnv)->CurrentExpression->argList->nextArg == NULL))
     {
      EnvArgRangeCheck(theEnv,"if",2,3);
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
      return;
     }

   if (EvaluationData(theEnv)->CurrentExpression->argList->nextArg->nextArg == NULL)
     { numArgs = 2; }
   else if (EvaluationData(theEnv)->CurrentExpression->argList->nextArg->nextArg->nextArg == NULL)
     { numArgs = 3; }
   else
     {
      EnvArgRangeCheck(theEnv,"if",2,3);
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
      return;
     }

   /*=========================*/
   /* Evaluate the condition. */
   /*=========================*/

   EvaluateExpression(theEnv,EvaluationData(theEnv)->CurrentExpression->argList,returnValue);

   if ((ProcedureFunctionData(theEnv)->BreakFlag == true) || (ProcedureFunctionData(theEnv)->ReturnFlag == true))
     {
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
      return;
     }

   /*=========================================*/
   /* If the condition evaluated to false and */
   /* an "else" portion exists, evaluate it   */
   /* and return the value.                   */
   /*=========================================*/

   if ((returnValue->value == EnvFalseSymbol(theEnv)) &&
       (returnValue->type == SYMBOL) &&
       (numArgs == 3))
     {
      theExpr = EvaluationData(theEnv)->CurrentExpression->argList->nextArg->nextArg;
      switch (theExpr->type)
        {
         case INTEGER:
         case FLOAT:
         case SYMBOL:
         case STRING:
#if OBJECT_SYSTEM
         case INSTANCE_NAME:
         case INSTANCE_ADDRESS:
#endif
         case EXTERNAL_ADDRESS:
           returnValue->type = theExpr->type;
           returnValue->value = theExpr->value;
           break;

         default:
           EvaluateExpression(theEnv,theExpr,returnValue);
           break;
        }
      return;
     }

   /*===================================================*/
   /* Otherwise if the symbol evaluated to a non-false  */
   /* value, evaluate the "then" portion and return it. */
   /*===================================================*/

   else if ((returnValue->value != EnvFalseSymbol(theEnv)) ||
            (returnValue->type != SYMBOL))
     {
      theExpr = EvaluationData(theEnv)->CurrentExpression->argList->nextArg;
      switch (theExpr->type)
        {
         case INTEGER:
         case FLOAT:
         case SYMBOL:
         case STRING:
#if OBJECT_SYSTEM
         case INSTANCE_NAME:
         case INSTANCE_ADDRESS:
#endif
         case EXTERNAL_ADDRESS:
           returnValue->type = theExpr->type;
           returnValue->value = theExpr->value;
           break;
           
         default:
           EvaluateExpression(theEnv,theExpr,returnValue);
           break;
        }
      return;
     }

   /*=========================================*/
   /* Return false if the condition evaluated */
   /* to false and there is no "else" portion */
   /* of the if statement.                    */
   /*=========================================*/

   returnValue->type = SYMBOL;
   returnValue->value = EnvFalseSymbol(theEnv);
   return;
  }

/**************************************/
/* BindFunction: H/L access routine   */
/*   for the bind function.           */
/**************************************/
void BindFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT *theBind, *lastBind;
   bool found = false,
       unbindVar = false;
   SYMBOL_HN *variableName = NULL;
#if DEFGLOBAL_CONSTRUCT
   struct defglobal *theGlobal = NULL;
#endif
   Environment *theEnv = UDFContextEnvironment(context);

   /*===============================================*/
   /* Determine the name of the variable to be set. */
   /*===============================================*/

#if DEFGLOBAL_CONSTRUCT
   if (GetFirstArgument()->type == DEFGLOBAL_PTR)
     { theGlobal = (struct defglobal *) GetFirstArgument()->value; }
   else
#endif
     {
      EvaluateExpression(theEnv,GetFirstArgument(),returnValue);
      variableName = (SYMBOL_HN *) DOPToPointer(returnValue);
     }

   /*===========================================*/
   /* Determine the new value for the variable. */
   /*===========================================*/

   if (GetFirstArgument()->nextArg == NULL)
     { unbindVar = true; }
   else if (GetFirstArgument()->nextArg->nextArg == NULL)
     { EvaluateExpression(theEnv,GetFirstArgument()->nextArg,returnValue); }
   else
     { StoreInMultifield(theEnv,returnValue,GetFirstArgument()->nextArg,true); }

   /*==================================*/
   /* Bind a defglobal if appropriate. */
   /*==================================*/

#if DEFGLOBAL_CONSTRUCT
   if (theGlobal != NULL)
     {
      QSetDefglobalValue(theEnv,theGlobal,returnValue,unbindVar);
      return;
     }
#endif

   /*===============================================*/
   /* Search for the variable in the list of binds. */
   /*===============================================*/

   theBind = ProcedureFunctionData(theEnv)->BindList;
   lastBind = NULL;

   while ((theBind != NULL) && (found == false))
     {
      if (theBind->supplementalInfo == (void *) variableName)
        { found = true; }
      else
        {
         lastBind = theBind;
         theBind = theBind->next;
        }
     }

   /*========================================================*/
   /* If variable was not in the list of binds, then add it. */
   /* Make sure that this operation preserves the bind list  */
   /* as a stack.                                            */
   /*========================================================*/

   if (found == false)
     {
      if (unbindVar == false)
        {
         theBind = get_struct(theEnv,dataObject);
         theBind->supplementalInfo = (void *) variableName;
         IncrementSymbolCount(variableName);
         theBind->next = NULL;
         if (lastBind == NULL)
           { ProcedureFunctionData(theEnv)->BindList = theBind; }
         else
           { lastBind->next = theBind; }
        }
      else
        {
         returnValue->type = SYMBOL;
         returnValue->value = EnvFalseSymbol(theEnv);
         return;
        }
     }
   else
     { ValueDeinstall(theEnv,theBind); }

   /*================================*/
   /* Set the value of the variable. */
   /*================================*/

   if (unbindVar == false)
     {
      theBind->type = returnValue->type;
      theBind->value = returnValue->value;
      theBind->begin = returnValue->begin;
      theBind->end = returnValue->end;
      ValueInstall(theEnv,returnValue);
     }
   else
     {
      if (lastBind == NULL) ProcedureFunctionData(theEnv)->BindList = theBind->next;
      else lastBind->next = theBind->next;
      DecrementSymbolCount(theEnv,(struct symbolHashNode *) theBind->supplementalInfo);
      rtn_struct(theEnv,dataObject,theBind);
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
     }
  }

/*******************************************/
/* GetBoundVariable: Searches the BindList */
/*   for a specified variable.             */
/*******************************************/
bool GetBoundVariable(
  void *theEnv,
  DATA_OBJECT_PTR vPtr,
  SYMBOL_HN *varName)
  {
   DATA_OBJECT_PTR bindPtr;
   
   for (bindPtr = ProcedureFunctionData(theEnv)->BindList; bindPtr != NULL; bindPtr = bindPtr->next)
     {
      if (bindPtr->supplementalInfo == (void *) varName)
        {
         vPtr->type = bindPtr->type;
         vPtr->value = bindPtr->value;
         vPtr->begin = bindPtr->begin;
         vPtr->end = bindPtr->end;
         return(true);
        }
     }

   return(false);
  }

/*************************************************/
/* FlushBindList: Removes all variables from the */
/*   list of currently bound local variables.    */
/*************************************************/
void FlushBindList(
  void *theEnv)
  {
   ReturnValues(theEnv,ProcedureFunctionData(theEnv)->BindList,true);
   ProcedureFunctionData(theEnv)->BindList = NULL;
  }

/***************************************/
/* PrognFunction: H/L access routine   */
/*   for the progn function.           */
/***************************************/
void PrognFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   struct expr *argPtr;
   Environment *theEnv = UDFContextEnvironment(context);

   argPtr = EvaluationData(theEnv)->CurrentExpression->argList;

   if (argPtr == NULL)
     {
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
      return;
     }

   while ((argPtr != NULL) && (EnvGetHaltExecution(theEnv) != true))
     {
      EvaluateExpression(theEnv,argPtr,returnValue);

      if ((ProcedureFunctionData(theEnv)->BreakFlag == true) || (ProcedureFunctionData(theEnv)->ReturnFlag == true))
        break;
      argPtr = argPtr->nextArg;
     }

   if (EnvGetHaltExecution(theEnv) == true)
     {
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
      return;
     }

   return;
  }

/*****************************************************************/
/* ReturnFunction: H/L access routine for the return function.   */
/*****************************************************************/
void ReturnFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   Environment *theEnv = UDFContextEnvironment(context);
   
   if (EnvRtnArgCount(theEnv) == 0)
     { mCVSetVoid(returnValue); }
   else
     { EnvRtnUnknown(theEnv,1,returnValue); }
   ProcedureFunctionData(theEnv)->ReturnFlag = true;
  }

/***************************************************************/
/* BreakFunction: H/L access routine for the break function.   */
/***************************************************************/
void BreakFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   ProcedureFunctionData(UDFContextEnvironment(context))->BreakFlag = true;
  }

/*****************************************************************/
/* SwitchFunction: H/L access routine for the switch function.   */
/*****************************************************************/
void SwitchFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT switch_val,case_val;
   EXPRESSION *theExp;
   Environment *theEnv = UDFContextEnvironment(context);

   mCVSetBoolean(returnValue,false);

   /* ==========================
      Get the value to switch on
      ========================== */
   EvaluateExpression(theEnv,GetFirstArgument(),&switch_val);
   if (EvaluationData(theEnv)->EvaluationError)
     return;
   for (theExp = GetFirstArgument()->nextArg ; theExp != NULL ; theExp = theExp->nextArg->nextArg)
     {
      /* =================================================
         RVOID is the default case (if any) for the switch
         ================================================= */
      if (theExp->type == RVOID)
        {
         EvaluateExpression(theEnv,theExp->nextArg,returnValue);
         return;
        }

      /* ====================================================
         If the case matches, evaluate the actions and return
         ==================================================== */
      EvaluateExpression(theEnv,theExp,&case_val);
      if (EvaluationData(theEnv)->EvaluationError)
        return;
      if (switch_val.type == case_val.type)
        {
         if ((case_val.type == MULTIFIELD) ? MultifieldDOsEqual(&switch_val,&case_val) :
             (switch_val.value == case_val.value))
           {
            EvaluateExpression(theEnv,theExp->nextArg,returnValue);
            return;
           }
        }
     }
  }





