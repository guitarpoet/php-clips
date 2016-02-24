   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*               STRING FUNCTIONS MODULE               */
   /*******************************************************/

/*************************************************************/
/* Purpose: Contains the code for several string functions   */
/*   including str-cat, sym-cat, str-length, str-compare,    */
/*   upcase, lowcase, sub-string, str-index, eval, and       */
/*   build.                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Barry Cameron                                        */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*      6.30: Support for long long integers.                */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Used gensprintf instead of sprintf.            */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Added support for UTF-8 strings to str-length, */
/*            str-index, and sub-string functions.           */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Added code to keep track of pointers to        */
/*            constructs that are contained externally to    */
/*            to constructs, DanglingConstructs.             */
/*                                                           */
/*            Fixed str-cat bug that could be invoked by     */
/*            (funcall str-cat).                             */
/*                                                           */
/*      6.40: Added Env prefix to GetEvaluationError and     */
/*            SetEvaluationError functions.                  */
/*                                                           */
/*            Prior error flags are cleared before EnvEval   */
/*            and EnvBuild are processed.                    */
/*                                                           */
/*            The eval function can now access any local     */
/*            variables that have been defined.              */
/*                                                           */
/*************************************************************/

#include "setup.h"

#if STRING_FUNCTIONS

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "argacces.h"
#include "commline.h"
#include "constrct.h"
#include "cstrcpsr.h"
#include "engine.h"
#include "envrnmnt.h"
#include "exprnpsr.h"
#include "extnfunc.h"
#include "memalloc.h"
#include "prcdrpsr.h"
#include "router.h"
#include "strngrtr.h"
#include "scanner.h"
#include "sysdep.h"

#if DEFRULE_CONSTRUCT
#include "drive.h"
#endif

#include "strngfun.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static void                    StrOrSymCatFunction(UDFContext *,CLIPSValue *,unsigned short);

/******************************************/
/* StringFunctionDefinitions: Initializes */
/*   the string manipulation functions.   */
/******************************************/
void StringFunctionDefinitions(
  void *theEnv)
  {
#if ! RUN_TIME
   EnvAddUDF(theEnv,"str-cat",        "sy", StrCatFunction, "StrCatFunction", 1, UNBOUNDED, "synld" ,NULL);
   EnvAddUDF(theEnv,"sym-cat",        "sy", SymCatFunction, "SymCatFunction",  1, UNBOUNDED, "synld" ,NULL);
   EnvAddUDF(theEnv,"str-length",      "l", StrLengthFunction, "StrLengthFunction", 1,1,"syn",NULL);
   EnvAddUDF(theEnv,"str-compare",     "l", StrCompareFunction, "StrCompareFunction", 2,3, "*;syn;syn;l" ,NULL);
   EnvAddUDF(theEnv,"upcase",        "syn", UpcaseFunction, "UpcaseFunction", 1,1,"syn",NULL);
   EnvAddUDF(theEnv,"lowcase",       "syn",  LowcaseFunction, "LowcaseFunction", 1,1,"syn",NULL);
   EnvAddUDF(theEnv,"sub-string",      "s", SubStringFunction, "SubStringFunction",3,3, "*;l;l;syn",NULL);
   EnvAddUDF(theEnv,"str-index",      "bl", StrIndexFunction, "StrIndexFunction", 2,2,"syn",NULL);
   EnvAddUDF(theEnv,"eval",            "*", EvalFunction, "EvalFunction", 1,1,"sy",NULL);
   EnvAddUDF(theEnv,"build",           "b", BuildFunction, "BuildFunction", 1,1,"sy",NULL);
   EnvAddUDF(theEnv,"string-to-field", "*",  StringToFieldFunction, "StringToFieldFunction", 1,1,"syn",NULL);
#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif
  }

/****************************************/
/* StrCatFunction: H/L access routine   */
/*   for the str-cat function.          */
/****************************************/
void StrCatFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {   
   StrOrSymCatFunction(context,returnValue,STRING);
  }

/****************************************/
/* SymCatFunction: H/L access routine   */
/*   for the sym-cat function.          */
/****************************************/
void SymCatFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   StrOrSymCatFunction(context,returnValue,SYMBOL);
  }

