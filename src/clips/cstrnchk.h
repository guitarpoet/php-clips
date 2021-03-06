   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*            CONSTRAINT CHECKING HEADER FILE          */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides functions for constraint checking of    */
/*   data types.                                             */
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
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Dynamic constraint checking for the            */
/*            allowed-classes constraint now searches        */
/*            imported modules.                              */
/*                                                           */
/*************************************************************/

#ifndef _H_cstrnchk

#pragma once

#define _H_cstrnchk

#include "constrnt.h"
#include "evaluatn.h"

#define NO_VIOLATION                    0
#define TYPE_VIOLATION                  1
#define RANGE_VIOLATION                 2
#define ALLOWED_VALUES_VIOLATION        3
#define FUNCTION_RETURN_TYPE_VIOLATION  4
#define CARDINALITY_VIOLATION           5
#define ALLOWED_CLASSES_VIOLATION       6

   bool                           CheckCardinalityConstraint(void *,long,CONSTRAINT_RECORD *);
   bool                           CheckAllowedValuesConstraint(int,void *,CONSTRAINT_RECORD *);
   bool                           CheckAllowedClassesConstraint(void *,int,void *,CONSTRAINT_RECORD *);
   int                            ConstraintCheckExpressionChain(void *,struct expr *,
                                                                     CONSTRAINT_RECORD *);
   void                           ConstraintViolationErrorMessage(void *,const char *,const char *,bool,int,
                                                                      struct symbolHashNode *,
                                                                      int,int,CONSTRAINT_RECORD *,
                                                                      bool);
   int                            ConstraintCheckValue(void *,int,void *,CONSTRAINT_RECORD *);
   int                            ConstraintCheckDataObject(void *,DATA_OBJECT *,CONSTRAINT_RECORD *);
#if (! BLOAD_ONLY) && (! RUN_TIME)
   int                            ConstraintCheckExpression(void *,struct expr *,
                                                                CONSTRAINT_RECORD *);
#endif
#if (! RUN_TIME)
   bool                           UnmatchableConstraint(struct constraintRecord *);
#endif

#endif /* _H_cstrnchk */



