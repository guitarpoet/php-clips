   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/20/16             */
   /*                                                     */
   /*                RULE COMMANDS MODULE                 */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides the matches command. Also provides the  */
/*   the developer commands show-joins and rule-complexity.  */
/*   Also provides the initialization routine which          */
/*   registers rule commands found in other modules.         */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Removed CONFLICT_RESOLUTION_STRATEGIES         */
/*            INCREMENTAL_RESET, and LOGICAL_DEPENDENCIES    */
/*            compilation flags.                             */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Added support for hashed memories.             */
/*                                                           */
/*            Improvements to matches command.               */
/*                                                           */
/*            Add join-activity and join-activity-reset      */
/*            commands.                                      */
/*                                                           */
/*            Added get-beta-memory-resizing and             */
/*            set-beta-memory-resizing functions.            */
/*                                                           */
/*            Added timetag function.                        */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*      6.40: Fixes for show-joins command.                  */
/*                                                           */
/*            Fixes for matches command where the            */
/*            activations listed were not correct if the     */
/*            current module was different than the module   */
/*            for the specified rule.                        */
/*                                                           */
/*            Added Env prefix to GetHaltExecution and       */
/*            SetHaltExecution functions.                    */
/*                                                           */
/*            Incremental reset is always enabled.           */
/*                                                           */
/*************************************************************/

#include <stdio.h>
#include <string.h>

#include "setup.h"

#if DEFRULE_CONSTRUCT

#include "argacces.h"
#include "constant.h"
#include "constrct.h"
#include "crstrtgy.h"
#include "engine.h"
#include "envrnmnt.h"
#include "evaluatn.h"
#include "extnfunc.h"
#include "incrrset.h"
#include "lgcldpnd.h"
#include "memalloc.h"
#include "multifld.h"
#include "pattern.h"
#include "reteutil.h"
#include "router.h"
#include "ruledlt.h"
#include "sysdep.h"
#include "watch.h"

#if BLOAD || BLOAD_AND_BSAVE || BLOAD_ONLY
#include "rulebin.h"
#endif

#include "rulecom.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

#if DEVELOPER
   static void                    ShowJoins(void *,void *);
#endif
#if DEBUGGING_FUNCTIONS
   static long long               ListAlphaMatches(void *,struct joinInformation *,int);
   static long long               ListBetaMatches(void *,struct joinInformation *,long,long,int);
   static void                    ListBetaJoinActivity(void *,struct joinInformation *,long,long,int,DATA_OBJECT *);
   static long                    AlphaJoinCountDriver(void *,struct joinNode *);
   static long                    BetaJoinCountDriver(void *,struct joinNode *);
   static void                    AlphaJoinsDriver(void *,struct joinNode *,long,struct joinInformation *);
   static void                    BetaJoinsDriver(void *,struct joinNode *,long,struct joinInformation *,struct betaMemory *,struct joinNode *);
   static int                     CountPatterns(void *,struct joinNode *,bool);
   static const char             *BetaHeaderString(void *,struct joinInformation *,long,long);
   static const char             *ActivityHeaderString(void *,struct joinInformation *,long,long);
   static void                    JoinActivityReset(void *,struct constructHeader *,void *);
#endif

/****************************************************************/
/* DefruleCommands: Initializes defrule commands and functions. */
/****************************************************************/
void DefruleCommands(
  void *theEnv)
  {
#if ! RUN_TIME
   EnvAddUDF(theEnv,"run","v", RunCommand,"RunCommand", 0,1,"l",NULL);
   EnvAddUDF(theEnv,"halt","v", HaltCommand,"HaltCommand",0,0,NULL,NULL);
   EnvAddUDF(theEnv,"focus","b",  FocusCommand,"FocusCommand",1,UNBOUNDED,"y", NULL);
   EnvAddUDF(theEnv,"clear-focus-stack","v", ClearFocusStackCommand,
                                       "ClearFocusStackCommand",0,0,NULL,NULL);
   EnvAddUDF(theEnv,"get-focus-stack","m", GetFocusStackFunction,
                                     "GetFocusStackFunction",0,0,NULL,NULL);
   EnvAddUDF(theEnv,"pop-focus","y", PopFocusFunction,
                               "PopFocusFunction",0,0,NULL,NULL);
   EnvAddUDF(theEnv,"get-focus","y", GetFocusFunction,
                               "GetFocusFunction",0,0,NULL,NULL);
#if DEBUGGING_FUNCTIONS
   EnvAddUDF(theEnv,"set-break","v", SetBreakCommand,
                               "SetBreakCommand",1,1,"y",NULL);
   EnvAddUDF(theEnv,"remove-break","v", RemoveBreakCommand,
                                  "RemoveBreakCommand", 0,1,"y",NULL);
   EnvAddUDF(theEnv,"show-breaks","v", ShowBreaksCommand,
                                 "ShowBreaksCommand", 0,1,"y",NULL);
   EnvAddUDF(theEnv,"matches","bm", MatchesCommand,"MatchesCommand",1,2,"y",NULL);
   EnvAddUDF(theEnv,"join-activity","bm", JoinActivityCommand,"JoinActivityCommand",1,2,"y",NULL);
   EnvAddUDF(theEnv,"join-activity-reset","v",  JoinActivityResetCommand,
                                  "JoinActivityResetCommand", 0,0,NULL,NULL);
   EnvAddUDF(theEnv,"list-focus-stack","v", ListFocusStackCommand,
                                      "ListFocusStackCommand", 0,0,NULL,NULL);
   EnvAddUDF(theEnv,"dependencies", "v", DependenciesCommand,
                                   "DependenciesCommand", 1,1,"infly",NULL);
   EnvAddUDF(theEnv,"dependents",  "v",  DependentsCommand,
                                   "DependentsCommand", 1,1,"infly",NULL);
      
   EnvAddUDF(theEnv,"timetag","l",TimetagFunction,
                                   "TimetagFunction", 1,1,"infly" ,NULL);
#endif /* DEBUGGING_FUNCTIONS */

   EnvAddUDF(theEnv,"get-beta-memory-resizing","b",
                    GetBetaMemoryResizingCommand,"GetBetaMemoryResizingCommand",0,0,NULL,NULL);
   EnvAddUDF(theEnv,"set-beta-memory-resizing","b",
                    SetBetaMemoryResizingCommand,"SetBetaMemoryResizingCommand",1,1,NULL,NULL);

   EnvAddUDF(theEnv,"get-strategy", "y", GetStrategyCommand,  "GetStrategyCommand", 0,0,NULL,NULL);
   EnvAddUDF(theEnv,"set-strategy", "y", SetStrategyCommand,  "SetStrategyCommand", 1,1,"y",NULL);

#if DEVELOPER && (! BLOAD_ONLY)
   EnvAddUDF(theEnv,"rule-complexity","l", RuleComplexityCommand,"RuleComplexityCommand", 1,1,"y",NULL);
   EnvAddUDF(theEnv,"show-joins",  "v", ShowJoinsCommand,    "ShowJoinsCommand", 1,1,"y",NULL);
   EnvAddUDF(theEnv,"show-aht",   "v", ShowAlphaHashTable,    "ShowAlphaHashTable", 0,0,NULL,NULL);
#if DEBUGGING_FUNCTIONS
   AddWatchItem(theEnv,"rule-analysis",0,&DefruleData(theEnv)->WatchRuleAnalysis,0,NULL,NULL);
#endif
#endif /* DEVELOPER && (! BLOAD_ONLY) */

#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif /* ! RUN_TIME */
  }