/********************************************************/
/* StrOrSymCatFunction: Driver routine for implementing */
/*   the str-cat and sym-cat functions.                 */
/********************************************************/
static void StrOrSymCatFunction(
  UDFContext *context,
  CLIPSValue *returnValue,
  unsigned short returnType)
  {
   DATA_OBJECT theArg;
   int numArgs, i, total, j;
   char *theString;
   SYMBOL_HN **arrayOfStrings;
   SYMBOL_HN *hashPtr;
   const char *functionName;
   void *theEnv = UDFContextEnvironment(context);

   /*============================================*/
   /* Determine the calling function name.       */
   /* Store the null string or the symbol nil as */
   /* the return value in the event of an error. */
   /*============================================*/

   SetpType(returnValue,returnType);
   if (returnType == STRING)
     {
      functionName = "str-cat";
      SetpValue(returnValue,(void *) EnvAddSymbol(theEnv,""));
     }
   else
     {
      functionName = "sym-cat";
      SetpValue(returnValue,(void *) EnvAddSymbol(theEnv,"nil"));
     }

   /*===============================================*/
   /* Determine the number of arguments as create a */
   /* string array which is large enough to store   */
   /* the string representation of each argument.   */
   /*===============================================*/

   numArgs = EnvRtnArgCount(theEnv);
   if (numArgs == 0) return;
   
   arrayOfStrings = (SYMBOL_HN **) gm1(theEnv,(int) sizeof(SYMBOL_HN *) * numArgs);
   for (i = 0; i < numArgs; i++)   
     { arrayOfStrings[i] = NULL; }

   /*=============================================*/
   /* Evaluate each argument and store its string */
   /* representation in the string array.         */
   /*=============================================*/

   total = 1;
   for (i = 1 ; i <= numArgs ; i++)
     {
      EnvRtnUnknown(theEnv,i,&theArg);

      switch(GetType(theArg))
        {
         case STRING:
#if OBJECT_SYSTEM
         case INSTANCE_NAME:
#endif
         case SYMBOL:
           hashPtr = (SYMBOL_HN *) GetValue(theArg);
           arrayOfStrings[i-1] = hashPtr;
           IncrementSymbolCount(hashPtr); 
           break;

         case FLOAT:
           hashPtr = (SYMBOL_HN *) EnvAddSymbol(theEnv,FloatToString(theEnv,ValueToDouble(GetValue(theArg))));
           arrayOfStrings[i-1] = hashPtr;
           IncrementSymbolCount(hashPtr);
           break;

         case INTEGER:
           hashPtr = (SYMBOL_HN *) EnvAddSymbol(theEnv,LongIntegerToString(theEnv,ValueToLong(GetValue(theArg))));
           arrayOfStrings[i-1] = hashPtr;
           IncrementSymbolCount(hashPtr);
           break;

         default:
           ExpectedTypeError1(theEnv,functionName,i,"string, instance name, symbol, float, or integer");
           EnvSetEvaluationError(theEnv,true);
           break;
        }

      if (EvaluationData(theEnv)->EvaluationError)
        {
         for (i = 0; i < numArgs; i++)
           {
            if (arrayOfStrings[i] != NULL)
              { DecrementSymbolCount(theEnv,arrayOfStrings[i]); }
           }

         rm(theEnv,arrayOfStrings,sizeof(SYMBOL_HN *) * numArgs);
         return;
        }

      total += (int) strlen(ValueToString(arrayOfStrings[i - 1]));
     }

   /*=========================================================*/
   /* Allocate the memory to store the concatenated string or */
   /* symbol, then copy the values in the string array to the */
   /* memory just allocated.                                  */
   /*=========================================================*/

   theString = (char *) gm2(theEnv,(sizeof(char) * total));

   j = 0;
   for (i = 0 ; i < numArgs ; i++)
     {
      gensprintf(&theString[j],"%s",ValueToString(arrayOfStrings[i]));
      j += (int) strlen(ValueToString(arrayOfStrings[i]));
     }

   /*=========================================*/
   /* Return the concatenated value and clean */
   /* up the temporary memory used.           */
   /*=========================================*/

   SetpValue(returnValue,(void *) EnvAddSymbol(theEnv,theString));
   rm(theEnv,theString,sizeof(char) * total);

   for (i = 0; i < numArgs; i++)
     {
      if (arrayOfStrings[i] != NULL)
        { DecrementSymbolCount(theEnv,arrayOfStrings[i]); }
     }

   rm(theEnv,arrayOfStrings,sizeof(SYMBOL_HN *) * numArgs);
  }

