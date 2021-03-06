   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*           CONSTRUCT COMMAND HEADER MODULE           */
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
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added ConstructsDeletable function.            */
/*                                                           */
/*      6.30: Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Changed find construct functionality so that   */
/*            imported modules are search when locating a    */
/*            named construct.                               */
/*                                                           */
/*************************************************************/

#ifndef _H_cstrccom

#pragma once

#define _H_cstrccom

#include "moduldef.h"
#include "constrct.h"

#if (! RUN_TIME)
   void                           AddConstructToModule(struct constructHeader *);
#endif
   bool                           DeleteNamedConstruct(void *,const char *,struct construct *);
   void                          *FindNamedConstructInModule(void *,const char *,struct construct *);
   void                          *FindNamedConstructInModuleOrImports(void *,const char *,struct construct *);
   void                           UndefconstructCommand(UDFContext *,const char *,struct construct *);
   bool                           PPConstruct(void *,const char *,const char *,struct construct *);
   SYMBOL_HN                     *GetConstructModuleCommand(UDFContext *,const char *,struct construct *);
   struct defmodule              *GetConstructModule(void *,const char *,struct construct *);
   bool                           Undefconstruct(void *,void *,struct construct *);
   void                           SaveConstruct(void *,void *,const char *,struct construct *);
   const char                    *GetConstructNameString(struct constructHeader *);
   const char                    *EnvGetConstructNameString(void *,struct constructHeader *);
   const char                    *GetConstructModuleName(struct constructHeader *);
   SYMBOL_HN                     *GetConstructNamePointer(struct constructHeader *);
   void                           GetConstructListFunction(UDFContext *,const char *,DATA_OBJECT_PTR,
                                                                  struct construct *);
   void                           GetConstructList(void *,DATA_OBJECT_PTR,struct construct *,
                                                          struct defmodule *);
   void                           ListConstructCommand(UDFContext *,const char *,struct construct *);
   void                           ListConstruct(void *,struct construct *,const char *,struct defmodule *);
   void                           SetNextConstruct(struct constructHeader *,struct constructHeader *);
   struct defmoduleItemHeader    *GetConstructModuleItem(struct constructHeader *);
   const char                    *GetConstructPPForm(void *,struct constructHeader *);
   void                           PPConstructCommand(UDFContext *,const char *,struct construct *);
   struct constructHeader        *GetNextConstructItem(void *,struct constructHeader *,int);
   struct defmoduleItemHeader    *GetConstructModuleItemByIndex(void *,struct defmodule *,int);
   void                           FreeConstructHeaderModule(void *,struct defmoduleItemHeader *,
                                                                   struct construct *);
   long                           DoForAllConstructs(void *,
                                                            void (*)(void *,struct constructHeader *,void *),
                                                            int,bool,void *);
   void                           DoForAllConstructsInModule(void *,void *,
                                                            void (*)(void *,struct constructHeader *,void *),
                                                            int,int,void *);
   void                           InitializeConstructHeader(void *,const char *,struct constructHeader *,SYMBOL_HN *);
   void                           SetConstructPPForm(void *,struct constructHeader *,const char *);
   void                          *LookupConstruct(void *,struct construct *,const char *,bool);
#if DEBUGGING_FUNCTIONS
   bool                           ConstructPrintWatchAccess(void *,struct construct *,const char *,
                                            EXPRESSION *,
                                            bool (*)(void *,void *),
                                            void (*)(void *,bool,void *));
   bool                           ConstructSetWatchAccess(void *,struct construct *,bool,
                                            EXPRESSION *,
                                            bool (*)(void *,void *),
                                            void (*)(void *,bool,void *));
#endif
   bool                           ConstructsDeletable(void *);

#endif



