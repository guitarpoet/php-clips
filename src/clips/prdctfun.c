   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*              PREDICATE FUNCTIONS MODULE             */
   /*******************************************************/

/*************************************************************/
/* Purpose: Contains the code for several predicate          */
/*   functions including not, and, or, eq, neq, <=, >=, <,   */
/*   >, =, <>, symbolp, stringp, lexemep, numberp, integerp, */
/*   floatp, oddp, evenp, multifieldp, sequencep, and        */
/*   pointerp.                                               */
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
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Support for long long integers.                */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*************************************************************/

#include <stdio.h>

#include "setup.h"

#include "argacces.h"
#include "envrnmnt.h"
#include "exprnpsr.h"
#include "multifld.h"
#include "router.h"

#include "prdctfun.h"

/**************************************************/
/* PredicateFunctionDefinitions: Defines standard */
/*   math and predicate functions.                */
/**************************************************/
void PredicateFunctionDefinitions(
  void *theEnv)
  {
#if ! RUN_TIME
   EnvAddUDF(theEnv,"not", "b", NotFunction, "NotFunction",   1,1,          NULL,NULL);
   EnvAddUDF(theEnv,"and", "b", AndFunction, "AndFunction",   2,UNBOUNDED , NULL,NULL);
   EnvAddUDF(theEnv,"or",  "b", OrFunction,  "OrFunction",    2,UNBOUNDED , NULL,NULL);

   EnvAddUDF(theEnv,"eq",  "b", EqFunction,  "EqFunction",  2, UNBOUNDED, NULL, NULL);
   EnvAddUDF(theEnv,"neq", "b", NeqFunction, "NeqFunction", 2, UNBOUNDED, NULL, NULL);

   EnvAddUDF(theEnv,"<=", "b", LessThanOrEqualFunction,    "LessThanOrEqualFunction",    2,UNBOUNDED , "ld",NULL);
   EnvAddUDF(theEnv,">=", "b", GreaterThanOrEqualFunction, "GreaterThanOrEqualFunction", 2,UNBOUNDED , "ld",NULL);
   EnvAddUDF(theEnv,"<",  "b", LessThanFunction,           "LessThanFunction",           2,UNBOUNDED , "ld",NULL);
   EnvAddUDF(theEnv,">",  "b", GreaterThanFunction,        "GreaterThanFunction",        2,UNBOUNDED , "ld",NULL);
   EnvAddUDF(theEnv,"=",  "b", NumericEqualFunction,       "NumericEqualFunction",       2,UNBOUNDED , "ld",NULL);
   EnvAddUDF(theEnv,"<>", "b", NumericNotEqualFunction,    "NumericNotEqualFunction",    2,UNBOUNDED , "ld",NULL);
   EnvAddUDF(theEnv,"!=", "b", NumericNotEqualFunction,    "NumericNotEqualFunction",    2,UNBOUNDED , "ld",NULL);

   EnvAddUDF(theEnv,"symbolp",     "b",  SymbolpFunction,     "SymbolpFunction", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"wordp",       "b",  SymbolpFunction,     "SymbolpFunction", 1,1,NULL,NULL);  // TBD Remove?
   EnvAddUDF(theEnv,"stringp",     "b",  StringpFunction,     "StringpFunction", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"lexemep",     "b",  LexemepFunction,     "LexemepFunction", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"numberp",     "b",  NumberpFunction,     "NumberpFunction", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"integerp",    "b",  IntegerpFunction,    "IntegerpFunction", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"floatp",      "b",  FloatpFunction,      "FloatpFunction", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"oddp",        "b",  OddpFunction,        "OddpFunction", 1,1,"l", NULL);
   EnvAddUDF(theEnv,"evenp",       "b",  EvenpFunction,       "EvenpFunction",  1,1,"l", NULL);
   EnvAddUDF(theEnv,"multifieldp", "b",  MultifieldpFunction, "MultifieldpFunction", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"sequencep",   "b",  MultifieldpFunction, "MultifieldpFunction", 1,1,NULL,NULL); // TBD Remove?
   EnvAddUDF(theEnv,"pointerp",    "b",  PointerpFunction,    "PointerpFunction", 1,1,NULL,NULL);
#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif
  }

