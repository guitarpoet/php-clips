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
/* Revision History:                                         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*************************************************************/

#ifndef _H_dfinsbin

#pragma once

#define _H_dfinsbin

#if DEFINSTANCES_CONSTRUCT && (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE)

#include "defins.h"

#define DFINSBIN_DATA 25

struct definstancesBinaryData
  { 
   DEFINSTANCES *DefinstancesArray;
   long DefinstancesCount;
   long ModuleCount;
   DEFINSTANCES_MODULE *ModuleArray;
  };
  
#define DefinstancesBinaryData(theEnv) ((struct definstancesBinaryData *) GetEnvironmentData(theEnv,DFINSBIN_DATA))

   void                           SetupDefinstancesBload(void *);
   void                          *BloadDefinstancesModuleRef(void *,int);

#endif /* DEFINSTANCES_CONSTRUCT && (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE) */

#endif /* _H_dfinsbin */



