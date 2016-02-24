   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                  WATCH HEADER FILE                  */
   /*******************************************************/

/*************************************************************/
/* Purpose: Support functions for the watch and unwatch      */
/*   commands.                                               */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Changed name of variable log to logName        */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added EnvSetWatchItem function.                */
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
/*************************************************************/

#ifndef _H_watch

#pragma once

#define _H_watch

#include "expressn.h"

#define WATCH_DATA 54

struct watchItem
  {
   const char *name;
   unsigned *flag;
   int code,priority;
   bool (*accessFunc)(void *,int,bool,struct expr *);
   bool (*printFunc)(void *,const char *,int,struct expr *);
   struct watchItem *next;
  };

struct watchData
  { 
   struct watchItem *ListOfWatchItems;
  };

#define WatchData(theEnv) ((struct watchData *) GetEnvironmentData(theEnv,WATCH_DATA))

   bool                           EnvWatch(void *,const char *);
   bool                           EnvUnwatch(void *,const char *);
   void                           InitializeWatchData(void *);   
   bool                           EnvSetWatchItem(void *,const char *,bool,struct expr *);
   int                            EnvGetWatchItem(void *,const char *);
   bool                           AddWatchItem(void *,const char *,int,unsigned *,int,
                                                      bool (*)(void *,int,bool,struct expr *),
                                                      bool (*)(void *,const char *,int,struct expr *));
   const char                    *GetNthWatchName(void *,int);
   int                            GetNthWatchValue(void *,int);
   void                           WatchCommand(UDFContext *,CLIPSValue *);
   void                           UnwatchCommand(UDFContext *,CLIPSValue *);
   void                           ListWatchItemsCommand(UDFContext *,CLIPSValue *);
   void                           WatchFunctionDefinitions(void *);
   void                           GetWatchItemCommand(UDFContext *,CLIPSValue *);

#endif /* _H_watch */



