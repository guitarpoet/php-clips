   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*          DEFTEMPLATE RHS PARSING HEADER FILE        */
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
/*      6.24: Added additional argument required for         */
/*            DeriveDefaultFromConstraints.                  */
/*                                                           */
/*            Added additional argument required for         */
/*            InvalidDeftemplateSlotMessage.                 */
/*                                                           */
/*      6.30: Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_tmpltrhs

#pragma once

#define _H_tmpltrhs

#include "expressn.h"
#include "scanner.h"
#include "tmpltdef.h"

   struct expr                   *ParseAssertTemplate(void *,const char *,struct token *,bool *,
                                                             int,bool,struct deftemplate *);

#endif /* _H_tmpltrhs */



