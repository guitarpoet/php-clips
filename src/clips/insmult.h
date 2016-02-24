   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                                                     */
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
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*************************************************************/

#ifndef _H_insmult

#pragma once

#define _H_insmult

#include "evaluatn.h"

#if (! RUN_TIME)
   void                           SetupInstanceMultifieldCommands(void *);
#endif

   void                           MVSlotReplaceCommand(UDFContext *,CLIPSValue *);
   void                           MVSlotInsertCommand(UDFContext *,CLIPSValue *);
   void                           MVSlotDeleteCommand(UDFContext *,CLIPSValue *);
   void                           DirectMVReplaceCommand(UDFContext *,CLIPSValue *);
   void                           DirectMVInsertCommand(UDFContext *,CLIPSValue *);
   void                           DirectMVDeleteCommand(UDFContext *,CLIPSValue *);

#endif /* _H_insmult */



