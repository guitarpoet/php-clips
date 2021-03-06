   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/13/16             */
   /*                                                     */
   /*             MULTIFIELD FUNCTIONS MODULE             */
   /*******************************************************/

/*************************************************************/
/* Purpose: Contains the code for several multifield         */
/*   functions including first$, rest$, subseq$, delete$,    */
/*   delete-member$, replace-member$                         */
/*   replace$, insert$, explode$, implode$, nth$, member$,   */
/*   subsetp, progn$, str-implode, str-explode, subset, nth, */
/*   mv-replace, member, mv-subseq, and mv-delete.           */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*      Brian Dantes                                         */
/*      Barry Cameron                                        */
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
/*            Moved ImplodeMultifield to multifld.c.         */
/*                                                           */
/*      6.30: Changed integer type/precision.                */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Fixed memory leaks when error occurred.        */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Fixed linkage issue when DEFMODULE_CONSTRUCT   */
/*            compiler flag is set to 0.                     */
/*                                                           */
/*      6.40: Added Env prefix to GetEvaluationError and     */
/*            SetEvaluationError functions.                  */
/*                                                           */
/*            Added Env prefix to GetHaltExecution and       */
/*            SetHaltExecution functions.                    */
/*                                                           */
/*            Removed mv-replace, mv-subseq, and  mv-delete  */
/*            functions.                                     */
/*                                                           */
/*************************************************************/

#include "setup.h"

#if MULTIFIELD_FUNCTIONS || OBJECT_SYSTEM

#include <stdio.h>
#include <string.h>

#include "argacces.h"
#include "envrnmnt.h"
#include "exprnpsr.h"
#include "memalloc.h"
#include "multifld.h"
#include "multifun.h"
#if OBJECT_SYSTEM
#include "object.h"
#endif
#include "prcdrpsr.h"
#include "prcdrfun.h"
#include "router.h"
#if (! BLOAD_ONLY) && (! RUN_TIME)
#include "scanner.h"
#endif
#include "utility.h"

/**************/
/* STRUCTURES */
/**************/

typedef struct fieldVarStack
  {
   unsigned short type;
   void *value;
   long index;
   struct fieldVarStack *nxt;
  } FIELD_VAR_STACK;

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

#if MULTIFIELD_FUNCTIONS
   static bool                    MVRangeCheck(long,long,long *,int);
   static void                    MultifieldPrognDriver(void *,DATA_OBJECT_PTR,const char *);
#if (! BLOAD_ONLY) && (! RUN_TIME)
   static struct expr            *MultifieldPrognParser(void *,struct expr *,const char *);
   static struct expr            *ForeachParser(void *,struct expr *,const char *);
   static void                    ReplaceMvPrognFieldVars(void *,SYMBOL_HN *,struct expr *,int);
#endif /* (! BLOAD_ONLY) && (! RUN_TIME) */
#endif /* MULTIFIELD_FUNCTIONS */
   static void                    MVRangeError(void *,long,long,long,const char *);
#endif /* MULTIFIELD_FUNCTIONS || OBJECT_SYSTEM */

/***************************************/
/* LOCAL INTERNAL VARIABLE DEFINITIONS */
/***************************************/

#if MULTIFIELD_FUNCTIONS

#define MULTIFUN_DATA 10

struct multiFunctionData
  { 
   FIELD_VAR_STACK *FieldVarStack;
  };

#define MultiFunctionData(theEnv) ((struct multiFunctionData *) GetEnvironmentData(theEnv,MULTIFUN_DATA))

/**********************************************/
/* MultifieldFunctionDefinitions: Initializes */
/*   the multifield functions.                */
/**********************************************/
void MultifieldFunctionDefinitions(
  void *theEnv)
  {
   AllocateEnvironmentData(theEnv,MULTIFUN_DATA,sizeof(struct multiFunctionData),NULL);

#if ! RUN_TIME
   EnvAddUDF(theEnv,"first$",          "m",  FirstFunction, "FirstFunction", 1,1,"m",NULL);
   EnvAddUDF(theEnv,"rest$",           "m",  RestFunction, "RestFunction",1,1,"m",NULL);
   EnvAddUDF(theEnv,"subseq$",         "m",  SubseqFunction, "SubseqFunction", 3,3,"l;m",NULL);
   EnvAddUDF(theEnv,"delete-member$",  "m",  DeleteMemberFunction, "DeleteMemberFunction", 2,UNBOUNDED,"*;m",NULL);
   EnvAddUDF(theEnv,"replace-member$", "m",  ReplaceMemberFunction, "x",3,UNBOUNDED,"*;m",NULL);
   EnvAddUDF(theEnv,"delete$",         "m",  DeleteFunction, "DeleteFunction", 3,3,"l;m",NULL);
   EnvAddUDF(theEnv,"replace$",        "m",  ReplaceFunction, "ReplaceFunction",4,UNBOUNDED,"*;m;l;l",NULL);
   EnvAddUDF(theEnv,"insert$",         "m",  InsertFunction, "InsertFunction", 3,UNBOUNDED,"*;m;l",NULL);
   EnvAddUDF(theEnv,"explode$",        "m", ExplodeFunction, "ExplodeFunction", 1,1,"s",NULL);
   EnvAddUDF(theEnv,"implode$",        "s", ImplodeFunction, "ImplodeFunction", 1,1,"m",NULL);
   EnvAddUDF(theEnv,"nth$",     "synldife", NthFunction, "NthFunction", 2,2,";l;m",NULL);
   EnvAddUDF(theEnv,"member$",       "blm",
                    MemberFunction, "MemberFunction", 2,2,";*;m",NULL);
   EnvAddUDF(theEnv,"member", "blm",
                    MemberFunction, "MemberFunction", 2,2, ";*;m",NULL);
   EnvAddUDF(theEnv,"subsetp",     "b",  SubsetpFunction, "SubsetpFunction", 2,2,";m;m",NULL);
   EnvAddUDF(theEnv,"progn$",      "*", MultifieldPrognFunction, "MultifieldPrognFunction",  0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"foreach",     "*", ForeachFunction, "ForeachFunction", 0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"str-implode", "s",  ImplodeFunction, "ImplodeFunction",1,1,"m",NULL);
   EnvAddUDF(theEnv,"str-explode", "m",  ExplodeFunction, "ExplodeFunction",1,1,"s",NULL);
   EnvAddUDF(theEnv,"subset",      "b",  SubsetpFunction, "SubsetpFunction", 2,2,";m;m",NULL);
   EnvAddUDF(theEnv,"nth", "synldife",  NthFunction, "NthFunction", 2,2,";l;m",NULL);
#if ! BLOAD_ONLY
   AddFunctionParser(theEnv,"progn$",MultifieldPrognParser);
   AddFunctionParser(theEnv,"foreach",ForeachParser);
#endif
   FuncSeqOvlFlags(theEnv,"progn$",false,false);
   FuncSeqOvlFlags(theEnv,"foreach",false,false);
   EnvAddUDF(theEnv,"(get-progn$-field)", "*",  GetMvPrognField, "GetMvPrognField", 0,0,NULL,NULL);
   EnvAddUDF(theEnv,"(get-progn$-index)", "l", GetMvPrognIndex, "GetMvPrognIndex", 0,0,NULL,NULL);
#endif
  }