/***********************************************/
/* EnvGetBetaMemoryResizing: C access routine  */
/*   for the get-beta-memory-resizing command. */
/***********************************************/
bool EnvGetBetaMemoryResizing(
  void *theEnv)
  {   
   return(DefruleData(theEnv)->BetaMemoryResizingFlag);
  }

/***********************************************/
/* EnvSetBetaMemoryResizing: C access routine  */
/*   for the set-beta-memory-resizing command. */
/***********************************************/
bool EnvSetBetaMemoryResizing(
  void *theEnv,
  bool value)
  {
   bool ov;

   ov = DefruleData(theEnv)->BetaMemoryResizingFlag;

   DefruleData(theEnv)->BetaMemoryResizingFlag = value;

   return(ov);
  }

/****************************************************/
/* SetBetaMemoryResizingCommand: H/L access routine */
/*   for the set-beta-memory-resizing command.      */
/****************************************************/
void SetBetaMemoryResizingCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;
   Environment *theEnv = UDFContextEnvironment(context);

   mCVSetBoolean(returnValue,EnvGetBetaMemoryResizing(theEnv));

   /*=================================================*/
   /* The symbol FALSE disables beta memory resizing. */
   /* Any other value enables beta memory resizing.   */
   /*=================================================*/

   if (! UDFFirstArgument(context,ANY_TYPE,&theArg))
     { return; }
    
   if (CVIsFalseSymbol(&theArg))
     { EnvSetBetaMemoryResizing(theEnv,false); }
   else
     { EnvSetBetaMemoryResizing(theEnv,true); }
  }

/****************************************************/
/* GetBetaMemoryResizingCommand: H/L access routine */
/*   for the get-beta-memory-resizing command.      */
/****************************************************/
void GetBetaMemoryResizingCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   mCVSetBoolean(returnValue,EnvGetBetaMemoryResizing(UDFContextEnvironment(context)));
  }

#if DEBUGGING_FUNCTIONS

/****************************************/
/* MatchesCommand: H/L access routine   */
/*   for the matches command.           */
/****************************************/
void MatchesCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *ruleName, *argument;
   void *rulePtr;
   CLIPSValue theArg;
   int output;
   Environment *theEnv = UDFContextEnvironment(context);

   if (! UDFFirstArgument(context,SYMBOL_TYPE,&theArg))
     { return; }
     
   ruleName = mCVToString(&theArg);

   rulePtr = EnvFindDefrule(theEnv,ruleName);
   if (rulePtr == NULL)
     {
      CantFindItemErrorMessage(theEnv,"defrule",ruleName);
      mCVSetBoolean(returnValue,false);
      return;
     }

   if (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,SYMBOL_TYPE,&theArg))
        { return; }

      argument = mCVToString(&theArg);
      if (strcmp(argument,"verbose") == 0)
        { output = VERBOSE; }
      else if (strcmp(argument,"succinct") == 0)
        { output = SUCCINCT; }
      else if (strcmp(argument,"terse") == 0)
        { output = TERSE; }
      else
        {
         UDFInvalidArgumentMessage(context,"symbol with value verbose, succinct, or terse");
         mCVSetBoolean(returnValue,false);
         return;
        }
     }
   else
     { output = VERBOSE; }

   EnvMatches(theEnv,rulePtr,output,returnValue);
  }

