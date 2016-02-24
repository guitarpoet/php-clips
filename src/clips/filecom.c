   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                 FILE COMMANDS MODULE                */
   /*******************************************************/

/*************************************************************/
/* Purpose: Contains the code for file commands including    */
/*   batch, dribble-on, dribble-off, save, load, bsave, and  */
/*   bload.                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Bebe Ly                                              */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Added environment parameter to GenClose.       */
/*            Added environment parameter to GenOpen.        */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Added code for capturing errors/warnings.      */
/*                                                           */
/*            Added AwaitingInput flag.                      */
/*                                                           */             
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*            Fixed linkage issue when BLOAD_ONLY compiler   */
/*            flag is set to 1.                              */
/*                                                           */
/*            Added STDOUT and STDIN logical name            */
/*            definitions.                                   */
/*                                                           */
/*      6.40: Added Env prefix to GetEvaluationError and     */
/*            SetEvaluationError functions.                  */
/*                                                           */
/*            Added Env prefix to GetHaltExecution and       */
/*            SetHaltExecution functions.                    */
/*                                                           */
/*************************************************************/

#include <stdio.h>
#include <string.h>

#include "setup.h"

#include "argacces.h"
#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "bload.h"
#include "bsave.h"
#endif
#include "commline.h"
#include "constrct.h"
#include "cstrcpsr.h"
#include "envrnmnt.h"
#include "extnfunc.h"
#include "memalloc.h"
#include "prcdrfun.h"
#include "router.h"
#include "strngrtr.h"
#include "sysdep.h"
#include "utility.h"

#include "filecom.h"

/***************/
/* STRUCTURES  */
/***************/

struct batchEntry
  {
   int batchType;
   void *inputSource;
   const char *theString;
   const char *fileName;
   long lineNumber;
   struct batchEntry *next;
  };

/***************/
/* DEFINITIONS */
/***************/

#define FILE_BATCH      0
#define STRING_BATCH    1

#define BUFFER_SIZE   120

#define FILECOM_DATA 14

struct fileCommandData
  { 
#if DEBUGGING_FUNCTIONS
   FILE *DribbleFP;
   char *DribbleBuffer;
   size_t DribbleCurrentPosition;
   size_t DribbleMaximumPosition;
   int (*DribbleStatusFunction)(void *,bool);
#endif
   int BatchType;
   void *BatchSource;
   char *BatchBuffer;
   size_t BatchCurrentPosition;
   size_t BatchMaximumPosition;
   struct batchEntry *TopOfBatchList;
   struct batchEntry *BottomOfBatchList;
   char *batchPriorParsingFile;
  };

#define FileCommandData(theEnv) ((struct fileCommandData *) GetEnvironmentData(theEnv,FILECOM_DATA))

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

#if DEBUGGING_FUNCTIONS
   static bool                    FindDribble(void *,const char *);
   static int                     GetcDribble(void *,const char *);
   static int                     UngetcDribble(void *,int,const char *);
   static int                     ExitDribble(void *,int);
   static int                     PrintDribble(void *,const char *,const char *);
   static void                    PutcDribbleBuffer(void *,int);
#endif
   static bool                    FindBatch(void *,const char *);
   static int                     GetcBatch(void *,const char *);
   static int                     UngetcBatch(void *,int,const char *);
   static int                     ExitBatch(void *,int);
   static void                    AddBatch(void *,bool,void *,int,const char *,const char *);
   static void                    DeallocateFileCommandData(void *);

/***************************************/
/* FileCommandDefinitions: Initializes */
/*   file commands.                    */
/***************************************/
void FileCommandDefinitions(
  void *theEnv)
  {
   AllocateEnvironmentData(theEnv,FILECOM_DATA,sizeof(struct fileCommandData),DeallocateFileCommandData);

#if ! RUN_TIME
#if DEBUGGING_FUNCTIONS
   EnvAddUDF(theEnv,"batch","b", BatchCommand,"BatchCommand",1,1,"sy",NULL);
   EnvAddUDF(theEnv,"batch*","b", BatchStarCommand,"BatchStarCommand",1,1,"sy",NULL);
   EnvAddUDF(theEnv,"dribble-on","b", DribbleOnCommand,"DribbleOnCommand",1,1,"sy",NULL);
   EnvAddUDF(theEnv,"dribble-off","b", DribbleOffCommand,"DribbleOffCommand",0,0,NULL,NULL);
   EnvAddUDF(theEnv,"save","b", SaveCommand,"SaveCommand",1,1,"sy",NULL);
#endif
   EnvAddUDF(theEnv,"load","b", LoadCommand,"LoadCommand",1,1,"sy",NULL);
   EnvAddUDF(theEnv,"load*","b", LoadStarCommand,"LoadStarCommand",1,1,"sy",NULL);
#if BLOAD_AND_BSAVE
   EnvAddUDF(theEnv,"bsave","b", BsaveCommand,"BsaveCommand",1,1,"sy",NULL);
#endif
#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
   InitializeBsaveData(theEnv);
   InitializeBloadData(theEnv);
   EnvAddUDF(theEnv,"bload","b", BloadCommand,"BloadCommand",1,1,"sy",NULL);
#endif
#endif
  }
  
