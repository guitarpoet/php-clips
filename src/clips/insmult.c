   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*           INSTANCE MULTIFIELD SLOT MODULE           */
   /*******************************************************/

/*************************************************************/
/* Purpose:  Access routines for Instance Multifield Slots   */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*      6.40: Added Env prefix to GetEvaluationError and     */
/*            SetEvaluationError functions.                  */
/*                                                           */
/*************************************************************/

/* =========================================
   *****************************************
               EXTERNAL DEFINITIONS
   =========================================
   ***************************************** */
#include "setup.h"

#if OBJECT_SYSTEM

#include "argacces.h"
#include "envrnmnt.h"
#include "extnfunc.h"
#include "insfun.h"
#include "msgfun.h"
#include "msgpass.h"
#include "multifun.h"
#include "router.h"

#include "insmult.h"

/* =========================================
   *****************************************
                   CONSTANTS
   =========================================
   ***************************************** */
#define INSERT         0
#define REPLACE        1
#define DELETE_OP      2

/* =========================================
   *****************************************
      INTERNALLY VISIBLE FUNCTION HEADERS
   =========================================
   ***************************************** */

static INSTANCE_TYPE *CheckMultifieldSlotInstance(void *,const char *);
static INSTANCE_SLOT *CheckMultifieldSlotModify(void *,int,const char *,INSTANCE_TYPE *,
                                       EXPRESSION *,long *,long *,DATA_OBJECT *);
static void AssignSlotToDataObject(DATA_OBJECT *,INSTANCE_SLOT *);

/* =========================================
   *****************************************
          EXTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

#if (! RUN_TIME)

/***************************************************
  NAME         : SetupInstanceMultifieldCommands
  DESCRIPTION  : Defines function interfaces for
                 manipulating instance multislots
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : Functions defined to KB
  NOTES        : None
 ***************************************************/
void SetupInstanceMultifieldCommands(
  void *theEnv)
  {
   EnvAddUDF(theEnv,"slot-direct-replace$","b", DirectMVReplaceCommand,
                    "DirectMVReplaceCommand",4,UNBOUNDED,"*;y;l;l",NULL);
   EnvAddUDF(theEnv,"slot-direct-insert$","b", DirectMVInsertCommand,
                    "DirectMVInsertCommand",3,UNBOUNDED,"*;y;l",NULL);
   EnvAddUDF(theEnv,"slot-direct-delete$","b", DirectMVDeleteCommand,
                    "DirectMVDeleteCommand",3,3,"l;y",NULL);
   EnvAddUDF(theEnv,"slot-replace$","*", MVSlotReplaceCommand,
                    "MVSlotReplaceCommand",5, UNBOUNDED, "*;iny;y;l;l",NULL);
   EnvAddUDF(theEnv,"slot-insert$","*", MVSlotInsertCommand,
                    "MVSlotInsertCommand",4,UNBOUNDED,"*;iny;y;l",NULL);
   EnvAddUDF(theEnv,"slot-delete$","*", MVSlotDeleteCommand,
                    "MVSlotDeleteCommand",4,4, "l;iny;y" ,NULL);
  }

#endif

/***********************************************************************************
  NAME         : MVSlotReplaceCommand
  DESCRIPTION  : Allows user to replace a specified field of a multi-value slot
                 The slot is directly read (w/o a get- message) and the new
                   slot-value is placed via a put- message.
                 This function is not valid for single-value slots.
  INPUTS       : Caller's result buffer
  RETURNS      : true if multi-value slot successfully modified,
                 false otherwise
  SIDE EFFECTS : Put messsage sent for slot
  NOTES        : H/L Syntax : (slot-replace$ <instance> <slot>
                                 <range-begin> <range-end> <value>)
 ***********************************************************************************/
void MVSlotReplaceCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT newval,newseg,oldseg;
   INSTANCE_TYPE *ins;
   INSTANCE_SLOT *sp;
   long rb,re;
   EXPRESSION arg;
   Environment *theEnv = UDFContextEnvironment(context);

   mCVSetBoolean(returnValue,false);
   ins = CheckMultifieldSlotInstance(theEnv,"slot-replace$");
   if (ins == NULL)
     return;
   sp = CheckMultifieldSlotModify(theEnv,REPLACE,"slot-replace$",ins,
                            GetFirstArgument()->nextArg,&rb,&re,&newval);
   if (sp == NULL)
     return;
   AssignSlotToDataObject(&oldseg,sp);
   if (ReplaceMultiValueField(theEnv,&newseg,&oldseg,rb,re,&newval,"slot-replace$") == false)
     return;
   arg.type = MULTIFIELD;
   arg.value = (void *) &newseg;
   arg.nextArg = NULL;
   arg.argList = NULL;
   DirectMessage(theEnv,sp->desc->overrideMessage,ins,returnValue,&arg);
  }