/************************************/
/* EqFunction: H/L access routine   */
/*   for the eq function.           */
/************************************/
void EqFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT item, nextItem;
   int numArgs, i;
   struct expr *theExpression;
   Environment *theEnv = UDFContextEnvironment(context);

   /*====================================*/
   /* Determine the number of arguments. */
   /*====================================*/

   numArgs = EnvRtnArgCount(theEnv);
   if (numArgs == 0)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*==============================================*/
   /* Get the value of the first argument against  */
   /* which subsequent arguments will be compared. */
   /*==============================================*/

   theExpression = GetFirstArgument();
   EvaluateExpression(theEnv,theExpression,&item);

   /*=====================================*/
   /* Compare all arguments to the first. */
   /* If any are the same, return false.  */
   /*=====================================*/

   theExpression = GetNextArgument(theExpression);
   for (i = 2 ; i <= numArgs ; i++)
     {
      EvaluateExpression(theEnv,theExpression,&nextItem);

      if (GetType(nextItem) != GetType(item))
        {
         mCVSetBoolean(returnValue,false);
         return;
        }

      if (GetType(nextItem) == MULTIFIELD)
        {
         if (MultifieldDOsEqual(&nextItem,&item) == false)
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else if (nextItem.value != item.value)
        {
         mCVSetBoolean(returnValue,false);
         return;
        }

      theExpression = GetNextArgument(theExpression);
     }

   /*=====================================*/
   /* All of the arguments were different */
   /* from the first. Return true.        */
   /*=====================================*/

   mCVSetBoolean(returnValue,true);
  }

/*************************************/
/* NeqFunction: H/L access routine   */
/*   for the neq function.           */
/*************************************/
void NeqFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT item, nextItem;
   int numArgs, i;
   struct expr *theExpression;
   Environment *theEnv = UDFContextEnvironment(context);

   /*====================================*/
   /* Determine the number of arguments. */
   /*====================================*/

   numArgs = EnvRtnArgCount(theEnv);
   if (numArgs == 0)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*==============================================*/
   /* Get the value of the first argument against  */
   /* which subsequent arguments will be compared. */
   /*==============================================*/

   theExpression = GetFirstArgument();
   EvaluateExpression(theEnv,theExpression,&item);

   /*=====================================*/
   /* Compare all arguments to the first. */
   /* If any are different, return false. */
   /*=====================================*/

   for (i = 2, theExpression = GetNextArgument(theExpression);
        i <= numArgs;
        i++, theExpression = GetNextArgument(theExpression))
     {
      EvaluateExpression(theEnv,theExpression,&nextItem);
      if (GetType(nextItem) != GetType(item))
        { continue; }
      else if (nextItem.type == MULTIFIELD)
        {
         if (MultifieldDOsEqual(&nextItem,&item) == true)
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else if (nextItem.value == item.value)
        {
         mCVSetBoolean(returnValue,false);
         return;
        }
     }

   /*=====================================*/
   /* All of the arguments were identical */
   /* to the first. Return true.          */
   /*=====================================*/

   mCVSetBoolean(returnValue,true);
  }

/*****************************************/
/* StringpFunction: H/L access routine   */
/*   for the stringp function.           */
/*****************************************/
void StringpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,STRING_TYPE))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/*****************************************/
/* SymbolpFunction: H/L access routine   */
/*   for the symbolp function.           */
/*****************************************/
void SymbolpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,SYMBOL_TYPE))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/*****************************************/
/* LexemepFunction: H/L access routine   */
/*   for the lexemep function.           */
/*****************************************/
void LexemepFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,LEXEME_TYPES))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/*****************************************/
/* NumberpFunction: H/L access routine   */
/*   for the numberp function.           */
/*****************************************/
void NumberpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,NUMBER_TYPES))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/****************************************/
/* FloatpFunction: H/L access routine   */
/*   for the floatp function.           */
/****************************************/
void FloatpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,FLOAT_TYPE))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/******************************************/
/* IntegerpFunction: H/L access routine   */
/*   for the integerp function.           */
/******************************************/
void IntegerpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,INTEGER_TYPE))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/*********************************************/
/* MultifieldpFunction: H/L access routine   */
/*   for the multifieldp function.           */
/*********************************************/
void MultifieldpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,MULTIFIELD_TYPE))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/******************************************/
/* PointerpFunction: H/L access routine   */
/*   for the pointerp function.           */
/******************************************/
void PointerpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;

   if (! UDFFirstArgument(context,ANY_TYPE,&item))
     { return; }

   if (mCVIsType(&item,EXTERNAL_ADDRESS_TYPE))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/***********************************/
