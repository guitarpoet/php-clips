   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/13/16             */
   /*                                                     */
   /*                  EVALUATION MODULE                  */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides routines for evaluating expressions.    */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added EvaluateAndStoreInDataObject function.   */
/*                                                           */
/*      6.30: Added support for passing context information  */ 
/*            to user defined functions.                     */
/*                                                           */
/*            Added support for external address hash table  */
/*            and subtyping.                                 */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Support for DATA_OBJECT_ARRAY primitive.       */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*      6.40: Added Env prefix to GetEvaluationError and     */
/*            SetEvaluationError functions.                  */
/*                                                           */
/*            Added Env prefix to GetHaltExecution and       */
/*            SetHaltExecution functions.                    */
/*                                                           */
/*            Callbacks must be environment aware.           */
/*                                                           */
/*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "setup.h"

#include "argacces.h"
#include "commline.h"
#include "constant.h"
#include "envrnmnt.h"
#include "memalloc.h"
#include "router.h"
#include "prcdrfun.h"
#include "multifld.h"
#include "prntutil.h"
#include "exprnpsr.h"
#include "utility.h"
#include "proflfun.h"
#include "sysdep.h"

#if DEFFUNCTION_CONSTRUCT
#include "dffnxfun.h"
#endif

#if DEFGENERIC_CONSTRUCT
#include "genrccom.h"
#endif

#if OBJECT_SYSTEM
#include "object.h"
#include "inscom.h"
#endif

#include "evaluatn.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static void                    DeallocateEvaluationData(void *);
   static void                    PrintCAddress(void *,const char *,void *);
   static void                    NewCAddress(void *,DATA_OBJECT *);
   /*
   static bool                    DiscardCAddress(void *,void *);
   */
   
/**************************************************/
/* InitializeEvaluationData: Allocates environment */
/*    data for expression evaluation.             */
/**************************************************/
void InitializeEvaluationData(
  void *theEnv)
  {
   struct externalAddressType cPointer = { "C", PrintCAddress, PrintCAddress, NULL, NewCAddress, NULL };
   
   AllocateEnvironmentData(theEnv,EVALUATION_DATA,sizeof(struct evaluationData),DeallocateEvaluationData);

   InstallExternalAddressType(theEnv,&cPointer);
  }

/*****************************************************/
/* DeallocateEvaluationData: Deallocates environment */
/*    data for evaluation data.                      */
/*****************************************************/
static void DeallocateEvaluationData(
  void *theEnv)
  {
   int i;
   
   for (i = 0; i < EvaluationData(theEnv)->numberOfAddressTypes; i++)
     { rtn_struct(theEnv,externalAddressType,EvaluationData(theEnv)->ExternalAddressTypes[i]); }
  }