/****************************************/
/* DeleteFunction: H/L access routine   */
/*   for the delete$ function.          */
/****************************************/
void DeleteFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue value1, value2, value3;
   Environment *theEnv = UDFContextEnvironment(context);

   /*=======================================*/
   /* Check for the correct argument types. */
   /*=======================================*/

   if ((! UDFFirstArgument(context,MULTIFIELD_TYPE,&value1)) ||
       (! UDFNextArgument(context,INTEGER_TYPE,&value2)) ||
       (! UDFNextArgument(context,INTEGER_TYPE,&value3)))
     { return; }

   /*=================================================*/
   /* Delete the section out of the multifield value. */
   /*=================================================*/

   if (DeleteMultiValueField(theEnv,returnValue,&value1,
            mCVToInteger(&value2),mCVToInteger(&value3),"delete$") == false)/* TBD */
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
     }
  }

/*****************************************/
/* ReplaceFunction: H/L access routine   */
/*   for the replace$ function.          */
/*****************************************/
void ReplaceFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue value1, value2, value3, value4;
   EXPRESSION *fieldarg;
   Environment *theEnv = UDFContextEnvironment(context);

   /*=======================================*/
   /* Check for the correct argument types. */
   /*=======================================*/

   if ((! UDFFirstArgument(context,MULTIFIELD_TYPE,&value1)) ||
       (! UDFNextArgument(context,INTEGER_TYPE,&value2)) ||
       (! UDFNextArgument(context,INTEGER_TYPE,&value3)))
     { return; }

   /*===============================*/
   /* Create the replacement value. */
   /*===============================*/

   fieldarg = GetFirstArgument()->nextArg->nextArg->nextArg;
   if (fieldarg->nextArg != NULL)
     { StoreInMultifield(theEnv,&value4,fieldarg,true); }
   else
     { EvaluateExpression(theEnv,fieldarg,&value4); }

   /*==============================================*/
   /* Replace the section in the multifield value. */
   /*==============================================*/

   if (ReplaceMultiValueField(theEnv,returnValue,&value1,(long) DOToLong(value2),
                   (long) DOToLong(value3),&value4,"replace$") == false) /* TBD */
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
     }
  }

/**********************************************/
/* DeleteMemberFunction: H/L access routine   */
/*   for the delete-member$ function.         */
/**********************************************/
void DeleteMemberFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT resultValue,*delVals,tmpVal;
   int i,argCnt;
   unsigned delSize;
   long j,k;
   Environment *theEnv = UDFContextEnvironment(context);

   /*============================================*/
   /* Check for the correct number of arguments. */
   /*============================================*/

   argCnt = EnvArgCountCheck(theEnv,"delete-member$",AT_LEAST,2);
   if (argCnt == -1)
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }

   /*=======================================*/
   /* Check for the correct argument types. */
   /*=======================================*/
   if (EnvArgTypeCheck(theEnv,"delete-member$",1,MULTIFIELD,&resultValue) == false)
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }

   /*=================================================
     For every value specified, delete all occurrences
     of those values from the multifield
     ================================================= */
   delSize = (sizeof(DATA_OBJECT) * (argCnt-1));
   delVals = (DATA_OBJECT_PTR) gm2(theEnv,delSize);
   for (i = 2 ; i <= argCnt ; i++)
     {
      if (!EnvRtnUnknown(theEnv,i,&delVals[i-2]))
        {
         rm(theEnv,(void *) delVals,delSize);
         EnvSetEvaluationError(theEnv,true);
         EnvSetMultifieldErrorValue(theEnv,returnValue);
         return;
        }
     }

   while (FindDOsInSegment(delVals,argCnt-1,&resultValue,&j,&k,NULL,0))
     {
      if (DeleteMultiValueField(theEnv,&tmpVal,&resultValue,
                                j,k,"delete-member$") == false)
        {
         rm(theEnv,(void *) delVals,delSize);
         EnvSetEvaluationError(theEnv,true);
         EnvSetMultifieldErrorValue(theEnv,returnValue);
         return;
        }
      GenCopyMemory(DATA_OBJECT,1,&resultValue,&tmpVal);
     }
   rm(theEnv,(void *) delVals,delSize);
   GenCopyMemory(DATA_OBJECT,1,returnValue,&resultValue);
  }