/********************************/
/* EnvMatches: C access routine */
/*   for the matches command.   */
/********************************/
void EnvMatches(
  void *theEnv,
  void *theRule,
  int output,
  DATA_OBJECT *result)
  {
   struct defrule *rulePtr;
   struct defrule *topDisjunct = (struct defrule *) theRule;
   long joinIndex;
   long arraySize;
   struct joinInformation *theInfo;
   long long alphaMatchCount = 0;
   long long betaMatchCount = 0;
   long long activations = 0;
   ACTIVATION *agendaPtr;

   /*==========================*/
   /* Set up the return value. */
   /*==========================*/
   
   result->type = MULTIFIELD;
   result->begin = 0;
   result->end = 2;
   result->value = EnvCreateMultifield(theEnv,3L);
   
   SetMFType(result->value,1,INTEGER);
   SetMFValue(result->value,1,SymbolData(theEnv)->Zero);
   SetMFType(result->value,2,INTEGER);
   SetMFValue(result->value,2,SymbolData(theEnv)->Zero);
   SetMFType(result->value,3,INTEGER);
   SetMFValue(result->value,3,SymbolData(theEnv)->Zero);

   /*=================================================*/
   /* Loop through each of the disjuncts for the rule */
   /*=================================================*/

   for (rulePtr = topDisjunct; rulePtr != NULL; rulePtr = rulePtr->disjunct)
     {
      /*===============================================*/
      /* Create the array containing the list of alpha */
      /* join nodes (those connected to a pattern CE). */
      /*===============================================*/
      
      arraySize = EnvAlphaJoinCount(theEnv,rulePtr);
      
      theInfo = EnvCreateJoinArray(theEnv,arraySize);
      
      EnvAlphaJoins(theEnv,rulePtr,arraySize,theInfo);
       
      /*=========================*/
      /* List the alpha matches. */
      /*=========================*/
      
      for (joinIndex = 0; joinIndex < arraySize; joinIndex++)
        {
         alphaMatchCount += ListAlphaMatches(theEnv,&theInfo[joinIndex],output);
         
         SetMFType(result->value,1,INTEGER);
         SetMFValue(result->value,1,EnvAddLong(theEnv,alphaMatchCount));
        }

      /*================================*/
      /* Free the array of alpha joins. */
      /*================================*/
      
      EnvFreeJoinArray(theEnv,theInfo,arraySize);

      /*==============================================*/
      /* Create the array containing the list of beta */
      /* join nodes (joins from the right plus joins  */
      /* connected to a pattern CE).                  */
      /*==============================================*/
      
      arraySize = EnvBetaJoinCount(theEnv,rulePtr);
      
      theInfo = EnvCreateJoinArray(theEnv,arraySize);
      
      EnvBetaJoins(theEnv,rulePtr,arraySize,theInfo);

      /*======================================*/
      /* List the beta matches (for all joins */
      /* except the first pattern CE).        */
      /*======================================*/

      for (joinIndex = 1; joinIndex < arraySize; joinIndex++)
        {
         betaMatchCount += ListBetaMatches(theEnv,theInfo,joinIndex,arraySize,output);
         
         SetMFType(result->value,2,INTEGER);
         SetMFValue(result->value,2,EnvAddLong(theEnv,betaMatchCount));
        }

      /*================================*/
      /* Free the array of alpha joins. */
      /*================================*/
      
      EnvFreeJoinArray(theEnv,theInfo,arraySize);
     }

   /*===================*/
   /* List activations. */
   /*===================*/

   if (output == VERBOSE)
     { EnvPrintRouter(theEnv,WDISPLAY,"Activations\n"); }
     
   for (agendaPtr = ((struct defruleModule *) topDisjunct->header.whichModule)->agenda;
        agendaPtr != NULL;
        agendaPtr = (struct activation *) EnvGetNextActivation(theEnv,agendaPtr))
     {
      if (EnvGetHaltExecution(theEnv) == true) return;

      if (((struct activation *) agendaPtr)->theRule->header.name == topDisjunct->header.name)
        {
         activations++;
      
         if (output == VERBOSE)
           {
            PrintPartialMatch(theEnv,WDISPLAY,EnvGetActivationBasis(theEnv,agendaPtr));
            EnvPrintRouter(theEnv,WDISPLAY,"\n");
           }
        }
     }

   if (output == SUCCINCT)
     {
      EnvPrintRouter(theEnv,WDISPLAY,"Activations: ");
      PrintLongInteger(theEnv,WDISPLAY,activations);
      EnvPrintRouter(theEnv,WDISPLAY,"\n");
     }
     
   if ((activations == 0) && (output == VERBOSE)) EnvPrintRouter(theEnv,WDISPLAY," None\n");

   SetMFType(result->value,3,INTEGER);
   SetMFValue(result->value,3,EnvAddLong(theEnv,activations));
  }

/****************************************************/
/* AlphaJoinCountDriver: Driver routine to iterate  */
/*   over a rule's joins to determine the number of */
/*   alpha joins.                                   */
/****************************************************/
static long AlphaJoinCountDriver(
  void *theEnv,
  struct joinNode *theJoin)
  {
   long alphaCount = 0;

   if (theJoin == NULL) 
     { return(alphaCount); }
   
   if (theJoin->joinFromTheRight)
     { return AlphaJoinCountDriver(theEnv,(struct joinNode *) theJoin->rightSideEntryStructure); }
   else if (theJoin->lastLevel != NULL)
     { alphaCount += AlphaJoinCountDriver(theEnv,theJoin->lastLevel); }
     
   alphaCount++;
   
   return(alphaCount);
  }

/**************************************************/
/* EnvAlphaJoinCount: Returns the number of alpha */
/*   joins associated with the specified rule.    */
/**************************************************/
long EnvAlphaJoinCount(
  void *theEnv,
  void *vTheDefrule)
  {
   struct defrule *theDefrule = (struct defrule *) vTheDefrule;
   
   return AlphaJoinCountDriver(theEnv,theDefrule->lastJoin->lastLevel);
  }

/***************************************/
/* AlphaJoinsDriver: Driver routine to */
/*   retrieve a rule's alpha joins.    */
/***************************************/
static void AlphaJoinsDriver(
  void *theEnv,
  struct joinNode *theJoin,
  long alphaIndex,
  struct joinInformation *theInfo)
  {
   if (theJoin == NULL)
     { return; }
   
   if (theJoin->joinFromTheRight)
     {
      AlphaJoinsDriver(theEnv,(struct joinNode *) theJoin->rightSideEntryStructure,alphaIndex,theInfo);
      return;
     }
   else if (theJoin->lastLevel != NULL)
     { AlphaJoinsDriver(theEnv,theJoin->lastLevel,alphaIndex-1,theInfo); }
     
   theInfo[alphaIndex-1].whichCE = alphaIndex;
   theInfo[alphaIndex-1].theJoin = theJoin;
   
   return;
  }

/********************************************/
/* EnvAlphaJoins: Retrieves the alpha joins */
/*   associated with the specified rule.    */
/********************************************/
void EnvAlphaJoins(
  void *theEnv,
  void *vTheDefrule,
  long alphaCount,
  struct joinInformation *theInfo)
  {
   struct defrule *theDefrule = (struct defrule *) vTheDefrule;
   
   AlphaJoinsDriver(theEnv,theDefrule->lastJoin->lastLevel,alphaCount,theInfo);
  }

/****************************************************/
/* BetaJoinCountDriver: Driver routine to iterate  */
/*   over a rule's joins to determine the number of */
/*   beta joins.                                   */
/****************************************************/
static long BetaJoinCountDriver(
  void *theEnv,
  struct joinNode *theJoin)
  {
   long betaCount = 0;

   if (theJoin == NULL)
     { return(betaCount); }
   
   betaCount++;
   
   if (theJoin->joinFromTheRight)
     { betaCount += BetaJoinCountDriver(theEnv,(struct joinNode *) theJoin->rightSideEntryStructure); }
   else if (theJoin->lastLevel != NULL)
     { betaCount += BetaJoinCountDriver(theEnv,theJoin->lastLevel); }
     
   return(betaCount);
  }

/************************************************/
/* EnvBetaJoinCount: Returns the number of beta */
/*   joins associated with the specified rule.  */
/************************************************/
long EnvBetaJoinCount(
  void *theEnv,
  void *vTheDefrule)
  {
   struct defrule *theDefrule = (struct defrule *) vTheDefrule;
   
   return BetaJoinCountDriver(theEnv,theDefrule->lastJoin->lastLevel);
  }