/**************************************************************/
/* EvaluateExpression: Evaluates an expression. Returns false */
/*   if no errors occurred during evaluation, otherwise true. */
/**************************************************************/
bool EvaluateExpression(
  void *theEnv,
  struct expr *problem,
  DATA_OBJECT_PTR returnValue)
  {
   struct expr *oldArgument;
   void *oldContext;
   struct FunctionDefinition *fptr;
   UDFContext theUDFContext;
#if PROFILING_FUNCTIONS
   struct profileFrameInfo profileFrame;
#endif

   returnValue->environment = theEnv;
   returnValue->type = RVOID;
   returnValue->bitType = VOID_TYPE;
   
   if (problem == NULL)
     {
      returnValue->type = SYMBOL;
      returnValue->value = EnvFalseSymbol(theEnv);
      return(EvaluationData(theEnv)->EvaluationError);
     }

   switch (problem->type)
     {
      case STRING:
      case SYMBOL:
      case FLOAT:
      case INTEGER:
#if OBJECT_SYSTEM
      case INSTANCE_NAME:
      case INSTANCE_ADDRESS:
#endif
      case EXTERNAL_ADDRESS:
        returnValue->type = problem->type;
        returnValue->value = problem->value;
        break;

      case DATA_OBJECT_ARRAY: /* TBD Remove with AddPrimitive */
        returnValue->type = problem->type;
        returnValue->value = problem->value;
        break;

      case FCALL:
        {
         fptr = (struct FunctionDefinition *) problem->value;
         oldContext = SetEnvironmentFunctionContext(theEnv,fptr->context);

#if PROFILING_FUNCTIONS   
         StartProfile(theEnv,&profileFrame,
                      &fptr->usrData,
                      ProfileFunctionData(theEnv)->ProfileUserFunctions);
#endif

         oldArgument = EvaluationData(theEnv)->CurrentExpression;
         EvaluationData(theEnv)->CurrentExpression = problem;

         switch(fptr->returnValueType)
           {
            case 'v' :
              (* (void (*)(void *)) fptr->functionPointer)(theEnv);
              returnValue->type = RVOID;
              returnValue->value = EnvFalseSymbol(theEnv);
              break;
            case 'b' :
              returnValue->type = SYMBOL;
              if ((* (bool (*)(void *)) fptr->functionPointer)(theEnv))
                returnValue->value = EnvTrueSymbol(theEnv);
              else
                returnValue->value = EnvFalseSymbol(theEnv);
              break;
            case 'a' :
              returnValue->type = EXTERNAL_ADDRESS;
              returnValue->value =
                             (* (void *(*)(void *)) fptr->functionPointer)(theEnv);
              break;
            case 'g' :
              returnValue->type = INTEGER;
              returnValue->value = (void *)
                EnvAddLong(theEnv,(* (long long (*)(void *)) fptr->functionPointer)(theEnv));
              break;
            case 'i' :
              returnValue->type = INTEGER;
              returnValue->value = (void *)
                EnvAddLong(theEnv,(long long) (* (int (*)(void *)) fptr->functionPointer)(theEnv));
              break;
            case 'l' :
              returnValue->type = INTEGER;
              returnValue->value = (void *)
                 EnvAddLong(theEnv,(long long) (* (long int (*)(void *)) fptr->functionPointer)(theEnv));
              break;
            case 'f' :
              returnValue->type = FLOAT;
              returnValue->value = (void *)
                 EnvAddDouble(theEnv,(double) (* (float (*)(void *)) fptr->functionPointer)(theEnv));
              break;
            case 'd' :
              returnValue->type = FLOAT;
              returnValue->value = (void *)
                 EnvAddDouble(theEnv,(* (double (*)(void *)) fptr->functionPointer)(theEnv));
              break;
            case 's' :
              returnValue->type = STRING;
              returnValue->value = (void *)
                (* (SYMBOL_HN *(*)(void *)) fptr->functionPointer)(theEnv);
              break;
            case 'w' :
              returnValue->type = SYMBOL;
              returnValue->value = (void *)
                (* (SYMBOL_HN *(*)(void *)) fptr->functionPointer)(theEnv);
              break;
#if OBJECT_SYSTEM
            case 'x' :
              returnValue->type = INSTANCE_ADDRESS;
              returnValue->value =
                              (* (void *(*)(void *)) fptr->functionPointer)(theEnv);
              if (returnValue->value == NULL)
                { returnValue->value = (void *) &InstanceData(theEnv)->DummyInstance; }
                
              break;
            case 'o' :
              returnValue->type = INSTANCE_NAME;
              returnValue->value = (void *)
                (* (SYMBOL_HN *(*)(void *)) fptr->functionPointer)(theEnv);
              break;
#endif

#if DEFTEMPLATE_CONSTRUCT
            case 'y' :
              returnValue->type = FACT_ADDRESS;
              returnValue->value =
                             (* (void *(*)(void *)) fptr->functionPointer)(theEnv);
              if (returnValue->value == NULL)
                { returnValue->value = (void *) &FactData(theEnv)->DummyFact; }
                
              break;
#endif

            case 'c' :
              {
               char cbuff[2];
               cbuff[0] = (* (char (*)(void *)) fptr->functionPointer)(theEnv);
               cbuff[1] = EOS;
               returnValue->type = SYMBOL;
               returnValue->value = (void *) EnvAddSymbol(theEnv,cbuff);
               break;
              }

            case 'j' :
            case 'k' :
            case 'm' :
            case 'n' :
            case 'u' :
              (* (void (*)(void *,DATA_OBJECT_PTR)) fptr->functionPointer)(theEnv,returnValue);
              break;

            case 'z' :
              theUDFContext.environment = theEnv;
              theUDFContext.theFunction = fptr;
              theUDFContext.lastArg = problem->argList;
              theUDFContext.lastPosition = 1;
              theUDFContext.returnValue = returnValue;
              (* (void (*)(UDFContext *,DATA_OBJECT_PTR)) fptr->functionPointer)(&theUDFContext,returnValue);
              break;

            default :
               SystemError(theEnv,"EVALUATN",2);
               EnvExitRouter(theEnv,EXIT_FAILURE);
               break;
            }

#if PROFILING_FUNCTIONS 
        EndProfile(theEnv,&profileFrame);
#endif

        SetEnvironmentFunctionContext(theEnv,oldContext);
        EvaluationData(theEnv)->CurrentExpression = oldArgument;
        break;
        }

     case MULTIFIELD:
        returnValue->type = MULTIFIELD;
        returnValue->value = ((DATA_OBJECT_PTR) (problem->value))->value;
        returnValue->begin = ((DATA_OBJECT_PTR) (problem->value))->begin;
        returnValue->end = ((DATA_OBJECT_PTR) (problem->value))->end;
        break;

     case MF_VARIABLE:
     case SF_VARIABLE:
        if (GetBoundVariable(theEnv,returnValue,(SYMBOL_HN *) problem->value) == false)
          {
           PrintErrorID(theEnv,"EVALUATN",1,false);
           EnvPrintRouter(theEnv,WERROR,"Variable ");
           EnvPrintRouter(theEnv,WERROR,ValueToString(problem->value));
           EnvPrintRouter(theEnv,WERROR," is unbound\n");
           returnValue->type = SYMBOL;
           returnValue->value = EnvFalseSymbol(theEnv);
           EnvSetEvaluationError(theEnv,true);
          }
        break;

      default:
        if (EvaluationData(theEnv)->PrimitivesArray[problem->type] == NULL)
          {
           SystemError(theEnv,"EVALUATN",3);
           EnvExitRouter(theEnv,EXIT_FAILURE);
          }

        if (EvaluationData(theEnv)->PrimitivesArray[problem->type]->copyToEvaluate)
          {
           returnValue->type = problem->type;
           returnValue->value = problem->value;
           break;
          }

        if (EvaluationData(theEnv)->PrimitivesArray[problem->type]->evaluateFunction == NULL)
          {
           SystemError(theEnv,"EVALUATN",4);
           EnvExitRouter(theEnv,EXIT_FAILURE);
          }

        oldArgument = EvaluationData(theEnv)->CurrentExpression;
        EvaluationData(theEnv)->CurrentExpression = problem;

#if PROFILING_FUNCTIONS 
        StartProfile(theEnv,&profileFrame,
                     &EvaluationData(theEnv)->PrimitivesArray[problem->type]->usrData,
                     ProfileFunctionData(theEnv)->ProfileUserFunctions);
#endif

        (*EvaluationData(theEnv)->PrimitivesArray[problem->type]->evaluateFunction)(theEnv,problem->value,returnValue);

#if PROFILING_FUNCTIONS
        EndProfile(theEnv,&profileFrame);
#endif

        EvaluationData(theEnv)->CurrentExpression = oldArgument;
        break;
     }

   return(EvaluationData(theEnv)->EvaluationError);
  }