/***********************************************/
/* ReplaceMemberFunction: H/L access routine   */
/*   for the replace-member$ function.         */
/***********************************************/
void ReplaceMemberFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT resultValue,replVal,*delVals,tmpVal;
   int i,argCnt;
   unsigned delSize;
   long j,k,mink[2],*minkp;
   long replLen = 1L;
   Environment *theEnv = UDFContextEnvironment(context);

   /*============================================*/
   /* Check for the correct number of arguments. */
   /*============================================*/
   argCnt = EnvArgCountCheck(theEnv,"replace-member$",AT_LEAST,3);
   if (argCnt == -1)
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }

   /*=======================================*/
   /* Check for the correct argument types. */
   /*=======================================*/
   if (EnvArgTypeCheck(theEnv,"replace-member$",1,MULTIFIELD,&resultValue) == false)
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }

   if (!EnvRtnUnknown(theEnv,2,&replVal))
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }
   if (GetType(replVal) == MULTIFIELD)
     replLen = GetDOLength(replVal);

   /*=====================================================
     For the value (or values from multifield ) specified,
     replace all occurrences of those values with all
     values specified
     ===================================================== */
   delSize = (sizeof(DATA_OBJECT) * (argCnt-2));
   delVals = (DATA_OBJECT_PTR) gm2(theEnv,delSize);
   for (i = 3 ; i <= argCnt ; i++)
     {
      if (!EnvRtnUnknown(theEnv,i,&delVals[i-3]))
        {
         rm(theEnv,(void *) delVals,delSize);
         EnvSetEvaluationError(theEnv,true);
         EnvSetMultifieldErrorValue(theEnv,returnValue);
         return;
        }
     }
   minkp = NULL;
   while (FindDOsInSegment(delVals,argCnt-2,&resultValue,&j,&k,minkp,minkp ? 1 : 0))
     {
      if (ReplaceMultiValueField(theEnv,&tmpVal,&resultValue,j,k,
                                 &replVal,"replace-member$") == false)
        {
         rm(theEnv,(void *) delVals,delSize);
         EnvSetEvaluationError(theEnv,true);
         EnvSetMultifieldErrorValue(theEnv,returnValue);
         return;
        }
      GenCopyMemory(DATA_OBJECT,1,&resultValue,&tmpVal);
      mink[0] = 1L;
      mink[1] = j + replLen - 1L;
      minkp = mink;
     }
   rm(theEnv,(void *) delVals,delSize);
   GenCopyMemory(DATA_OBJECT,1,returnValue,&resultValue);
  }

/****************************************/
/* InsertFunction: H/L access routine   */
/*   for the insert$ function.          */
/****************************************/
void InsertFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT value1, value2, value3;
   EXPRESSION *fieldarg;
   Environment *theEnv = UDFContextEnvironment(context);

   /*=======================================*/
   /* Check for the correct argument types. */
   /*=======================================*/

   if ((! UDFFirstArgument(context,MULTIFIELD_TYPE,&value1)) ||
       (! UDFNextArgument(context,INTEGER_TYPE,&value2)))
     { return; }

   /*=============================*/
   /* Create the insertion value. */
   /*=============================*/

   fieldarg = GetFirstArgument()->nextArg->nextArg;
   if (fieldarg->nextArg != NULL)
     StoreInMultifield(theEnv,&value3,fieldarg,true);
   else
     EvaluateExpression(theEnv,fieldarg,&value3);

   /*===========================================*/
   /* Insert the value in the multifield value. */
   /*===========================================*/

   if (InsertMultiValueField(theEnv,returnValue,&value1,(long) DOToLong(value2), /* TBD */
                             &value3,"insert$") == false)
     {
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
     }
  }

/*****************************************/
/* ExplodeFunction: H/L access routine   */
/*   for the explode$ function.          */
/*****************************************/
void ExplodeFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT value;
   struct multifield *theMultifield;
   unsigned long end;
   Environment *theEnv = UDFContextEnvironment(context);

   /*=====================================*/
   /* Explode$ expects a single argument. */
   /*=====================================*/

   if (EnvArgCountCheck(theEnv,"explode$",EXACTLY,1) == -1)
     {
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }

   /*==================================*/
   /* The argument should be a string. */
   /*==================================*/

   if (EnvArgTypeCheck(theEnv,"explode$",1,STRING,&value) == false)
     {
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }

   /*=====================================*/
   /* Convert the string to a multifield. */
   /*=====================================*/

   theMultifield = StringToMultifield(theEnv,DOToString(value));
   if (theMultifield == NULL)
     {
      theMultifield = (struct multifield *) EnvCreateMultifield(theEnv,0L);
      end = 0;
     }
   else
     { end = GetMFLength(theMultifield); }

   /*========================*/
   /* Return the multifield. */
   /*========================*/

   SetpType(returnValue,MULTIFIELD);
   SetpDOBegin(returnValue,1);
   SetpDOEnd(returnValue,end);
   SetpValue(returnValue,(void *) theMultifield);
   return;
  }

/*****************************************/
/* ImplodeFunction: H/L access routine   */
/*   for the implode$ function.          */
/*****************************************/
void ImplodeFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;
   Environment *theEnv = UDFContextEnvironment(context);

   /*======================================*/
   /* The argument should be a multifield. */
   /*======================================*/

   if (! UDFFirstArgument(context,MULTIFIELD_TYPE,&theArg))
     { return; }

   /*====================*/
   /* Return the string. */
   /*====================*/

   CVSetCLIPSString(returnValue,ImplodeMultifield(theEnv,&theArg));
  }

/****************************************/
/* SubseqFunction: H/L access routine   */
/*   for the subseq$ function.          */
/****************************************/
void SubseqFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;
   struct multifield *theList;
   long long offset, start, end, length; /* 6.04 Bug Fix */
   Environment *theEnv = UDFContextEnvironment(context);

   /*===================================*/
   /* Get the segment to be subdivided. */
   /*===================================*/

   if (! UDFFirstArgument(context,MULTIFIELD_TYPE,&theArg))
     { return; }
     
   theList = (struct multifield *) DOToPointer(theArg);
   offset = GetDOBegin(theArg);
   length = GetDOLength(theArg);

   /*=============================================*/
   /* Get range arguments. If they are not within */
   /* appropriate ranges, return a null segment.  */
   /*=============================================*/

   if (! UDFNextArgument(context,INTEGER_TYPE,&theArg))
     { return; }
     
   start = DOToLong(theArg);

   if (! UDFNextArgument(context,INTEGER_TYPE,&theArg))
     { return; }
   end = DOToLong(theArg);

   if ((end < 1) || (end < start))
     {
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }

   /*===================================================*/
   /* Adjust lengths  to conform to segment boundaries. */
   /*===================================================*/

   if (start > length)
     {
      EnvSetMultifieldErrorValue(theEnv,returnValue);
      return;
     }
   if (end > length) end = length;
   if (start < 1) start = 1;

   /*=========================*/
   /* Return the new segment. */
   /*=========================*/

   SetpType(returnValue,MULTIFIELD);
   SetpValue(returnValue,theList);
   SetpDOEnd(returnValue,offset + end - 1);
   SetpDOBegin(returnValue,offset + start - 1);
  }