/**************************************/
/* BetaJoinsDriver: Driver routine to */
/*   retrieve a rule's beta joins.    */
/**************************************/
static void BetaJoinsDriver(
  void *theEnv,
  struct joinNode *theJoin,
  long betaIndex,
  struct joinInformation *theJoinInfoArray,
  struct betaMemory *lastMemory,
  struct joinNode *nextJoin)
  {
   int theCE = 0, theCount;
   struct joinNode *tmpPtr;
   
   if (theJoin == NULL)
     { return; }

   theJoinInfoArray[betaIndex-1].theJoin = theJoin;
   theJoinInfoArray[betaIndex-1].theMemory = lastMemory;
   theJoinInfoArray[betaIndex-1].nextJoin = nextJoin;

   /*===================================*/
   /* Determine the conditional element */
   /* index for this join.              */
   /*===================================*/
     
   for (tmpPtr = theJoin; tmpPtr != NULL; tmpPtr = tmpPtr->lastLevel)
      { theCE++; }
     
   theJoinInfoArray[betaIndex-1].whichCE = theCE;

   /*==============================================*/
   /* The end pattern in the range of patterns for */
   /* this join is always the number of patterns   */
   /* remaining to be encountered.                 */
   /*==============================================*/

   theCount = CountPatterns(theEnv,theJoin,true);
   theJoinInfoArray[betaIndex-1].patternEnd = theCount;

   /*========================================================*/
   /* Determine where the block of patterns for a CE begins. */
   /*========================================================*/


   theCount = CountPatterns(theEnv,theJoin,false);
   theJoinInfoArray[betaIndex-1].patternBegin = theCount;
   
   /*==========================*/
   /* Find the next beta join. */
   /*==========================*/
   
   if (theJoin->joinFromTheRight)
     {
      BetaJoinsDriver(theEnv,(struct joinNode *) theJoin->rightSideEntryStructure,betaIndex-1,theJoinInfoArray,theJoin->rightMemory,theJoin);
     }
   else if (theJoin->lastLevel != NULL)
     {
      BetaJoinsDriver(theEnv,theJoin->lastLevel,betaIndex-1,theJoinInfoArray,theJoin->leftMemory,theJoin);
     }
     
   return;
  }

/******************************************/
/* EnvBetaJoins: Retrieves the beta joins */
/*   associated with the specified rule.  */
/******************************************/
void EnvBetaJoins(
  void *theEnv,
  void *vTheDefrule,
  long betaArraySize,
  struct joinInformation *theInfo)
  {
   struct defrule *theDefrule = (struct defrule *) vTheDefrule;
      
   BetaJoinsDriver(theEnv,theDefrule->lastJoin->lastLevel,betaArraySize,theInfo,theDefrule->lastJoin->leftMemory,theDefrule->lastJoin);
  }

/**************************************************/
/* EnvCreateJoinArray: Creates a join information */
/*    array of the specified size.                */
/**************************************************/
struct joinInformation *EnvCreateJoinArray(
   void *theEnv,
   long size)
   {
    if (size == 0) return (NULL);
    
    return (struct joinInformation *) genalloc(theEnv,sizeof(struct joinInformation) * size);
   }

/**********************************************/
/* EnvFreeJoinArray: Frees a join information */
/*    array of the specified size.            */
/**********************************************/
void EnvFreeJoinArray(
   void *theEnv,
   struct joinInformation *theArray,
   long size)
   {
    if (size == 0) return;
    
    genfree(theEnv,theArray,sizeof(struct joinInformation) * size);
   }

/*********************/
/* ListAlphaMatches: */
/*********************/
static long long ListAlphaMatches(
  void *theEnv,
  struct joinInformation *theInfo,
  int output)
  {
   struct alphaMemoryHash *listOfHashNodes;
   struct partialMatch *listOfMatches;
   long long count;
   struct joinNode *theJoin;
   long long alphaCount = 0;

   if (EnvGetHaltExecution(theEnv) == true)
     { return(alphaCount); }

   theJoin = theInfo->theJoin;
   
   if (output == VERBOSE)
     {
      EnvPrintRouter(theEnv,WDISPLAY,"Matches for Pattern ");
      PrintLongInteger(theEnv,WDISPLAY,theInfo->whichCE);
      EnvPrintRouter(theEnv,WDISPLAY,"\n");
     }
     
   if (theJoin->rightSideEntryStructure == NULL)
     {
      if (theJoin->rightMemory->beta[0]->children != NULL)
        { alphaCount += 1; }
        
      if (output == VERBOSE)
        {
         if (theJoin->rightMemory->beta[0]->children != NULL)
           { EnvPrintRouter(theEnv,WDISPLAY,"*\n"); }
         else
           { EnvPrintRouter(theEnv,WDISPLAY," None\n"); }
        }
      else if (output == SUCCINCT)
        {
         EnvPrintRouter(theEnv,WDISPLAY,"Pattern ");
         PrintLongInteger(theEnv,WDISPLAY,theInfo->whichCE);
         EnvPrintRouter(theEnv,WDISPLAY,": ");

         if (theJoin->rightMemory->beta[0]->children != NULL)
           { EnvPrintRouter(theEnv,WDISPLAY,"1"); }
         else
           { EnvPrintRouter(theEnv,WDISPLAY,"0"); }
         EnvPrintRouter(theEnv,WDISPLAY,"\n");
        }
        
      return(alphaCount);
     }

   listOfHashNodes =  ((struct patternNodeHeader *) theJoin->rightSideEntryStructure)->firstHash;

   for (count = 0;
        listOfHashNodes != NULL;
        listOfHashNodes = listOfHashNodes->nextHash)
     {
      listOfMatches = listOfHashNodes->alphaMemory;

      while (listOfMatches != NULL)
        {
         if (EnvGetHaltExecution(theEnv) == true)
           { return(alphaCount); }
                 
         count++;
         if (output == VERBOSE)
           {
            PrintPartialMatch(theEnv,WDISPLAY,listOfMatches);
            EnvPrintRouter(theEnv,WDISPLAY,"\n");
           }
         listOfMatches = listOfMatches->nextInMemory;
        }
     }
      
   alphaCount += count;
   
   if ((count == 0) && (output == VERBOSE)) EnvPrintRouter(theEnv,WDISPLAY," None\n");
   
   if (output == SUCCINCT)
     {
      EnvPrintRouter(theEnv,WDISPLAY,"Pattern ");
      PrintLongInteger(theEnv,WDISPLAY,theInfo->whichCE);
      EnvPrintRouter(theEnv,WDISPLAY,": ");
      PrintLongInteger(theEnv,WDISPLAY,count);
      EnvPrintRouter(theEnv,WDISPLAY,"\n");
     }
   
   return(alphaCount);
  }