/******************************************/
/* InstallPrimitive: Installs a primitive */
/*   data type in the primitives array.   */
/******************************************/
void InstallPrimitive(
  void *theEnv,
  struct entityRecord *thePrimitive,
  int whichPosition)
  {
   if (EvaluationData(theEnv)->PrimitivesArray[whichPosition] != NULL)
     {
      SystemError(theEnv,"EVALUATN",5);
      EnvExitRouter(theEnv,EXIT_FAILURE);
     }

   EvaluationData(theEnv)->PrimitivesArray[whichPosition] = thePrimitive;
  }

/******************************************************/
/* InstallExternalAddressType: Installs an external   */
/*   address type in the external address type array. */
/******************************************************/
int InstallExternalAddressType(
  void *theEnv,
  struct externalAddressType *theAddressType)
  {
   struct externalAddressType *copyEAT;
   
   int rv = EvaluationData(theEnv)->numberOfAddressTypes;
   
   if (EvaluationData(theEnv)->numberOfAddressTypes == MAXIMUM_EXTERNAL_ADDRESS_TYPES)
     {
      SystemError(theEnv,"EVALUATN",6);
      EnvExitRouter(theEnv,EXIT_FAILURE);
     }

   copyEAT = (struct externalAddressType *) genalloc(theEnv,sizeof(struct externalAddressType));
   memcpy(copyEAT,theAddressType,sizeof(struct externalAddressType));   
   EvaluationData(theEnv)->ExternalAddressTypes[EvaluationData(theEnv)->numberOfAddressTypes++] = copyEAT;
   
   return rv;
  }

/******************************************************/
/* SetEvaluationError: Sets the EvaluationError flag. */
/******************************************************/
void EnvSetEvaluationError(
  void *theEnv,
  bool value)
  {
   EvaluationData(theEnv)->EvaluationError = value;
   if (value == true)
     { EvaluationData(theEnv)->HaltExecution = true; }
  }

/*********************************************************/
/* GetEvaluationError: Returns the EvaluationError flag. */
/*********************************************************/
bool EnvGetEvaluationError(
  void *theEnv)
  {
   return(EvaluationData(theEnv)->EvaluationError);
  }

/*****************************************************/
/* EnvSetHaltExecution: Sets the HaltExecution flag. */
/*****************************************************/
void EnvSetHaltExecution(
  void *theEnv,
  bool value)
  { 
   EvaluationData(theEnv)->HaltExecution = value; 
  }

/********************************************************/
/* EnvGetHaltExecution: Returns the HaltExecution flag. */
/********************************************************/
bool EnvGetHaltExecution(
  void *theEnv)
  {
   return(EvaluationData(theEnv)->HaltExecution);
  }

/******************************************************/
/* ReturnValues: Returns a linked list of DATA_OBJECT */
/*   structures to the pool of free memory.           */
/******************************************************/
void ReturnValues(
  void *theEnv,
  DATA_OBJECT_PTR garbagePtr,
  bool decrementSupplementalInfo)
  {
   DATA_OBJECT_PTR nextPtr;

   while (garbagePtr != NULL)
     {
      nextPtr = garbagePtr->next;
      ValueDeinstall(theEnv,garbagePtr);
      if ((garbagePtr->supplementalInfo != NULL) && decrementSupplementalInfo)
        { DecrementSymbolCount(theEnv,(struct symbolHashNode *) garbagePtr->supplementalInfo); }
      rtn_struct(theEnv,dataObject,garbagePtr);
      garbagePtr = nextPtr;
     }
  }

