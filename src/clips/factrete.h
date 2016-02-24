   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*        FACT RETE ACCESS FUNCTIONS HEADER FILE       */
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
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*      6.24: Removed INCREMENTAL_RESET compilation flag.    */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Support for hashing optimizations.             */
/*                                                           */
/*************************************************************/

#ifndef _H_factrete

#pragma once

#define _H_factrete

#include "evaluatn.h"

   bool                           FactPNGetVar1(void *,void *,DATA_OBJECT_PTR);
   bool                           FactPNGetVar2(void *,void *,DATA_OBJECT_PTR);
   bool                           FactPNGetVar3(void *,void *,DATA_OBJECT_PTR);
   bool                           FactJNGetVar1(void *,void *,DATA_OBJECT_PTR);
   bool                           FactJNGetVar2(void *,void *,DATA_OBJECT_PTR);
   bool                           FactJNGetVar3(void *,void *,DATA_OBJECT_PTR);
   bool                           FactSlotLength(void *,void *,DATA_OBJECT_PTR);
   bool                           FactJNCompVars1(void *,void *,DATA_OBJECT_PTR);
   bool                           FactJNCompVars2(void *,void *,DATA_OBJECT_PTR);
   bool                           FactPNCompVars1(void *,void *,DATA_OBJECT_PTR);
   bool                           FactPNConstant1(void *,void *,DATA_OBJECT_PTR);
   bool                           FactPNConstant2(void *,void *,DATA_OBJECT_PTR);
   bool                           FactStoreMultifield(void *,void *,DATA_OBJECT_PTR);
   unsigned short                 AdjustFieldPosition(void *,struct multifieldMarker *,
                                                             unsigned short,unsigned short,int *);

#endif


