   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*           INSTANCE MODIFY AND DUPLICATE MODULE      */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
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
/*            Changed name of variable exp to theExp         */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                           */
/*      6.24: Converted INSTANCE_PATTERN_MATCHING to         */
/*            DEFRULE_CONSTRUCT.                             */
/*                                                           */
/*      6.30: Added DATA_OBJECT_ARRAY primitive type.        */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            The return value of DirectMessage indicates    */
/*            whether an execution error has occurred.       */
/*                                                           */
/*************************************************************/

#ifndef _H_insmoddp

#pragma once

#define _H_insmoddp

#define DIRECT_MODIFY_STRING    "direct-modify"
#define MSG_MODIFY_STRING       "message-modify"
#define DIRECT_DUPLICATE_STRING "direct-duplicate"
#define MSG_DUPLICATE_STRING    "message-duplicate"

#ifndef _H_evaluatn
#include "evaluatn.h"
#endif

#if (! RUN_TIME)
   void                           SetupInstanceModDupCommands(void *);
#endif

   void                           ModifyInstance(UDFContext *,CLIPSValue *);
   void                           MsgModifyInstance(UDFContext *,CLIPSValue *);
   void                           DuplicateInstance(UDFContext *,CLIPSValue *);
   void                           MsgDuplicateInstance(UDFContext *,CLIPSValue *);

#if DEFRULE_CONSTRUCT && OBJECT_SYSTEM
   void                           InactiveModifyInstance(UDFContext *,CLIPSValue *);
   void                           InactiveMsgModifyInstance(UDFContext *,CLIPSValue *);
   void                           InactiveDuplicateInstance(UDFContext *,CLIPSValue *);
   void                           InactiveMsgDuplicateInstance(UDFContext *,CLIPSValue *);
#endif

   void                           DirectModifyMsgHandler(UDFContext *,CLIPSValue *);
   void                           MsgModifyMsgHandler(UDFContext *,CLIPSValue *);
   void                           DirectDuplicateMsgHandler(UDFContext *,CLIPSValue *);
   void                           MsgDuplicateMsgHandler(UDFContext *,CLIPSValue *);

#endif /* _H_insmoddp */