/***************************************************/
/* PrintDataObject: Prints a DATA_OBJECT structure */
/*   to the specified logical name.                */
/***************************************************/
void PrintDataObject(
  void *theEnv,
  const char *fileid,
  DATA_OBJECT_PTR argPtr)
  {
   switch(argPtr->type)
     {
      case RVOID:
      case SYMBOL:
      case STRING:
      case INTEGER:
      case FLOAT:
      case EXTERNAL_ADDRESS:
      case DATA_OBJECT_ARRAY: // TBD Remove with AddPrimitive
      case FACT_ADDRESS:
#if OBJECT_SYSTEM
      case INSTANCE_NAME:
      case INSTANCE_ADDRESS:
#endif
        PrintAtom(theEnv,fileid,argPtr->type,argPtr->value);
        break;

      case MULTIFIELD:
        PrintMultifield(theEnv,fileid,(struct multifield *) argPtr->value,
                        argPtr->begin,argPtr->end,true);
        break;

      default:
        if (EvaluationData(theEnv)->PrimitivesArray[argPtr->type] != NULL)
          {
           if (EvaluationData(theEnv)->PrimitivesArray[argPtr->type]->longPrintFunction)
             {
              (*EvaluationData(theEnv)->PrimitivesArray[argPtr->type]->longPrintFunction)(theEnv,fileid,argPtr->value);
              break;
             }
           else if (EvaluationData(theEnv)->PrimitivesArray[argPtr->type]->shortPrintFunction)
             {
              (*EvaluationData(theEnv)->PrimitivesArray[argPtr->type]->shortPrintFunction)(theEnv,fileid,argPtr->value);
              break;
             }
          }

        EnvPrintRouter(theEnv,fileid,"<UnknownPrintType");
        PrintLongInteger(theEnv,fileid,(long int) argPtr->type);
        EnvPrintRouter(theEnv,fileid,">");
        EnvSetHaltExecution(theEnv,true);
        EnvSetEvaluationError(theEnv,true);
        break;
     }
  }

/****************************************************/
/* EnvSetMultifieldErrorValue: Creates a multifield */
/*   value of length zero for error returns.        */
/****************************************************/
void EnvSetMultifieldErrorValue(
  void *theEnv,
  DATA_OBJECT_PTR returnValue)
  {
   returnValue->type = MULTIFIELD;
   returnValue->bitType = MULTIFIELD_TYPE;
   returnValue->value = EnvCreateMultifield(theEnv,0L);
   returnValue->begin = 1;
   returnValue->end = 0;
  }

/**************************************************/
/* ValueInstall: Increments the appropriate count */
/*   (in use) values for a DATA_OBJECT structure. */
/**************************************************/
void ValueInstall(
  void *theEnv,
  DATA_OBJECT *vPtr)
  {
   if (vPtr->type == MULTIFIELD) MultifieldInstall(theEnv,(struct multifield *) vPtr->value);
   else AtomInstall(theEnv,vPtr->type,vPtr->value);
  }

/****************************************************/
/* ValueDeinstall: Decrements the appropriate count */
/*   (in use) values for a DATA_OBJECT structure.   */
/****************************************************/
void ValueDeinstall(
  void *theEnv,
  DATA_OBJECT *vPtr)
  {
   if (vPtr->type == MULTIFIELD) MultifieldDeinstall(theEnv,(struct multifield *) vPtr->value);
   else AtomDeinstall(theEnv,vPtr->type,vPtr->value);
  }

/*****************************************/
/* AtomInstall: Increments the reference */
/*   count of an atomic data type.       */
/*****************************************/
void AtomInstall(
  void *theEnv,
  int type,
  void *vPtr)
  {
   switch (type)
     {
      case SYMBOL:
      case STRING:
#if DEFGLOBAL_CONSTRUCT
      case GBL_VARIABLE:
#endif
#if OBJECT_SYSTEM
      case INSTANCE_NAME:
#endif
        IncrementSymbolCount(vPtr);
        break;

      case FLOAT:
        IncrementFloatCount(vPtr);
        break;

      case INTEGER:
        IncrementIntegerCount(vPtr);
        break;

      case EXTERNAL_ADDRESS:
        IncrementExternalAddressCount(vPtr);
        break;

      case MULTIFIELD:
        MultifieldInstall(theEnv,(struct multifield *) vPtr);
        break;

      case RVOID:
        break;

      default:
        if (EvaluationData(theEnv)->PrimitivesArray[type] == NULL) break;
        if (EvaluationData(theEnv)->PrimitivesArray[type]->bitMap) IncrementBitMapCount(vPtr);
        else if (EvaluationData(theEnv)->PrimitivesArray[type]->incrementBusyCount)
          { (*EvaluationData(theEnv)->PrimitivesArray[type]->incrementBusyCount)(theEnv,vPtr); }
        break;
     }
  }

