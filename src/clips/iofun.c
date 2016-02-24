   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                I/O FUNCTIONS MODULE                 */
   /*******************************************************/

/*************************************************************/
/* Purpose: Contains the code for several I/O functions      */
/*   including printout, read, open, close, remove, rename,  */
/*   format, and readline.                                   */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*      Gary D. Riley                                        */
/*      Bebe Ly                                              */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Added the get-char, set-locale, and            */
/*            read-number functions.                         */
/*                                                           */
/*            Modified printing of floats in the format      */
/*            function to use the locale from the set-locale */
/*            function.                                      */
/*                                                           */
/*            Moved IllegalLogicalNameMessage function to    */
/*            argacces.c.                                    */
/*                                                           */
/*      6.30: Changed integer type/precision.                */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Removed the undocumented use of t in the       */
/*            printout command to perform the same function  */
/*            as crlf.                                       */
/*                                                           */
/*            Replaced EXT_IO and BASIC_IO compiler flags    */
/*            with IO_FUNCTIONS compiler flag.               */
/*                                                           */
/*            Added rb and ab and removed r+ modes for the   */
/*            open function.                                 */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Used gensprintf instead of sprintf.            */
/*                                                           */
/*            Added put-char function.                       */
/*                                                           */
/*            Added SetFullCRLF which allows option to       */
/*            specify crlf as \n or \r\n.                    */
/*                                                           */
/*            Added AwaitingInput flag.                      */
/*                                                           */             
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
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
/*            Modified ReadTokenFromStdin to capture         */
/*            carriage returns in the input buffer so that   */
/*            input buffer count will accurately reflect     */
/*            the number of characters typed for GUI         */
/*            interfaces that support deleting carriage      */
/*            returns.                                       */
/*                                                           */
/*            Added print and println functions.             */
/*                                                           */
/*************************************************************/

#include "setup.h"

#if IO_FUNCTIONS
#include <locale.h>
#include <stdlib.h>
#include <ctype.h>
#endif

#include <stdio.h>
#include <string.h>

#include "argacces.h"
#include "commline.h"
#include "constant.h"
#include "envrnmnt.h"
#include "extnfunc.h"
#include "filertr.h"
#include "memalloc.h"
#include "router.h"
#include "scanner.h"
#include "strngrtr.h"
#include "sysdep.h"
#include "utility.h"

#include "iofun.h"

/***************/
/* DEFINITIONS */
/***************/

#define FORMAT_MAX 512
#define FLAG_MAX    80

/********************/
/* ENVIRONMENT DATA */
/********************/

#define IO_FUNCTION_DATA 64

struct IOFunctionData
  { 
   void *locale;
   bool useFullCRLF;
  };

#define IOFunctionData(theEnv) ((struct IOFunctionData *) GetEnvironmentData(theEnv,IO_FUNCTION_DATA))

/****************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS  */
/****************************************/

#if IO_FUNCTIONS
   static void             ReadTokenFromStdin(void *,struct token *);
   static const char      *ControlStringCheck(void *,int);
   static char             FindFormatFlag(const char *,size_t *,char *,size_t);
   static const char      *PrintFormatFlag(void *,const char *,int,int);
   static char            *FillBuffer(void *,const char *,size_t *,size_t *);
   static void             ReadNumber(void *,const char *,struct token *,bool);
   static void             PrintDriver(UDFContext *,const char *,bool);
#endif

/**************************************/
/* IOFunctionDefinitions: Initializes */
/*   the I/O functions.               */
/**************************************/
void IOFunctionDefinitions(
  void *theEnv)
  {
   AllocateEnvironmentData(theEnv,IO_FUNCTION_DATA,sizeof(struct IOFunctionData),NULL);

#if IO_FUNCTIONS
   IOFunctionData(theEnv)->useFullCRLF = false;
   IOFunctionData(theEnv)->locale = (SYMBOL_HN *) EnvAddSymbol(theEnv,setlocale(LC_ALL,NULL));
   IncrementSymbolCount(IOFunctionData(theEnv)->locale);
#endif

#if ! RUN_TIME
#if IO_FUNCTIONS
   EnvAddUDF(theEnv,"printout",    "v",        PrintoutFunction,  "PrintoutFunction",  1,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"print",       "v",        PrintFunction,     "PrintFunction",     0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"println",     "v",        PrintlnFunction,   "PrintlnFunction",   0,UNBOUNDED,NULL,NULL);
   EnvAddUDF(theEnv,"read",        "synldfie", ReadFunction,      "ReadFunction",      0,1,NULL,NULL);
   EnvAddUDF(theEnv,"open",        "b",        OpenFunction,      "OpenFunction",      2,3,"*;sy",NULL);
   EnvAddUDF(theEnv,"close",       "b",        CloseFunction,     "CloseFunction",     0,1,NULL,NULL);
   EnvAddUDF(theEnv,"get-char",    "l",        GetCharFunction,   "GetCharFunction",   0,1,NULL,NULL);
   EnvAddUDF(theEnv,"put-char",    "v",        PutCharFunction,   "PutCharFunction",   1,2,NULL,NULL);
   EnvAddUDF(theEnv,"remove",      "b",        RemoveFunction,    "RemoveFunction",    1,1,"sy",NULL);
   EnvAddUDF(theEnv,"rename",      "b",        RenameFunction,    "RenameFunction",    2,2,"sy",NULL);
   EnvAddUDF(theEnv,"format",      "s",        FormatFunction,    "FormatFunction",    2,UNBOUNDED, "*;*;s",NULL);
   EnvAddUDF(theEnv,"readline",    "sy",       ReadlineFunction,  "ReadlineFunction",  0,1,NULL,NULL);
   EnvAddUDF(theEnv,"set-locale",  "sy",       SetLocaleFunction, "SetLocaleFunction", 0,1,NULL,NULL);
   EnvAddUDF(theEnv,"read-number", "syld",     ReadNumberFunction, "ReadNumberFunction", 0,1,NULL,NULL);
#endif
#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif
  }