/******************************************************/
/* DeallocateFileCommandData: Deallocates environment */
/*    data for file commands.                         */
/******************************************************/
static void DeallocateFileCommandData(
  void *theEnv)
  {
   struct batchEntry *theEntry, *nextEntry;
   
   theEntry = FileCommandData(theEnv)->TopOfBatchList;
   while (theEntry != NULL)
     {
      nextEntry = theEntry->next;

      if (theEntry->batchType == FILE_BATCH)
        { GenClose(theEnv,(FILE *) FileCommandData(theEnv)->TopOfBatchList->inputSource); }
      else
        { rm(theEnv,(void *) theEntry->theString,strlen(theEntry->theString) + 1); }

      rtn_struct(theEnv,batchEntry,theEntry);
         
      theEntry = nextEntry;
     }
     
   if (FileCommandData(theEnv)->BatchBuffer != NULL)
     { rm(theEnv,FileCommandData(theEnv)->BatchBuffer,FileCommandData(theEnv)->BatchMaximumPosition); }

   DeleteString(theEnv,FileCommandData(theEnv)->batchPriorParsingFile);
   FileCommandData(theEnv)->batchPriorParsingFile = NULL;
   
#if DEBUGGING_FUNCTIONS
   if (FileCommandData(theEnv)->DribbleBuffer != NULL)
     { rm(theEnv,FileCommandData(theEnv)->DribbleBuffer,FileCommandData(theEnv)->DribbleMaximumPosition); }
     
   if (FileCommandData(theEnv)->DribbleFP != NULL) 
     { GenClose(theEnv,FileCommandData(theEnv)->DribbleFP); }
#endif
  }
  
#if DEBUGGING_FUNCTIONS
/*****************************************************/
/* FindDribble: Find routine for the dribble router. */
/*****************************************************/
static bool FindDribble(
  void *theEnv,
  const char *logicalName)
  {
#if MAC_XCD
#pragma unused(theEnv)
#endif

   if ( (strcmp(logicalName,STDOUT) == 0) ||
        (strcmp(logicalName,STDIN) == 0) ||
        (strcmp(logicalName,WPROMPT) == 0) ||
        (strcmp(logicalName,WTRACE) == 0) ||
        (strcmp(logicalName,WERROR) == 0) ||
        (strcmp(logicalName,WWARNING) == 0) ||
        (strcmp(logicalName,WDISPLAY) == 0) ||
        (strcmp(logicalName,WDIALOG) == 0) )
     { return(true); }

    return(false);
  }

/*******************************************************/
/* PrintDribble: Print routine for the dribble router. */
/*******************************************************/
static int PrintDribble(
  void *theEnv,
  const char *logicalName,
  const char *str)
  {
   int i;

   /*======================================*/
   /* Send the output to the dribble file. */
   /*======================================*/

   for (i = 0 ; str[i] != EOS ; i++)
     { PutcDribbleBuffer(theEnv,str[i]); }

   /*===========================================================*/
   /* Send the output to any routers interested in printing it. */
   /*===========================================================*/

   EnvDeactivateRouter(theEnv,"dribble");
   EnvPrintRouter(theEnv,logicalName,str);
   EnvActivateRouter(theEnv,"dribble");

   return(1);
  }

/*****************************************************/
/* GetcDribble: Getc routine for the dribble router. */
/*****************************************************/
static int GetcDribble(
  void *theEnv,
  const char *logicalName)
  {
   int rv;

   /*===========================================*/
   /* Deactivate the dribble router and get the */
   /* character from another active router.     */
   /*===========================================*/

   EnvDeactivateRouter(theEnv,"dribble");
   rv = EnvGetcRouter(theEnv,logicalName);
   EnvActivateRouter(theEnv,"dribble");

   /*==========================================*/
   /* Put the character retrieved from another */
   /* router into the dribble buffer.          */
   /*==========================================*/

   PutcDribbleBuffer(theEnv,rv);

   /*=======================*/
   /* Return the character. */
   /*=======================*/

   return(rv);
  }

