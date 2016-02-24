   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*           DEFRULE LHS PARSING HEADER FILE           */
   /*******************************************************/

/*************************************************************/
/* Purpose: Coordinates parsing of the LHS conditional       */
/*   elements of a rule.                                     */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.30: Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_rulelhs

#pragma once

#define _H_rulelhs

#include "expressn.h"
#include "pattern.h"
#include "reorder.h"
#include "scanner.h"

struct lhsParseNode           *ParseRuleLHS(void *,const char *,struct token *,const char *,bool *);
void                           PropagatePatternType(struct lhsParseNode *,struct patternParser *);

#endif