#if IO_FUNCTIONS

/******************************************/
/* PrintoutFunction: H/L access routine   */
/*   for the printout function.           */
/******************************************/
void PrintoutFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *logicalName;
   Environment *theEnv = UDFContextEnvironment(context);

   /*=====================================================*/
   /* Get the logical name to which output is to be sent. */
   /*=====================================================*/

   logicalName = GetLogicalName(context,STDOUT);
   if (logicalName == NULL)
     {
      IllegalLogicalNameMessage(theEnv,"printout");
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      return;
     }

   /*============================================================*/
   /* Determine if any router recognizes the output destination. */
   /*============================================================*/

   if (strcmp(logicalName,"nil") == 0)
     { return; }
   else if (QueryRouters(theEnv,logicalName) == false)
     {
      UnrecognizedRouterMessage(theEnv,logicalName);
      return;
     }

   /*========================*/
   /* Call the print driver. */
   /*========================*/
   
   PrintDriver(context,logicalName,false);
  }

/*************************************/
/* PrintFunction: H/L access routine */
/*   for the print function.         */
/*************************************/
void PrintFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   PrintDriver(context,STDOUT,false);
  }

/*************************************/
/* PrintlnFunction: H/L access routine */
/*   for the println function.         */
/*************************************/
void PrintlnFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   PrintDriver(context,STDOUT,true);
  }

/*************************************************/
/* PrintDriver: Driver routine for the printout, */
/*   print, and println functions.               */
/*************************************************/
static void PrintDriver(
  UDFContext *context,
  const char *logicalName,
  bool endCRLF)
  {
   CLIPSValue theArg;
   Environment *theEnv = UDFContextEnvironment(context);

   /*==============================*/
   /* Print each of the arguments. */
   /*==============================*/

   while (UDFHasNextArgument(context))
     {
      if (! UDFNextArgument(context,ANY_TYPE,&theArg))
        { break; }
      
      if (EvaluationData(theEnv)->HaltExecution) break;

      switch(GetType(theArg))
        {
         case SYMBOL:
           if (strcmp(mCVToString(&theArg),"crlf") == 0)
             {    
              if (IOFunctionData(theEnv)->useFullCRLF)
                { EnvPrintRouter(theEnv,logicalName,"\r\n"); }
              else
                { EnvPrintRouter(theEnv,logicalName,"\n"); }
             }
           else if (strcmp(mCVToString(&theArg),"tab") == 0)
             { EnvPrintRouter(theEnv,logicalName,"\t"); }
           else if (strcmp(mCVToString(&theArg),"vtab") == 0)
             { EnvPrintRouter(theEnv,logicalName,"\v"); }
           else if (strcmp(mCVToString(&theArg),"ff") == 0)
             { EnvPrintRouter(theEnv,logicalName,"\f"); }
           else
             { EnvPrintRouter(theEnv,logicalName,mCVToString(&theArg)); }
           break;

         case STRING:
           EnvPrintRouter(theEnv,logicalName,mCVToString(&theArg));
           break;

         default:
           PrintDataObject(theEnv,logicalName,&theArg);
           break;
        }
     }
     
   if (endCRLF)
     {
      if (IOFunctionData(theEnv)->useFullCRLF)
        { EnvPrintRouter(theEnv,logicalName,"\r\n"); }
      else
        { EnvPrintRouter(theEnv,logicalName,"\n"); }
     }
  }

/*****************************************************/
/* SetFullCRLF: Set the flag which indicates whether */
/*   crlf is treated just as '\n' or '\r\n'.         */
/*****************************************************/
bool SetFullCRLF(
  void *theEnv,
  bool value)
  {
   bool oldValue = IOFunctionData(theEnv)->useFullCRLF;
   
   IOFunctionData(theEnv)->useFullCRLF = value;
   
   return(oldValue);
  }

/*************************************************************/
/* ReadFunction: H/L access routine for the read function.   */
/*************************************************************/
void ReadFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   struct token theToken;
   const char *logicalName = NULL;
   Environment *theEnv = UDFContextEnvironment(context);

   /*======================================================*/
   /* Determine the logical name from which input is read. */
   /*======================================================*/

   if (! UDFHasNextArgument(context))
     { logicalName = STDIN; }
   else
     {
      logicalName = GetLogicalName(context,STDIN);
      if (logicalName == NULL)
        {
         IllegalLogicalNameMessage(theEnv,"read");
         EnvSetHaltExecution(theEnv,true);
         EnvSetEvaluationError(theEnv,true);
         mCVSetString(returnValue,"*** READ ERROR ***");
         return;
        }
     }

   /*============================================*/
   /* Check to see that the logical name exists. */
   /*============================================*/

   if (QueryRouters(theEnv,logicalName) == false)
     {
      UnrecognizedRouterMessage(theEnv,logicalName);
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      mCVSetString(returnValue,"*** READ ERROR ***");
      return;
     }

   /*=======================================*/
   /* Collect input into string if the read */
   /* source is stdin, else just get token. */
   /*=======================================*/

   if (strcmp(logicalName,STDIN) == 0)
     { ReadTokenFromStdin(theEnv,&theToken); }
   else
     { GetToken(theEnv,logicalName,&theToken); }

   RouterData(theEnv)->CommandBufferInputCount = 0;
   RouterData(theEnv)->AwaitingInput = false;

   /*====================================================*/
   /* Copy the token to the return value data structure. */
   /*====================================================*/

   returnValue->type = theToken.type;
   if ((theToken.type == FLOAT) || (theToken.type == STRING) ||
#if OBJECT_SYSTEM
       (theToken.type == INSTANCE_NAME) ||
#endif
       (theToken.type == SYMBOL) || (theToken.type == INTEGER))
     { returnValue->value = theToken.value; }
   else if (theToken.type == STOP)
     {
      returnValue->type = SYMBOL;
      returnValue->value = (void *) EnvAddSymbol(theEnv,"EOF");
     }
   else if (theToken.type == UNKNOWN_VALUE)
     {
      mCVSetString(returnValue,"*** READ ERROR ***");
     }
   else
     {
      returnValue->type = STRING;
      returnValue->value = (void *) EnvAddSymbol(theEnv,theToken.printForm);
     }

   return;
  }