/***************************************/
/* FirstFunction: H/L access routine   */
/*   for the first$ function.          */
/***************************************/
void FirstFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;
   struct multifield *theList;

   /*===================================*/
   /* Get the segment to be subdivided. */
   /*===================================*/

   if (! UDFFirstArgument(context,MULTIFIELD_TYPE,&theArg))
     { return; }

   theList = (struct multifield *) DOToPointer(theArg);

   /*=========================*/
   /* Return the new segment. */
   /*=========================*/

   SetpType(returnValue,MULTIFIELD);
   SetpValue(returnValue,theList);
   if (GetDOEnd(theArg) >= GetDOBegin(theArg))
     { SetpDOEnd(returnValue,GetDOBegin(theArg)); }
   else
     { SetpDOEnd(returnValue,GetDOEnd(theArg)); }
   SetpDOBegin(returnValue,GetDOBegin(theArg));
  }

/**************************************/
/* RestFunction: H/L access routine   */
/*   for the rest$ function.          */
/**************************************/
void RestFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;
   struct multifield *theList;

   /*===================================*/
   /* Get the segment to be subdivided. */
   /*===================================*/

   if (! UDFFirstArgument(context,MULTIFIELD_TYPE,&theArg))
     { return; }

   theList = (struct multifield *) DOToPointer(theArg);

   /*=========================*/
   /* Return the new segment. */
   /*=========================*/

   SetpType(returnValue,MULTIFIELD);
   SetpValue(returnValue,theList);
   if (GetDOBegin(theArg) > GetDOEnd(theArg))
     { SetpDOBegin(returnValue,GetDOBegin(theArg)); }
   else
     { SetpDOBegin(returnValue,GetDOBegin(theArg) + 1); }
   SetpDOEnd(returnValue,GetDOEnd(theArg));
  }

/*************************************/
/* NthFunction: H/L access routine   */
/*   for the nth$ function.          */
/*************************************/
void NthFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue value1, value2;
   struct multifield *elm_ptr;
   long long n; /* 6.04 Bug Fix */

   if ((! UDFFirstArgument(context,INTEGER_TYPE,&value1)) ||
	   (! UDFNextArgument(context,MULTIFIELD_TYPE,&value2)))
     { return; }

   n = DOToLong(value1); /* 6.04 Bug Fix */
   if ((n > GetDOLength(value2)) || (n < 1))
	 {
      mCVSetSymbol(returnValue,"nil");
	  return;
	 }

   elm_ptr = (struct multifield *) GetValue(value2);
   SetpType(returnValue,GetMFType(elm_ptr,((long) n) + GetDOBegin(value2) - 1));
   SetpValue(returnValue,GetMFValue(elm_ptr,((long) n) + GetDOBegin(value2) - 1));
  }

/* ------------------------------------------------------------------
 *    SubsetFunction:
 *               This function compares two multi-field variables
 *               to see if the first is a subset of the second. It
 *               does not consider order.
 *
 *    INPUTS:    Two arguments via argument stack. First is the sublist
 *               multi-field variable, the second is the list to be
 *               compared to. Both should be of type MULTIFIELD.
 *
 *    OUTPUTS:   true if the first list is a subset of the
 *               second, else false
 *
 *    NOTES:     This function is called from H/L with the subset
 *               command. Repeated values in the sublist must also
 *               be repeated in the main list.
 * ------------------------------------------------------------------
 */

void SubsetpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT item1, item2, tmpItem;
   long i,j,k; 

   if (! UDFFirstArgument(context,MULTIFIELD_TYPE,&item1))
     { return; }

   if (! UDFNextArgument(context,MULTIFIELD_TYPE,&item2))
     { return; }
 
   if (MFLength(&item1) == 0)
     {
      mCVSetBoolean(returnValue,true);
      return;
     }
     
   if (MFLength(&item2) == 0)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   for (i = GetDOBegin(item1) ; i <= GetDOEnd(item1) ; i++)
     {
      SetType(tmpItem,GetMFType((struct multifield *) GetValue(item1),i));
      SetValue(tmpItem,GetMFValue((struct multifield *) GetValue(item1),i));


      if (! FindDOsInSegment(&tmpItem,1,&item2,&j,&k,NULL,0))
        {
         mCVSetBoolean(returnValue,false);
         return;
        }
     }

   mCVSetBoolean(returnValue,true);
  }

/****************************************/
/* MemberFunction: H/L access routine   */
/*   for the member$ function.          */
/****************************************/
void MemberFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT item1, item2;
   long j, k;
   Environment *theEnv = UDFContextEnvironment(context);

   mCVSetBoolean(returnValue,false);

   if (! UDFFirstArgument(context,ANY_TYPE,&item1)) return;
   
   if (! UDFNextArgument(context,MULTIFIELD_TYPE,&item2)) return;

   if (FindDOsInSegment(&item1,1,&item2,&j,&k,NULL,0))
     {
      if (j == k)
        {
         mCVSetInteger(returnValue,j);
        }
      else
        {
         returnValue->type = MULTIFIELD;
         returnValue->value = EnvCreateMultifield(theEnv,2);
         SetMFType(returnValue->value,1,INTEGER);
         SetMFValue(returnValue->value,1,EnvAddLong(theEnv,j));
         SetMFType(returnValue->value,2,INTEGER);
         SetMFValue(returnValue->value,2,EnvAddLong(theEnv,k));
         SetpDOBegin(returnValue,1);
         SetpDOEnd(returnValue,2);
        }
     }
  }

/*********************/
/* FindDOsInSegment: */
/*********************/
/* 6.05 Bug Fix */
bool FindDOsInSegment(
  DATA_OBJECT_PTR searchDOs,
  int scnt,
  DATA_OBJECT_PTR value,
  long *si,
  long *ei,
  long *excludes,
  int epaircnt)
  {
   long mul_length,slen,i,k; /* 6.04 Bug Fix */
   int j;

   mul_length = GetpDOLength(value);
   for (i = 0 ; i < mul_length ; i++)
     {
      for (j = 0 ; j < scnt ; j++)
        {
         if (GetType(searchDOs[j]) == MULTIFIELD)
           {
            slen = GetDOLength(searchDOs[j]);
            if (MVRangeCheck(i+1L,i+slen,excludes,epaircnt))
              {
               for (k = 0L ; (k < slen) && ((k + i) < mul_length) ; k++)
                 if ((GetMFType(GetValue(searchDOs[j]),k+GetDOBegin(searchDOs[j])) !=
                      GetMFType(GetpValue(value),k+i+GetpDOBegin(value))) ||
                     (GetMFValue(GetValue(searchDOs[j]),k+GetDOBegin(searchDOs[j])) !=
                      GetMFValue(GetpValue(value),k+i+GetpDOBegin(value))))
                   break;
               if (k >= slen)
                 {
                  *si = i + 1L;
                  *ei = i + slen;
                  return(true);
                 }
              }
           }
         else if ((GetValue(searchDOs[j]) == GetMFValue(GetpValue(value),i + GetpDOBegin(value))) &&
                  (GetType(searchDOs[j]) == GetMFType(GetpValue(value),i + GetpDOBegin(value))) &&
                  MVRangeCheck(i+1L,i+1L,excludes,epaircnt))
           {
            *si = *ei = i+1L;
            return(true);
           }
        }
     }

   return(false);
  }