/*******************************************/
/* StrLengthFunction: H/L access routine   */
/*   for the str-length function.          */
/*******************************************/
void StrLengthFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;

   /*==================================================================*/
   /* The argument should be of type symbol, string, or instance name. */
   /*==================================================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,&theArg))
     { return; }
     
   /*============================================*/
   /* Return the length of the string or symbol. */
   /*============================================*/
   
   mCVSetInteger(returnValue,UTF8Length(mCVToString(&theArg)));
  }

/****************************************/
/* UpcaseFunction: H/L access routine   */
/*   for the upcase function.           */
/****************************************/
void UpcaseFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   unsigned i;
   size_t slen;
   const char *osptr;
   char *nsptr;
   Environment *theEnv = UDFContextEnvironment(context);

   /*==================================================*/
   /* The argument should be of type symbol or string. */
   /*==================================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,returnValue))
     { return; }

   /*======================================================*/
   /* Allocate temporary memory and then copy the original */
   /* string or symbol to that memory, while uppercasing   */
   /* lower case alphabetic characters.                    */
   /*======================================================*/

   osptr = mCVToString(returnValue);
   slen = strlen(osptr) + 1;
   nsptr = (char *) gm2(theEnv,slen);

   for (i = 0  ; i < slen ; i++)
     {
      if (islower(osptr[i]))
        { nsptr[i] = (char) toupper(osptr[i]); }
      else
        { nsptr[i] = osptr[i]; }
     }

   /*========================================*/
   /* Return the uppercased string and clean */
   /* up the temporary memory used.          */
   /*========================================*/

   CVSetRawValue(returnValue,(void *) EnvAddSymbol(theEnv,nsptr));
   rm(theEnv,nsptr,slen);
  }

/*****************************************/
/* LowcaseFunction: H/L access routine   */
/*   for the lowcase function.           */
/*****************************************/
void LowcaseFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   unsigned i;
   size_t slen;
   const char *osptr;
   char *nsptr;
   Environment *theEnv = UDFContextEnvironment(context);

   /*==================================================*/
   /* The argument should be of type symbol or string. */
   /*==================================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,returnValue))
     { return; }

   /*======================================================*/
   /* Allocate temporary memory and then copy the original */
   /* string or symbol to that memory, while lowercasing   */
   /* upper case alphabetic characters.                    */
   /*======================================================*/

   osptr = mCVToString(returnValue);
   slen = strlen(osptr) + 1;
   nsptr = (char *) gm2(theEnv,slen);

   for (i = 0  ; i < slen ; i++)
     {
      if (isupper(osptr[i]))
        { nsptr[i] = (char) tolower(osptr[i]); }
      else
        { nsptr[i] = osptr[i]; }
     }

   /*========================================*/
   /* Return the lowercased string and clean */
   /* up the temporary memory used.          */
   /*========================================*/

   CVSetRawValue(returnValue,(void *) EnvAddSymbol(theEnv,nsptr));
   rm(theEnv,nsptr,slen);
  }