/*******************************************/
/* AtomDeinstall: Decrements the reference */
/*   count of an atomic data type.         */
/*******************************************/
void AtomDeinstall(
  void *theEnv,
  int type,
  void *vPtr)
  {
   switch (type)
     {
      case SYMBOL:
      case STRING:
#if DEFGLOBAL_CONSTRUCT
      case GBL_VARIABLE:
#endif
#if OBJECT_SYSTEM
      case INSTANCE_NAME:
#endif
        DecrementSymbolCount(theEnv,(SYMBOL_HN *) vPtr);
        break;

      case FLOAT:
        DecrementFloatCount(theEnv,(FLOAT_HN *) vPtr);
        break;

      case INTEGER:
        DecrementIntegerCount(theEnv,(INTEGER_HN *) vPtr);
        break;

      case EXTERNAL_ADDRESS:
        DecrementExternalAddressCount(theEnv,(EXTERNAL_ADDRESS_HN *) vPtr);
        break;

      case MULTIFIELD:
        MultifieldDeinstall(theEnv,(struct multifield *) vPtr);
        break;

      case RVOID:
        break;

      default:
        if (EvaluationData(theEnv)->PrimitivesArray[type] == NULL) break;
        if (EvaluationData(theEnv)->PrimitivesArray[type]->bitMap) DecrementBitMapCount(theEnv,(BITMAP_HN *) vPtr);
        else if (EvaluationData(theEnv)->PrimitivesArray[type]->decrementBusyCount)
          { (*EvaluationData(theEnv)->PrimitivesArray[type]->decrementBusyCount)(theEnv,vPtr); }
     }
  }

#if DEFFUNCTION_CONSTRUCT || DEFGENERIC_CONSTRUCT

/********************************************/
/* EnvFunctionCall: Allows Deffunctions and */
/*   Generic Functions to be called from C. */
/*   Allows only constants as arguments.    */
/********************************************/
bool EnvFunctionCall(
  void *theEnv,
  const char *name,
  const char *args,
  DATA_OBJECT *result)
  {
   FUNCTION_REFERENCE theReference;

   /*=======================================*/
   /* Call the function if it can be found. */
   /*=======================================*/

   if (GetFunctionReference(theEnv,name,&theReference))
     { return(FunctionCall2(theEnv,&theReference,args,result)); }

   /*=========================================================*/
   /* Otherwise signal an error if a deffunction, defgeneric, */
   /* or user defined function doesn't exist that matches     */
   /* the specified function name.                            */
   /*=========================================================*/

   PrintErrorID(theEnv,"EVALUATN",2,false);
   EnvPrintRouter(theEnv,WERROR,"No function, generic function or deffunction of name ");
   EnvPrintRouter(theEnv,WERROR,name);
   EnvPrintRouter(theEnv,WERROR," exists for external call.\n");
   return(true);
  }

/********************************************/
/* FunctionCall2: Allows Deffunctions and    */
/*   Generic Functions to be called from C. */
/*   Allows only constants as arguments.    */
/********************************************/
bool FunctionCall2(
  void *theEnv,
  FUNCTION_REFERENCE *theReference,
  const char *args,
  DATA_OBJECT *result)
  {
   EXPRESSION *argexps;
   bool error = false;

   /*=============================================*/
   /* Force periodic cleanup if the function call */
   /* was executed from an embedded application.  */
   /*=============================================*/

   if ((UtilityData(theEnv)->CurrentGarbageFrame->topLevel) && (! CommandLineData(theEnv)->EvaluatingTopLevelCommand) &&
       (EvaluationData(theEnv)->CurrentExpression == NULL) && (UtilityData(theEnv)->GarbageCollectionLocks == 0))
     {
      CleanCurrentGarbageFrame(theEnv,NULL);
      CallPeriodicTasks(theEnv);
     }

   /*========================*/
   /* Reset the error state. */
   /*========================*/

   if (UtilityData(theEnv)->CurrentGarbageFrame->topLevel) EnvSetHaltExecution(theEnv,false);
   EvaluationData(theEnv)->EvaluationError = false;

   /*======================================*/
   /* Initialize the default return value. */
   /*======================================*/

   result->type = SYMBOL;
   result->value = EnvFalseSymbol(theEnv);

   /*============================*/
   /* Parse the argument string. */
   /*============================*/

   argexps = ParseConstantArguments(theEnv,args,&error);
   if (error == true) return(true);

   /*====================*/
   /* Call the function. */
   /*====================*/

   theReference->argList = argexps;
   error = EvaluateExpression(theEnv,theReference,result);

   /*========================*/
   /* Return the expression. */
   /*========================*/

   ReturnExpression(theEnv,argexps);
   theReference->argList = NULL;

   /*==========================*/
   /* Return the error status. */
   /*==========================*/

   return(error);
  }

#endif

/***************************************************/
/* CopyDataObject: Copies the values from a source */
/*   DATA_OBJECT to a destination DATA_OBJECT.     */
/***************************************************/
void CopyDataObject(
  void *theEnv,
  DATA_OBJECT *dst,
  DATA_OBJECT *src,
  int garbageMultifield)
  {
   if (src->type != MULTIFIELD)
     {
      dst->type = src->type;
      dst->value = src->value;
     }
   else
     {
      DuplicateMultifield(theEnv,dst,src);
      if (garbageMultifield)
        { AddToMultifieldList(theEnv,(struct multifield *) dst->value); }
     }
  }