/******************************************************/
/* MVRangeCheck:  */
/******************************************************/
static bool MVRangeCheck(
  long si,
  long ei,
  long *elist,
  int epaircnt)
{
  int i;

  if (!elist || !epaircnt)
    return(true);
  for (i = 0 ; i < epaircnt ; i++)
    if (((si >= elist[i*2]) && (si <= elist[i*2+1])) ||
        ((ei >= elist[i*2]) && (ei <= elist[i*2+1])))
    return(false);

  return(true);
}

#if (! BLOAD_ONLY) && (! RUN_TIME)

/******************************************************/
/* MultifieldPrognParser: Parses the progn$ function. */
/******************************************************/
static struct expr *MultifieldPrognParser(
  void *theEnv,
  struct expr *top,
  const char *infile)
  {
   struct BindInfo *oldBindList,*newBindList,*prev;
   struct token tkn;
   struct expr *tmp;
   SYMBOL_HN *fieldVar = NULL;

   SavePPBuffer(theEnv," ");
   GetToken(theEnv,infile,&tkn);

   /* ================================
      Simple form: progn$ <mf-exp> ...
      ================================ */
   if (tkn.type != LPAREN)
     {
      top->argList = ParseAtomOrExpression(theEnv,infile,&tkn);
      if (top->argList == NULL)
        {
         ReturnExpression(theEnv,top);
         return(NULL);
        }
     }
   else
     {
      GetToken(theEnv,infile,&tkn);
      if (tkn.type != SF_VARIABLE)
        {
         if (tkn.type != SYMBOL)
           goto MvPrognParseError;
         top->argList = Function2Parse(theEnv,infile,ValueToString(tkn.value));
         if (top->argList == NULL)
           {
            ReturnExpression(theEnv,top);
            return(NULL);
           }
        }

      /* =========================================
         Complex form: progn$ (<var> <mf-exp>) ...
         ========================================= */
      else
        {
         fieldVar = (SYMBOL_HN *) tkn.value;
         SavePPBuffer(theEnv," ");
         top->argList = ParseAtomOrExpression(theEnv,infile,NULL);
         if (top->argList == NULL)
           {
            ReturnExpression(theEnv,top);
            return(NULL);
           }
         GetToken(theEnv,infile,&tkn);
         if (tkn.type != RPAREN)
           goto MvPrognParseError;
         PPBackup(theEnv);
         /* PPBackup(theEnv); */
         SavePPBuffer(theEnv,tkn.printForm);
         SavePPBuffer(theEnv," ");
        }
     }

   if (CheckArgumentAgainstRestriction(theEnv,top->argList,(int) 'm'))
     goto MvPrognParseError;
   oldBindList = GetParsedBindNames(theEnv);
   SetParsedBindNames(theEnv,NULL);
   IncrementIndentDepth(theEnv,3);
   ExpressionData(theEnv)->BreakContext = true;
   ExpressionData(theEnv)->ReturnContext = ExpressionData(theEnv)->svContexts->rtn;
   PPCRAndIndent(theEnv);
   top->argList->nextArg = GroupActions(theEnv,infile,&tkn,true,NULL,false);
   DecrementIndentDepth(theEnv,3);
   PPBackup(theEnv);
   PPBackup(theEnv);
   SavePPBuffer(theEnv,tkn.printForm);
   if (top->argList->nextArg == NULL)
     {
      ClearParsedBindNames(theEnv);
      SetParsedBindNames(theEnv,oldBindList);
      ReturnExpression(theEnv,top);
      return(NULL);
     }
   tmp = top->argList->nextArg;
   top->argList->nextArg = tmp->argList;
   tmp->argList = NULL;
   ReturnExpression(theEnv,tmp);
   newBindList = GetParsedBindNames(theEnv);
   prev = NULL;
   while (newBindList != NULL)
     {
      if ((fieldVar == NULL) ? false :
          (strcmp(ValueToString(newBindList->name),ValueToString(fieldVar)) == 0))
        {
         ClearParsedBindNames(theEnv);
         SetParsedBindNames(theEnv,oldBindList);
         PrintErrorID(theEnv,"MULTIFUN",2,false);
         EnvPrintRouter(theEnv,WERROR,"Cannot rebind field variable in function progn$.\n");
         ReturnExpression(theEnv,top);
         return(NULL);
        }
      prev = newBindList;
      newBindList = newBindList->next;
     }
   if (prev == NULL)
     SetParsedBindNames(theEnv,oldBindList);
   else
     prev->next = oldBindList;
   if (fieldVar != NULL)
     ReplaceMvPrognFieldVars(theEnv,fieldVar,top->argList->nextArg,0);
   return(top);

MvPrognParseError:
   SyntaxErrorMessage(theEnv,"progn$");
   ReturnExpression(theEnv,top);
   return(NULL);
  }