/********************************************/
/* StrCompareFunction: H/L access routine   */
/*   for the str-compare function.          */
/********************************************/
void StrCompareFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT arg1, arg2, arg3;
   int compareResult;

   /*=============================================================*/
   /* The first two arguments should be of type symbol or string. */
   /*=============================================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,&arg1))
     { return; }

   if (! UDFNextArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,&arg2))
     { return; }

   /*===================================================*/
   /* Compare the strings. Use the 3rd argument for the */
   /* maximum length of comparison, if it is provided.  */
   /*===================================================*/

   if (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,INTEGER_TYPE,&arg3))
        { return; }

      compareResult = strncmp(mCVToString(&arg1),mCVToString(&arg2),
                            (STD_SIZE) mCVToInteger(&arg3));
     }
   else
     { compareResult = strcmp(mCVToString(&arg1),mCVToString(&arg2)); }

   /*========================================================*/
   /* Return Values are as follows:                          */
   /* -1 is returned if <string-1> is less than <string-2>.  */
   /*  1 is return if <string-1> is greater than <string-2>. */
   /*  0 is returned if <string-1> is equal to <string-2>.   */
   /*========================================================*/

   if (compareResult < 0)
     { mCVSetInteger(returnValue,-1L); }
   else if (compareResult > 0)
     { mCVSetInteger(returnValue,1L); }
   else
     { mCVSetInteger(returnValue,0L); }
  }

/*******************************************/
/* SubStringFunction: H/L access routine   */
/*   for the sub-string function.          */
/*******************************************/
void SubStringFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;
   const char *tempString;
   char *returnString;
   size_t start, end, i, j, length;
   Environment *theEnv = UDFContextEnvironment(context);

   /*===================================*/
   /* Check and retrieve the arguments. */
   /*===================================*/

   if (! UDFFirstArgument(context,INTEGER_TYPE,&theArg))
     { return; }

   if (mCVToInteger(&theArg) < 1)
     { start = 0; }
   else
     { start = (size_t) mCVToInteger(&theArg) - 1; }

   if (! UDFNextArgument(context,INTEGER_TYPE,&theArg))
     { return; }

   if (mCVToInteger(&theArg) < 1)
     {
      mCVSetString(returnValue,"");
      return;
     }
   else
     { end = (size_t) mCVToInteger(&theArg) - 1; }

   if (! UDFNextArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,&theArg))
     { return; }
   
   tempString = mCVToString(&theArg);
   
   /*================================================*/
   /* If parameters are out of range return an error */
   /*================================================*/
   
   length = UTF8Length(tempString);
   
   if (end > length)
     { end = length; }

   /*==================================*/
   /* If the start is greater than the */
   /* end, return a null string.       */
   /*==================================*/

   if ((start > end) || (length == 0))
     {
      mCVSetString(returnValue,"");
      return;
     }

   /*=============================================*/
   /* Otherwise, allocate the string and copy the */
   /* designated portion of the old string to the */
   /* new string.                                 */
   /*=============================================*/

   else
     {
      start = UTF8Offset(tempString,start);
      end = UTF8Offset(tempString,end + 1) - 1;
      
      returnString = (char *) gm2(theEnv,(unsigned) (end - start + 2));  /* (end - start) inclusive + EOS */
      for(j=0, i=start;i <= end; i++, j++)
        { *(returnString+j) = *(tempString+i); }
      *(returnString+j) = '\0';
     }

   /*========================*/
   /* Return the new string. */
   /*========================*/

   mCVSetString(returnValue,returnString);
   rm(theEnv,returnString,(unsigned) (end - start + 2));
  }

/******************************************/
/* StrIndexFunction: H/L access routine   */
/*   for the sub-index function.          */
/******************************************/
void StrIndexFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg1, theArg2;
   const char *strg1, *strg2, *strg3;
   size_t i, j;

   mCVSetBoolean(returnValue,false);

   /*===================================*/
   /* Check and retrieve the arguments. */
   /*===================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,&theArg1))
     { return; }

   if (! UDFNextArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,&theArg2))
     { return; }

   strg1 = mCVToString(&theArg1);
   strg2 = mCVToString(&theArg2);

   /*=================================*/
   /* Find the position in string2 of */
   /* string1 (counting from 1).      */
   /*=================================*/

   if (strlen(strg1) == 0)
     {
      mCVSetInteger(returnValue,(long long) UTF8Length(strg2) + 1LL);
      return;
     }
     
   strg3 = strg2;
   for (i=1; *strg2; i++, strg2++)
     {
      for (j=0; *(strg1+j) && *(strg1+j) == *(strg2+j); j++)
        { /* Do Nothing */ }

      if (*(strg1+j) == '\0')
        {
         mCVSetInteger(returnValue,(long long) UTF8CharNum(strg3,i));
         return;
        }
     }

   return;
  }