/***********************************************************/
/* PutcDribbleBuffer: Putc routine for the dribble router. */
/***********************************************************/
static void PutcDribbleBuffer(
  void *theEnv,
  int rv)
  {
   /*===================================================*/
   /* Receiving an end-of-file character will cause the */
   /* contents of the dribble buffer to be flushed.     */
   /*===================================================*/

   if (rv == EOF)
     {
      if (FileCommandData(theEnv)->DribbleCurrentPosition > 0)
        {
         fprintf(FileCommandData(theEnv)->DribbleFP,"%s",FileCommandData(theEnv)->DribbleBuffer);
         FileCommandData(theEnv)->DribbleCurrentPosition = 0;
         FileCommandData(theEnv)->DribbleBuffer[0] = EOS;
        }
     }

   /*===========================================================*/
   /* If we aren't receiving command input, then the character  */
   /* just received doesn't need to be placed in the dribble    */
   /* buffer--It can be written directly to the file. This will */
   /* occur for example when the command prompt is being        */
   /* printed (the AwaitingInput variable will be false because */
   /* command input has not been receivied yet). Before writing */
   /* the character to the file, the dribble buffer is flushed. */
   /*===========================================================*/

   else if (RouterData(theEnv)->AwaitingInput == false)
     {
      if (FileCommandData(theEnv)->DribbleCurrentPosition > 0)
        {
         fprintf(FileCommandData(theEnv)->DribbleFP,"%s",FileCommandData(theEnv)->DribbleBuffer);
         FileCommandData(theEnv)->DribbleCurrentPosition = 0;
         FileCommandData(theEnv)->DribbleBuffer[0] = EOS;
        }

      fputc(rv,FileCommandData(theEnv)->DribbleFP);
     }

   /*=====================================================*/
   /* Otherwise, add the character to the dribble buffer. */
   /*=====================================================*/

   else
     {
      FileCommandData(theEnv)->DribbleBuffer = ExpandStringWithChar(theEnv,rv,FileCommandData(theEnv)->DribbleBuffer,
                                           &FileCommandData(theEnv)->DribbleCurrentPosition,
                                           &FileCommandData(theEnv)->DribbleMaximumPosition,
                                           FileCommandData(theEnv)->DribbleMaximumPosition+BUFFER_SIZE);
     }
  }

/*********************************************************/
/* UngetcDribble: Ungetc routine for the dribble router. */
/*********************************************************/
static int UngetcDribble(
  void *theEnv,
  int ch,
  const char *logicalName)
  {
   int rv;

   /*===============================================*/
   /* Remove the character from the dribble buffer. */
   /*===============================================*/

   if (FileCommandData(theEnv)->DribbleCurrentPosition > 0) FileCommandData(theEnv)->DribbleCurrentPosition--;
   FileCommandData(theEnv)->DribbleBuffer[FileCommandData(theEnv)->DribbleCurrentPosition] = EOS;

   /*=============================================*/
   /* Deactivate the dribble router and pass the  */
   /* ungetc request to the other active routers. */
   /*=============================================*/

   EnvDeactivateRouter(theEnv,"dribble");
   rv = EnvUngetcRouter(theEnv,ch,logicalName);
   EnvActivateRouter(theEnv,"dribble");

   /*==========================================*/
   /* Return the result of the ungetc request. */
   /*==========================================*/

   return(rv);
  }

/*****************************************************/
/* ExitDribble: Exit routine for the dribble router. */
/*****************************************************/
static int ExitDribble(
  void *theEnv,
  int num)
  {
#if MAC_XCD
#pragma unused(num)
#endif

   if (FileCommandData(theEnv)->DribbleCurrentPosition > 0)
     { fprintf(FileCommandData(theEnv)->DribbleFP,"%s",FileCommandData(theEnv)->DribbleBuffer); }
 
   if (FileCommandData(theEnv)->DribbleFP != NULL) GenClose(theEnv,FileCommandData(theEnv)->DribbleFP);
   return(1);
  }

/******************************************/
/* DribbleOnCommand: H/L access routine   */
/*   for the dribble-on command.          */
/******************************************/
void DribbleOnCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *fileName;

   if ((fileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   mCVSetBoolean(returnValue,EnvDribbleOn(UDFContextEnvironment(context),fileName));
  }

/**********************************/
/* EnvDribbleOn: C access routine */
/*   for the dribble-on command.  */
/**********************************/
bool EnvDribbleOn(
  void *theEnv,
  const char *fileName)
  {
   /*==============================*/
   /* If a dribble file is already */
   /* open, then close it.         */
   /*==============================*/

   if (FileCommandData(theEnv)->DribbleFP != NULL)
     { EnvDribbleOff(theEnv); }

   /*========================*/
   /* Open the dribble file. */
   /*========================*/

   FileCommandData(theEnv)->DribbleFP = GenOpen(theEnv,fileName,"w");
   if (FileCommandData(theEnv)->DribbleFP == NULL)
     {
      OpenErrorMessage(theEnv,"dribble-on",fileName);
      return(false);
     }

   /*============================*/
   /* Create the dribble router. */
   /*============================*/

   EnvAddRouter(theEnv,"dribble", 40,
             FindDribble, PrintDribble,
             GetcDribble, UngetcDribble,
             ExitDribble);

   FileCommandData(theEnv)->DribbleCurrentPosition = 0;

   /*================================================*/
   /* Call the dribble status function. This is used */
   /* by some of the machine specific interfaces to  */
   /* do things such as changing the wording of menu */
   /* items from "Turn Dribble On..." to             */
   /* "Turn Dribble Off..."                          */
   /*================================================*/

   if (FileCommandData(theEnv)->DribbleStatusFunction != NULL)
     { (*FileCommandData(theEnv)->DribbleStatusFunction)(theEnv,true); }

   /*=====================================*/
   /* Return true to indicate the dribble */
   /* file was successfully opened.       */
   /*=====================================*/

   return(true);
  }

/*************************************************/
/* EnvDribbleActive: Returns true if the dribble */
/*   router is active, otherwise false.          */
/*************************************************/
bool EnvDribbleActive(
  void *theEnv)
  {
   if (FileCommandData(theEnv)->DribbleFP != NULL) return(true);

   return(false);
  }

/*******************************************/
/* DribbleOffCommand: H/L access  routine  */
/*   for the dribble-off command.          */
/*******************************************/
void DribbleOffCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   mCVSetBoolean(returnValue,EnvDribbleOff(UDFContextEnvironment(context)));
  }