/******************************************************/
/* ForeachParser: Parses the foreach function. */
/******************************************************/
static struct expr *ForeachParser(
  void *theEnv,
  struct expr *top,
  const char *infile)
  {
   struct BindInfo *oldBindList,*newBindList,*prev;
   struct token tkn;
   struct expr *tmp;
   SYMBOL_HN *fieldVar;

   SavePPBuffer(theEnv," ");
   GetToken(theEnv,infile,&tkn);

   if (tkn.type != SF_VARIABLE)
     { goto ForeachParseError; }

   fieldVar = (SYMBOL_HN *) tkn.value;
   SavePPBuffer(theEnv," ");
   top->argList = ParseAtomOrExpression(theEnv,infile,NULL);
   if (top->argList == NULL)
     {
      ReturnExpression(theEnv,top);
      return(NULL);
     }

   if (CheckArgumentAgainstRestriction(theEnv,top->argList,(int) 'm'))
     goto ForeachParseError;
   oldBindList = GetParsedBindNames(theEnv);
   SetParsedBindNames(theEnv,NULL);
   IncrementIndentDepth(theEnv,3);
   ExpressionData(theEnv)->BreakContext = true;
   ExpressionData(theEnv)->ReturnContext = ExpressionData(theEnv)->svContexts->rtn;
   PPCRAndIndent(theEnv);
   top->argList->nextArg = GroupActions(theEnv,infile,&tkn,true,NULL,false);
   DecrementIndentDepth(theEnv,3);
   PPBackup(theEnv);
   PPBackup(theEnv);
   SavePPBuffer(theEnv,tkn.printForm);
   if (top->argList->nextArg == NULL)
     {
      ClearParsedBindNames(theEnv);
      SetParsedBindNames(theEnv,oldBindList);
      ReturnExpression(theEnv,top);
      return(NULL);
     }
   tmp = top->argList->nextArg;
   top->argList->nextArg = tmp->argList;
   tmp->argList = NULL;
   ReturnExpression(theEnv,tmp);
   newBindList = GetParsedBindNames(theEnv);
   prev = NULL;
   while (newBindList != NULL)
     {
      if ((fieldVar == NULL) ? false :
          (strcmp(ValueToString(newBindList->name),ValueToString(fieldVar)) == 0))
        {
         ClearParsedBindNames(theEnv);
         SetParsedBindNames(theEnv,oldBindList);
         PrintErrorID(theEnv,"MULTIFUN",2,false);
         EnvPrintRouter(theEnv,WERROR,"Cannot rebind field variable in function foreach.\n");
         ReturnExpression(theEnv,top);
         return(NULL);
        }
      prev = newBindList;
      newBindList = newBindList->next;
     }
   if (prev == NULL)
     SetParsedBindNames(theEnv,oldBindList);
   else
     prev->next = oldBindList;
   if (fieldVar != NULL)
     ReplaceMvPrognFieldVars(theEnv,fieldVar,top->argList->nextArg,0);
   return(top);

ForeachParseError:
   SyntaxErrorMessage(theEnv,"foreach");
   ReturnExpression(theEnv,top);
   return(NULL);
  }
  
/**********************************************/
/* ReplaceMvPrognFieldVars: Replaces variable */
/*   references found in the progn$ function. */
/**********************************************/
static void ReplaceMvPrognFieldVars(
  void *theEnv,
  SYMBOL_HN *fieldVar,
  struct expr *theExp,
  int depth)
  {
   size_t flen;

   flen = strlen(ValueToString(fieldVar));
   while (theExp != NULL)
     {
      if ((theExp->type != SF_VARIABLE) ? false :
          (strncmp(ValueToString(theExp->value),ValueToString(fieldVar),
                   (STD_SIZE) flen) == 0))
        {
         if (ValueToString(theExp->value)[flen] == '\0')
           {
            theExp->type = FCALL;
            theExp->value = (void *) FindFunction(theEnv,"(get-progn$-field)");
            theExp->argList = GenConstant(theEnv,INTEGER,EnvAddLong(theEnv,(long long) depth));
           }
         else if (strcmp(ValueToString(theExp->value) + flen,"-index") == 0)
           {
            theExp->type = FCALL;
            theExp->value = (void *) FindFunction(theEnv,"(get-progn$-index)");
            theExp->argList = GenConstant(theEnv,INTEGER,EnvAddLong(theEnv,(long long) depth));
           }
        }
      else if (theExp->argList != NULL)
        {
         if ((theExp->type == FCALL) && ((theExp->value == (void *) FindFunction(theEnv,"progn$")) ||
                                        (theExp->value == (void *) FindFunction(theEnv,"foreach")) ))
           ReplaceMvPrognFieldVars(theEnv,fieldVar,theExp->argList,depth+1);
         else
           ReplaceMvPrognFieldVars(theEnv,fieldVar,theExp->argList,depth);
        }
      theExp = theExp->nextArg;
     }
  }

#endif /* (! BLOAD_ONLY) && (! RUN_TIME) */

/*****************************************/
/* MultifieldPrognFunction: H/L access   */
/*   routine for the progn$ function.    */
/*****************************************/
void MultifieldPrognFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   MultifieldPrognDriver(UDFContextEnvironment(context),returnValue,"progn$");
  }

/***************************************/
/* ForeachFunction: H/L access routine */
/*   for the foreach function.         */
/***************************************/
void ForeachFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   MultifieldPrognDriver(UDFContextEnvironment(context),returnValue,"foreach");
  }
    
