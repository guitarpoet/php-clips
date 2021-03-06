   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*              DEFGLOBAL COMMANDS MODULE              */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides the show-defglobals, set-reset-globals, */
/*   and get-reset-globals commands.                         */
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
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#include "setup.h"

#if DEFGLOBAL_CONSTRUCT

#include "argacces.h"
#include "envrnmnt.h"
#include "extnfunc.h"
#include "globldef.h"
#include "prntutil.h"
#include "router.h"

#include "globlcom.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

#if DEBUGGING_FUNCTIONS
   static void                       PrintDefglobalValueForm(void *,const char *,void *);
#endif

/************************************************************/
/* DefglobalCommandDefinitions: Defines defglobal commands. */
/************************************************************/
void DefglobalCommandDefinitions(
  void *theEnv)
  {
#if ! RUN_TIME
   EnvAddUDF(theEnv,"set-reset-globals","b",
                   SetResetGlobalsCommand,"SetResetGlobalsCommand", 1,1,NULL,NULL);
   EnvAddUDF(theEnv,"get-reset-globals","b",
                    GetResetGlobalsCommand,"GetResetGlobalsCommand", 0,0,NULL,NULL);

#if DEBUGGING_FUNCTIONS
   EnvAddUDF(theEnv,"show-defglobals","v",
                    ShowDefglobalsCommand,"ShowDefglobalsCommand", 0,1,"y",NULL);
#endif

#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif
  }

/************************************************/
/* SetResetGlobalsCommand: H/L access routine   */
/*   for the get-reset-globals command.         */
/************************************************/
void SetResetGlobalsCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   bool oldValue;
   DATA_OBJECT result;
   void *theEnv = UDFContextEnvironment(context);

   /*===========================================*/
   /* Remember the old value of this attribute. */
   /*===========================================*/

   oldValue = EnvGetResetGlobals(theEnv);

   /*===========================================*/
   /* Determine the new value of the attribute. */
   /*===========================================*/

   if (! UDFFirstArgument(context,ANY_TYPE,&result))
     { return; }
   
   if (CVIsFalseSymbol(&result))
     { EnvSetResetGlobals(theEnv,false); }
   else
     { EnvSetResetGlobals(theEnv,true); }

   /*========================================*/
   /* Return the old value of the attribute. */
   /*========================================*/

   mCVSetBoolean(returnValue,oldValue);
  }

/****************************************/
/* EnvSetResetGlobals: C access routine */
/*   for the set-reset-globals command. */
/****************************************/
bool EnvSetResetGlobals(
  void *theEnv,
  bool value)
  {
   bool ov;

   ov = DefglobalData(theEnv)->ResetGlobals;
   DefglobalData(theEnv)->ResetGlobals = value;
   return(ov);
  }

/************************************************/
/* GetResetGlobalsCommand: H/L access routine   */
/*   for the get-reset-globals command.         */
/************************************************/
void GetResetGlobalsCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   mCVSetBoolean(returnValue,EnvGetResetGlobals(UDFContextEnvironment(context)));
  }

/****************************************/
/* EnvGetResetGlobals: C access routine */
/*   for the get-reset-globals command. */
/****************************************/
bool EnvGetResetGlobals(
  void *theEnv)
  {   
   return(DefglobalData(theEnv)->ResetGlobals); 
  }

#if DEBUGGING_FUNCTIONS

/***********************************************/
/* ShowDefglobalsCommand: H/L access routine   */
/*   for the show-defglobals command.          */
/***********************************************/
void ShowDefglobalsCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   struct defmodule *theModule;
   int numArgs;
   bool error;
   void *theEnv = UDFContextEnvironment(context);

   numArgs = UDFArgumentCount(context);
   if (numArgs == 1)
     {
      theModule = GetModuleName(theEnv,"show-defglobals",1,&error);
      if (error) return;
     }
   else
     { theModule = ((struct defmodule *) EnvGetCurrentModule(theEnv)); }

   EnvShowDefglobals(theEnv,WDISPLAY,theModule);
  }

/***************************************/
/* EnvShowDefglobals: C access routine */
/*   for the show-defglobals command.  */
/***************************************/
void EnvShowDefglobals(
  void *theEnv,
  const char *logicalName,
  void *vTheModule)
  {
   struct defmodule *theModule = (struct defmodule *) vTheModule;
   struct constructHeader *constructPtr;
   bool allModules = false;
   struct defmoduleItemHeader *theModuleItem;

   /*=======================================*/
   /* If the module specified is NULL, then */
   /* list all constructs in all modules.   */
   /*=======================================*/

   if (theModule == NULL)
     {
      theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,NULL);
      allModules = true;
     }

   /*======================================================*/
   /* Print out the constructs in the specified module(s). */
   /*======================================================*/

   for (;
        theModule != NULL;
        theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,theModule))
     {
      /*===========================================*/
      /* Print the module name before every group  */
      /* of defglobals listed if we're listing the */
      /* defglobals from every module.             */
      /*===========================================*/

      if (allModules)
        {
         EnvPrintRouter(theEnv,logicalName,EnvGetDefmoduleName(theEnv,theModule));
         EnvPrintRouter(theEnv,logicalName,":\n");
        }

      /*=====================================*/
      /* Print every defglobal in the module */
      /* currently being examined.           */
      /*=====================================*/

      theModuleItem = (struct defmoduleItemHeader *) GetModuleItem(theEnv,theModule,DefglobalData(theEnv)->DefglobalModuleIndex);

      for (constructPtr = theModuleItem->firstItem;
           constructPtr != NULL;
           constructPtr = constructPtr->next)
        {
         if (EvaluationData(theEnv)->HaltExecution == true) return;

         if (allModules) EnvPrintRouter(theEnv,logicalName,"   ");
         PrintDefglobalValueForm(theEnv,logicalName,(void *) constructPtr);
         EnvPrintRouter(theEnv,logicalName,"\n");
        }

      /*===================================*/
      /* If we're only listing the globals */
      /* for one module, then return.      */
      /*===================================*/

      if (! allModules) return;
     }
  }

/*****************************************************/
/* PrintDefglobalValueForm: Prints the value form of */
/*   a defglobal (the current value). For example,   */
/*   ?*x* = 3                                        */
/*****************************************************/
static void PrintDefglobalValueForm(
  void *theEnv,
  const char *logicalName,
  void *vTheGlobal)
  {
   struct defglobal *theGlobal = (struct defglobal *) vTheGlobal;

   EnvPrintRouter(theEnv,logicalName,"?*");
   EnvPrintRouter(theEnv,logicalName,ValueToString(theGlobal->header.name));
   EnvPrintRouter(theEnv,logicalName,"* = ");
   PrintDataObject(theEnv,logicalName,&theGlobal->current);
  }

#endif /* DEBUGGING_FUNCTIONS */

#endif /* DEFGLOBAL_CONSTRUCT */

