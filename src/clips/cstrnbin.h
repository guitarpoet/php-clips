   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*    CONSTRAINT BLOAD/BSAVE/CONSTRUCTS-TO-C HEADER    */
   /*******************************************************/

/*************************************************************/
/* Purpose: Implements the binary save/load feature for      */
/*    constraint records.                                    */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Added allowed-classes slot facet.              */
/*                                                           */
/*      6.30: Changed integer type/precision.                */
/*                                                           */
/*************************************************************/

#ifndef _H_cstrnbin

#pragma once

#define _H_cstrnbin

#include <stdio.h>

#include "evaluatn.h"
#include "constrnt.h"

#define ConstraintIndex(theConstraint) (((! EnvGetDynamicConstraintChecking(theEnv)) || (theConstraint == NULL)) ? -1L : ((long) theConstraint->bsaveIndex))
#define ConstraintPointer(i) (((i) == -1L) ? NULL : (CONSTRAINT_RECORD *) &ConstraintData(theEnv)->ConstraintArray[i])

#if BLOAD_AND_BSAVE
   void                           WriteNeededConstraints(void *,FILE *);
#endif
   void                           ReadNeededConstraints(void *);
   void                           ClearBloadedConstraints(void *);

#endif /* _H_cstrnbin */