/********************/
/* BetaHeaderString */
/********************/
static const char *BetaHeaderString(
  void *theEnv,
  struct joinInformation *infoArray,
  long joinIndex,
  long arraySize)
  {
   struct joinNode *theJoin;
   struct joinInformation *theInfo;
   long i, j, startPosition, endPosition, positionsToPrint = 0;
   bool nestedCEs = false;
   const char *returnString = "";
   long lastIndex;
   char buffer[32];
   
   /*=============================================*/
   /* Determine which joins need to be traversed. */
   /*=============================================*/
   
   for (i = 0; i < arraySize; i++)
     { infoArray[i].marked = false; }
     
   theInfo = &infoArray[joinIndex];
   theJoin = theInfo->theJoin;
   lastIndex = joinIndex;
   
   while (theJoin != NULL)
     {
      for (i = lastIndex; i >= 0; i--)
        {
         if (infoArray[i].theJoin == theJoin)
           {
            positionsToPrint++;
            infoArray[i].marked = true;
            if (infoArray[i].patternBegin != infoArray[i].patternEnd)
              { nestedCEs = true; }
            lastIndex = i - 1;
            break;
           }
        }
      theJoin = theJoin->lastLevel;
     }
   
   for (i = 0; i <= joinIndex; i++)
     {
      if (infoArray[i].marked == false) continue;

      positionsToPrint--;
      startPosition = i;
      endPosition = i;
      
      if (infoArray[i].patternBegin == infoArray[i].patternEnd)
        {
         for (j = i + 1; j <= joinIndex; j++)
           {
            if (infoArray[j].marked == false) continue;
         
            if (infoArray[j].patternBegin != infoArray[j].patternEnd) break;
         
            positionsToPrint--;
            i = j;
            endPosition = j;
           }
        }
        
      theInfo = &infoArray[startPosition];

      gensprintf(buffer,"%d",theInfo->whichCE);
      returnString = AppendStrings(theEnv,returnString,buffer);
      
      if (nestedCEs)
        {
         if (theInfo->patternBegin == theInfo->patternEnd)
           {
            returnString = AppendStrings(theEnv,returnString," (P");
            gensprintf(buffer,"%d",theInfo->patternBegin);
            returnString = AppendStrings(theEnv,returnString,buffer);
            returnString = AppendStrings(theEnv,returnString,")");
           }
         else
           {
            returnString = AppendStrings(theEnv,returnString," (P");
            gensprintf(buffer,"%d",theInfo->patternBegin);
            returnString = AppendStrings(theEnv,returnString,buffer);
            returnString = AppendStrings(theEnv,returnString," - P");
            gensprintf(buffer,"%d",theInfo->patternEnd);
            returnString = AppendStrings(theEnv,returnString,buffer);
            returnString = AppendStrings(theEnv,returnString,")");
           }
        }
      
      if (startPosition != endPosition)
        {
         theInfo = &infoArray[endPosition];
         
         returnString = AppendStrings(theEnv,returnString," - ");
         gensprintf(buffer,"%d",theInfo->whichCE);
         returnString = AppendStrings(theEnv,returnString,buffer);
      
         if (nestedCEs)
           {
            if (theInfo->patternBegin == theInfo->patternEnd)
              {
               returnString = AppendStrings(theEnv,returnString," (P");
               gensprintf(buffer,"%d",theInfo->patternBegin);
               returnString = AppendStrings(theEnv,returnString,buffer);
               returnString = AppendStrings(theEnv,returnString,")");
              }
            else
              {
               returnString = AppendStrings(theEnv,returnString," (P");
               gensprintf(buffer,"%d",theInfo->patternBegin);
               returnString = AppendStrings(theEnv,returnString,buffer);
               returnString = AppendStrings(theEnv,returnString," - P");
               gensprintf(buffer,"%d",theInfo->patternEnd);
               returnString = AppendStrings(theEnv,returnString,buffer);
               returnString = AppendStrings(theEnv,returnString,")");
              }
           }
        }
      
      if (positionsToPrint > 0)
        { returnString = AppendStrings(theEnv,returnString," , "); }
     }
      
   return returnString;
  }

/********************/
/* ListBetaMatches: */
/********************/
static long long ListBetaMatches(
  void *theEnv,
  struct joinInformation *infoArray,
  long joinIndex,
  long arraySize,
  int output)
  {
   long betaCount = 0;
   struct joinInformation *theInfo;
   long int count;

   if (EnvGetHaltExecution(theEnv) == true)
     { return(betaCount); }

   theInfo = &infoArray[joinIndex];
   
   if (output == VERBOSE)
     {
      EnvPrintRouter(theEnv,WDISPLAY,"Partial matches for CEs ");
      EnvPrintRouter(theEnv,WDISPLAY,
                     BetaHeaderString(theEnv,infoArray,joinIndex,arraySize));
      EnvPrintRouter(theEnv,WDISPLAY,"\n");
     }

   count = PrintBetaMemory(theEnv,WDISPLAY,theInfo->theMemory,true,"",output);
   
   betaCount += count;
   
   if ((output == VERBOSE) && (count == 0))
     { EnvPrintRouter(theEnv,WDISPLAY," None\n"); }
   else if (output == SUCCINCT)
     {
      EnvPrintRouter(theEnv,WDISPLAY,"CEs ");
      EnvPrintRouter(theEnv,WDISPLAY,
                     BetaHeaderString(theEnv,infoArray,joinIndex,arraySize));
      EnvPrintRouter(theEnv,WDISPLAY,": ");
      PrintLongInteger(theEnv,WDISPLAY,betaCount);
      EnvPrintRouter(theEnv,WDISPLAY,"\n");
     }

   return(betaCount);
  }

/******************/
/* CountPatterns: */
/******************/
static int CountPatterns(
  void *theEnv,
  struct joinNode *theJoin,
  bool followRight)
  {
   int theCount = 0;

   if (theJoin == NULL) return theCount;
   
   if (theJoin->joinFromTheRight && (followRight == false))
     { theCount++; }
    
   while (theJoin != NULL)
     {
      if (theJoin->joinFromTheRight)
        {
         if (followRight)
           { theJoin = (struct joinNode *) theJoin->rightSideEntryStructure; }
         else
           { theJoin = theJoin->lastLevel; }
        }
      else
        {
         theCount++;
         theJoin = theJoin->lastLevel;
        }
        
      followRight = true;
     }
     
   return theCount;
  }

