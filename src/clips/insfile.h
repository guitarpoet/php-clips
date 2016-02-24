   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                                                     */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Added environment parameter to GenClose.       */
/*            Added environment parameter to GenOpen.        */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Corrected code to remove compiler warnings.    */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*************************************************************/

#ifndef _H_insfile

#pragma once

#define _H_insfile

#include "expressn.h"

#define INSTANCE_FILE_DATA 30

#if BLOAD_INSTANCES || BSAVE_INSTANCES
struct instanceFileData
  { 
   const char *InstanceBinaryPrefixID;
   const char *InstanceBinaryVersionID;
   unsigned long BinaryInstanceFileSize;

#if BLOAD_INSTANCES
   unsigned long BinaryInstanceFileOffset;
   char *CurrentReadBuffer;
   unsigned long CurrentReadBufferSize;
   unsigned long CurrentReadBufferOffset;
#endif
  };

#define InstanceFileData(theEnv) ((struct instanceFileData *) GetEnvironmentData(theEnv,INSTANCE_FILE_DATA))

#endif /* BLOAD_INSTANCES || BSAVE_INSTANCES */

   void                           SetupInstanceFileCommands(void *);
   void                           SaveInstancesCommand(UDFContext *,CLIPSValue *);
   void                           LoadInstancesCommand(UDFContext *,CLIPSValue *);
   void                           RestoreInstancesCommand(UDFContext *,CLIPSValue *);
   long                           EnvSaveInstancesDriver(void *,const char *,int,EXPRESSION *,bool);
   long                           EnvSaveInstances(void *,const char *,int);
#if BSAVE_INSTANCES
   void                           BinarySaveInstancesCommand(UDFContext *,CLIPSValue *);
   long                           EnvBinarySaveInstancesDriver(void *,const char *,int,EXPRESSION *,bool);
   long                           EnvBinarySaveInstances(void *,const char *,int);
#endif
#if BLOAD_INSTANCES
   void                           BinaryLoadInstancesCommand(UDFContext *,CLIPSValue *);
   long                           EnvBinaryLoadInstances(void *,const char *);
#endif
   long                           EnvLoadInstances(void *,const char *);
   long                           EnvLoadInstancesFromString(void *,const char *,size_t);
   long                           EnvRestoreInstances(void *,const char *);
   long                           EnvRestoreInstancesFromString(void *,const char *,size_t);

#endif /* _H_insfile */