/********************************************/
/* StringToFieldFunction: H/L access routine */
/*   for the string-to-field function.       */
/********************************************/
void StringToFieldFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;

   /*==================================================*/
   /* The argument should be of type symbol or string. */
   /*==================================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES | INSTANCE_NAME_TYPE,&theArg))
     {
      mCVSetSymbol(returnValue,"*** ERROR ***");
      return;
     }

   /*================================*/
   /* Convert the string to an atom. */
   /*================================*/

   StringToField(UDFContextEnvironment(context),mCVToString(&theArg),returnValue);
  }

/*************************************************************/
/* StringToField: Converts a string to an atomic data value. */
/*************************************************************/
void StringToField(
  void *theEnv,
  const char *theString,
  DATA_OBJECT *returnValue)
  {
   struct token theToken;

   /*====================================*/
   /* Open the string as an input source */
   /* and retrieve the first value.      */
   /*====================================*/

   OpenStringSource(theEnv,"string-to-field-str",theString,0);
   GetToken(theEnv,"string-to-field-str",&theToken);
   CloseStringSource(theEnv,"string-to-field-str");

   /*====================================================*/
   /* Copy the token to the return value data structure. */
   /*====================================================*/

   returnValue->type = theToken.type;
   if ((theToken.type == FLOAT) || (theToken.type == STRING) ||
#if OBJECT_SYSTEM
       (theToken.type == INSTANCE_NAME) ||
#endif
       (theToken.type == SYMBOL) || (theToken.type == INTEGER))
     { returnValue->value = theToken.value; }
   else if (theToken.type == STOP)
     {
      returnValue->type = SYMBOL;
      returnValue->value = (void *) EnvAddSymbol(theEnv,"EOF");
     }
   else if (theToken.type == UNKNOWN_VALUE)
     {
      returnValue->type = STRING;
      returnValue->value = (void *) EnvAddSymbol(theEnv,"*** ERROR ***");
     }
   else
     {
      returnValue->type = STRING;
      returnValue->value = (void *) EnvAddSymbol(theEnv,theToken.printForm);
     }
  }
  
#if (! RUN_TIME) && (! BLOAD_ONLY)

/**************************************/
/* EvalFunction: H/L access routine   */
/*   for the eval function.           */
/**************************************/
void EvalFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;

   /*==================================================*/
   /* The argument should be of type SYMBOL or STRING. */
   /*==================================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES,&theArg))
     { return; }

   /*======================*/
   /* Evaluate the string. */
   /*======================*/

   EnvEval(UDFContextEnvironment(context),mCVToString(&theArg),returnValue);
  }
  
