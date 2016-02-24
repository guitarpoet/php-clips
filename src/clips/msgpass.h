   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                                                     */
   /*******************************************************/

/*************************************************************/
/* Purpose: Message-passing support functions                */
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
/*      6.24: Removed IMPERATIVE_MESSAGE_HANDLERS and        */
/*            AUXILIARY_MESSAGE_HANDLERS compilation flags.  */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: The return value of DirectMessage indicates    */
/*            whether an execution error has occurred.       */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_msgpass

#pragma once

#define _H_msgpass

#define GetActiveInstance(theEnv) ((INSTANCE_TYPE *) GetNthMessageArgument(theEnv,0)->value)

#include "object.h"

typedef struct messageHandlerLink
  {
   HANDLER *hnd;
   struct messageHandlerLink *nxt;
   struct messageHandlerLink *nxtInStack;
  } HANDLER_LINK;

   bool             DirectMessage(void *,SYMBOL_HN *,INSTANCE_TYPE *,
                                         DATA_OBJECT *,EXPRESSION *);
   void             EnvSend(void *,DATA_OBJECT *,const char *,const char *,DATA_OBJECT *);
   void             DestroyHandlerLinks(void *,HANDLER_LINK *);
   void             SendCommand(UDFContext *,CLIPSValue *);
   DATA_OBJECT     *GetNthMessageArgument(void *,int);

   bool             NextHandlerAvailable(void *);
   void             NextHandlerAvailableFunction(UDFContext *,CLIPSValue *);

   void             CallNextHandler(UDFContext *,CLIPSValue *);

   void             FindApplicableOfName(void *,DEFCLASS *,HANDLER_LINK *[],
                                                HANDLER_LINK *[],SYMBOL_HN *);
   HANDLER_LINK    *JoinHandlerLinks(void *,HANDLER_LINK *[],HANDLER_LINK *[],SYMBOL_HN *);

   void             PrintHandlerSlotGetFunction(void *,const char *,void *);
   bool             HandlerSlotGetFunction(void *,void *,DATA_OBJECT *);
   void             PrintHandlerSlotPutFunction(void *,const char *,void *);
   bool             HandlerSlotPutFunction(void *,void *,DATA_OBJECT *);
   void             DynamicHandlerGetSlot(UDFContext *,CLIPSValue *);
   void             DynamicHandlerPutSlot(UDFContext *,CLIPSValue *);

#endif /* _H_object */