/* NotFunction: H/L access routine */
/*   for the not function.         */
/***********************************/
void NotFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT result;

   if (! UDFFirstArgument(context,ANY_TYPE,&result))
     { return; }

   if (CVIsFalseSymbol(&result))
     {
      mCVSetBoolean(returnValue,true);
      return;
     }

   mCVSetBoolean(returnValue,false);
  }

/*************************************/
/* AndFunction: H/L access routine   */
/*   for the and function.           */
/*************************************/
void AndFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT result;

   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,ANY_TYPE,&result))
        { return; }
        
      if (CVIsFalseSymbol(&result))
        {
         mCVSetBoolean(returnValue,false);
         return;
        }
     }

   mCVSetBoolean(returnValue,true);
  }

/************************************/
/* OrFunction: H/L access routine   */
/*   for the or function.           */
/************************************/
void OrFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT result;

   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,ANY_TYPE,&result))
        { return; }
        
      if (! CVIsFalseSymbol(&result))
        {
         mCVSetBoolean(returnValue,true);
         return;
        }
     }

   mCVSetBoolean(returnValue,false);
  }

/*****************************************/
/* LessThanOrEqualFunction: H/L access   */
/*   routine for the <= function.        */
/*****************************************/
void LessThanOrEqualFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT rv1, rv2;

   /*=========================*/
   /* Get the first argument. */
   /*=========================*/

   if (! UDFFirstArgument(context,NUMBER_TYPES,&rv1))
     { return; }

   /*====================================================*/
   /* Compare each of the subsequent arguments to its    */
   /* predecessor. If any is greater, then return false. */
   /*====================================================*/

   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,NUMBER_TYPES,&rv2))
        { return; }

      if (mCVIsType(&rv1,INTEGER_TYPE) && mCVIsType(&rv2,INTEGER_TYPE))
        {
         if (mCVToInteger(&rv1) > mCVToInteger(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else
        {
         if (mCVToFloat(&rv1) > mCVToFloat(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }

      CVSetCLIPSValue(&rv1,&rv2);
     }

   /*======================================*/
   /* Each argument was less than or equal */
   /* to its predecessor. Return true.     */
   /*======================================*/

   mCVSetBoolean(returnValue,true);
  }

/********************************************/
/* GreaterThanOrEqualFunction: H/L access   */
/*   routine for the >= function.           */
/********************************************/
void GreaterThanOrEqualFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT rv1, rv2;

   /*=========================*/
   /* Get the first argument. */
   /*=========================*/

   if (! UDFFirstArgument(context,NUMBER_TYPES,&rv1))
     { return; }

   /*===================================================*/
   /* Compare each of the subsequent arguments to its   */
   /* predecessor. If any is lesser, then return false. */
   /*===================================================*/

   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,NUMBER_TYPES,&rv2))
        { return; }

      if (mCVIsType(&rv1,INTEGER_TYPE) && mCVIsType(&rv2,INTEGER_TYPE))
        {
         if (mCVToInteger(&rv1) < mCVToInteger(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else
        {
         if (mCVToFloat(&rv1) < mCVToFloat(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }

      CVSetCLIPSValue(&rv1,&rv2);
     }

   /*=========================================*/
   /* Each argument was greater than or equal */
   /* to its predecessor. Return true.        */
   /*=========================================*/

   mCVSetBoolean(returnValue,true);
  }

/**********************************/
/* LessThanFunction: H/L access   */
/*   routine for the < function.  */
/**********************************/
void LessThanFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue rv1, rv2;

   /*=========================*/
   /* Get the first argument. */
   /*=========================*/

   if (! UDFFirstArgument(context,NUMBER_TYPES,&rv1))
     { return; }
   
   /*==========================================*/
   /* Compare each of the subsequent arguments */
   /* to its predecessor. If any is greater or */
   /* equal, then return false.                */
   /*==========================================*/
   
   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,NUMBER_TYPES,&rv2))
        { return; }
        
      if (mCVIsType(&rv1,INTEGER_TYPE) && mCVIsType(&rv2,INTEGER_TYPE))
        {
         if (mCVToInteger(&rv1) >= mCVToInteger(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else
        {
         if (mCVToFloat(&rv1) >= mCVToFloat(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
        
      CVSetCLIPSValue(&rv1,&rv2);
     }

   /*=================================*/
   /* Each argument was less than its */
   /* predecessor. Return true.       */
   /*=================================*/

   mCVSetBoolean(returnValue,true);
  }

/*************************************/
/* GreaterThanFunction: H/L access   */
/*   routine for the > function.     */
/*************************************/
void GreaterThanFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue rv1, rv2;

   /*=========================*/
   /* Get the first argument. */
   /*=========================*/

   if (! UDFFirstArgument(context,NUMBER_TYPES,&rv1))
     { return; }
   
   /*==========================================*/
   /* Compare each of the subsequent arguments */
   /* to its predecessor. If any is lesser or  */
   /* equal, then return false.                */
   /*==========================================*/
   
   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,NUMBER_TYPES,&rv2))
        { return; }
        
      if (mCVIsType(&rv1,INTEGER_TYPE) && mCVIsType(&rv2,INTEGER_TYPE))
        {
         if (mCVToInteger(&rv1) <= mCVToInteger(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else
        {
         if (mCVToFloat(&rv1) <= mCVToFloat(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
        
      CVSetCLIPSValue(&rv1,&rv2);
     }

   /*=================================*/
   /* Each argument was less than its */
   /* predecessor. Return true.       */
   /*=================================*/

   mCVSetBoolean(returnValue,true);
  }

/**************************************/
/* NumericEqualFunction: H/L access   */
/*   routine for the = function.      */
/**************************************/
void NumericEqualFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue rv1, rv2;

   /*=========================*/
   /* Get the first argument. */
   /*=========================*/

   if (! UDFFirstArgument(context,NUMBER_TYPES,&rv1))
     { return; }

   /*=================================================*/
   /* Compare each of the subsequent arguments to the */
   /* first. If any is unequal, then return false.    */
   /*=================================================*/

   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,NUMBER_TYPES,&rv2))
        { return; }
        
      if (mCVIsType(&rv1,INTEGER_TYPE) && mCVIsType(&rv2,INTEGER_TYPE))
        {
         if (mCVToInteger(&rv1) != mCVToInteger(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else
        {
         if (mCVToFloat(&rv1) != mCVToFloat(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
     }
     
   /*=================================*/
   /* All arguments were equal to the */
   /* first argument. Return true.    */
   /*=================================*/

   mCVSetBoolean(returnValue,true);
  }

/*****************************************/
/* NumericNotEqualFunction: H/L access   */
/*   routine for the <> function.        */
/*****************************************/
void NumericNotEqualFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue rv1, rv2;

   /*=========================*/
   /* Get the first argument. */
   /*=========================*/

   if (! UDFFirstArgument(context,NUMBER_TYPES,&rv1))
     { return; }

   /*=================================================*/
   /* Compare each of the subsequent arguments to the */
   /* first. If any is equal, then return false.      */
   /*=================================================*/

   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,NUMBER_TYPES,&rv2))
        { return; }
        
      if (mCVIsType(&rv1,INTEGER_TYPE) && mCVIsType(&rv2,INTEGER_TYPE))
        {
         if (mCVToInteger(&rv1) == mCVToInteger(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
      else
        {
         if (mCVToFloat(&rv1) == mCVToFloat(&rv2))
           {
            mCVSetBoolean(returnValue,false);
            return;
           }
        }
     }
     
   /*===================================*/
   /* All arguments were unequal to the */
   /* first argument. Return true.      */
   /*===================================*/

   mCVSetBoolean(returnValue,true);
  }

/**************************************/
/* OddpFunction: H/L access routine   */
/*   for the oddp function.           */
/**************************************/
void OddpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;
   CLIPSInteger num, halfnum;
      
   /*===========================================*/
   /* Check for the correct types of arguments. */
   /*===========================================*/
 
   if (! UDFFirstArgument(context,INTEGER_TYPE,&item))
     { return; }
    
   /*===========================*/
   /* Compute the return value. */
   /*===========================*/
   
   num = mCVToInteger(&item);
   halfnum = (num / 2) * 2;

   if (num == halfnum) mCVSetBoolean(returnValue,false);
   else mCVSetBoolean(returnValue,true);
  }

/***************************************/
/* EvenpFunction: H/L access routine   */
/*   for the evenp function.           */
/***************************************/
void EvenpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue item;
   CLIPSInteger num, halfnum;
   
   /*===========================================*/
   /* Check for the correct types of arguments. */
   /*===========================================*/
     
   if (! UDFFirstArgument(context,INTEGER_TYPE,&item))
     { return; }

   /*===========================*/
   /* Compute the return value. */
   /*===========================*/
   
   num = mCVToInteger(&item);
   halfnum = (num / 2) * 2;
   
   if (num != halfnum) mCVSetBoolean(returnValue,false);
   else mCVSetBoolean(returnValue,true);
  }



