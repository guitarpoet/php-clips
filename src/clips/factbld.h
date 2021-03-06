   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                FACT BUILD HEADER FILE               */
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
/*      6.30: Added support for hashed alpha memories.       */
/*                                                           */
/*            Added support for hashed comparisons to        */
/*            constants.                                     */
/*                                                           */
/*************************************************************/

#ifndef _H_factbld

#pragma once

#define _H_factbld

struct factPatternNode;

#include "network.h"
#include "expressn.h"

struct factPatternNode
  {
   struct patternNodeHeader header;
   long bsaveID;
   unsigned short whichField;
   unsigned short whichSlot;
   unsigned short leaveFields;
   struct expr *networkTest;
   struct factPatternNode *nextLevel;
   struct factPatternNode *lastLevel;
   struct factPatternNode *leftNode;
   struct factPatternNode *rightNode;
  };

#include "pattern.h" // TBD Needed?

   void                           InitializeFactPatterns(void *);
   void                           DestroyFactPatternNetwork(void *,struct factPatternNode *);

#endif /* _H_factbld */