/*******************************************/
/* MultifieldPrognDriver: Driver routine   */
/*   for the progn$ and foreach functions. */
/******************************************/
static void MultifieldPrognDriver(
  void *theEnv,
  DATA_OBJECT_PTR result,
  const char *functionName)
  {
   EXPRESSION *theExp;
   DATA_OBJECT argval;
   long i, end; /* 6.04 Bug Fix */
   FIELD_VAR_STACK *tmpField;
   struct CLIPSBlock gcBlock;
   
   tmpField = get_struct(theEnv,fieldVarStack);
   tmpField->type = SYMBOL;
   tmpField->value = EnvFalseSymbol(theEnv);
   tmpField->nxt = MultiFunctionData(theEnv)->FieldVarStack;
   MultiFunctionData(theEnv)->FieldVarStack = tmpField;
   result->type = SYMBOL;
   result->value = EnvFalseSymbol(theEnv);
   if (EnvArgTypeCheck(theEnv,functionName,1,MULTIFIELD,&argval) == false)
     {
      MultiFunctionData(theEnv)->FieldVarStack = tmpField->nxt;
      rtn_struct(theEnv,fieldVarStack,tmpField);
      return;
     }
     
   CLIPSBlockStart(theEnv,&gcBlock);

   end = GetDOEnd(argval);
   for (i = GetDOBegin(argval) ; i <= end ; i++)
     {
      tmpField->type = GetMFType(argval.value,i);
      tmpField->value = GetMFValue(argval.value,i);
      /* tmpField->index = i; */
      tmpField->index = (i - GetDOBegin(argval)) + 1; 
      for (theExp = GetFirstArgument()->nextArg ; theExp != NULL ; theExp = theExp->nextArg)
        {
         EvaluateExpression(theEnv,theExp,result);
        
         if (EvaluationData(theEnv)->HaltExecution || ProcedureFunctionData(theEnv)->BreakFlag || ProcedureFunctionData(theEnv)->ReturnFlag)
           {
            ProcedureFunctionData(theEnv)->BreakFlag = false;
            if (EvaluationData(theEnv)->HaltExecution)
              {
               result->type = SYMBOL;
               result->value = EnvFalseSymbol(theEnv);
              }
            MultiFunctionData(theEnv)->FieldVarStack = tmpField->nxt;
            rtn_struct(theEnv,fieldVarStack,tmpField);
            CLIPSBlockEnd(theEnv,&gcBlock,result);
            return;
           }

         /*===================================*/
         /* Garbage collect if this isn't the */
         /* last evaluation of the progn$.    */
         /*===================================*/
         
         if ((i < end) || (theExp->nextArg != NULL))
           {
            CleanCurrentGarbageFrame(theEnv,NULL);
            CallPeriodicTasks(theEnv);
           }
        }
     }
     
   ProcedureFunctionData(theEnv)->BreakFlag = false;
   MultiFunctionData(theEnv)->FieldVarStack = tmpField->nxt;
   rtn_struct(theEnv,fieldVarStack,tmpField);
   
   CLIPSBlockEnd(theEnv,&gcBlock,result);
   CallPeriodicTasks(theEnv);
  }

/*******************/
/* GetMvPrognField */
/*******************/
void GetMvPrognField(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   int depth;
   FIELD_VAR_STACK *tmpField;
   Environment *theEnv = UDFContextEnvironment(context);

   depth = ValueToInteger(GetFirstArgument()->value);
   tmpField = MultiFunctionData(theEnv)->FieldVarStack;
   while (depth > 0)
     {
      tmpField = tmpField->nxt;
      depth--;
     }
   returnValue->type = tmpField->type;
   returnValue->value = tmpField->value;
  }

/*******************/
/* GetMvPrognIndex */
/*******************/
void GetMvPrognIndex(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   int depth;
   FIELD_VAR_STACK *tmpField;
   Environment *theEnv = UDFContextEnvironment(context);

   depth = ValueToInteger(GetFirstArgument()->value);
   tmpField = MultiFunctionData(theEnv)->FieldVarStack;
   while (depth > 0)
     {
      tmpField = tmpField->nxt;
      depth--;
     }
   mCVSetInteger(returnValue,tmpField->index);
  }

#endif /* MULTIFIELD_FUNCTIONS */

#if OBJECT_SYSTEM || MULTIFIELD_FUNCTIONS

/**************************************************************************
  NAME         : ReplaceMultiValueField
  DESCRIPTION  : Performs a replace on the src multi-field value
                   storing the results in the dst multi-field value
  INPUTS       : 1) The destination value buffer
                 2) The source value (can be NULL)
                 3) Beginning of index range
                 4) End of range
                 5) The new field value
  RETURNS      : true if successful, false otherwise
  SIDE EFFECTS : Allocates and sets a ephemeral segment (even if new
                   number of fields is 0)
                 Src value segment is not changed
  NOTES        : index is NOT guaranteed to be valid
                 src is guaranteed to be a multi-field variable or NULL
 **************************************************************************/
bool ReplaceMultiValueField(
  void *theEnv,
  DATA_OBJECT *dst,
  DATA_OBJECT *src,
  long rb,
  long re,
  DATA_OBJECT *field,
  const char *funcName)
  {
   long i,j,k;
   struct field *deptr;
   struct field *septr;
   long srclen,dstlen;

   srclen = ((src != NULL) ? (src->end - src->begin + 1) : 0);
   if ((re < rb) ||
	   (rb < 1) || (re < 1) ||
	   (rb > srclen) || (re > srclen))
	 {
	  MVRangeError(theEnv,rb,re,srclen,funcName);
	  return(false);
	 }
   rb = src->begin + rb - 1;
   re = src->begin + re - 1;
   if (field->type == MULTIFIELD)
	 dstlen = srclen + GetpDOLength(field) - (re-rb+1);
   else
	 dstlen = srclen + 1 - (re-rb+1);
   dst->type = MULTIFIELD;
   dst->begin = 0;
   dst->value = EnvCreateMultifield(theEnv,dstlen);
   SetpDOEnd(dst,dstlen);
   for (i = 0 , j = src->begin ; j < rb ; i++ , j++)
	 {
	  deptr = &((struct multifield *) dst->value)->theFields[i];
	  septr = &((struct multifield *) src->value)->theFields[j];
	  deptr->type = septr->type;
	  deptr->value = septr->value;
	 }
   if (field->type != MULTIFIELD)
	 {
	  deptr = &((struct multifield *) dst->value)->theFields[i++];
	  deptr->type = field->type;
	  deptr->value = field->value;
	 }
   else
	 {
	  for (k = field->begin ; k <= field->end ; k++ , i++)
		{
		 deptr = &((struct multifield *) dst->value)->theFields[i];
		 septr = &((struct multifield *) field->value)->theFields[k];
		 deptr->type = septr->type;
		 deptr->value = septr->value;
		}
	 }
   while (j < re)
	 j++;
   for (j++ ; i < dstlen ; i++ , j++)
	 {
	  deptr = &((struct multifield *) dst->value)->theFields[i];
	  septr = &((struct multifield *) src->value)->theFields[j];
	  deptr->type = septr->type;
	  deptr->value = septr->value;
	 }
   return(true);
  }

/**************************************************************************
  NAME         : InsertMultiValueField
  DESCRIPTION  : Performs an insert on the src multi-field value
                   storing the results in the dst multi-field value
  INPUTS       : 1) The destination value buffer
                 2) The source value (can be NULL)
                 3) The index for the change
                 4) The new field value
  RETURNS      : true if successful, false otherwise
  SIDE EFFECTS : Allocates and sets a ephemeral segment (even if new
                   number of fields is 0)
                 Src value segment is not changed
  NOTES        : index is NOT guaranteed to be valid
                 src is guaranteed to be a multi-field variable or NULL
 **************************************************************************/
