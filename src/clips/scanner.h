   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                 SCANNER HEADER FILE                 */
   /*******************************************************/

/*************************************************************/
/* Purpose: Routines for scanning lexical tokens from an     */
/*   input source.                                           */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.30: Changed integer type/precision.                */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Added SetLineCount function.                   */
/*                                                           */
/*            Added UTF-8 support.                           */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

#ifndef _H_scanner

#pragma once

#define _H_scanner

struct token;

#include "pprint.h"

struct token
  {
   unsigned short type;
   void *value;
   const char *printForm;
  };

#define SCANNER_DATA 57

struct scannerData
  { 
   char *GlobalString;
   size_t GlobalMax;
   size_t GlobalPos;
   long LineCount;
   bool IgnoreCompletionErrors;
  };

#define ScannerData(theEnv) ((struct scannerData *) GetEnvironmentData(theEnv,SCANNER_DATA))

   void                           InitializeScannerData(void *);
   void                           GetToken(void *,const char *,struct token *);
   void                           CopyToken(struct token *,struct token *);
   void                           ResetLineCount(void *);
   long                           GetLineCount(void *);
   long                           SetLineCount(void *,long);
   void                           IncrementLineCount(void *);
   void                           DecrementLineCount(void *);

#endif /* _H_scanner */