/***********************************************/
/* TransferDataObjectValues: Copies the values */
/*   directly from a source DATA_OBJECT to a   */
/*   destination DATA_OBJECT.                  */
/***********************************************/
void TransferDataObjectValues(
  DATA_OBJECT *dst,
  DATA_OBJECT *src)
  {
   dst->type = src->type;
   dst->value = src->value;
   dst->begin = src->begin;
   dst->end = src->end;
   dst->supplementalInfo = src->supplementalInfo;
   dst->next = src->next;
  }

/************************************************************************/
/* ConvertValueToExpression: Converts the value stored in a data object */
/*   into an expression. For multifield values, a chain of expressions  */
/*   is generated and the chain is linked by the nextArg field. For a   */
/*   single field value, a single expression is created.                */
/************************************************************************/
struct expr *ConvertValueToExpression(
  void *theEnv,
  DATA_OBJECT *theValue)
  {
   long i;
   struct expr *head = NULL, *last = NULL, *newItem;

   if (GetpType(theValue) != MULTIFIELD)
     { return(GenConstant(theEnv,GetpType(theValue),GetpValue(theValue))); }

   for (i = GetpDOBegin(theValue); i <= GetpDOEnd(theValue); i++)
     {
      newItem = GenConstant(theEnv,GetMFType(GetpValue(theValue),i),
                        GetMFValue(GetpValue(theValue),i));
      if (last == NULL) head = newItem;
      else last->nextArg = newItem;
      last = newItem;
     }

   if (head == NULL)
     return(GenConstant(theEnv,FCALL,(void *) FindFunction(theEnv,"create$")));

   return(head);
  }

/****************************************/
/* GetAtomicHashValue: Returns the hash */
/*   value for an atomic data type.     */
/****************************************/
unsigned long GetAtomicHashValue(
  unsigned short type,
  void *value,
  int position)
  {
   unsigned long tvalue;
   union
     {
      double fv;
      void *vv;
      unsigned long liv;
     } fis;

   switch (type)
     {
      case FLOAT:
        fis.liv = 0;
        fis.fv = ValueToDouble(value);
        tvalue = fis.liv;
        break;

      case INTEGER:
        tvalue = (unsigned long) ValueToLong(value);
        break;

      case EXTERNAL_ADDRESS:
         fis.liv = 0;
         fis.vv = ValueToExternalAddress(value);
         tvalue = (unsigned long) fis.liv;
         break;

      case FACT_ADDRESS:
#if OBJECT_SYSTEM
      case INSTANCE_ADDRESS:
#endif
         fis.liv = 0;
         fis.vv = value;
         tvalue = (unsigned long) fis.liv;
         break;
         
      case STRING:
#if OBJECT_SYSTEM
      case INSTANCE_NAME:
#endif
      case SYMBOL:
        tvalue = ((SYMBOL_HN *) value)->bucket;
        break;

      default:
        tvalue = type;
     }

   if (position < 0) return(tvalue);

   return((unsigned long) (tvalue * (((unsigned long) position) + 29)));
  }

/***********************************************************/
/* FunctionReferenceExpression: Returns an expression with */
/*   an appropriate expression reference to the specified  */
/*   name if it is the name of a deffunction, defgeneric,  */
/*   or user/system defined function.                      */
/***********************************************************/
struct expr *FunctionReferenceExpression(
  void *theEnv,
  const char *name)
  {
#if DEFGENERIC_CONSTRUCT
   void *gfunc;
#endif
#if DEFFUNCTION_CONSTRUCT
   void *dptr;
#endif
   struct FunctionDefinition *fptr;

   /*=====================================================*/
   /* Check to see if the function call is a deffunction. */
   /*=====================================================*/

#if DEFFUNCTION_CONSTRUCT
   if ((dptr = (void *) LookupDeffunctionInScope(theEnv,name)) != NULL)
     { return(GenConstant(theEnv,PCALL,dptr)); }
#endif

   /*====================================================*/
   /* Check to see if the function call is a defgeneric. */
   /*====================================================*/

#if DEFGENERIC_CONSTRUCT
   if ((gfunc = (void *) LookupDefgenericInScope(theEnv,name)) != NULL)
     { return(GenConstant(theEnv,GCALL,gfunc)); }
#endif

   /*======================================*/
   /* Check to see if the function call is */
   /* a system or user defined function.   */
   /*======================================*/

   if ((fptr = FindFunction(theEnv,name)) != NULL)
     { return(GenConstant(theEnv,FCALL,fptr)); }

   /*===================================================*/
   /* The specified function name is not a deffunction, */
   /* defgeneric, or user/system defined function.      */
   /*===================================================*/

   return(NULL);
  }