/*******************************************/
/* JoinActivityCommand: H/L access routine */
/*   for the join-activity command.        */
/*******************************************/
void JoinActivityCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *ruleName, *argument;
   void *rulePtr;
   CLIPSValue theArg;
   int output;
   Environment *theEnv = UDFContextEnvironment(context);

   if (! UDFFirstArgument(context,SYMBOL_TYPE,&theArg))
     { return; }
     
   ruleName = mCVToString(&theArg);

   rulePtr = EnvFindDefrule(theEnv,ruleName);
   if (rulePtr == NULL)
     {
      CantFindItemErrorMessage(theEnv,"defrule",ruleName);
      mCVSetBoolean(returnValue,false);
      return;
     }

   if (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,SYMBOL_TYPE,&theArg))
        { return; }

      argument = mCVToString(&theArg);
      if (strcmp(argument,"verbose") == 0)
        { output = VERBOSE; }
      else if (strcmp(argument,"succinct") == 0)
        { output = SUCCINCT; }
      else if (strcmp(argument,"terse") == 0)
        { output = TERSE; }
      else
        {
         UDFInvalidArgumentMessage(context,"symbol with value verbose, succinct, or terse");
         mCVSetBoolean(returnValue,false);
         return;
        }
     }
   else
     { output = VERBOSE; }

   EnvJoinActivity(theEnv,rulePtr,output,returnValue);
  }

/*************************************/
/* EnvJoinActivity: C access routine */
/*   for the join-activity command.  */
/*************************************/
void EnvJoinActivity(
  void *theEnv,
  void *theRule,
  int output,
  DATA_OBJECT *result)
  {
   struct defrule *rulePtr;
   long disjunctCount, disjunctIndex, joinIndex;
   long arraySize;
   struct joinInformation *theInfo;

   /*==========================*/
   /* Set up the return value. */
   /*==========================*/
   
   result->type = MULTIFIELD;
   result->begin = 0;
   result->end = 2;
   result->value = EnvCreateMultifield(theEnv,3L);
   
   SetMFType(result->value,1,INTEGER);
   SetMFValue(result->value,1,SymbolData(theEnv)->Zero);
   SetMFType(result->value,2,INTEGER);
   SetMFValue(result->value,2,SymbolData(theEnv)->Zero);
   SetMFType(result->value,3,INTEGER);
   SetMFValue(result->value,3,SymbolData(theEnv)->Zero);

   /*=================================================*/
   /* Loop through each of the disjuncts for the rule */
   /*=================================================*/

   disjunctCount = EnvGetDisjunctCount(theEnv,theRule);

   for (disjunctIndex = 1; disjunctIndex <= disjunctCount; disjunctIndex++)
     {
      rulePtr = (struct defrule *) EnvGetNthDisjunct(theEnv,theRule,disjunctIndex);
      
      /*==============================================*/
      /* Create the array containing the list of beta */
      /* join nodes (joins from the right plus joins  */
      /* connected to a pattern CE).                  */
      /*==============================================*/
      
      arraySize = EnvBetaJoinCount(theEnv,rulePtr);
      
      theInfo = EnvCreateJoinArray(theEnv,arraySize);
      
      EnvBetaJoins(theEnv,rulePtr,arraySize,theInfo);

      /*======================================*/
      /* List the beta matches (for all joins */
      /* except the first pattern CE).        */
      /*======================================*/

      for (joinIndex = 0; joinIndex < arraySize; joinIndex++)
        { ListBetaJoinActivity(theEnv,theInfo,joinIndex,arraySize,output,result); }

      /*================================*/
      /* Free the array of alpha joins. */
      /*================================*/
      
      EnvFreeJoinArray(theEnv,theInfo,arraySize);
     }
  }

/************************/
/* ActivityHeaderString */
/************************/
static const char *ActivityHeaderString(
  void *theEnv,
  struct joinInformation *infoArray,
  long joinIndex,
  long arraySize)
  {
   struct joinNode *theJoin;
   struct joinInformation *theInfo;
   long i;
   bool nestedCEs = false;
   const char *returnString = "";
   long lastIndex;
   char buffer[32];
   
   /*=============================================*/
   /* Determine which joins need to be traversed. */
   /*=============================================*/
   
   for (i = 0; i < arraySize; i++)
     { infoArray[i].marked = false; }
     
   theInfo = &infoArray[joinIndex];
   theJoin = theInfo->theJoin;
   lastIndex = joinIndex;

   while (theJoin != NULL)
     {
      for (i = lastIndex; i >= 0; i--)
        {
         if (infoArray[i].theJoin == theJoin)
           {
            if (infoArray[i].patternBegin != infoArray[i].patternEnd)
              { nestedCEs = true; }
            lastIndex = i - 1;
            break;
           }
        }
      theJoin = theJoin->lastLevel;
     }
  
   gensprintf(buffer,"%d",theInfo->whichCE);
   returnString = AppendStrings(theEnv,returnString,buffer);
   if (nestedCEs == false)
     { return returnString; }

   if (theInfo->patternBegin == theInfo->patternEnd)
     {
      returnString = AppendStrings(theEnv,returnString," (P");
      gensprintf(buffer,"%d",theInfo->patternBegin);
      returnString = AppendStrings(theEnv,returnString,buffer);

      returnString = AppendStrings(theEnv,returnString,")");
     }
   else
     {
      returnString = AppendStrings(theEnv,returnString," (P");
            
      gensprintf(buffer,"%d",theInfo->patternBegin);
      returnString = AppendStrings(theEnv,returnString,buffer);

      returnString = AppendStrings(theEnv,returnString," - P");
            
      gensprintf(buffer,"%d",theInfo->patternEnd);
      returnString = AppendStrings(theEnv,returnString,buffer);

      returnString = AppendStrings(theEnv,returnString,")");
     }
      
   return returnString;
  }