/*****************************/
/* EnvEval: C access routine */
/*   for the eval function.  */
/*****************************/
bool EnvEval(
  void *theEnv,
  const char *theString,
  DATA_OBJECT_PTR returnValue)
  {
   struct expr *top;
   bool ov;
   static int depth = 0;
   char logicalNameBuffer[20];
   struct BindInfo *oldBinds;
   int danglingConstructs;

   returnValue->environment = theEnv;
   
   /*=====================================*/
   /* If embedded, clear the error flags. */
   /*=====================================*/
   
   if ((! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL))
     {
      EnvSetEvaluationError(theEnv,false);
      EnvSetHaltExecution(theEnv,false);
     }

   /*======================================================*/
   /* Evaluate the string. Create a different logical name */
   /* for use each time the eval function is called.       */
   /*======================================================*/

   depth++;
   gensprintf(logicalNameBuffer,"Eval-%d",depth);
   if (OpenStringSource(theEnv,logicalNameBuffer,theString,0) == 0)
     {
      mCVSetBoolean(returnValue,false);
      depth--;
      return(false);
     }

   /*================================================*/
   /* Save the current parsing state before routines */
   /* are called to parse the eval string.           */
   /*================================================*/

   ov = GetPPBufferStatus(theEnv);
   SetPPBufferStatus(theEnv,false);
   oldBinds = GetParsedBindNames(theEnv);
   SetParsedBindNames(theEnv,NULL);
   danglingConstructs = ConstructData(theEnv)->DanglingConstructs;

   /*========================================================*/
   /* Parse the string argument passed to the eval function. */
   /*========================================================*/

   top = ParseAtomOrExpression(theEnv,logicalNameBuffer,NULL);

   /*============================*/
   /* Restore the parsing state. */
   /*============================*/

   SetPPBufferStatus(theEnv,ov);
   ClearParsedBindNames(theEnv);
   SetParsedBindNames(theEnv,oldBinds);

   /*===========================================*/
   /* Return if an error occured while parsing. */
   /*===========================================*/

   if (top == NULL)
     {
      EnvSetEvaluationError(theEnv,true);
      CloseStringSource(theEnv,logicalNameBuffer);
      mCVSetBoolean(returnValue,false);
      depth--;
      ConstructData(theEnv)->DanglingConstructs = danglingConstructs;
      return(false);
     }

   /*==============================================*/
   /* The sequence expansion operator must be used */
   /* within the argument list of a function call. */
   /*==============================================*/

   if ((top->type == MF_GBL_VARIABLE) || (top->type == MF_VARIABLE))
     {
      PrintErrorID(theEnv,"MISCFUN",1,false);
      EnvPrintRouter(theEnv,WERROR,"expand$ must be used in the argument list of a function call.\n");
      EnvSetEvaluationError(theEnv,true);
      CloseStringSource(theEnv,logicalNameBuffer);
      mCVSetBoolean(returnValue,false);
      ReturnExpression(theEnv,top);
      depth--;
      ConstructData(theEnv)->DanglingConstructs = danglingConstructs;
      return(false);
     }

   /*====================================*/
   /* Evaluate the expression and return */
   /* the memory used to parse it.       */
   /*====================================*/

   ExpressionInstall(theEnv,top);
   EvaluateExpression(theEnv,top,returnValue);
   ExpressionDeinstall(theEnv,top);

   depth--;
   ReturnExpression(theEnv,top);
   CloseStringSource(theEnv,logicalNameBuffer);

   /*==============================================*/
   /* If embedded, reset dangling construct count. */
   /*==============================================*/
   
   if ((! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL))
     { ConstructData(theEnv)->DanglingConstructs = danglingConstructs; }

   /*==========================================*/
   /* Perform periodic cleanup if the eval was */
   /* issued from an embedded controller.      */
   /*==========================================*/

   if ((UtilityData(theEnv)->CurrentGarbageFrame->topLevel) && (! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL) && (UtilityData(theEnv)->GarbageCollectionLocks == 0))
     { 
      CleanCurrentGarbageFrame(theEnv,returnValue);
      CallPeriodicTasks(theEnv);
     }

   if (EnvGetEvaluationError(theEnv)) return(false);
   return(true);
  }

#else

/*************************************************/
/* EvalFunction: This is the non-functional stub */
/*   provided for use with a run-time version.   */
/*************************************************/
void EvalFunction(
  void *theEnv,
  DATA_OBJECT_PTR returnValue)
  {
   returnValue->environment = theEnv;
   PrintErrorID(theEnv,"STRNGFUN",1,false);
   EnvPrintRouter(theEnv,WERROR,"Function eval does not work in run time modules.\n");
   mCVSetBoolean(returnValue,false);
  }

/*****************************************************/
/* EnvEval: This is the non-functional stub provided */
/*   for use with a run-time version.                */
/*****************************************************/
bool EnvEval(
  void *theEnv,
  const char *theString,
  DATA_OBJECT_PTR returnValue)
  {
   returnValue->environment = theEnv;
   PrintErrorID(theEnv,"STRNGFUN",1,false);
   EnvPrintRouter(theEnv,WERROR,"Function eval does not work in run time modules.\n");
   mCVSetBoolean(returnValue,false);
   return(false);
  }

#endif