/********************************************************/
/* ReadTokenFromStdin: Special routine used by the read */
/*   function to read a token from standard input.      */
/********************************************************/
static void ReadTokenFromStdin(
  void *theEnv,
  struct token *theToken)
  {
   char *inputString;
   size_t inputStringSize;
   int inchar;
   
   /*===========================================*/
   /* Initialize the variables used for storing */
   /* the characters retrieved from stdin.      */
   /*===========================================*/

   inputString = NULL;
   RouterData(theEnv)->CommandBufferInputCount = 0;
   RouterData(theEnv)->AwaitingInput = true;
   inputStringSize = 0;

   /*=============================================*/
   /* Continue processing until a token is found. */
   /*=============================================*/

   theToken->type = STOP;
   while (theToken->type == STOP)
     {
      /*========================================================*/
      /* Continue reading characters until a carriage return is */
      /* entered or the user halts execution (usually with      */
      /* control-c). Waiting for the carriage return prevents   */
      /* the input from being prematurely parsed (such as when  */
      /* a space is entered after a symbol has been typed).     */
      /*========================================================*/

      inchar = EnvGetcRouter(theEnv,STDIN);
     
      while ((inchar != '\n') && (inchar != '\r') && (inchar != EOF) &&
             (! EnvGetHaltExecution(theEnv)))
        {
         inputString = ExpandStringWithChar(theEnv,inchar,inputString,&RouterData(theEnv)->CommandBufferInputCount,
                                            &inputStringSize,inputStringSize + 80);
         inchar = EnvGetcRouter(theEnv,STDIN);
        }

      /*====================================================*/
      /* Add the final carriage return to the input buffer. */
      /*====================================================*/
      
      if  ((inchar == '\n') || (inchar == '\r'))
        {
         inputString = ExpandStringWithChar(theEnv,inchar,inputString,&RouterData(theEnv)->CommandBufferInputCount,
                                            &inputStringSize,inputStringSize + 80);
        }
        
      /*==================================================*/
      /* Open a string input source using the characters  */
      /* retrieved from stdin and extract the first token */
      /* contained in the string.                         */
      /*==================================================*/

      OpenStringSource(theEnv,"read",inputString,0);
      GetToken(theEnv,"read",theToken);
      CloseStringSource(theEnv,"read");

      /*===========================================*/
      /* Pressing control-c (or comparable action) */
      /* aborts the read function.                 */
      /*===========================================*/

      if (EnvGetHaltExecution(theEnv))
        {
         theToken->type = STRING;
         theToken->value = (void *) EnvAddSymbol(theEnv,"*** READ ERROR ***");
        }

      /*====================================================*/
      /* Return the EOF symbol if the end of file for stdin */
      /* has been encountered. This typically won't occur,  */
      /* but is possible (for example by pressing control-d */
      /* in the UNIX operating system).                     */
      /*====================================================*/

      if ((theToken->type == STOP) && (inchar == EOF))
        {
         theToken->type = SYMBOL;
         theToken->value = (void *) EnvAddSymbol(theEnv,"EOF");
        }
     }
     
   if (inputStringSize > 0) rm(theEnv,inputString,inputStringSize);
  }

