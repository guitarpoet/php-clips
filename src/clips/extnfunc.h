   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/13/16             */
   /*                                                     */
   /*            EXTERNAL FUNCTIONS HEADER FILE           */
   /*******************************************************/

/*************************************************************/
/* Purpose: Routines for adding new user or system defined   */
/*   functions.                                              */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Added support for passing context information  */ 
/*            to user defined functions.                     */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Replaced ALLOW_ENVIRONMENT_GLOBALS macros      */
/*            with functions.                                */
/*                                                           */
/*      6.40: Changed restrictions from char * to            */
/*            symbolHashNode * to support strings            */
/*            originating from sources that are not          */
/*            statically allocated.                          */
/*                                                           */
/*            Callbacks must be environment aware.           */
/*                                                           */
/*************************************************************/

#ifndef _H_extnfunc

#pragma once

#define _H_extnfunc

struct FunctionDefinition;
struct UDFContext_t;
typedef struct UDFContext_t UDFContext;

#include "evaluatn.h"
#include "expressn.h"
#include "symbol.h"
#include "userdata.h"

struct FunctionDefinition
  {
   struct symbolHashNode *callFunctionName;
   const char *actualFunctionName;
   char returnValueType;
   unsigned unknownReturnValueType;
   //int (*functionPointer)(void);
   void (*functionPointer)(UDFContext *,CLIPSValue *);
   struct expr *(*parser)(void *,struct expr *,const char *);
   struct symbolHashNode *restrictions;
   int minArgs;
   int maxArgs;
   bool overloadable; // TBD Use unsigned ints here
   bool sequenceuseok;
   short int bsaveIndex;
   struct FunctionDefinition *next;
   struct userData *usrData;
   void *context;
  };

#define ValueFunctionType(target) (((struct FunctionDefinition *) target)->returnValueType)
#define UnknownFunctionType(target) (((struct FunctionDefinition *) target)->unknownReturnValueType)
#define ExpressionFunctionType(target) (((struct FunctionDefinition *) ((target)->value))->returnValueType)
#define ExpressionFunctionPointer(target) (((struct FunctionDefinition *) ((target)->value))->functionPointer)
#define ExpressionFunctionCallName(target) (((struct FunctionDefinition *) ((target)->value))->callFunctionName)
#define ExpressionFunctionRealName(target) (((struct FunctionDefinition *) ((target)->value))->actualFunctionName)
#define ExpressionUnknownFunctionType(target) (((struct FunctionDefinition *) ((target)->value))->unknownReturnValueType)

#define PTIF (int (*)(void))
#define PTIEF (int (*)(void *))

/*==================*/
/* ENVIRONMENT DATA */
/*==================*/

#define EXTERNAL_FUNCTION_DATA 50

struct externalFunctionData
  {
   struct FunctionDefinition *ListOfFunctions;
   struct FunctionHash **FunctionHashtable;
  };

struct UDFContext_t
  {
   Environment *environment;
   struct FunctionDefinition *theFunction;
   int lastPosition;
   struct expr *lastArg;
   CLIPSValue *returnValue;
  };

#define ExternalFunctionData(theEnv) ((struct externalFunctionData *) GetEnvironmentData(theEnv,EXTERNAL_FUNCTION_DATA))

struct FunctionHash
  {
   struct FunctionDefinition *fdPtr;
   struct FunctionHash *next;
  };

#define SIZE_FUNCTION_HASH 517

   void                           InitializeExternalFunctionData(void *);
   bool                           EnvDefineFunction(void *,const char *,int,
                                                           int (*)(void *),const char *);
   bool                           EnvDefineFunction2(void *,const char *,int,
                                                            int (*)(void *),const char *,const char *);
   bool                           EnvDefineFunctionWithContext(void *,const char *,int,
                                                           int (*)(void *),const char *,void *);
   bool                           EnvDefineFunction2WithContext(void *,const char *,int,
                                                            int (*)(void *),const char *,const char *,void *);
   bool                           DefineFunction3(void *,const char *,int,unsigned,
                                                  void (*)(UDFContext *,CLIPSValue *),
                                                  const char *,int,int,const char *,void *);
   bool                           EnvAddUDF(Environment *,const char *,const char *,
                                            void (*)(UDFContext *,CLIPSValue *),
                                            const char *,int,int,const char *,void *);


   int                            AddFunctionParser(void *,const char *,
                                                           struct expr *(*)( void *,struct expr *,const char *));
   int                            RemoveFunctionParser(void *,const char *);
   bool                           FuncSeqOvlFlags(void *,const char *,bool,bool);
   struct FunctionDefinition     *GetFunctionList(void *);
   void                           InstallFunctionList(void *,struct FunctionDefinition *);
   struct FunctionDefinition     *FindFunction(void *,const char *);
   int                            GetNthRestriction(struct FunctionDefinition *,int);
   unsigned                       GetNthRestriction2(Environment *,struct FunctionDefinition *,int);
   const char                    *GetArgumentTypeName(int);
   bool                           UndefineFunction(void *,const char *);
   int                            GetMinimumArgs(struct FunctionDefinition *);
   int                            GetMaximumArgs(struct FunctionDefinition *);

   int                            UDFArgumentCount(UDFContext *);
   bool                           UDFNthArgument(UDFContext *,int,unsigned,struct dataObject *);
   void                           UDFInvalidArgumentMessage(UDFContext *,const char *);
   Environment                   *UDFContextEnvironment(UDFContext *);
   void                          *UDFContextUserContext(UDFContext *);
   const char                    *UDFContextFunctionName(UDFContext *);
   void                           PrintTypesString(void *,const char *,unsigned,bool);
   bool                           UDFFirstArgument(UDFContext *,unsigned,CLIPSValue *);
   bool                           UDFNextArgument(UDFContext *,unsigned,CLIPSValue *);
   void                           UDFThrowError(UDFContext *);

#define UDFHasNextArgument(context) (context->lastArg != NULL)

#endif /* _H_extnfunc */



