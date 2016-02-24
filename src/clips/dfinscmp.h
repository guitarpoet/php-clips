   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                                                     */
   /*******************************************************/

/*************************************************************/
/* Purpose: Definstances Construct Compiler Code             */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.30: Added support for path name argument to        */
/*            constructs-to-c.                               */
/*                                                           */
/*************************************************************/

#ifndef _H_dfinscmp

#pragma once

#define _H_dfinscmp

#if DEFINSTANCES_CONSTRUCT && CONSTRUCT_COMPILER && (! RUN_TIME)

#include <stdio.h>

   void                           SetupDefinstancesCompiler(void *);
   void                           DefinstancesCModuleReference(void *,FILE *,int,int,int);

#endif /* DEFINSTANCES_CONSTRUCT && CONSTRUCT_COMPILER && (! RUN_TIME) */

#endif /* _H_dfinscmp */