/*************************/
/* ListBetaJoinActivity: */
/*************************/
static void ListBetaJoinActivity(
  void *theEnv,
  struct joinInformation *infoArray,
  long joinIndex,
  long arraySize,
  int output,
  DATA_OBJECT *result)
  {
   long long activity = 0;
   long long compares, adds, deletes;
   struct joinNode *theJoin, *nextJoin;
   struct joinInformation *theInfo;

   if (EnvGetHaltExecution(theEnv) == true)
     { return; }

   theInfo = &infoArray[joinIndex];
   
   theJoin = theInfo->theJoin;
   nextJoin = theInfo->nextJoin;
   
   compares = theJoin->memoryCompares;
   if (theInfo->nextJoin->joinFromTheRight)
     {
      adds = nextJoin->memoryRightAdds;
      deletes = nextJoin->memoryRightDeletes;
     }
   else
     {
      adds = nextJoin->memoryLeftAdds;
      deletes = nextJoin->memoryLeftDeletes;
     }

   activity = compares + adds + deletes;
   
   if (output == VERBOSE)
     {
      char buffer[100];
      
      EnvPrintRouter(theEnv,WDISPLAY,"Activity for CE ");
      EnvPrintRouter(theEnv,WDISPLAY,
                     ActivityHeaderString(theEnv,infoArray,joinIndex,arraySize));
      EnvPrintRouter(theEnv,WDISPLAY,"\n");
      
      sprintf(buffer,"   Compares: %10lld\n",compares);
      EnvPrintRouter(theEnv,WDISPLAY,buffer);
      sprintf(buffer,"   Adds:     %10lld\n",adds);
      EnvPrintRouter(theEnv,WDISPLAY,buffer);
      sprintf(buffer,"   Deletes:  %10lld\n",deletes);
      EnvPrintRouter(theEnv,WDISPLAY,buffer);
     }
   else if (output == SUCCINCT)
     {
      EnvPrintRouter(theEnv,WDISPLAY,"CE ");
      EnvPrintRouter(theEnv,WDISPLAY,
                     ActivityHeaderString(theEnv,infoArray,joinIndex,arraySize));
      EnvPrintRouter(theEnv,WDISPLAY,": ");
      PrintLongInteger(theEnv,WDISPLAY,activity);
      EnvPrintRouter(theEnv,WDISPLAY,"\n");
     }

   compares += ValueToLong(GetMFValue(result->value,1));
   adds += ValueToLong(GetMFValue(result->value,2));
   deletes += ValueToLong(GetMFValue(result->value,3));
   
   SetMFType(result->value,1,INTEGER);
   SetMFValue(result->value,1,EnvAddLong(theEnv,compares));
   SetMFType(result->value,2,INTEGER);
   SetMFValue(result->value,2,EnvAddLong(theEnv,adds));
   SetMFType(result->value,3,INTEGER);
   SetMFValue(result->value,3,EnvAddLong(theEnv,deletes));
  }

/*********************************************/
/* JoinActivityReset: Sets the join activity */
/*   counts for each rule back to 0.         */
/*********************************************/
static void JoinActivityReset(
  void *theEnv,
  struct constructHeader *theConstruct,
  void *buffer)
  {
#if MAC_XCD
#pragma unused(buffer)
#endif
   struct defrule *theDefrule = (struct defrule *) theConstruct;
   struct joinNode *theJoin = theDefrule->lastJoin;
   
   while (theJoin != NULL)
     {
      theJoin->memoryCompares = 0;
      theJoin->memoryLeftAdds = 0;
      theJoin->memoryRightAdds = 0;
      theJoin->memoryLeftDeletes = 0;
      theJoin->memoryRightDeletes = 0;
      
      if (theJoin->joinFromTheRight)
        { theJoin = (struct joinNode *) theJoin->rightSideEntryStructure; }
      else
        { theJoin = theJoin->lastLevel; }
     }
  }

/************************************************/
/* JoinActivityResetCommand: H/L access routine */
/*   for the reset-join-activity command.       */
/************************************************/
void JoinActivityResetCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   void *theEnv = UDFContextEnvironment(context);
   DoForAllConstructs(theEnv,JoinActivityReset,DefruleData(theEnv)->DefruleModuleIndex,true,NULL);
  }

/***************************************/
/* TimetagFunction: H/L access routine */
/*   for the timetag function.         */
/***************************************/
void TimetagFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue theArg;
   void *ptr;

   ptr = GetFactOrInstanceArgument(UDFContextEnvironment(context),1,&theArg,"timetag");

   if (ptr == NULL)
     {
      mCVSetInteger(returnValue,-1LL);
      return;
     }

   mCVSetInteger(returnValue,((struct patternEntity *) ptr)->timeTag);
  }

#endif /* DEBUGGING_FUNCTIONS */

#if DEVELOPER
/***********************************************/
/* RuleComplexityCommand: H/L access routine   */
/*   for the rule-complexity function.         */
/***********************************************/
void RuleComplexityCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *ruleName;
   struct defrule *rulePtr;
   void *theEnv = UDFContextEnvironment(context);

   ruleName = GetConstructName(context,"rule-complexity","rule name");
   if (ruleName == NULL)
     {
      mCVSetInteger(returnValue,-1);
      return;
     }

   rulePtr = (struct defrule *) EnvFindDefrule(theEnv,ruleName);
   if (rulePtr == NULL)
     {
      CantFindItemErrorMessage(theEnv,"defrule",ruleName);
      mCVSetInteger(returnValue,-1);
      return;
     }

   mCVSetInteger(returnValue,rulePtr->complexity);
  }

/******************************************/
/* ShowJoinsCommand: H/L access routine   */
/*   for the show-joins command.          */
/******************************************/
void ShowJoinsCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *ruleName;
   void *rulePtr;
   void *theEnv = UDFContextEnvironment(context);

   ruleName = GetConstructName(context,"show-joins","rule name");
   if (ruleName == NULL) return;

   rulePtr = EnvFindDefrule(theEnv,ruleName);
   if (rulePtr == NULL)
     {
      CantFindItemErrorMessage(theEnv,"defrule",ruleName);
      return;
     }

   ShowJoins(theEnv,rulePtr);

   return;
  }