/*************************************************************/
/* OpenFunction: H/L access routine for the open function.   */
/*************************************************************/
void OpenFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *fileName, *logicalName, *accessMode = NULL;
   CLIPSValue theArg;
   Environment *theEnv = UDFContextEnvironment(context);

   /*====================*/
   /* Get the file name. */
   /*====================*/

   if ((fileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*=======================================*/
   /* Get the logical name to be associated */
   /* with the opened file.                 */
   /*=======================================*/

   logicalName = GetLogicalName(context,NULL);
   if (logicalName == NULL)
     {
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      IllegalLogicalNameMessage(theEnv,"open");
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*==================================*/
   /* Check to see if the logical name */
   /* is already in use.               */
   /*==================================*/

   if (FindFile(theEnv,logicalName))
     {
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      PrintErrorID(theEnv,"IOFUN",2,false);
      EnvPrintRouter(theEnv,WERROR,"Logical name ");
      EnvPrintRouter(theEnv,WERROR,logicalName);
      EnvPrintRouter(theEnv,WERROR," already in use.\n");
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*===========================*/
   /* Get the file access mode. */
   /*===========================*/

   if (! UDFHasNextArgument(context))
     { accessMode = "r"; }
   else
     {
      if (! UDFNextArgument(context,STRING_TYPE,&theArg))
        { return; }
      accessMode = mCVToString(&theArg);
     }

   /*=====================================*/
   /* Check for a valid file access mode. */
   /*=====================================*/

   if ((strcmp(accessMode,"r") != 0) &&
       (strcmp(accessMode,"w") != 0) &&
       (strcmp(accessMode,"a") != 0) &&
       (strcmp(accessMode,"rb") != 0) &&
       (strcmp(accessMode,"wb") != 0) &&
       (strcmp(accessMode,"ab") != 0))
     {
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      ExpectedTypeError1(theEnv,"open",3,"string with value \"r\", \"w\", \"a\", \"rb\", \"wb\", or \"ab\"");
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*================================================*/
   /* Open the named file and associate it with the  */
   /* specified logical name. Return true if the     */
   /* file was opened successfully, otherwise false. */
   /*================================================*/

   mCVSetBoolean(returnValue,OpenAFile(theEnv,fileName,accessMode,logicalName));
  }

/***************************************************************/
/* CloseFunction: H/L access routine for the close function.   */
/***************************************************************/
void CloseFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *logicalName;
   Environment *theEnv = UDFContextEnvironment(context);

   /*=====================================================*/
   /* If no arguments are specified, then close all files */
   /* opened with the open command. Return true if all    */
   /* files were closed successfully, otherwise false.    */
   /*=====================================================*/

   if (! UDFHasNextArgument(context))
     {
      mCVSetBoolean(returnValue,CloseAllFiles(theEnv));
      return;
     }

   /*================================*/
   /* Get the logical name argument. */
   /*================================*/

   logicalName = GetLogicalName(context,NULL);
   if (logicalName == NULL)
     {
      IllegalLogicalNameMessage(theEnv,"close");
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*========================================================*/
   /* Close the file associated with the specified logical   */
   /* name. Return true if the file was closed successfully, */
   /* otherwise false.                                       */
   /*========================================================*/

   mCVSetBoolean(returnValue,CloseFile(theEnv,logicalName));
  }

/***************************************/
/* GetCharFunction: H/L access routine */
/*   for the get-char function.        */
/***************************************/
void GetCharFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *logicalName;
   Environment *theEnv = UDFContextEnvironment(context);

   if (! UDFHasNextArgument(context))
     { logicalName = STDIN; }
   else
     {
      logicalName = GetLogicalName(context,STDIN);
      if (logicalName == NULL)
        {
         IllegalLogicalNameMessage(theEnv,"get-char");
         EnvSetHaltExecution(theEnv,true);
         EnvSetEvaluationError(theEnv,true);
         mCVSetInteger(returnValue,-1);
         return;
        }
     }

   if (QueryRouters(theEnv,logicalName) == false)
     {
      UnrecognizedRouterMessage(theEnv,logicalName);
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      mCVSetInteger(returnValue,-1);
      return;
     }

   mCVSetInteger(returnValue,EnvGetcRouter(theEnv,logicalName));
  }

/***************************************/
/* PutCharFunction: H/L access routine */
/*   for the put-char function.        */
/***************************************/
void PutCharFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   int numberOfArguments;
   const char *logicalName;
   CLIPSValue theArg;
   long long theChar;
   FILE *theFile;
   Environment *theEnv = UDFContextEnvironment(context);

   numberOfArguments = UDFArgumentCount(context);
     
   /*=======================*/
   /* Get the logical name. */
   /*=======================*/
   
   if (numberOfArguments == 1)
     { logicalName = STDOUT; }
   else
     {
      logicalName = GetLogicalName(context,STDOUT);
      if (logicalName == NULL)
        {
         IllegalLogicalNameMessage(theEnv,"put-char");
         EnvSetHaltExecution(theEnv,true);
         EnvSetEvaluationError(theEnv,true);
         return;
        }
     }

   if (QueryRouters(theEnv,logicalName) == false)
     {
      UnrecognizedRouterMessage(theEnv,logicalName);
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      return;
     }

   /*===========================*/
   /* Get the character to put. */
   /*===========================*/
   
   if (! UDFNextArgument(context,INTEGER_TYPE,&theArg))
     { return; }
      
   theChar = mCVToInteger(&theArg);
   
   /*===================================================*/
   /* If the "fast load" option is being used, then the */
   /* logical name is actually a pointer to a file and  */
   /* we can bypass the router and directly output the  */
   /* value.                                            */
   /*===================================================*/
      
   theFile = FindFptr(theEnv,logicalName);
   if (theFile != NULL)
     { putc((int) theChar,theFile); }
  }

/****************************************/
/* RemoveFunction: H/L access routine   */
/*   for the remove function.           */
/****************************************/
void RemoveFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *theFileName;

   /*====================*/
   /* Get the file name. */
   /*====================*/

   if ((theFileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*==============================================*/
   /* Remove the file. Return true if the file was */
   /* sucessfully removed, otherwise false.        */
   /*==============================================*/

   mCVSetBoolean(returnValue,genremove(theFileName));
  }

/****************************************/
/* RenameFunction: H/L access routine   */
/*   for the rename function.           */
/****************************************/
void RenameFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   const char *oldFileName, *newFileName;

   /*===========================*/
   /* Check for the file names. */
   /*===========================*/

   if ((oldFileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   if ((newFileName = GetFileName(context)) == NULL)
     {
      mCVSetBoolean(returnValue,false);
      return;
     }

   /*==============================================*/
   /* Rename the file. Return true if the file was */
   /* sucessfully renamed, otherwise false.        */
   /*==============================================*/

   mCVSetBoolean(returnValue,genrename(oldFileName,newFileName));
  }

/****************************************/
/* FormatFunction: H/L access routine   */
/*   for the format function.           */
/****************************************/
void FormatFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   int argCount;
   size_t start_pos;
   const char *formatString;
   const char *logicalName;
   char formatFlagType;
   int  f_cur_arg = 3;
   size_t form_pos = 0;
   char percentBuffer[FLAG_MAX];
   char *fstr = NULL;
   size_t fmaxm = 0;
   size_t fpos = 0;
   void *hptr;
   const char *theString;
   Environment *theEnv = UDFContextEnvironment(context);

   /*======================================*/
   /* Set default return value for errors. */
   /*======================================*/

   hptr = EnvAddSymbol(theEnv,"");

   /*=========================================*/
   /* Format requires at least two arguments: */
   /* a logical name and a format string.     */
   /*=========================================*/

   argCount = UDFArgumentCount(context);

   /*========================================*/
   /* First argument must be a logical name. */
   /*========================================*/

   if ((logicalName = GetLogicalName(context,STDOUT)) == NULL)
     {
      IllegalLogicalNameMessage(theEnv,"format");
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      CVSetCLIPSString(returnValue,hptr);
      return;
     }

   if (strcmp(logicalName,"nil") == 0)
     { /* do nothing */ }
   else if (QueryRouters(theEnv,logicalName) == false)
     {
      UnrecognizedRouterMessage(theEnv,logicalName);
      CVSetCLIPSString(returnValue,hptr);
      return;
     }

   /*=====================================================*/
   /* Second argument must be a string.  The appropriate  */
   /* number of arguments specified by the string must be */
   /* present in the argument list.                       */
   /*=====================================================*/

   if ((formatString = ControlStringCheck(theEnv,argCount)) == NULL)
     {
      CVSetCLIPSString(returnValue,hptr);
      return;
     }

   /*========================================*/
   /* Search the format string, printing the */
   /* format flags as they are encountered.  */
   /*========================================*/

   while (formatString[form_pos] != '\0')
     {
      if (formatString[form_pos] != '%')
        {
         start_pos = form_pos;
         while ((formatString[form_pos] != '%') &&
                (formatString[form_pos] != '\0'))
           { form_pos++; }
         fstr = AppendNToString(theEnv,&formatString[start_pos],fstr,form_pos-start_pos,&fpos,&fmaxm);
        }
      else
        {
		 form_pos++;
         formatFlagType = FindFormatFlag(formatString,&form_pos,percentBuffer,FLAG_MAX);
         if (formatFlagType != ' ')
           {
            if ((theString = PrintFormatFlag(theEnv,percentBuffer,f_cur_arg,formatFlagType)) == NULL)
              {
               if (fstr != NULL) rm(theEnv,fstr,fmaxm);
               CVSetCLIPSString(returnValue,hptr);
               return;
              }
            fstr = AppendToString(theEnv,theString,fstr,&fpos,&fmaxm);
            if (fstr == NULL)
              {
               CVSetCLIPSString(returnValue,hptr);
               return;
              }
            f_cur_arg++;
           }
         else
           {
            fstr = AppendToString(theEnv,percentBuffer,fstr,&fpos,&fmaxm);
            if (fstr == NULL)
              {
               CVSetCLIPSString(returnValue,hptr);
               return;
              }
           }
        }
     }

   if (fstr != NULL)
     {
      hptr = EnvAddSymbol(theEnv,fstr);
      if (strcmp(logicalName,"nil") != 0) EnvPrintRouter(theEnv,logicalName,fstr);
      rm(theEnv,fstr,fmaxm);
     }
   else
     { hptr = EnvAddSymbol(theEnv,""); }

   CVSetCLIPSString(returnValue,hptr);
  }

/*********************************************************************/
/* ControlStringCheck:  Checks the 2nd parameter which is the format */
/*   control string to see if there are enough matching arguments.   */
/*********************************************************************/
static const char *ControlStringCheck(
  void *theEnv,
  int argCount)
  {
   DATA_OBJECT t_ptr;
   const char *str_array;
   char print_buff[FLAG_MAX];
   size_t i;
   int per_count;
   char formatFlag;

   if (EnvArgTypeCheck(theEnv,"format",2,STRING,&t_ptr) == false) return(NULL);

   per_count = 0;
   str_array = ValueToString(t_ptr.value);
   for (i= 0 ; str_array[i] != '\0' ; )
     {
      if (str_array[i] == '%')
        {
         i++;
         formatFlag = FindFormatFlag(str_array,&i,print_buff,FLAG_MAX);
         if (formatFlag == '-')
           { 
            PrintErrorID(theEnv,"IOFUN",3,false);
            EnvPrintRouter(theEnv,WERROR,"Invalid format flag \"");
            EnvPrintRouter(theEnv,WERROR,print_buff);
            EnvPrintRouter(theEnv,WERROR,"\" specified in format function.\n");
            EnvSetEvaluationError(theEnv,true);
            return (NULL);
           }
         else if (formatFlag != ' ')
           { per_count++; }
        }
      else
        { i++; }
     }

   if (per_count != (argCount - 2))
     {
      ExpectedCountError(theEnv,"format",EXACTLY,per_count+2);
      EnvSetEvaluationError(theEnv,true);
      return (NULL);
     }

   return(str_array);
  }

/***********************************************/
/* FindFormatFlag:  This function searches for */
/*   a format flag in the format string.       */
/***********************************************/
static char FindFormatFlag(
  const char *formatString,
  size_t *a,
  char *formatBuffer,
  size_t bufferMax)
  {
   char inchar, formatFlagType;
   size_t copy_pos = 0;

   /*====================================================*/
   /* Set return values to the default value. A blank    */
   /* character indicates that no format flag was found  */
   /* which requires a parameter.                        */
   /*====================================================*/

   formatFlagType = ' ';

   /*=====================================================*/
   /* The format flags for carriage returns, line feeds,  */
   /* horizontal and vertical tabs, and the percent sign, */
   /* do not require a parameter.                         */
   /*=====================================================*/

   if (formatString[*a] == 'n')
     {
      gensprintf(formatBuffer,"\n");
      (*a)++;
      return(formatFlagType);
     }
   else if (formatString[*a] == 'r')
     {
      gensprintf(formatBuffer,"\r");
      (*a)++;
      return(formatFlagType);
     }
   else if (formatString[*a] == 't')
     {
      gensprintf(formatBuffer,"\t");
      (*a)++;
      return(formatFlagType);
     }
   else if (formatString[*a] == 'v')
     {
      gensprintf(formatBuffer,"\v");
      (*a)++;
      return(formatFlagType);
     }
   else if (formatString[*a] == '%')
     {
      gensprintf(formatBuffer,"%%");
      (*a)++;
      return(formatFlagType);
     }

   /*======================================================*/
   /* Identify the format flag which requires a parameter. */
   /*======================================================*/

   formatBuffer[copy_pos++] = '%';
   formatBuffer[copy_pos] = '\0';
   while ((formatString[*a] != '%') &&
          (formatString[*a] != '\0') &&
          (copy_pos < (bufferMax - 5)))
     {
      inchar = formatString[*a];
      (*a)++;

      if ( (inchar == 'd') ||
           (inchar == 'o') ||
           (inchar == 'x') ||
           (inchar == 'u'))
        {
         formatFlagType = inchar;
         formatBuffer[copy_pos++] = 'l';
         formatBuffer[copy_pos++] = 'l';
         formatBuffer[copy_pos++] = inchar;
         formatBuffer[copy_pos] = '\0';
         return(formatFlagType);
        }
      else if ( (inchar == 'c') ||
                (inchar == 's') ||
                (inchar == 'e') ||
                (inchar == 'f') ||
                (inchar == 'g') )
        {
         formatBuffer[copy_pos++] = inchar;
         formatBuffer[copy_pos] = '\0';
         formatFlagType = inchar;
         return(formatFlagType);
        }
      
      /*=======================================================*/
      /* If the type hasn't been read, then this should be the */
      /* -M.N part of the format specification (where M and N  */
      /* are integers).                                        */
      /*=======================================================*/
      
      if ( (! isdigit(inchar)) &&
           (inchar != '.') &&
           (inchar != '-') )
        { 
         formatBuffer[copy_pos++] = inchar;
         formatBuffer[copy_pos] = '\0';
         return('-'); 
        }

      formatBuffer[copy_pos++] = inchar;
      formatBuffer[copy_pos] = '\0';
     }

   return(formatFlagType);
  }

/**********************************************************************/
/* PrintFormatFlag:  Prints out part of the total format string along */
/*   with the argument for that part of the format string.            */
/**********************************************************************/
static const char *PrintFormatFlag(
  void *theEnv,
  const char *formatString,
  int whichArg,
  int formatType)
  {
   DATA_OBJECT theResult;
   const char *theString;
   char *printBuffer;
   size_t theLength;
   void *oldLocale;
      
   /*=================*/
   /* String argument */
   /*=================*/

   switch (formatType)
     {
      case 's':
        if (EnvArgTypeCheck(theEnv,"format",whichArg,SYMBOL_OR_STRING,&theResult) == false) return(NULL);
        theLength = strlen(formatString) + strlen(ValueToString(theResult.value)) + 200;
        printBuffer = (char *) gm2(theEnv,(sizeof(char) * theLength));
        gensprintf(printBuffer,formatString,ValueToString(theResult.value));
        break;

      case 'c':
        EnvRtnUnknown(theEnv,whichArg,&theResult);
        if ((GetType(theResult) == STRING) ||
            (GetType(theResult) == SYMBOL))
          {
           theLength = strlen(formatString) + 200;
           printBuffer = (char *) gm2(theEnv,(sizeof(char) * theLength));
           gensprintf(printBuffer,formatString,(ValueToString(theResult.value))[0]);
          }
        else if (GetType(theResult) == INTEGER)
          {
           theLength = strlen(formatString) + 200;
           printBuffer = (char *) gm2(theEnv,(sizeof(char) * theLength));
           gensprintf(printBuffer,formatString,(char) DOToLong(theResult));
          }
        else
          {
           ExpectedTypeError1(theEnv,"format",whichArg,"symbol, string, or integer");
           return(NULL);
          }
        break;

      case 'd':
      case 'x':
      case 'o':
      case 'u':
        if (EnvArgTypeCheck(theEnv,"format",whichArg,INTEGER_OR_FLOAT,&theResult) == false) return(NULL);
        theLength = strlen(formatString) + 200;
        printBuffer = (char *) gm2(theEnv,(sizeof(char) * theLength));
        
        oldLocale = EnvAddSymbol(theEnv,setlocale(LC_NUMERIC,NULL));
        setlocale(LC_NUMERIC,ValueToString(IOFunctionData(theEnv)->locale));

        if (GetType(theResult) == FLOAT)
          { gensprintf(printBuffer,formatString,(long long) ValueToDouble(theResult.value)); }
        else
          { gensprintf(printBuffer,formatString,(long long) ValueToLong(theResult.value)); }
          
        setlocale(LC_NUMERIC,ValueToString(oldLocale));
        break;

      case 'f':
      case 'g':
      case 'e':
        if (EnvArgTypeCheck(theEnv,"format",whichArg,INTEGER_OR_FLOAT,&theResult) == false) return(NULL);
        theLength = strlen(formatString) + 200;
        printBuffer = (char *) gm2(theEnv,(sizeof(char) * theLength));

        oldLocale = EnvAddSymbol(theEnv,setlocale(LC_NUMERIC,NULL));
        
        setlocale(LC_NUMERIC,ValueToString(IOFunctionData(theEnv)->locale));

        if (GetType(theResult) == FLOAT)
          { gensprintf(printBuffer,formatString,ValueToDouble(theResult.value)); }
        else
          { gensprintf(printBuffer,formatString,(double) ValueToLong(theResult.value)); }
        
        setlocale(LC_NUMERIC,ValueToString(oldLocale));
        
        break;

      default:
         EnvPrintRouter(theEnv,WERROR," Error in format, the conversion character");
         EnvPrintRouter(theEnv,WERROR," for formatted output is not valid\n");
         return(NULL);
     }

   theString = ValueToString(EnvAddSymbol(theEnv,printBuffer));
   rm(theEnv,printBuffer,sizeof(char) * theLength);
   return(theString);
  }

/******************************************/
/* ReadlineFunction: H/L access routine   */
/*   for the readline function.           */
/******************************************/
void ReadlineFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   char *buffer;
   size_t line_max = 0;
   const char *logicalName;
   Environment *theEnv = UDFContextEnvironment(context);

   if (! UDFHasNextArgument(context))
     { logicalName = STDIN; }
   else
     {
      logicalName = GetLogicalName(context,STDIN);
      if (logicalName == NULL)
        {
         IllegalLogicalNameMessage(theEnv,"readline");
         EnvSetHaltExecution(theEnv,true);
         EnvSetEvaluationError(theEnv,true);
         mCVSetString(returnValue,"*** READ ERROR ***");
         return;
        }
     }

   if (QueryRouters(theEnv,logicalName) == false)
     {
      UnrecognizedRouterMessage(theEnv,logicalName);
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      mCVSetString(returnValue,"*** READ ERROR ***");
      return;
     }

   RouterData(theEnv)->CommandBufferInputCount = 0;
   RouterData(theEnv)->AwaitingInput = true;
   buffer = FillBuffer(theEnv,logicalName,&RouterData(theEnv)->CommandBufferInputCount,&line_max);
   RouterData(theEnv)->CommandBufferInputCount = 0;
   RouterData(theEnv)->AwaitingInput = false;

   if (EnvGetHaltExecution(theEnv))
     {
      mCVSetString(returnValue,"*** READ ERROR ***");
      if (buffer != NULL) rm(theEnv,buffer,(int) sizeof (char) * line_max);
      return;
     }

   if (buffer == NULL)
     {
      mCVSetSymbol(returnValue,"EOF");
      return;
     }

   mCVSetString(returnValue,buffer);
   rm(theEnv,buffer,(int) sizeof (char) * line_max);
   return;
  }

/*************************************************************/
/* FillBuffer: Read characters from a specified logical name */
/*   and places them into a buffer until a carriage return   */
/*   or end-of-file character is read.                       */
/*************************************************************/
static char *FillBuffer(
  void *theEnv,
  const char *logicalName,
  size_t *currentPosition,
  size_t *maximumSize)
  {
   int c;
   char *buf = NULL;

   /*================================*/
   /* Read until end of line or eof. */
   /*================================*/

   c = EnvGetcRouter(theEnv,logicalName);

   if (c == EOF)
     { return(NULL); }

   /*==================================*/
   /* Grab characters until cr or eof. */
   /*==================================*/

   while ((c != '\n') && (c != '\r') && (c != EOF) &&
          (! EnvGetHaltExecution(theEnv)))
     {
      buf = ExpandStringWithChar(theEnv,c,buf,currentPosition,maximumSize,*maximumSize+80);
      c = EnvGetcRouter(theEnv,logicalName);
     }

   /*==================*/
   /* Add closing EOS. */
   /*==================*/

   buf = ExpandStringWithChar(theEnv,EOS,buf,currentPosition,maximumSize,*maximumSize+80);
   return (buf);
  }
  
/*****************************************/
/* SetLocaleFunction: H/L access routine */
/*   for the set-locale function.        */
/*****************************************/
void SetLocaleFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT theArg;
   Environment *theEnv = UDFContextEnvironment(context);
   
   /*=================================*/
   /* If there are no arguments, just */
   /* return the current locale.      */
   /*=================================*/
   
   if (! UDFHasNextArgument(context))
     {
      CVSetCLIPSString(returnValue,IOFunctionData(theEnv)->locale);
      return;
     }

   /*=================*/
   /* Get the locale. */
   /*=================*/
   
   if (! UDFFirstArgument(context,STRING_TYPE,&theArg))
     { return; }
     
   /*=====================================*/
   /* Return the old value of the locale. */
   /*=====================================*/
   
   CVSetCLIPSString(returnValue,IOFunctionData(theEnv)->locale);
   
   /*======================================================*/
   /* Change the value of the locale to the one specified. */
   /*======================================================*/
   
   DecrementSymbolCount(theEnv,(struct symbolHashNode *) IOFunctionData(theEnv)->locale);
   IOFunctionData(theEnv)->locale = CVToRawValue(&theArg);
   IncrementSymbolCount(IOFunctionData(theEnv)->locale);
  }

/******************************************/
/* ReadNumberFunction: H/L access routine */
/*   for the read-number function.        */
/******************************************/
void ReadNumberFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   struct token theToken;
   const char *logicalName = NULL;
   Environment *theEnv = UDFContextEnvironment(context);

   /*======================================================*/
   /* Determine the logical name from which input is read. */
   /*======================================================*/

   if (! UDFHasNextArgument(context))
     { logicalName = STDIN; }
   else
     {
      logicalName = GetLogicalName(context,STDIN);
      if (logicalName == NULL)
        {
         IllegalLogicalNameMessage(theEnv,"read");
         EnvSetHaltExecution(theEnv,true);
         EnvSetEvaluationError(theEnv,true);
         mCVSetString(returnValue,"*** READ ERROR ***");
         return;
        }
     }

   /*============================================*/
   /* Check to see that the logical name exists. */
   /*============================================*/

   if (QueryRouters(theEnv,logicalName) == false)
     {
      UnrecognizedRouterMessage(theEnv,logicalName);
      EnvSetHaltExecution(theEnv,true);
      EnvSetEvaluationError(theEnv,true);
      mCVSetString(returnValue,"*** READ ERROR ***");
      return;
     }

   /*=======================================*/
   /* Collect input into string if the read */
   /* source is stdin, else just get token. */
   /*=======================================*/

   if (strcmp(logicalName,STDIN) == 0)
     { ReadNumber(theEnv,logicalName,&theToken,true); }
   else
     { ReadNumber(theEnv,logicalName,&theToken,false); }

   RouterData(theEnv)->CommandBufferInputCount = 0;
   RouterData(theEnv)->AwaitingInput = false;

   /*====================================================*/
   /* Copy the token to the return value data structure. */
   /*====================================================*/

   returnValue->type = theToken.type;
   if ((theToken.type == FLOAT) || (theToken.type == STRING) ||
#if OBJECT_SYSTEM
       (theToken.type == INSTANCE_NAME) ||
#endif
       (theToken.type == SYMBOL) || (theToken.type == INTEGER))
     { returnValue->value = theToken.value; }
   else if (theToken.type == STOP)
     {
      returnValue->type = SYMBOL;
      returnValue->value = (void *) EnvAddSymbol(theEnv,"EOF");
     }
   else if (theToken.type == UNKNOWN_VALUE)
     {
      returnValue->type = STRING;
      returnValue->value = (void *) EnvAddSymbol(theEnv,"*** READ ERROR ***");
     }
   else
     {
      returnValue->type = STRING;
      returnValue->value = (void *) EnvAddSymbol(theEnv,theToken.printForm);
     }

   return;
  }
  