#if (! RUN_TIME) && (! BLOAD_ONLY)
/***************************************/
/* BuildFunction: H/L access routine   */
/*   for the build function.           */
/***************************************/
void BuildFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT theArg;

   /*==================================================*/
   /* The argument should be of type SYMBOL or STRING. */
   /*==================================================*/

   if (! UDFFirstArgument(context,LEXEME_TYPES,&theArg))
     { return; }

   /*======================*/
   /* Build the construct. */
   /*======================*/

   mCVSetBoolean(returnValue,(EnvBuild(UDFContextEnvironment(context),mCVToString(&theArg))));
  }
  
/******************************/
/* EnvBuild: C access routine */
/*   for the build function.  */
/******************************/
bool EnvBuild(
  void *theEnv,
  const char *theString)
  {
   const char *constructType;
   struct token theToken;
   int errorFlag;
   
   /*=====================================*/
   /* If embedded, clear the error flags. */
   /*=====================================*/
   
   if ((! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL))
     {
      EnvSetEvaluationError(theEnv,false);
      EnvSetHaltExecution(theEnv,false);
     }

   /*====================================================*/
   /* No additions during defrule join network activity. */
   /*====================================================*/

#if DEFRULE_CONSTRUCT
   if (EngineData(theEnv)->JoinOperationInProgress) return(false);
#endif

   /*===========================================*/
   /* Create a string source router so that the */
   /* string can be used as an input source.    */
   /*===========================================*/

   if (OpenStringSource(theEnv,"build",theString,0) == 0)
     { return(false); }

   /*================================*/
   /* The first token of a construct */
   /* must be a left parenthesis.    */
   /*================================*/

   GetToken(theEnv,"build",&theToken);

   if (theToken.type != LPAREN)
     {
      CloseStringSource(theEnv,"build");
      return(false);
     }

   /*==============================================*/
   /* The next token should be the construct type. */
   /*==============================================*/

   GetToken(theEnv,"build",&theToken);
   if (theToken.type != SYMBOL)
     {
      CloseStringSource(theEnv,"build");
      return(false);
     }

   constructType = ValueToString(theToken.value);

   /*======================*/
   /* Parse the construct. */
   /*======================*/
   
   errorFlag = ParseConstruct(theEnv,constructType,"build");

   /*=================================*/
   /* Close the string source router. */
   /*=================================*/

   CloseStringSource(theEnv,"build");

   /*=========================================*/
   /* If an error occured while parsing the   */
   /* construct, then print an error message. */
   /*=========================================*/

   if (errorFlag == 1)
     {
      EnvPrintRouter(theEnv,WERROR,"\nERROR:\n");
      PrintInChunks(theEnv,WERROR,GetPPBuffer(theEnv));
      EnvPrintRouter(theEnv,WERROR,"\n");
     }

   DestroyPPBuffer(theEnv);

   /*===========================================*/
   /* Perform periodic cleanup if the build was */
   /* issued from an embedded controller.       */
   /*===========================================*/

   if ((UtilityData(theEnv)->CurrentGarbageFrame->topLevel) && (! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL) && (UtilityData(theEnv)->GarbageCollectionLocks == 0))
     {
      CleanCurrentGarbageFrame(theEnv,NULL);
      CallPeriodicTasks(theEnv);
     }

   /*===============================================*/
   /* Return true if the construct was successfully */
   /* parsed, otherwise return false.               */
   /*===============================================*/

   if (errorFlag == 0) return(true);

   return(false);
  }
#else
/**************************************************/
/* BuildFunction: This is the non-functional stub */
/*   provided for use with a run-time version.    */
/**************************************************/
void BuildFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   PrintErrorID(theEnv,"STRNGFUN",1,false);
   EnvPrintRouter(theEnv,WERROR,"Function build does not work in run time modules.\n");
   mCVSetBoolean(returnValue,false);
  }

/******************************************************/
/* EnvBuild: This is the non-functional stub provided */
/*   for use with a run-time version.                 */
/******************************************************/
bool EnvBuild(
  void *theEnv,
  const char *theString)
  { 
   PrintErrorID(theEnv,"STRNGFUN",1,false);
   EnvPrintRouter(theEnv,WERROR,"Function build does not work in run time modules.\n");
   return(false);
  }
#endif /* (! RUN_TIME) && (! BLOAD_ONLY) */

#endif /* STRING_FUNCTIONS */