/***********************************/
/* EnvDribbleOff: C access routine */
/*   for the dribble-off command.  */
/***********************************/
bool EnvDribbleOff(
  void *theEnv)
  {
   bool rv = false;

   /*================================================*/
   /* Call the dribble status function. This is used */
   /* by some of the machine specific interfaces to  */
   /* do things such as changing the wording of menu */
   /* items from "Turn Dribble On..." to             */
   /* "Turn Dribble Off..."                          */
   /*================================================*/

   if (FileCommandData(theEnv)->DribbleStatusFunction != NULL)
     { (*FileCommandData(theEnv)->DribbleStatusFunction)(theEnv,false); }

   /*=======================================*/
   /* Close the dribble file and deactivate */
   /* the dribble router.                   */
   /*=======================================*/

   if (FileCommandData(theEnv)->DribbleFP != NULL)
     {
      if (FileCommandData(theEnv)->DribbleCurrentPosition > 0)
        { fprintf(FileCommandData(theEnv)->DribbleFP,"%s",FileCommandData(theEnv)->DribbleBuffer); }
      EnvDeleteRouter(theEnv,"dribble");
      if (GenClose(theEnv,FileCommandData(theEnv)->DribbleFP) == 0) rv = true;
     }
   else
     { rv = true; }

   FileCommandData(theEnv)->DribbleFP = NULL;

   /*============================================*/
   /* Free the space used by the dribble buffer. */
   /*============================================*/

   if (FileCommandData(theEnv)->DribbleBuffer != NULL)
     {
      rm(theEnv,FileCommandData(theEnv)->DribbleBuffer,FileCommandData(theEnv)->DribbleMaximumPosition);
      FileCommandData(theEnv)->DribbleBuffer = NULL;
     }

   FileCommandData(theEnv)->DribbleCurrentPosition = 0;
   FileCommandData(theEnv)->DribbleMaximumPosition = 0;

   /*============================================*/
   /* Return true if the dribble file was closed */
   /* without error, otherwise return false.     */
   /*============================================*/

   return(rv);
  }

/*****************************************************/
/* SetDribbleStatusFunction: Sets the function which */
/*   is called whenever the dribble router is turned */
/*   on or off.                                      */
/*****************************************************/
void SetDribbleStatusFunction(
  void *theEnv,
  int (*fnptr)(void *,bool))
  {
   FileCommandData(theEnv)->DribbleStatusFunction = fnptr;
  }

#endif /* DEBUGGING_FUNCTIONS */

/*************************************************/
/* FindBatch: Find routine for the batch router. */
/*************************************************/
static bool FindBatch(
  void *theEnv,
  const char *logicalName)
  {
#if MAC_XCD
#pragma unused(theEnv)
#endif

   if (strcmp(logicalName,STDIN) == 0)
     { return(true); }

   return(false);
  }

/*************************************************/
/* GetcBatch: Getc routine for the batch router. */
/*************************************************/
static int GetcBatch(
  void *theEnv,
  const char *logicalName)
  {
   return(LLGetcBatch(theEnv,logicalName,false));
  }