/******************************************************************/
/* GetFunctionReference: Fills an expression with an appropriate  */
/*   expression reference to the specified name if it is the      */
/*   name of a deffunction, defgeneric, or user/system defined    */
/*   function.                                                    */
/******************************************************************/
bool GetFunctionReference(
  void *theEnv,
  const char *name,
  FUNCTION_REFERENCE *theReference)
  {
#if DEFGENERIC_CONSTRUCT
   void *gfunc;
#endif
#if DEFFUNCTION_CONSTRUCT
   void *dptr;
#endif
   struct FunctionDefinition *fptr;

   theReference->nextArg = NULL;
   theReference->argList = NULL;
   theReference->type = RVOID;
   theReference->value = NULL;

   /*=====================================================*/
   /* Check to see if the function call is a deffunction. */
   /*=====================================================*/

#if DEFFUNCTION_CONSTRUCT
   if ((dptr = (void *) LookupDeffunctionInScope(theEnv,name)) != NULL)
     {
      theReference->type = PCALL;
      theReference->value = dptr;
      return(true);
     }
#endif

   /*====================================================*/
   /* Check to see if the function call is a defgeneric. */
   /*====================================================*/

#if DEFGENERIC_CONSTRUCT
   if ((gfunc = (void *) LookupDefgenericInScope(theEnv,name)) != NULL)
     {
      theReference->type = GCALL;
      theReference->value = gfunc;
      return(true);
     }
#endif

   /*======================================*/
   /* Check to see if the function call is */
   /* a system or user defined function.   */
   /*======================================*/

   if ((fptr = FindFunction(theEnv,name)) != NULL)
     {
      theReference->type = FCALL;
      theReference->value = fptr;
      return(true);
     }

   /*===================================================*/
   /* The specified function name is not a deffunction, */
   /* defgeneric, or user/system defined function.      */
   /*===================================================*/

   return(false);
  }

/*******************************************************/
/* DOsEqual: Determines if two DATA_OBJECTS are equal. */
/*******************************************************/
bool DOsEqual(
  DATA_OBJECT_PTR dobj1,
  DATA_OBJECT_PTR dobj2)
  {
   if (GetpType(dobj1) != GetpType(dobj2))
     { return(false); }

   if (GetpType(dobj1) == MULTIFIELD)
     {
      if (MultifieldDOsEqual(dobj1,dobj2) == false)
        { return(false); }
     }
   else if (GetpValue(dobj1) != GetpValue(dobj2))
     { return(false); }

   return(true);
  }

/***********************************************************
  NAME         : EvaluateAndStoreInDataObject
  DESCRIPTION  : Evaluates slot-value expressions
                   and stores the result in a
                   Kernel data object
  INPUTS       : 1) Flag indicating if multifields are OK
                 2) The value-expression
                 3) The data object structure
                 4) Flag indicating if a multifield value
                    should be placed on the garbage list.
  RETURNS      : false on errors, true otherwise
  SIDE EFFECTS : Segment allocated for storing
                 multifield values
  NOTES        : None
 ***********************************************************/
bool EvaluateAndStoreInDataObject(
  void *theEnv,
  bool mfp,
  EXPRESSION *theExp,
  DATA_OBJECT *val,
  bool garbageSegment)
  {
   val->type = MULTIFIELD;
   val->begin = 0;
   val->end = -1;
   
   if (theExp == NULL)
     {
      if (garbageSegment) val->value = EnvCreateMultifield(theEnv,0L);
      else val->value = CreateMultifield2(theEnv,0L);

      return(true);
     }

   if ((mfp == false) && (theExp->nextArg == NULL))
     EvaluateExpression(theEnv,theExp,val);
   else
     StoreInMultifield(theEnv,val,theExp,garbageSegment);
   
   return(EvaluationData(theEnv)->EvaluationError ? false : true);
  }

/******************/
/* MFSetNthValueF */
/******************/
void MFSetNthValueF(
  CLIPSValue *mf,
  CLIPSInteger n,
  CLIPSValue *nv)
  {
   ((struct field *) ((struct multifield *) (mf->value))->theFields)[(mf->begin + n) - 1].type = nv->type;
   ((struct field *) ((struct multifield *) (mf->value))->theFields)[(mf->begin + n) - 1].value = nv->value;
  }

/***********************/
/* CVCreateMultifieldF */
/***********************/
void CVCreateMultifieldF(
  CLIPSValue *mf,
  CLIPSInteger size)
  {
   mf->value = EnvCreateMultifield(mf->environment,size);
   mf->bitType = MULTIFIELD_TYPE;
   mf->type = MULTIFIELD;
   mf->begin = 0;
   mf->end = size - 1;
  }

/************/
/* CVIsType */
/************/
bool CVIsType(
  CLIPSValue *theValue,
  unsigned expectedType)
  {
   return (theValue->bitType & expectedType);
  }

/*************/
/* EnvCVInit */
/*************/
void EnvCVInit(
  Environment *theEnv,
  CLIPSValue *theValue)
  {
   theValue->environment = theEnv;
  }

/**************/
/* CVToString */
/**************/
CLIPSString CVToString(
  CLIPSValue *theValue)
  {
   if (mCVIsType(theValue,LEXEME_TYPES | INSTANCE_NAME_TYPE))
     { return ((struct symbolHashNode *) theValue->value)->contents; }
   else
     { return NULL; }
  }