bool InsertMultiValueField(
  void *theEnv,
  DATA_OBJECT *dst,
  DATA_OBJECT *src,
  long theIndex,
  DATA_OBJECT *field,
  const char *funcName)
  {
   long i,j,k;
   register FIELD *deptr, *septr;
   long srclen,dstlen;

   srclen = (long) ((src != NULL) ? (src->end - src->begin + 1) : 0);
   if (theIndex < 1)
     {
      MVRangeError(theEnv,theIndex,theIndex,srclen+1,funcName);
      return(false);
     }
   if (theIndex > (srclen + 1))
     theIndex = (srclen + 1);
   dst->type = MULTIFIELD;
   dst->begin = 0;
   if (src == NULL)
     {
      if (field->type == MULTIFIELD)
        {
         DuplicateMultifield(theEnv,dst,field);
         AddToMultifieldList(theEnv,(struct multifield *) dst->value);
        }
      else
        {
         dst->value = EnvCreateMultifield(theEnv,0L);
         dst->end = 0;
         deptr = &((struct multifield *) dst->value)->theFields[0];
         deptr->type = field->type;
         deptr->value = field->value;
        }
      return(true);
     }
   dstlen = (field->type == MULTIFIELD) ? GetpDOLength(field) + srclen : srclen + 1;
   dst->value = EnvCreateMultifield(theEnv,dstlen);
   SetpDOEnd(dst,dstlen);
   theIndex--;
   for (i = 0 , j = src->begin ; i < theIndex ; i++ , j++)
     {
      deptr = &((struct multifield *) dst->value)->theFields[i];
      septr = &((struct multifield *) src->value)->theFields[j];
      deptr->type = septr->type;
      deptr->value = septr->value;
     }
   if (field->type != MULTIFIELD)
     {
      deptr = &((struct multifield *) dst->value)->theFields[theIndex];
      deptr->type = field->type;
      deptr->value = field->value;
      i++;
     }
   else
     {
      for (k = field->begin ; k <= field->end ; k++ , i++)
        {
         deptr = &((struct multifield *) dst->value)->theFields[i];
         septr = &((struct multifield *) field->value)->theFields[k];
         deptr->type = septr->type;
         deptr->value = septr->value;
        }
     }
   for ( ; j <= src->end ; i++ , j++)
     {
      deptr = &((struct multifield *) dst->value)->theFields[i];
      septr = &((struct multifield *) src->value)->theFields[j];
      deptr->type = septr->type;
      deptr->value = septr->value;
     }
   return(true);
  }

/*******************************************************
  NAME         : MVRangeError
  DESCRIPTION  : Prints out an error messages for index
                   out-of-range errors in multi-field
                   access functions
  INPUTS       : 1) The bad range start
                 2) The bad range end
                 3) The max end of the range (min is
                     assumed to be 1)
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : None
 ******************************************************/
static void MVRangeError(
  void *theEnv,
  long brb,
  long bre,
  long max,
  const char *funcName)
  {
   PrintErrorID(theEnv,"MULTIFUN",1,false);
   EnvPrintRouter(theEnv,WERROR,"Multifield index ");
   if (brb == bre)
     PrintLongInteger(theEnv,WERROR,(long long) brb);
   else
     {
      EnvPrintRouter(theEnv,WERROR,"range ");
      PrintLongInteger(theEnv,WERROR,(long long) brb);
      EnvPrintRouter(theEnv,WERROR,"..");
      PrintLongInteger(theEnv,WERROR,(long long) bre);
     }
   EnvPrintRouter(theEnv,WERROR," out of range 1..");
   PrintLongInteger(theEnv,WERROR,(long long) max);
   if (funcName != NULL)
     {
      EnvPrintRouter(theEnv,WERROR," in function ");
      EnvPrintRouter(theEnv,WERROR,funcName);
     }
   EnvPrintRouter(theEnv,WERROR,".\n");
  }

/**************************************************************************
  NAME         : DeleteMultiValueField
  DESCRIPTION  : Performs a modify on the src multi-field value
                   storing the results in the dst multi-field value
  INPUTS       : 1) The destination value buffer
                 2) The source value (can be NULL)
                 3) The beginning index for deletion
                 4) The ending index for deletion
  RETURNS      : true if successful, false otherwise
  SIDE EFFECTS : Allocates and sets a ephemeral segment (even if new
                   number of fields is 0)
                 Src value segment is not changed
  NOTES        : index is NOT guaranteed to be valid
                 src is guaranteed to be a multi-field variable or NULL
 **************************************************************************/
bool DeleteMultiValueField(
  void *theEnv,
  DATA_OBJECT *dst,
  DATA_OBJECT *src,
  long rb,
  long re,
  const char *funcName)
  {
   long i,j;
   register FIELD_PTR deptr,septr;
   long srclen, dstlen;

   srclen = (long) ((src != NULL) ? (src->end - src->begin + 1) : 0);
   if ((re < rb) ||
       (rb < 1) || (re < 1) ||
       (rb > srclen) || (re > srclen))
     {
      MVRangeError(theEnv,rb,re,srclen,funcName);
      return(false);
     }
   dst->type = MULTIFIELD;
   dst->begin = 0;
   if (srclen == 0)
    {
     dst->value = EnvCreateMultifield(theEnv,0L);
     dst->end = -1;
     return(true);
    }
   rb = src->begin + rb -1;
   re = src->begin + re -1;
   dstlen = srclen-(re-rb+1);
   SetpDOEnd(dst,dstlen);
   dst->value = EnvCreateMultifield(theEnv,dstlen);
   for (i = 0 , j = src->begin ; j < rb ; i++ , j++)
     {
      deptr = &((struct multifield *) dst->value)->theFields[i];
      septr = &((struct multifield *) src->value)->theFields[j];
      deptr->type = septr->type;
      deptr->value = septr->value;
     }
   while (j < re)
     j++;
   for (j++ ; i <= dst->end ; j++ , i++)
     {
      deptr = &((struct multifield *) dst->value)->theFields[i];
      septr = &((struct multifield *) src->value)->theFields[j];
      deptr->type = septr->type;
      deptr->value = septr->value;
     }
   return(true);
  }

#endif /* OBJECT_SYSTEM || MULTIFIELD_FUNCTIONS */