/***************************************************/
/* LLGetcBatch: Lower level routine for retrieving */
/*   a character when a batch file is active.      */
/***************************************************/
int LLGetcBatch(
  void *theEnv,
  const char *logicalName,
  bool returnOnEOF)
  {
   int rv = EOF, flag = 1;

   /*=================================================*/
   /* Get a character until a valid character appears */
   /* or no more batch files are left.                */
   /*=================================================*/

   while ((rv == EOF) && (flag == 1))
     {
      if (FileCommandData(theEnv)->BatchType == FILE_BATCH)
        { rv = getc((FILE *) FileCommandData(theEnv)->BatchSource); } 
      else
        { rv = EnvGetcRouter(theEnv,(char *) FileCommandData(theEnv)->BatchSource); }

      if (rv == EOF)
        {
         if (FileCommandData(theEnv)->BatchCurrentPosition > 0) EnvPrintRouter(theEnv,STDOUT,(char *) FileCommandData(theEnv)->BatchBuffer);
         flag = RemoveBatch(theEnv);
        }
     }

   /*=========================================================*/
   /* If the character retrieved is an end-of-file character, */
   /* then there are no batch files with character input      */
   /* remaining. Remove the batch router.                     */
   /*=========================================================*/

   if (rv == EOF)
     {
      if (FileCommandData(theEnv)->BatchCurrentPosition > 0) EnvPrintRouter(theEnv,STDOUT,(char *) FileCommandData(theEnv)->BatchBuffer);
      EnvDeleteRouter(theEnv,"batch");
      RemoveBatch(theEnv);
      if (returnOnEOF == true)
        { return (EOF); }
      else
        { return(EnvGetcRouter(theEnv,logicalName)); }
     }

   /*========================================*/
   /* Add the character to the batch buffer. */
   /*========================================*/

   FileCommandData(theEnv)->BatchBuffer = ExpandStringWithChar(theEnv,(char) rv,FileCommandData(theEnv)->BatchBuffer,&FileCommandData(theEnv)->BatchCurrentPosition,
                                      &FileCommandData(theEnv)->BatchMaximumPosition,FileCommandData(theEnv)->BatchMaximumPosition+BUFFER_SIZE);

   /*======================================*/
   /* If a carriage return is encountered, */
   /* then flush the batch buffer.         */
   /*======================================*/

   if ((char) rv == '\n')
     {
      EnvPrintRouter(theEnv,STDOUT,(char *) FileCommandData(theEnv)->BatchBuffer);
      FileCommandData(theEnv)->BatchCurrentPosition = 0;
      if ((FileCommandData(theEnv)->BatchBuffer != NULL) && (FileCommandData(theEnv)->BatchMaximumPosition > BUFFER_SIZE))
        {
         rm(theEnv,FileCommandData(theEnv)->BatchBuffer,FileCommandData(theEnv)->BatchMaximumPosition);
         FileCommandData(theEnv)->BatchMaximumPosition = 0;
         FileCommandData(theEnv)->BatchBuffer = NULL;
        }
     }

   /*=============================*/
   /* Increment the line counter. */
   /*=============================*/
   
   if (((char) rv == '\r') || ((char) rv == '\n'))
     { IncrementLineCount(theEnv); }

   /*=====================================================*/
   /* Return the character retrieved from the batch file. */
   /*=====================================================*/

   return(rv);
  }

/*****************************************************/
/* UngetcBatch: Ungetc routine for the batch router. */
/*****************************************************/
static int UngetcBatch(
  void *theEnv,
  int ch,
  const char *logicalName)
  {
#if MAC_XCD
#pragma unused(logicalName)
#endif

   if (FileCommandData(theEnv)->BatchCurrentPosition > 0) FileCommandData(theEnv)->BatchCurrentPosition--;
   if (FileCommandData(theEnv)->BatchBuffer != NULL) FileCommandData(theEnv)->BatchBuffer[FileCommandData(theEnv)->BatchCurrentPosition] = EOS;
   if (FileCommandData(theEnv)->BatchType == FILE_BATCH)
     { return(ungetc(ch,(FILE *) FileCommandData(theEnv)->BatchSource)); }

   return(EnvUngetcRouter(theEnv,ch,(char *) FileCommandData(theEnv)->BatchSource));
  }

/*************************************************/
/* ExitBatch: Exit routine for the batch router. */
/*************************************************/
static int ExitBatch(
  void *theEnv,
  int num)
  {
#if MAC_XCD
#pragma unused(num)
#endif
   CloseAllBatchSources(theEnv);
   return(1);
  }

