   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                 DEVELOPER HEADER FILE               */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Converted INSTANCE_PATTERN_MATCHING to         */
/*            DEFRULE_CONSTRUCT.                             */
/*                                                           */
/*      6.30: Added support for hashed alpha memories.       */
/*                                                           */
/*            Changed garbage collection algorithm.          */
/*            Functions enable-gc-heuristics and             */
/*            disable-gc-heuristics are no longer supported. */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*************************************************************/

#ifndef _H_developr

#pragma once

#define _H_developr

   void                           DeveloperCommands(void *);
   void                           PrimitiveTablesInfo(UDFContext *,CLIPSValue *);
   void                           PrimitiveTablesUsage(UDFContext *,CLIPSValue *);

#if DEFRULE_CONSTRUCT && DEFTEMPLATE_CONSTRUCT
   void                           ShowFactPatternNetwork(UDFContext *,CLIPSValue *);
   void                           ValidateFactIntegrity(UDFContext *,CLIPSValue *);
#endif
#if DEFRULE_CONSTRUCT && OBJECT_SYSTEM
   void                           PrintObjectPatternNetwork(UDFContext *,CLIPSValue *);
#endif
#if OBJECT_SYSTEM
   void                           InstanceTableUsage(UDFContext *,CLIPSValue *);
#endif
#if DEFRULE_CONSTRUCT
   void                           ValidateBetaMemories(void *);
#endif

#endif /* _H_developr */


