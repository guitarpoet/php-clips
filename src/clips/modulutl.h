   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*            DEFMODULE UTILITY HEADER FILE            */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides routines for parsing module/construct   */
/*   names and searching through modules for specific        */
/*   constructs.                                             */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.30: Used genstrncpy instead of strncpy.            */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_modulutl

#pragma once

#define _H_modulutl

#include "moduldef.h"
#include "symbol.h"

   unsigned                       FindModuleSeparator(const char *);
   SYMBOL_HN                     *ExtractModuleName(void *,unsigned,const char *);
   SYMBOL_HN                     *ExtractConstructName(void *,unsigned,const char *);
   const char                    *ExtractModuleAndConstructName(void *,const char *);
   void                          *FindImportedConstruct(void *,const char *,struct defmodule *,
                                                               const char *,int *,bool,struct defmodule *);
   void                           AmbiguousReferenceErrorMessage(void *,const char *,const char *);
   void                           MarkModulesAsUnvisited(void *);
   bool                           AllImportedModulesVisited(void *,struct defmodule *);
   void                           ListItemsDriver(void *,
                                                         const char *,struct defmodule *,
                                                         const char *,const char *,
                                                         void *(*)(void *,void *),
                                                         const char *(*)(void *),
                                                         void (*)(void *,const char *,void *),
                                                         bool (*)(void *,void *));
   long                           DoForAllModules(void *,
                                                         void (*)(struct defmodule *,void *),
                                                         int,void *);
   bool                           ConstructExported(void *,const char *,struct symbolHashNode *,struct symbolHashNode *);
   
#endif /* _H_modulutl */



