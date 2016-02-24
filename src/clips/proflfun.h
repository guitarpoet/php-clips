   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*      CONSTRUCT PROFILING FUNCTIONS HEADER FILE      */
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
/*      6.23: Modified OutputProfileInfo to allow a before   */
/*            and after prefix so that a string buffer does  */
/*            not need to be created to contain the entire   */
/*            prefix. This allows a buffer overflow problem  */
/*            to be corrected. DR0857.                       */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added pragmas to remove compilation warnings.  */
/*                                                           */
/*            Corrected code to remove run-time program      */
/*            compiler warnings.                             */
/*                                                           */
/*      6.30: Used gensprintf instead of sprintf.            */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_TBC).         */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_proflfun

#pragma once

#define _H_proflfun

#include "userdata.h"

struct constructProfileInfo
  {
   struct userData usrData;
   long numberOfEntries;
   unsigned int childCall : 1;
   double startTime;
   double totalSelfTime;
   double totalWithChildrenTime;
  };

struct profileFrameInfo
  {
   unsigned int parentCall : 1;
   unsigned int profileOnExit : 1;
   double parentStartTime;
   struct constructProfileInfo *oldProfileFrame;
  };
  
#define PROFLFUN_DATA 15

struct profileFunctionData
  { 
   double ProfileStartTime;
   double ProfileEndTime;
   double ProfileTotalTime;
   int LastProfileInfo;
   double PercentThreshold;
   struct userDataRecord ProfileDataInfo;
   unsigned char ProfileDataID;
   bool ProfileUserFunctions; // TBD bit fields?
   bool ProfileConstructs;
   struct constructProfileInfo *ActiveProfileFrame;
   const char *OutputString;
  };

#define ProfileFunctionData(theEnv) ((struct profileFunctionData *) GetEnvironmentData(theEnv,PROFLFUN_DATA))

   void                           ConstructProfilingFunctionDefinitions(void *);
   void                           ProfileCommand(UDFContext *,CLIPSValue *);
   void                           ProfileInfoCommand(UDFContext *,CLIPSValue *);
   void                           StartProfile(void *,
                                                      struct profileFrameInfo *,
                                                      struct userData **,
                                                      bool);
   void                           EndProfile(void *,struct profileFrameInfo *);
   void                           ProfileResetCommand(UDFContext *,CLIPSValue *);
   void                           ResetProfileInfo(struct constructProfileInfo *);

   void                           SetProfilePercentThresholdCommand(UDFContext *,CLIPSValue *);
   double                         SetProfilePercentThreshold(void *,double);
   void                           GetProfilePercentThresholdCommand(UDFContext *,CLIPSValue *);
   double                         GetProfilePercentThreshold(void *);
   bool                           Profile(void *,const char *);
   void                           DeleteProfileData(void *,void *);
   void                          *CreateProfileData(void *);
   const char                    *SetProfileOutputString(void *,const char *);

#endif /* _H_proflfun */


