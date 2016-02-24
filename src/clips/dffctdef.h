   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                DEFFACTS HEADER FILE                 */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*            Changed find construct functionality so that   */
/*            imported modules are search when locating a    */
/*            named construct.                               */
/*                                                           */
/*************************************************************/

#ifndef _H_dffctdef

#pragma once

#define _H_dffctdef

#include "constrct.h"
#include "conscomp.h"
#include "cstrccom.h"
#include "evaluatn.h"
#include "expressn.h"
#include "moduldef.h"
#include "symbol.h"

#define DEFFACTS_DATA 0

struct deffactsData
  { 
   struct construct *DeffactsConstruct;
   int DeffactsModuleIndex;  
#if CONSTRUCT_COMPILER && (! RUN_TIME)
   struct CodeGeneratorItem *DeffactsCodeItem;
#endif
  };
  
struct deffacts
  {
   struct constructHeader header;
   struct expr *assertList;
  };

struct deffactsModule
  {
   struct defmoduleItemHeader header;
  };

#define DeffactsData(theEnv) ((struct deffactsData *) GetEnvironmentData(theEnv,DEFFACTS_DATA))

   void                           InitializeDeffacts(void *);
   void                          *EnvFindDeffacts(void *,const char *);
   void                          *EnvFindDeffactsInModule(void *,const char *);
   void                          *EnvGetNextDeffacts(void *,void *);
   void                           CreateInitialFactDeffacts(void);
   bool                           EnvIsDeffactsDeletable(void *,void *);
   struct deffactsModule         *GetDeffactsModuleItem(void *,struct defmodule *);
   const char                    *EnvDeffactsModule(void *,void *);
   const char                    *EnvGetDeffactsName(void *,void *);
   const char                    *EnvGetDeffactsPPForm(void *,void *);

#endif /* _H_dffctdef */