/***********************************************************************************
  NAME         : MVSlotInsertCommand
  DESCRIPTION  : Allows user to insert a specified field of a multi-value slot
                 The slot is directly read (w/o a get- message) and the new
                   slot-value is placed via a put- message.
                 This function is not valid for single-value slots.
  INPUTS       : Caller's result buffer
  RETURNS      : true if multi-value slot successfully modified, false otherwise
  SIDE EFFECTS : Put messsage sent for slot
  NOTES        : H/L Syntax : (slot-insert$ <instance> <slot> <index> <value>)
 ***********************************************************************************/
void MVSlotInsertCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT newval,newseg,oldseg;
   INSTANCE_TYPE *ins;
   INSTANCE_SLOT *sp;
   long theIndex;
   EXPRESSION arg;
   Environment *theEnv = UDFContextEnvironment(context);

   mCVSetBoolean(returnValue,false);
   ins = CheckMultifieldSlotInstance(theEnv,"slot-insert$");
   if (ins == NULL)
     return;
   sp = CheckMultifieldSlotModify(theEnv,INSERT,"slot-insert$",ins,
                            GetFirstArgument()->nextArg,&theIndex,NULL,&newval);
   if (sp == NULL)
     return;
   AssignSlotToDataObject(&oldseg,sp);
   if (InsertMultiValueField(theEnv,&newseg,&oldseg,theIndex,&newval,"slot-insert$") == false)
     return;
   arg.type = MULTIFIELD;
   arg.value = (void *) &newseg;
   arg.nextArg = NULL;
   arg.argList = NULL;
   DirectMessage(theEnv,sp->desc->overrideMessage,ins,returnValue,&arg);
  }

/***********************************************************************************
  NAME         : MVSlotDeleteCommand
  DESCRIPTION  : Allows user to delete a specified field of a multi-value slot
                 The slot is directly read (w/o a get- message) and the new
                   slot-value is placed via a put- message.
                 This function is not valid for single-value slots.
  INPUTS       : Caller's result buffer
  RETURNS      : true if multi-value slot successfully modified, false otherwise
  SIDE EFFECTS : Put message sent for slot
  NOTES        : H/L Syntax : (slot-delete$ <instance> <slot>
                                 <range-begin> <range-end>)
 ***********************************************************************************/
void MVSlotDeleteCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT newseg,oldseg;
   INSTANCE_TYPE *ins;
   INSTANCE_SLOT *sp;
   long rb,re;
   EXPRESSION arg;
   Environment *theEnv = UDFContextEnvironment(context);

   mCVSetBoolean(returnValue,false);
   ins = CheckMultifieldSlotInstance(theEnv,"slot-delete$");
   if (ins == NULL)
     return;
   sp = CheckMultifieldSlotModify(theEnv,DELETE_OP,"slot-delete$",ins,
                            GetFirstArgument()->nextArg,&rb,&re,NULL);
   if (sp == NULL)
     return;
   AssignSlotToDataObject(&oldseg,sp);
   if (DeleteMultiValueField(theEnv,&newseg,&oldseg,rb,re,"slot-delete$") == false)
     return;
   arg.type = MULTIFIELD;
   arg.value = (void *) &newseg;
   arg.nextArg = NULL;
   arg.argList = NULL;
   DirectMessage(theEnv,sp->desc->overrideMessage,ins,returnValue,&arg);
  }

/*****************************************************************
  NAME         : DirectMVReplaceCommand
  DESCRIPTION  : Directly replaces a slot's value
  INPUTS       : None
  RETURNS      : true if put OK, false otherwise
  SIDE EFFECTS : Slot modified
  NOTES        : H/L Syntax: (direct-slot-replace$ <slot>
                                <range-begin> <range-end> <value>)
 *****************************************************************/
void DirectMVReplaceCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   INSTANCE_SLOT *sp;
   INSTANCE_TYPE *ins;
   long rb,re;
   DATA_OBJECT newval,newseg,oldseg;
   Environment *theEnv = UDFContextEnvironment(context);

   if (CheckCurrentMessage(theEnv,"direct-slot-replace$",true) == false)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }
     
   ins = GetActiveInstance(theEnv);
   sp = CheckMultifieldSlotModify(theEnv,REPLACE,"direct-slot-replace$",ins,
                            GetFirstArgument(),&rb,&re,&newval);
   if (sp == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }
     
   AssignSlotToDataObject(&oldseg,sp);
   if (! ReplaceMultiValueField(theEnv,&newseg,&oldseg,rb,re,&newval,"direct-slot-replace$"))
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   if (PutSlotValue(theEnv,ins,sp,&newseg,&newval,"function direct-slot-replace$"))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/************************************************************************
  NAME         : DirectMVInsertCommand
  DESCRIPTION  : Directly inserts a slot's value
  INPUTS       : None
  RETURNS      : true if put OK, false otherwise
  SIDE EFFECTS : Slot modified
  NOTES        : H/L Syntax: (direct-slot-insert$ <slot> <index> <value>)
 ************************************************************************/
void DirectMVInsertCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   INSTANCE_SLOT *sp;
   INSTANCE_TYPE *ins;
   long theIndex;
   DATA_OBJECT newval,newseg,oldseg;
   Environment *theEnv = UDFContextEnvironment(context);

   if (CheckCurrentMessage(theEnv,"direct-slot-insert$",true) == false)
     { mCVSetBoolean(returnValue,false); }
     
   ins = GetActiveInstance(theEnv);
   sp = CheckMultifieldSlotModify(theEnv,INSERT,"direct-slot-insert$",ins,
                            GetFirstArgument(),&theIndex,NULL,&newval);
   if (sp == NULL)
     { mCVSetBoolean(returnValue,false); }
   AssignSlotToDataObject(&oldseg,sp);
   if (! InsertMultiValueField(theEnv,&newseg,&oldseg,theIndex,&newval,"direct-slot-insert$"))
     { mCVSetBoolean(returnValue,false); }
     
   if (PutSlotValue(theEnv,ins,sp,&newseg,&newval,"function direct-slot-insert$"))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/*****************************************************************
  NAME         : DirectMVDeleteCommand
  DESCRIPTION  : Directly deletes a slot's value
  INPUTS       : None
  RETURNS      : true if put OK, false otherwise
  SIDE EFFECTS : Slot modified
  NOTES        : H/L Syntax: (direct-slot-delete$ <slot>
                                <range-begin> <range-end>)
 *****************************************************************/
void DirectMVDeleteCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   INSTANCE_SLOT *sp;
   INSTANCE_TYPE *ins;
   long rb,re;
   DATA_OBJECT newseg,oldseg;
   Environment *theEnv = UDFContextEnvironment(context);

   if (CheckCurrentMessage(theEnv,"direct-slot-delete$",true) == false)
     { mCVSetBoolean(returnValue,false); }

   ins = GetActiveInstance(theEnv);
   sp = CheckMultifieldSlotModify(theEnv,DELETE_OP,"direct-slot-delete$",ins,
                                  GetFirstArgument(),&rb,&re,NULL);
   if (sp == NULL)
     { mCVSetBoolean(returnValue,false); }
     
   AssignSlotToDataObject(&oldseg,sp);
   if (! DeleteMultiValueField(theEnv,&newseg,&oldseg,rb,re,"direct-slot-delete$"))
     { mCVSetBoolean(returnValue,false); }

   if (PutSlotValue(theEnv,ins,sp,&newseg,&oldseg,"function direct-slot-delete$"))
     { mCVSetBoolean(returnValue,true); }
   else
     { mCVSetBoolean(returnValue,false); }
  }

/* =========================================
   *****************************************
          INTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/**********************************************************************
  NAME         : CheckMultifieldSlotInstance
  DESCRIPTION  : Gets the instance for the functions slot-replace$,
                    insert and delete
  INPUTS       : The function name
  RETURNS      : The instance address, NULL on errors
  SIDE EFFECTS : None
  NOTES        : None
 **********************************************************************/
static INSTANCE_TYPE *CheckMultifieldSlotInstance(
  void *theEnv,
  const char *func)
  {
   INSTANCE_TYPE *ins;
   DATA_OBJECT temp;

   if (EnvArgTypeCheck(theEnv,func,1,INSTANCE_OR_INSTANCE_NAME,&temp) == false)
     {
      EnvSetEvaluationError(theEnv,true);
      return(NULL);
     }
   if (temp.type == INSTANCE_ADDRESS)
     {
      ins = (INSTANCE_TYPE *) temp.value;
      if (ins->garbage == 1)
        {
         StaleInstanceAddress(theEnv,func,0);
         EnvSetEvaluationError(theEnv,true);
         return(NULL);
        }
     }
   else
     {
      ins = FindInstanceBySymbol(theEnv,(SYMBOL_HN *) temp.value);
      if (ins == NULL)
        NoInstanceError(theEnv,ValueToString(temp.value),func);
     }
   return(ins);
  }