/*********************************/
/* ShowJoins: C access routine   */
/*   for the show-joins command. */
/*********************************/
static void ShowJoins(
  void *theEnv,
  void *theRule)
  {
   struct defrule *rulePtr;
   struct joinNode *theJoin;
   struct joinNode *joinList[MAXIMUM_NUMBER_OF_PATTERNS];
   int numberOfJoins;
   char rhsType;
   int disjunct = 0;
   unsigned long count = 0;

   rulePtr = (struct defrule *) theRule;
   
   if ((rulePtr != NULL) && (rulePtr->disjunct != NULL))
     { disjunct = 1; }
     
   /*=================================================*/
   /* Loop through each of the disjuncts for the rule */
   /*=================================================*/

   while (rulePtr != NULL)
     {
      if (disjunct > 0)
        {
         EnvPrintRouter(theEnv,WDISPLAY,"Disjunct #");
         PrintLongInteger(theEnv, WDISPLAY, (long long) disjunct++);
         EnvPrintRouter(theEnv,WDISPLAY,"\n");
        }
        
      /*=====================================*/
      /* Determine the number of join nodes. */
      /*=====================================*/

      numberOfJoins = -1;
      theJoin = rulePtr->lastJoin;
      while (theJoin != NULL)
        {
         if (theJoin->joinFromTheRight)
           {
            numberOfJoins++;
            joinList[numberOfJoins] = theJoin;
            theJoin = (struct joinNode *) theJoin->rightSideEntryStructure;
           }
         else
           {
            numberOfJoins++;
            joinList[numberOfJoins] = theJoin;
            theJoin = theJoin->lastLevel;
           }
        }

      /*====================*/
      /* Display the joins. */
      /*====================*/

      while (numberOfJoins >= 0)
        {
         char buffer[20];
         
         if (joinList[numberOfJoins]->patternIsNegated)
           { rhsType = 'n'; }
         else if (joinList[numberOfJoins]->patternIsExists)
           { rhsType = 'x'; }
         else
           { rhsType = ' '; }
           
         gensprintf(buffer,"%2d%c%c%c%c : ",(int) joinList[numberOfJoins]->depth,
                                     (joinList[numberOfJoins]->firstJoin) ? 'f' : ' ',
                                     rhsType,
                                     (joinList[numberOfJoins]->joinFromTheRight) ? 'j' : ' ',
                                     (joinList[numberOfJoins]->logicalJoin) ? 'l' : ' ');
         EnvPrintRouter(theEnv,WDISPLAY,buffer);
         PrintExpression(theEnv,WDISPLAY,joinList[numberOfJoins]->networkTest);
         EnvPrintRouter(theEnv,WDISPLAY,"\n");
         
         if (joinList[numberOfJoins]->ruleToActivate != NULL)
           {
            EnvPrintRouter(theEnv,WDISPLAY,"    RA : ");
            EnvPrintRouter(theEnv,WDISPLAY,EnvGetDefruleName(theEnv,joinList[numberOfJoins]->ruleToActivate));
            EnvPrintRouter(theEnv,WDISPLAY,"\n");
           }
         
         if (joinList[numberOfJoins]->secondaryNetworkTest != NULL)
           {
            EnvPrintRouter(theEnv,WDISPLAY,"    SNT : ");
            PrintExpression(theEnv,WDISPLAY,joinList[numberOfJoins]->secondaryNetworkTest);
            EnvPrintRouter(theEnv,WDISPLAY,"\n");
           }
         
         if (joinList[numberOfJoins]->leftHash != NULL)
           {
            EnvPrintRouter(theEnv,WDISPLAY,"    LH : ");
            PrintExpression(theEnv,WDISPLAY,joinList[numberOfJoins]->leftHash);
            EnvPrintRouter(theEnv,WDISPLAY,"\n");
           }

         if (joinList[numberOfJoins]->rightHash != NULL)
           {
            EnvPrintRouter(theEnv,WDISPLAY,"    RH : ");
            PrintExpression(theEnv,WDISPLAY,joinList[numberOfJoins]->rightHash);
            EnvPrintRouter(theEnv,WDISPLAY,"\n");
           }

         if (! joinList[numberOfJoins]->firstJoin)
           {
            EnvPrintRouter(theEnv,WDISPLAY,"    LM : ");
            count = PrintBetaMemory(theEnv,WDISPLAY,joinList[numberOfJoins]->leftMemory,false,"",SUCCINCT);
            if (count == 0)
              { EnvPrintRouter(theEnv,WDISPLAY,"None\n"); }
            else
              {
               sprintf(buffer,"%lu\n",count);
               EnvPrintRouter(theEnv,WDISPLAY,buffer);
              }
           }
         
         if (joinList[numberOfJoins]->joinFromTheRight)
           {
            EnvPrintRouter(theEnv,WDISPLAY,"    RM : ");
            count = PrintBetaMemory(theEnv,WDISPLAY,joinList[numberOfJoins]->rightMemory,false,"",SUCCINCT);
            if (count == 0)
              { EnvPrintRouter(theEnv,WDISPLAY,"None\n"); }
            else
              {
               sprintf(buffer,"%lu\n",count);
               EnvPrintRouter(theEnv,WDISPLAY,buffer);
              }
           }
         
         numberOfJoins--;
        };
  
      /*===============================*/
      /* Proceed to the next disjunct. */
      /*===============================*/

      rulePtr = rulePtr->disjunct;
      if (rulePtr != NULL) EnvPrintRouter(theEnv,WDISPLAY,"\n");
     }
  }

/******************************************************/
/* ShowAlphaHashTable: Displays the number of entries */
/*   in each slot of the alpha hash table.            */
/******************************************************/
void ShowAlphaHashTable(
   UDFContext *context,
   CLIPSValue *returnValue)
   {
    int i, count;
    long totalCount = 0;
    struct alphaMemoryHash *theEntry;
    struct partialMatch *theMatch;
    char buffer[40];
    Environment *theEnv = UDFContextEnvironment(context);

    for (i = 0; i < ALPHA_MEMORY_HASH_SIZE; i++)
      {
       for (theEntry =  DefruleData(theEnv)->AlphaMemoryTable[i], count = 0;
            theEntry != NULL;
            theEntry = theEntry->next)
         { count++; }

       if (count != 0)
         {
          totalCount += count;
          gensprintf(buffer,"%4d: %4d ->",i,count);
          EnvPrintRouter(theEnv,WDISPLAY,buffer);
          
          for (theEntry =  DefruleData(theEnv)->AlphaMemoryTable[i], count = 0;
               theEntry != NULL;
               theEntry = theEntry->next)
            {
             for (theMatch = theEntry->alphaMemory;
                  theMatch != NULL;
                  theMatch = theMatch->nextInMemory)
               { count++; }
               
             gensprintf(buffer," %4d",count);
             EnvPrintRouter(theEnv,WDISPLAY,buffer);
             if (theEntry->owner->rightHash == NULL)
               { EnvPrintRouter(theEnv,WDISPLAY,"*"); }
            }
          
          EnvPrintRouter(theEnv,WDISPLAY,"\n");
         }
      }
    gensprintf(buffer,"Total Count: %ld\n",totalCount);
    EnvPrintRouter(theEnv,WDISPLAY,buffer);
   }

#endif /* DEVELOPER */

#endif /* DEFRULE_CONSTRUCT */