/***************/
/* CVToInteger */
/***************/
CLIPSInteger CVToInteger(
  CLIPSValue *theValue)
  {
   if (mCVIsType(theValue,INTEGER_TYPE))
     { return ((struct integerHashNode *) theValue->value)->contents; }
   else if (mCVIsType(theValue,FLOAT_TYPE))
     { return (CLIPSInteger) ((struct floatHashNode *) theValue->value)->contents; }
   else
     { return 0LL; }
  }

/*************/
/* CVToFloat */
/*************/
CLIPSFloat CVToFloat(
  CLIPSValue *theValue)
  {
   if (mCVIsType(theValue,FLOAT_TYPE))
     { return ((struct floatHashNode *) theValue->value)->contents; }
   else if (mCVIsType(theValue,INTEGER_TYPE))
     { return (CLIPSFloat) ((struct integerHashNode *) theValue->value)->contents; }
   else
     { return 0.0; }
  }

/*************/
/* CVSetVoid */
/*************/
void CVSetVoid(
  CLIPSValue *theValue)
  {
   theValue->value = NULL;
   theValue->bitType = VOID_TYPE;
   theValue->type = RVOID;
  }

/****************/
/* CVSetInteger */
/****************/
void CVSetInteger(
  CLIPSValue *theValue,
  CLIPSInteger iv)
  {
   theValue->value = EnvAddLong(theValue->environment,iv);
   theValue->bitType = INTEGER_TYPE;
   theValue->type = INTEGER;
  }

/**************/
/* CVSetFloat */
/**************/
void CVSetFloat(
  CLIPSValue *theValue,
  CLIPSFloat fv)
  {
   theValue->value = EnvAddDouble(theValue->environment,fv);
   theValue->bitType = FLOAT_TYPE;
   theValue->type = FLOAT;
  }

/***************/
/* CVSetString */
/***************/
void CVSetString(
  CLIPSValue *theValue,
  CLIPSString sv)
  {
   theValue->value = EnvAddSymbol(theValue->environment,sv);
   theValue->bitType = STRING_TYPE;
   theValue->type = STRING;
  }

/***************/
/* CVSetSymbol */
/***************/
void CVSetSymbol(
  CLIPSValue *theValue,
  CLIPSString sv)
  {
   theValue->value = EnvAddSymbol(theValue->environment,sv);
   theValue->bitType = SYMBOL_TYPE;
   theValue->type = SYMBOL;
  }

/*********************/
/* CVSetInstanceName */
/*********************/
void CVSetInstanceName(
  CLIPSValue *theValue,
  CLIPSString sv)
  {
   theValue->value = EnvAddSymbol(theValue->environment,sv);
   theValue->bitType = INSTANCE_NAME_TYPE;
   theValue->type = INSTANCE_NAME;
  }

/********************/
/* CVSetFactAddress */
/********************/
void CVSetFactAddress(
  CLIPSValue *theValue,
  Fact *fv)
  {
   theValue->value = fv;
   theValue->bitType = FACT_ADDRESS_TYPE;
   theValue->type = FACT_ADDRESS;
  }

/************************/
/* CVSetInstanceAddress */
/************************/
void CVSetInstanceAddress(
  CLIPSValue *theValue,
  Instance *iv)
  {
   theValue->value = iv;
   theValue->bitType = INSTANCE_ADDRESS_TYPE;
   theValue->type = INSTANCE_ADDRESS;
  }

/****************/
/* CVSetBoolean */
/****************/
void CVSetBoolean(
  CLIPSValue *theValue,
  bool bv)
  {
   if (bv)
     { theValue->value = SymbolData(theValue->environment)->TrueSymbolHN; }
   else
     { theValue->value = SymbolData(theValue->environment)->FalseSymbolHN; }
   theValue->bitType = (SYMBOL_TYPE | BOOLEAN_TYPE);
   theValue->type = SYMBOL;
  }

/******************/
/* PrintCAddress: */
/******************/
static void PrintCAddress(
  void *theEnv,
  const char *logicalName,
  void *theValue)
  {
   char buffer[20];

   EnvPrintRouter(theEnv,logicalName,"<Pointer-C-");
        
   gensprintf(buffer,"%p",ValueToExternalAddress(theValue));
   EnvPrintRouter(theEnv,logicalName,buffer);
   EnvPrintRouter(theEnv,logicalName,">");
  }

/****************/
/* NewCAddress: */
/****************/
static void NewCAddress(
  void *theEnv,
  DATA_OBJECT *rv)
  {
   int numberOfArguments;

   numberOfArguments = EnvRtnArgCount(theEnv);
      
   if (numberOfArguments != 1)
     {
      PrintErrorID(theEnv,"NEW",1,false);
      EnvPrintRouter(theEnv,WERROR,"Function new expected no additional arguments for the C external language type.\n");
      EnvSetEvaluationError(theEnv,true);
      return;
     }

   SetpType(rv,EXTERNAL_ADDRESS);
   SetpValue(rv,EnvAddExternalAddress(theEnv,NULL,0));
  }

/*******************************/
/* DiscardCAddress: TBD Remove */
/*******************************/
/*
static bool DiscardCAddress(
  void *theEnv,
  void *theValue)
  {
   EnvPrintRouter(theEnv,WDISPLAY,"Discarding C Address\n");
   
   return true;
  }
*/