/********************************************/
/* ReadNumber: Special routine used by the  */
/*   read-number function to read a number. */
/********************************************/
static void ReadNumber(
  void *theEnv,
  const char *logicalName,
  struct token *theToken,
  bool isStdin)
  {
   char *inputString;
   char *charPtr = NULL;
   size_t inputStringSize;
   int inchar;
   long long theLong;
   double theDouble;
   void *oldLocale;

   theToken->type = STOP;

   /*===========================================*/
   /* Initialize the variables used for storing */
   /* the characters retrieved from stdin.      */
   /*===========================================*/

   inputString = NULL;
   RouterData(theEnv)->CommandBufferInputCount = 0;
   RouterData(theEnv)->AwaitingInput = true;
   inputStringSize = 0;
   inchar = EnvGetcRouter(theEnv,logicalName);
            
   /*====================================*/
   /* Skip whitespace before any number. */
   /*====================================*/
      
   while (isspace(inchar) && (inchar != EOF) && 
          (! EnvGetHaltExecution(theEnv)))
     { inchar = EnvGetcRouter(theEnv,logicalName); }

   /*=============================================================*/
   /* Continue reading characters until whitespace is found again */
   /* (for anything other than stdin) or a CR/LF (for stdin).     */
   /*=============================================================*/

   while ((((! isStdin) && (! isspace(inchar))) || 
          (isStdin && (inchar != '\n') && (inchar != '\r'))) &&
          (inchar != EOF) &&
          (! EnvGetHaltExecution(theEnv)))
     {
      inputString = ExpandStringWithChar(theEnv,inchar,inputString,&RouterData(theEnv)->CommandBufferInputCount,
                                         &inputStringSize,inputStringSize + 80);
      inchar = EnvGetcRouter(theEnv,logicalName);
     }

   /*===========================================*/
   /* Pressing control-c (or comparable action) */
   /* aborts the read-number function.          */
   /*===========================================*/

   if (EnvGetHaltExecution(theEnv))
     {
      theToken->type = STRING;
      theToken->value = (void *) EnvAddSymbol(theEnv,"*** READ ERROR ***");
      if (inputStringSize > 0) rm(theEnv,inputString,inputStringSize);
      return;
     }

   /*====================================================*/
   /* Return the EOF symbol if the end of file for stdin */
   /* has been encountered. This typically won't occur,  */
   /* but is possible (for example by pressing control-d */
   /* in the UNIX operating system).                     */
   /*====================================================*/

   if (inchar == EOF)
     {
      theToken->type = SYMBOL;
      theToken->value = (void *) EnvAddSymbol(theEnv,"EOF");
      if (inputStringSize > 0) rm(theEnv,inputString,inputStringSize);
      return;
     }

   /*==================================================*/
   /* Open a string input source using the characters  */
   /* retrieved from stdin and extract the first token */
   /* contained in the string.                         */
   /*==================================================*/
   
   /*=======================================*/
   /* Change the locale so that numbers are */
   /* converted using the localized format. */
   /*=======================================*/
   
   oldLocale = EnvAddSymbol(theEnv,setlocale(LC_NUMERIC,NULL));
   setlocale(LC_NUMERIC,ValueToString(IOFunctionData(theEnv)->locale));

   /*========================================*/
   /* Try to parse the number as a long. The */
   /* terminating character must either be   */
   /* white space or the string terminator.  */
   /*========================================*/

#if WIN_MVC
   theLong = _strtoi64(inputString,&charPtr,10);
#else
   theLong = strtoll(inputString,&charPtr,10);
#endif

   if ((charPtr != inputString) && 
       (isspace(*charPtr) || (*charPtr == '\0')))
     {
      theToken->type = INTEGER;
      theToken->value = (void *) EnvAddLong(theEnv,theLong);
      if (inputStringSize > 0) rm(theEnv,inputString,inputStringSize);
      setlocale(LC_NUMERIC,ValueToString(oldLocale));
      return;
     }
     
   /*==========================================*/
   /* Try to parse the number as a double. The */
   /* terminating character must either be     */
   /* white space or the string terminator.    */
   /*==========================================*/

   theDouble = strtod(inputString,&charPtr);  
   if ((charPtr != inputString) && 
       (isspace(*charPtr) || (*charPtr == '\0')))
     {
      theToken->type = FLOAT;
      theToken->value = (void *) EnvAddDouble(theEnv,theDouble);
      if (inputStringSize > 0) rm(theEnv,inputString,inputStringSize);
      setlocale(LC_NUMERIC,ValueToString(oldLocale));
      return;
     }

   /*============================================*/
   /* Restore the "C" locale so that any parsing */
   /* of numbers uses the C format.              */
   /*============================================*/
   
   setlocale(LC_NUMERIC,ValueToString(oldLocale));

   /*=========================================*/
   /* Return "*** READ ERROR ***" to indicate */
   /* a number was not successfully parsed.   */
   /*=========================================*/
         
   theToken->type = STRING;
   theToken->value = (void *) EnvAddSymbol(theEnv,"*** READ ERROR ***");
  }

#endif