/**************************************/
/* BatchCommand: H/L access routine   */
/*   for the batch command.           */
/**************************************/
void BatchCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *fileName;

   if ((fileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   mCVSetBoolean(returnValue,OpenBatch(UDFContextEnvironment(context),fileName,false));
  }

/**************************************************/
/* Batch: C access routine for the batch command. */
/**************************************************/
bool Batch(
  void *theEnv,
  const char *fileName)
  { return(OpenBatch(theEnv,fileName,false)); }

/***********************************************/
/* OpenBatch: Adds a file to the list of files */
/*   opened with the batch command.            */
/***********************************************/
bool OpenBatch(
  void *theEnv,
  const char *fileName,
  bool placeAtEnd)
  {
   FILE *theFile;

   /*======================*/
   /* Open the batch file. */
   /*======================*/

   theFile = GenOpen(theEnv,fileName,"r");

   if (theFile == NULL)
     {
      OpenErrorMessage(theEnv,"batch",fileName);
      return(false);
     }

   /*============================*/
   /* Create the batch router if */
   /* it doesn't already exist.  */
   /*============================*/

   if (FileCommandData(theEnv)->TopOfBatchList == NULL)
     {
      EnvAddRouter(theEnv,"batch", 20,
                 FindBatch, NULL,
                 GetcBatch, UngetcBatch,
                 ExitBatch);
     }

   /*===============================================================*/
   /* If a batch file is already open, save its current line count. */
   /*===============================================================*/

   if (FileCommandData(theEnv)->TopOfBatchList != NULL)
     { FileCommandData(theEnv)->TopOfBatchList->lineNumber = GetLineCount(theEnv); }

#if (! RUN_TIME) && (! BLOAD_ONLY)

   /*========================================================================*/
   /* If this is the first batch file, remember the prior parsing file name. */
   /*========================================================================*/
   
   if (FileCommandData(theEnv)->TopOfBatchList == NULL)
     { FileCommandData(theEnv)->batchPriorParsingFile = CopyString(theEnv,EnvGetParsingFileName(theEnv)); }
     
   /*=======================================================*/
   /* Create the error capture router if it does not exist. */
   /*=======================================================*/
   
   EnvSetParsingFileName(theEnv,fileName);
   SetLineCount(theEnv,0);

   CreateErrorCaptureRouter(theEnv);
#endif

   /*====================================*/
   /* Add the newly opened batch file to */
   /* the list of batch files opened.    */
   /*====================================*/

   AddBatch(theEnv,placeAtEnd,(void *) theFile,FILE_BATCH,NULL,fileName);

   /*===================================*/
   /* Return true to indicate the batch */
   /* file was successfully opened.     */
   /*===================================*/

   return(true);
  }

/*****************************************************************/
/* OpenStringBatch: Opens a string source for batch processing.  */
/*   The memory allocated for the argument stringName must be    */
/*   deallocated by the user. The memory allocated for theString */
/*   will be deallocated by the batch routines when batch        */
/*   processing for the  string is completed.                    */
/*****************************************************************/
bool OpenStringBatch(
  void *theEnv,
  const char *stringName,
  const char *theString,
  bool placeAtEnd)
  {
   if (OpenStringSource(theEnv,stringName,theString,0) == false)
     { return(false); }

   if (FileCommandData(theEnv)->TopOfBatchList == NULL)
     {
      EnvAddRouter(theEnv,"batch", 20,
                 FindBatch, NULL,
                 GetcBatch, UngetcBatch,
                 ExitBatch);
     }

   AddBatch(theEnv,placeAtEnd,(void *) stringName,STRING_BATCH,theString,NULL);

   return(true);
  }

/*******************************************************/
/* AddBatch: Creates the batch file data structure and */
/*   adds it to the list of opened batch files.        */
/*******************************************************/
static void AddBatch(
  void *theEnv,
  bool placeAtEnd,
  void *theSource,
  int type,
  const char *theString,
  const char *theFileName)
  {
   struct batchEntry *bptr;

   /*=========================*/
   /* Create the batch entry. */
   /*=========================*/

   bptr = get_struct(theEnv,batchEntry);
   bptr->batchType = type;
   bptr->inputSource = theSource;
   bptr->theString = theString;
   bptr->fileName = CopyString(theEnv,theFileName);
   bptr->lineNumber = 0;
   bptr->next = NULL;

   /*============================*/
   /* Add the entry to the list. */
   /*============================*/

   if (FileCommandData(theEnv)->TopOfBatchList == NULL)
     {
      FileCommandData(theEnv)->TopOfBatchList = bptr;
      FileCommandData(theEnv)->BottomOfBatchList = bptr;
      FileCommandData(theEnv)->BatchType = type;
      FileCommandData(theEnv)->BatchSource = theSource;
      FileCommandData(theEnv)->BatchCurrentPosition = 0;
     }
   else if (placeAtEnd == false)
     {
      bptr->next = FileCommandData(theEnv)->TopOfBatchList;
      FileCommandData(theEnv)->TopOfBatchList = bptr;
      FileCommandData(theEnv)->BatchType = type;
      FileCommandData(theEnv)->BatchSource = theSource;
      FileCommandData(theEnv)->BatchCurrentPosition = 0;
     }
   else
     {
      FileCommandData(theEnv)->BottomOfBatchList->next = bptr;
      FileCommandData(theEnv)->BottomOfBatchList = bptr;
     }
  }

/******************************************************************/
/* RemoveBatch: Removes the top entry on the list of batch files. */
/******************************************************************/
bool RemoveBatch(
  void *theEnv)
  {
   struct batchEntry *bptr;
   bool rv, fileBatch = false;

   if (FileCommandData(theEnv)->TopOfBatchList == NULL) return(false);

   /*==================================================*/
   /* Close the source from which batch input is read. */
   /*==================================================*/

   if (FileCommandData(theEnv)->TopOfBatchList->batchType == FILE_BATCH)
     {
      fileBatch = true;
      GenClose(theEnv,(FILE *) FileCommandData(theEnv)->TopOfBatchList->inputSource);
#if (! RUN_TIME) && (! BLOAD_ONLY)
      FlushParsingMessages(theEnv);
      DeleteErrorCaptureRouter(theEnv);
#endif
     }
   else
     {
      CloseStringSource(theEnv,(char *) FileCommandData(theEnv)->TopOfBatchList->inputSource);
      rm(theEnv,(void *) FileCommandData(theEnv)->TopOfBatchList->theString,
         strlen(FileCommandData(theEnv)->TopOfBatchList->theString) + 1);
     }

   /*=================================*/
   /* Remove the entry from the list. */
   /*=================================*/

   DeleteString(theEnv,(char *) FileCommandData(theEnv)->TopOfBatchList->fileName);
   bptr = FileCommandData(theEnv)->TopOfBatchList;
   FileCommandData(theEnv)->TopOfBatchList = FileCommandData(theEnv)->TopOfBatchList->next;

   rtn_struct(theEnv,batchEntry,bptr);

   /*========================================================*/
   /* If there are no batch files remaining to be processed, */
   /* then free the space used by the batch buffer.          */
   /*========================================================*/

   if (FileCommandData(theEnv)->TopOfBatchList == NULL)
     {
      FileCommandData(theEnv)->BottomOfBatchList = NULL;
      FileCommandData(theEnv)->BatchSource = NULL;
      if (FileCommandData(theEnv)->BatchBuffer != NULL)
        {
         rm(theEnv,FileCommandData(theEnv)->BatchBuffer,FileCommandData(theEnv)->BatchMaximumPosition);
         FileCommandData(theEnv)->BatchBuffer = NULL;
        }
      FileCommandData(theEnv)->BatchCurrentPosition = 0;
      FileCommandData(theEnv)->BatchMaximumPosition = 0;
      rv = false;

#if (! RUN_TIME) && (! BLOAD_ONLY)
      if (fileBatch)
        {
         EnvSetParsingFileName(theEnv,FileCommandData(theEnv)->batchPriorParsingFile);
         DeleteString(theEnv,FileCommandData(theEnv)->batchPriorParsingFile);
         FileCommandData(theEnv)->batchPriorParsingFile = NULL;
        }
#endif
     }

   /*===========================================*/
   /* Otherwise move on to the next batch file. */
   /*===========================================*/

   else
     {
      FileCommandData(theEnv)->BatchType = FileCommandData(theEnv)->TopOfBatchList->batchType;
      FileCommandData(theEnv)->BatchSource = FileCommandData(theEnv)->TopOfBatchList->inputSource;
      FileCommandData(theEnv)->BatchCurrentPosition = 0;
      rv = true;
#if (! RUN_TIME) && (! BLOAD_ONLY)
      if (FileCommandData(theEnv)->TopOfBatchList->batchType == FILE_BATCH)
        { EnvSetParsingFileName(theEnv,FileCommandData(theEnv)->TopOfBatchList->fileName); }
        
      SetLineCount(theEnv,FileCommandData(theEnv)->TopOfBatchList->lineNumber);
#endif
     }

   /*====================================================*/
   /* Return true if a batch file if there are remaining */
   /* batch files to be processed, otherwise false.      */
   /*====================================================*/

   return(rv);
  }

/****************************************/
/* BatchActive: Returns true if a batch */
/*   file is open, otherwise false.     */
/****************************************/
bool BatchActive(
  void *theEnv)
  {
   if (FileCommandData(theEnv)->TopOfBatchList != NULL) return(true);

   return(false);
  }

/******************************************************/
/* CloseAllBatchSources: Closes all open batch files. */
/******************************************************/
void CloseAllBatchSources(
  void *theEnv)
  {   
   /*================================================*/
   /* Free the batch buffer if it contains anything. */
   /*================================================*/

   if (FileCommandData(theEnv)->BatchBuffer != NULL)
     {
      if (FileCommandData(theEnv)->BatchCurrentPosition > 0) EnvPrintRouter(theEnv,STDOUT,(char *) FileCommandData(theEnv)->BatchBuffer);
      rm(theEnv,FileCommandData(theEnv)->BatchBuffer,FileCommandData(theEnv)->BatchMaximumPosition);
      FileCommandData(theEnv)->BatchBuffer = NULL;
      FileCommandData(theEnv)->BatchCurrentPosition = 0;
      FileCommandData(theEnv)->BatchMaximumPosition = 0;
     }

   /*==========================*/
   /* Delete the batch router. */
   /*==========================*/

   EnvDeleteRouter(theEnv,"batch");

   /*=====================================*/
   /* Close each of the open batch files. */
   /*=====================================*/

   while (RemoveBatch(theEnv))
     { /* Do Nothing */ }
  }

/******************************************/
/* BatchStarCommand: H/L access routine   */
/*   for the batch* command.              */
/******************************************/
void BatchStarCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *fileName;

   if ((fileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }
     
   mCVSetBoolean(returnValue,EnvBatchStar(UDFContextEnvironment(context),fileName));
  }

#if ! RUN_TIME

/**********************************************************/
/* EnvBatchStar: C access routine for the batch* command. */
/**********************************************************/
bool EnvBatchStar(
  void *theEnv,
  const char *fileName)
  {
   int inchar;
   FILE *theFile;
   char *theString = NULL;
   size_t position = 0;
   size_t maxChars = 0;
#if (! RUN_TIME) && (! BLOAD_ONLY)
   char *oldParsingFileName;
   long oldLineCountValue;
#endif
   /*======================*/
   /* Open the batch file. */
   /*======================*/

   theFile = GenOpen(theEnv,fileName,"r");

   if (theFile == NULL)
     {
      OpenErrorMessage(theEnv,"batch",fileName);
      return(false);
     }

   /*======================================*/
   /* Setup for capturing errors/warnings. */
   /*======================================*/

#if (! RUN_TIME) && (! BLOAD_ONLY)
   oldParsingFileName = CopyString(theEnv,EnvGetParsingFileName(theEnv));
   EnvSetParsingFileName(theEnv,fileName);

   CreateErrorCaptureRouter(theEnv);
     
   oldLineCountValue = SetLineCount(theEnv,1);
#endif

   /*========================*/
   /* Reset the error state. */
   /*========================*/

   EnvSetHaltExecution(theEnv,false);
   EnvSetEvaluationError(theEnv,false);

   /*=============================================*/
   /* Evaluate commands from the file one by one. */
   /*=============================================*/

   while ((inchar = getc(theFile)) != EOF)
     {
      theString = ExpandStringWithChar(theEnv,inchar,theString,&position,
                                       &maxChars,maxChars+80);

      if (CompleteCommand(theString) != 0)
        {
         FlushPPBuffer(theEnv);
         SetPPBufferStatus(theEnv,false);
         RouteCommand(theEnv,theString,false);
         FlushPPBuffer(theEnv);
         EnvSetHaltExecution(theEnv,false);
         EnvSetEvaluationError(theEnv,false);
         FlushBindList(theEnv);      
         genfree(theEnv,theString,(unsigned) maxChars);
         theString = NULL;
         maxChars = 0;
         position = 0;
#if (! RUN_TIME) && (! BLOAD_ONLY)
         FlushParsingMessages(theEnv);
#endif
        }
        
      if ((inchar == '\r') || (inchar == '\n'))
        { IncrementLineCount(theEnv); }
     }

   if (theString != NULL)
     { genfree(theEnv,theString,(unsigned) maxChars); }
     
   /*=======================*/
   /* Close the batch file. */
   /*=======================*/

   GenClose(theEnv,theFile);

   /*========================================*/
   /* Cleanup for capturing errors/warnings. */
   /*========================================*/

#if (! RUN_TIME) && (! BLOAD_ONLY)
   FlushParsingMessages(theEnv);
   DeleteErrorCaptureRouter(theEnv);
     
   SetLineCount(theEnv,oldLineCountValue);
   
   EnvSetParsingFileName(theEnv,oldParsingFileName);
   DeleteString(theEnv,oldParsingFileName);
#endif

   return(true);
  }

#else

/**************************************************/
/* EnvBatchStar: This is the non-functional stub  */
/*   provided for use with a run-time version.    */
/**************************************************/
bool EnvBatchStar(
  void *theEnv,
  const char *fileName)
  {
   PrintErrorID(theEnv,"FILECOM",1,false);
   EnvPrintRouter(theEnv,WERROR,"Function batch* does not work in run time modules.\n");
   return(false);
  }

#endif

/***********************************************************/
/* LoadCommand: H/L access routine for the load command.   */
/***********************************************************/
void LoadCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   Environment *theEnv = UDFContextEnvironment(context);
#if (! BLOAD_ONLY) && (! RUN_TIME)
   const char *theFileName;
   int rv;

   if ((theFileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   SetPrintWhileLoading(theEnv,true);

   if ((rv = EnvLoad(theEnv,theFileName)) == 0)
     {
      SetPrintWhileLoading(theEnv,false);
      OpenErrorMessage(theEnv,"load",theFileName);
      mCVSetBoolean(returnValue,false);
      return;
     }

   SetPrintWhileLoading(theEnv,false);
   
   if (rv == -1) mCVSetBoolean(returnValue,false);
   else mCVSetBoolean(returnValue,true);
#else
   EnvPrintRouter(theEnv,WDIALOG,"Load is not available in this environment\n");
   mCVSetBoolean(returnValue,false);
#endif
  }

/****************************************************************/
/* LoadStarCommand: H/L access routine for the load* command.   */
/****************************************************************/
void LoadStarCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   Environment *theEnv = UDFContextEnvironment(context);
#if (! BLOAD_ONLY) && (! RUN_TIME)
   const char *theFileName;
   int rv;

   if ((theFileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   if ((rv = EnvLoad(theEnv,theFileName)) == 0) // TBD Load Code
     {
      OpenErrorMessage(theEnv,"load*",theFileName);
      mCVSetBoolean(returnValue,false);
      return;
     }

   if (rv == -1) mCVSetBoolean(returnValue,false);
   else mCVSetBoolean(returnValue,true);
#else
   EnvPrintRouter(theEnv,WDIALOG,"Load* is not available in this environment\n");
   mCVSetBoolean(returnValue,false);
#endif
  }

#if DEBUGGING_FUNCTIONS
/*********************************************************/
/* SaveCommand: H/L access routine for the save command. */
/*********************************************************/
void SaveCommand(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   Environment *theEnv = UDFContextEnvironment(context);
#if (! BLOAD_ONLY) && (! RUN_TIME)
   const char *theFileName;

   if ((theFileName = GetFileName(context)) == NULL) 
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   if (EnvSave(theEnv,theFileName) == false)
     {
      OpenErrorMessage(theEnv,"save",theFileName);
      mCVSetBoolean(returnValue,false);
      return;
     }

   mCVSetBoolean(returnValue,true);
#else
   EnvPrintRouter(theEnv,WDIALOG,"Save is not available in this environment\n");
   mCVSetBoolean(returnValue,false);
#endif
  }
#endif