/*********************************************************************
  NAME         : CheckMultifieldSlotModify
  DESCRIPTION  : For the functions slot-replace$, insert, & delete
                    as well as direct-slot-replace$, insert, & delete
                    this function gets the slot, index, and optional
                    field-value for these functions
  INPUTS       : 1) A code indicating the type of operation
                      INSERT    (0) : Requires one index
                      REPLACE   (1) : Requires two indices
                      DELETE_OP (2) : Requires two indices
                 2) Function name-string
                 3) Instance address
                 4) Argument expression chain
                 5) Caller's buffer for index (or beginning of range)
                 6) Caller's buffer for end of range
                     (can be NULL for INSERT)
                 7) Caller's new-field value buffer
                     (can be NULL for DELETE_OP)
  RETURNS      : The address of the instance-slot,
                    NULL on errors
  SIDE EFFECTS : Caller's index buffer set
                 Caller's new-field value buffer set (if not NULL)
                   Will allocate an ephemeral segment to store more
                     than 1 new field value
                 EvaluationError set on errors
  NOTES        : Assume the argument chain is at least 2
                   expressions deep - slot, index, and optional values
 *********************************************************************/
static INSTANCE_SLOT *CheckMultifieldSlotModify(
  void *theEnv,
  int code,
  const char *func,
  INSTANCE_TYPE *ins,
  EXPRESSION *args,
  long *rb,
  long *re,
  DATA_OBJECT *newval)
  {
   DATA_OBJECT temp;
   INSTANCE_SLOT *sp;
   int start;

   start = (args == GetFirstArgument()) ? 1 : 2;
   EvaluationData(theEnv)->EvaluationError = false;
   EvaluateExpression(theEnv,args,&temp);
   if (temp.type != SYMBOL)
     {
      ExpectedTypeError1(theEnv,func,start,"symbol");
      EnvSetEvaluationError(theEnv,true);
      return(NULL);
     }
   sp = FindInstanceSlot(theEnv,ins,(SYMBOL_HN *) temp.value);
   if (sp == NULL)
     {
      SlotExistError(theEnv,ValueToString(temp.value),func);
      return(NULL);
     }
   if (sp->desc->multiple == 0)
     {
      PrintErrorID(theEnv,"INSMULT",1,false);
      EnvPrintRouter(theEnv,WERROR,"Function ");
      EnvPrintRouter(theEnv,WERROR,func);
      EnvPrintRouter(theEnv,WERROR," cannot be used on single-field slot ");
      EnvPrintRouter(theEnv,WERROR,ValueToString(sp->desc->slotName->name));
      EnvPrintRouter(theEnv,WERROR," in instance ");
      EnvPrintRouter(theEnv,WERROR,ValueToString(ins->name));
      EnvPrintRouter(theEnv,WERROR,".\n");
      EnvSetEvaluationError(theEnv,true);
      return(NULL);
     }
   EvaluateExpression(theEnv,args->nextArg,&temp);
   if (temp.type != INTEGER)
     {
      ExpectedTypeError1(theEnv,func,start+1,"integer");
      EnvSetEvaluationError(theEnv,true);
      return(NULL);
     }
   args = args->nextArg->nextArg;
   *rb = (long) ValueToLong(temp.value);
   if ((code == REPLACE) || (code == DELETE_OP))
     {
      EvaluateExpression(theEnv,args,&temp);
      if (temp.type != INTEGER)
        {
         ExpectedTypeError1(theEnv,func,start+2,"integer");
         EnvSetEvaluationError(theEnv,true);
         return(NULL);
        }
      *re = (long) ValueToLong(temp.value);
      args = args->nextArg;
     }
   if ((code == INSERT) || (code == REPLACE))
     {
      if (EvaluateAndStoreInDataObject(theEnv,1,args,newval,true) == false)
        return(NULL);
     }
   return(sp);
  }

/***************************************************
  NAME         : AssignSlotToDataObject
  DESCRIPTION  : Assigns the value of a multifield
                 slot to a data object
  INPUTS       : 1) The data object buffer
                 2) The instance slot
  RETURNS      : Nothing useful
  SIDE EFFECTS : Data object fields set
  NOTES        : Assumes slot is a multislot
 ***************************************************/
static void AssignSlotToDataObject(
  DATA_OBJECT *theDataObject,
  INSTANCE_SLOT *theSlot)
  {
   theDataObject->type = (unsigned short) theSlot->type;
   theDataObject->value = theSlot->value;
   theDataObject->begin = 0;
   SetpDOEnd(theDataObject,GetInstanceSlotLength(theSlot));
  }

#endif

/***************************************************
  NAME         :
  DESCRIPTION  :
  INPUTS       :
  RETURNS      :
  SIDE EFFECTS :
  NOTES        :
 ***************************************************/


