   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*            EXTENDED MATH FUNCTIONS MODULE           */
   /*******************************************************/

/*************************************************************/
/* Purpose: Contains the code for numerous extended math     */
/*   functions including cos, sin, tan, sec, csc, cot, acos, */
/*   asin, atan, asec, acsc, acot, cosh, sinh, tanh, sech,   */
/*   csch, coth, acosh, asinh, atanh, asech, acsch, acoth,   */
/*   mod, exp, log, log10, sqrt, pi, deg-rad, rad-deg,       */
/*   deg-grad, grad-deg, **, and round.                      */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Gary D. Riley                                        */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW and       */
/*            MAC_MCW).                                      */
/*                                                           */
/*            Support for long long integers.                */
/*                                                           */
/*            Renamed EX_MATH compiler flag to               */
/*            EXTENDED_MATH_FUNCTIONS.                       */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*      6.40: Added Env prefix to GetEvaluationError and     */
/*            SetEvaluationError functions.                  */
/*                                                           */
/*            Added Env prefix to GetHaltExecution and       */
/*            SetHaltExecution functions.                    */
/*                                                           */
/*************************************************************/

#include "setup.h"
#include "argacces.h"
#include "envrnmnt.h"
#include "extnfunc.h"
#include "router.h"

#include "emathfun.h"

#if EXTENDED_MATH_FUNCTIONS

#include <math.h>

/***************/
/* DEFINITIONS */
/***************/

#ifndef PI
#define PI   3.14159265358979323846
#endif

#ifndef PID2
#define PID2 1.57079632679489661923 /* PI divided by 2 */
#endif

#define SMALLEST_ALLOWED_NUMBER 1e-15
#define dtrunc(x) (((x) < 0.0) ? ceil(x) : floor(x))

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static bool                    SingleNumberCheck(UDFContext *,const char *,CLIPSValue *);
   static bool                    TestProximity(double,double);
   static void                    DomainErrorMessage(void *,const char *);
   static void                    ArgumentOverflowErrorMessage(void *,const char *);
   static void                    SingularityErrorMessage(void *,const char *);
   static double                  genacosh(double);
   static double                  genasinh(double);
   static double                  genatanh(double);
   static double                  genasech(double);
   static double                  genacsch(double);
   static double                  genacoth(double);

/************************************************/
/* ExtendedMathFunctionDefinitions: Initializes */
/*   the extended math functions.               */
/************************************************/
void ExtendedMathFunctionDefinitions(
  void *theEnv)
  {
#if ! RUN_TIME
   EnvAddUDF(theEnv,"cos",      "d",   CosFunction,      "CosFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"sin",      "d",   SinFunction,      "SinFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"tan",      "d",   TanFunction,      "TanFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"sec",      "d",   SecFunction,      "SecFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"csc",      "d",   CscFunction,      "CscFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"cot",      "d",   CotFunction,      "CotFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"acos",     "d",   AcosFunction,     "AcosFunction",    1,1,"ld",NULL);;
   EnvAddUDF(theEnv,"asin",     "d",   AsinFunction,     "AsinFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"atan",     "d",   AtanFunction,     "AtanFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"asec",     "d",   AsecFunction,     "AsecFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"acsc",     "d",   AcscFunction,     "AcscFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"acot",     "d",   AcotFunction,     "AcotFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"cosh",     "d",   CoshFunction,     "CoshFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"sinh",     "d",   SinhFunction,     "SinhFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"tanh",     "d",   TanhFunction,     "TanhFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"sech",     "d",   SechFunction,     "SechFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"csch",     "d",   CschFunction,     "CschFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"coth",     "d",   CothFunction,     "CothFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"acosh",    "d",   AcoshFunction,    "AcoshFunction",   1,1,"ld",NULL);
   EnvAddUDF(theEnv,"asinh",    "d",   AsinhFunction,    "AsinhFunction",   1,1,"ld",NULL);
   EnvAddUDF(theEnv,"atanh",    "d",   AtanhFunction,    "AtanhFunction",   1,1,"ld",NULL);
   EnvAddUDF(theEnv,"asech",    "d",   AsechFunction,    "AsechFunction",   1,1,"ld",NULL);
   EnvAddUDF(theEnv,"acsch",    "d",   AcschFunction,    "AcschFunction",   1,1,"ld",NULL);
   EnvAddUDF(theEnv,"acoth",    "d",   AcothFunction,    "AcothFunction",   1,1,"ld",NULL);

   EnvAddUDF(theEnv,"mod",      "ld", ModFunction,      "ModFunction",     2,2,"ld",NULL);
   EnvAddUDF(theEnv,"exp",      "d",   ExpFunction,      "ExpFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"log",      "d",   LogFunction,      "LogFunction",     1,1,"ld",NULL);
   EnvAddUDF(theEnv,"log10",    "d",   Log10Function,    "Log10Function",   1,1,"ld",NULL);
   EnvAddUDF(theEnv,"sqrt",     "d",   SqrtFunction,     "SqrtFunction",    1,1,"ld",NULL);
   EnvAddUDF(theEnv,"pi",       "d",   PiFunction,       "PiFunction",      0,0,NULL,NULL);
   EnvAddUDF(theEnv,"deg-rad",  "d",   DegRadFunction,   "DegRadFunction",  1,1,"ld",NULL);
   EnvAddUDF(theEnv,"rad-deg",  "d",   RadDegFunction,   "RadDegFunction",  1,1,"ld",NULL);
   EnvAddUDF(theEnv,"deg-grad", "d",   DegGradFunction,  "DegGradFunction", 1,1,"ld",NULL);
   EnvAddUDF(theEnv,"grad-deg", "d",   GradDegFunction,  "GradDegFunction", 1,1,"ld",NULL);
   EnvAddUDF(theEnv,"**",       "d",   PowFunction,      "PowFunction",     2,2,"ld",NULL);
   EnvAddUDF(theEnv,"round",    "l", RoundFunction,    "RoundFunction",   1,1,"ld",NULL);
#else
#if MAC_XCD
#pragma unused(theEnv)
#endif
#endif
  }

/************************************************************/
/* SingleNumberCheck: Retrieves the numeric argument for    */
/*   extended math functions which expect a single floating */
/*   point argument.                                        */
/************************************************************/
static bool SingleNumberCheck(
  UDFContext *context,
  const char *functionName,
  CLIPSValue *returnValue)
  {
   /*======================================*/
   /* Check that the argument is a number. */
   /*======================================*/

   if (! UDFNthArgument(context,1,NUMBER_TYPES,returnValue))
     { return false; }
  
   return(true);
  }

/**************************************************************/
/* TestProximity: Returns true if the specified number falls  */
/*   within the specified range, otherwise false is returned. */
/**************************************************************/
static bool TestProximity(
  double theNumber,
  double range)
  {
   if ((theNumber >= (- range)) && (theNumber <= range)) return true;
   else return false;
  }

/********************************************************/
/* DomainErrorMessage: Generic error message used when  */
/*   a domain error is detected during a call to one of */
/*   the extended math functions.                       */
/********************************************************/
static void DomainErrorMessage(
  void *theEnv,
  const char *functionName)
  {
   PrintErrorID(theEnv,"EMATHFUN",1,false);
   EnvPrintRouter(theEnv,WERROR,"Domain error for ");
   EnvPrintRouter(theEnv,WERROR,functionName);
   EnvPrintRouter(theEnv,WERROR," function.\n");
   EnvSetHaltExecution(theEnv,true);
   EnvSetEvaluationError(theEnv,true);
  }

/************************************************************/
/* ArgumentOverflowErrorMessage: Generic error message used */
/*   when an argument overflow is detected during a call to */
/*   one of the extended math functions.                    */
/************************************************************/
static void ArgumentOverflowErrorMessage(
  void *theEnv,
  const char *functionName)
  {
   PrintErrorID(theEnv,"EMATHFUN",2,false);
   EnvPrintRouter(theEnv,WERROR,"Argument overflow for ");
   EnvPrintRouter(theEnv,WERROR,functionName);
   EnvPrintRouter(theEnv,WERROR," function.\n");
   EnvSetHaltExecution(theEnv,true);
   EnvSetEvaluationError(theEnv,true);
  }

/************************************************************/
/* SingularityErrorMessage: Generic error message used when */
/*   a singularity is detected during a call to one of the  */
/*   extended math functions.                               */
/************************************************************/
static void SingularityErrorMessage(
  void *theEnv,
  const char *functionName)
  {
   PrintErrorID(theEnv,"EMATHFUN",3,false);
   EnvPrintRouter(theEnv,WERROR,"Singularity at asymptote in ");
   EnvPrintRouter(theEnv,WERROR,functionName);
   EnvPrintRouter(theEnv,WERROR," function.\n");
   EnvSetHaltExecution(theEnv,true);
   EnvSetEvaluationError(theEnv,true);
  }

/*************************************/
/* CosFunction: H/L access routine   */
/*   for the cos function.           */
/*************************************/
void CosFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"cos",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,cos(mCVToFloat(returnValue)));
  }

/*************************************/
/* SinFunction: H/L access routine   */
/*   for the sin function.           */
/*************************************/
void SinFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"sin",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,sin(mCVToFloat(returnValue)));
  }

/*************************************/
/* TanFunction: H/L access routine   */
/*   for the tan function.           */
/*************************************/
void TanFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat tv;

   if (SingleNumberCheck(context,"tan",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   tv = cos(mCVToFloat(returnValue));
   if ((tv < SMALLEST_ALLOWED_NUMBER) && (tv > -SMALLEST_ALLOWED_NUMBER))
     {
      SingularityErrorMessage(UDFContextEnvironment(context),"tan");
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,sin(mCVToFloat(returnValue)) / tv);
  }

/*************************************/
/* SecFunction: H/L access routine   */
/*   for the sec function.           */
/*************************************/
void SecFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   double tv;

   if (SingleNumberCheck(context,"sec",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   tv = cos(mCVToFloat(returnValue));
   if ((tv < SMALLEST_ALLOWED_NUMBER) && (tv > -SMALLEST_ALLOWED_NUMBER))
     {
      SingularityErrorMessage(UDFContextEnvironment(context),"sec");
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,1.0 / tv);
  }

/*************************************/
/* CscFunction: H/L access routine   */
/*   for the csc function.           */
/*************************************/
void CscFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   double tv;

   if (SingleNumberCheck(context,"csc",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   tv = sin(mCVToFloat(returnValue));
   if ((tv < SMALLEST_ALLOWED_NUMBER) && (tv > -SMALLEST_ALLOWED_NUMBER))
     {
      SingularityErrorMessage(UDFContextEnvironment(context),"csc");
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,1.0 / tv);
  }

/*************************************/
/* CotFunction: H/L access routine   */
/*   for the cot function.           */
/*************************************/
void CotFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
    double tv;

    if (SingleNumberCheck(context,"cot",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

    tv = sin(mCVToFloat(returnValue));
    if ((tv < SMALLEST_ALLOWED_NUMBER) && (tv > -SMALLEST_ALLOWED_NUMBER))
      {
       SingularityErrorMessage(UDFContextEnvironment(context),"cot");
       mCVSetFloat(returnValue,0.0);
       return;
      }

    mCVSetFloat(returnValue,cos(mCVToFloat(returnValue)) / tv);
  }

/**************************************/
/* AcosFunction: H/L access routine   */
/*   for the acos function.           */
/**************************************/
void AcosFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"acos",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   
   if ((num > 1.0) || (num < -1.0))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"acos");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
    mCVSetFloat(returnValue,acos(num));
  }

/**************************************/
/* AsinFunction: H/L access routine   */
/*   for the asin function.           */
/**************************************/
void AsinFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"asin",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if ((num > 1.0) || (num < -1.0))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"asin");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,asin(num));
  }

/**************************************/
/* AtanFunction: H/L access routine   */
/*   for the atan function.           */
/**************************************/
void AtanFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"atan",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,atan(mCVToFloat(returnValue)));
  }

/**************************************/
/* AsecFunction: H/L access routine   */
/*   for the asec function.           */
/**************************************/
void AsecFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"asec",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if ((num < 1.0) && (num > -1.0))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"asec");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
    num = 1.0 / num;
    mCVSetFloat(returnValue,acos(num));
  }

/**************************************/
/* AcscFunction: H/L access routine   */
/*   for the acsc function.           */
/**************************************/
void AcscFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"acsc",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if ((num < 1.0) && (num > -1.0))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"acsc");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
    num = 1.0 / num;
    mCVSetFloat(returnValue,asin(num));
  }

/**************************************/
/* AcotFunction: H/L access routine   */
/*   for the acot function.           */
/**************************************/
void AcotFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"acot",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   num = mCVToFloat(returnValue);
   if (TestProximity(num,1e-25) == true)
     {
      mCVSetFloat(returnValue,PID2);
      return;
     }
     
   num = 1.0 / num;
   mCVSetFloat(returnValue,atan(num));
  }

/**************************************/
/* CoshFunction: H/L access routine   */
/*   for the cosh function.           */
/**************************************/
void CoshFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"cosh",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,cosh(mCVToFloat(returnValue)));
  }

/**************************************/
/* SinhFunction: H/L access routine   */
/*   for the sinh function.           */
/**************************************/
void SinhFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"sinh",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,sinh(mCVToFloat(returnValue)));
  }

/**************************************/
/* TanhFunction: H/L access routine   */
/*   for the tanh function.           */
/**************************************/
void TanhFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"tanh",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,tanh(mCVToFloat(returnValue)));
  }

/**************************************/
/* SechFunction: H/L access routine   */
/*   for the sech function.           */
/**************************************/
void SechFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"sech",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,1.0 / cosh(mCVToFloat(returnValue)));
  }

/**************************************/
/* CschFunction: H/L access routine   */
/*   for the csch function.           */
/**************************************/
void CschFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;
   void *theEnv = UDFContextEnvironment(context);
   
   if (SingleNumberCheck(context,"csch",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if (num == 0.0)
     {
      SingularityErrorMessage(theEnv,"csch");
      mCVSetFloat(returnValue,0.0);
      return;
     }
   else if (TestProximity(num,1e-25) == true)
     {
      ArgumentOverflowErrorMessage(theEnv,"csch");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,1.0 / sinh(num));
  }

/**************************************/
/* CothFunction: H/L access routine   */
/*   for the coth function.           */
/**************************************/
void CothFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;
   void *theEnv = UDFContextEnvironment(context);

   if (SingleNumberCheck(context,"coth",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if (num == 0.0)
     {
      SingularityErrorMessage(theEnv,"coth");
      mCVSetFloat(returnValue,0.0);
      return;
     }
   else if (TestProximity(num,1e-25) == true)
     {
      ArgumentOverflowErrorMessage(theEnv,"coth");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,1.0 / tanh(num));
  }

/***************************************/
/* AcoshFunction: H/L access routine   */
/*   for the acosh function.           */
/***************************************/
void AcoshFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"acosh",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if (num < 1.0)
     {
      DomainErrorMessage(UDFContextEnvironment(context),"acosh");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,genacosh(num));
  }

/***************************************/
/* AsinhFunction: H/L access routine   */
/*   for the asinh function.           */
/***************************************/
void AsinhFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"asinh",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,genasinh(mCVToFloat(returnValue)));
  }

/***************************************/
/* AtanhFunction: H/L access routine   */
/*   for the atanh function.           */
/***************************************/
void AtanhFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"atanh",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if ((num >= 1.0) || (num <= -1.0))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"atanh");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,genatanh(num));
  }

/***************************************/
/* AsechFunction: H/L access routine   */
/*   for the asech function.           */
/***************************************/
void AsechFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"asech",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if ((num > 1.0) || (num <= 0.0))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"asech");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,genasech(num));
  }

/***************************************/
/* AcschFunction: H/L access routine   */
/*   for the acsch function.           */
/***************************************/
void AcschFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;
   
   if (SingleNumberCheck(context,"acsch",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   num = mCVToFloat(returnValue);
   if (num == 0.0)
     {
      DomainErrorMessage(UDFContextEnvironment(context),"acsch");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,genacsch(num));
  }

/***************************************/
/* AcothFunction: H/L access routine   */
/*   for the acoth function.           */
/***************************************/
void AcothFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"acoth",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if ((num <= 1.0) && (num >= -1.0))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"acoth");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,genacoth(num));
  }

/*************************************/
/* ExpFunction: H/L access routine   */
/*   for the exp function.           */
/*************************************/
void ExpFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"exp",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,exp(mCVToFloat(returnValue)));
  }

/*************************************/
/* LogFunction: H/L access routine   */
/*   for the log function.           */
/*************************************/
void LogFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;
   void *theEnv = UDFContextEnvironment(context);

   if (SingleNumberCheck(context,"log",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if (num < 0.0)
     {
      DomainErrorMessage(theEnv,"log");
      mCVSetFloat(returnValue,0.0);
      return;
     }
   else if (num == 0.0)
     {
      ArgumentOverflowErrorMessage(theEnv,"log");
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,log(num));
  }

/***************************************/
/* Log10Function: H/L access routine   */
/*   for the log10 function.           */
/***************************************/
void Log10Function(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;
   void *theEnv = UDFContextEnvironment(context);

   if (SingleNumberCheck(context,"log10",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   num = mCVToFloat(returnValue);
   if (num < 0.0)
     {
      DomainErrorMessage(theEnv,"log10");
      mCVSetFloat(returnValue,0.0);
      return;
     }
   else if (num == 0.0)
     {
      ArgumentOverflowErrorMessage(theEnv,"log10");
      mCVSetFloat(returnValue,0.0);
      return;
     }

    mCVSetFloat(returnValue,log10(num));
   }

/**************************************/
/* SqrtFunction: H/L access routine   */
/*   for the sqrt function.           */
/**************************************/
void SqrtFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSFloat num;

   if (SingleNumberCheck(context,"sqrt",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }
   
   num = mCVToFloat(returnValue);
   if (num < 0.00000)
     {
      DomainErrorMessage(UDFContextEnvironment(context),"sqrt");
      mCVSetFloat(returnValue,0.0);
      return;
     }
     
   mCVSetFloat(returnValue,sqrt(num));
  }

/*************************************/
/* PowFunction: H/L access routine   */
/*   for the pow function.           */
/*************************************/
void PowFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   CLIPSValue value1, value2;
   CLIPSFloat num1, num2;

   /*==================================*/
   /* Check for two numeric arguments. */
   /*==================================*/

   if (! UDFNthArgument(context,1,NUMBER_TYPES,&value1))
     { return; }

   if (! UDFNthArgument(context,2,NUMBER_TYPES,&value2))
     { return; }

    /*=====================*/
    /* Domain error check. */
    /*=====================*/
    
    num1 = mCVToFloat(&value1);
    num2 = mCVToFloat(&value2);
    
    if (((num1 == 0.0) && (num2 <= 0.0)) ||
       ((num1 < 0.0) && (dtrunc(num2) != num2)))
     {
      DomainErrorMessage(UDFContextEnvironment(context),"**");
      mCVSetFloat(returnValue,0.0);
      return;
     }

   /*============================*/
   /* Compute and set the value. */
   /*============================*/
   
   mCVSetFloat(returnValue,pow(num1,num2));
  }

/*************************************/
/* ModFunction: H/L access routine   */
/*   for the mod function.           */
/*************************************/
void ModFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   DATA_OBJECT item1, item2;
   CLIPSFloat fnum1, fnum2;
   CLIPSInteger lnum1, lnum2;
   void *theEnv = UDFContextEnvironment(context);

   /*==================================*/
   /* Check for two numeric arguments. */
   /*==================================*/
   
   if (! UDFNthArgument(context,1,NUMBER_TYPES,&item1))
     { return; }

   if (! UDFNthArgument(context,2,NUMBER_TYPES,&item2))
     { return; }

   /*===========================*/
   /* Check for divide by zero. */
   /*===========================*/

   if ((mCVIsType(&item2,INTEGER_TYPE) ? (mCVToInteger(&item2) == 0L) : false) ||
       (mCVIsType(&item2,FLOAT_TYPE) ? (mCVToFloat(&item2) == 0.0) : false))
     {
      DivideByZeroErrorMessage(theEnv,"mod");
      EnvSetEvaluationError(theEnv,true);
      mCVSetInteger(returnValue,0);
      return;
     }

   /*===========================*/
   /* Compute the return value. */
   /*===========================*/
   
   if (mCVIsType(&item1,FLOAT_TYPE) || mCVIsType(&item2,FLOAT_TYPE))
     {
      fnum1 = mCVToFloat(&item1);
      fnum2 = mCVToFloat(&item2);
      mCVSetFloat(returnValue,fnum1 - (dtrunc(fnum1 / fnum2) * fnum2));
     }
   else
     {
      lnum1 = mCVToInteger(&item1);
      lnum2 = mCVToInteger(&item2);
      mCVSetInteger(returnValue,lnum1 - (lnum1 / lnum2) * lnum2);
     }
  }

/************************************/
/* PiFunction: H/L access routine   */
/*   for the pi function.           */
/************************************/
void PiFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   mCVSetFloat(returnValue,acos(-1.0));
  }

/****************************************/
/* DegRadFunction: H/L access routine   */
/*   for the deg-rad function.          */
/****************************************/
void DegRadFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"deg-rad",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,mCVToFloat(returnValue) * PI / 180.0);
  }

/****************************************/
/* RadDegFunction: H/L access routine   */
/*   for the rad-deg function.          */
/****************************************/
void RadDegFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"rad-deg",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,mCVToFloat(returnValue) * 180.0 / PI);
  }

/*****************************************/
/* DegGradFunction: H/L access routine   */
/*   for the deg-grad function.          */
/*****************************************/
void DegGradFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"deg-grad",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,mCVToFloat(returnValue) / 0.9);
  }

/*****************************************/
/* GradDegFunction: H/L access routine   */
/*   for the grad-deg function.          */
/*****************************************/
void GradDegFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {
   if (SingleNumberCheck(context,"grad-deg",returnValue) == false)
     {
      mCVSetFloat(returnValue,0.0);
      return;
     }

   mCVSetFloat(returnValue,mCVToFloat(returnValue) * 0.9);
  }

/***************************************/
/* RoundFunction: H/L access routine   */
/*   for the round function.           */
/***************************************/
void RoundFunction(
  UDFContext *context,
  CLIPSValue *returnValue)
  {     
   /*======================================*/
   /* Check that the argument is a number. */
   /*======================================*/

   if (! UDFNthArgument(context,1,NUMBER_TYPES,returnValue))
     { return; }

   /*==============================*/
   /* Round float type to integer. */
   /*==============================*/
   
   if (mCVIsType(returnValue,FLOAT_TYPE))
     { mCVSetInteger(returnValue,ceil(mCVToFloat(returnValue) - 0.5)); }
  }

/*******************************************/
/* genacosh: Generic routine for computing */
/*   the hyperbolic arccosine.             */
/*******************************************/
static double genacosh(
  double num)
  {
   return(log(num + sqrt(num * num - 1.0)));
  }

/*******************************************/
/* genasinh: Generic routine for computing */
/*   the hyperbolic arcsine.               */
/*******************************************/
static double genasinh(
  double num)
  {
   return(log(num + sqrt(num * num + 1.0)));
  }

/*******************************************/
/* genatanh: Generic routine for computing */
/*   the hyperbolic arctangent.            */
/*******************************************/
static double genatanh(
  double num)
  {
   return((0.5) * log((1.0 + num) / (1.0 - num)));
  }

/*******************************************/
/* genasech: Generic routine for computing */
/*   the hyperbolic arcsecant.             */
/*******************************************/
static double genasech(
  double num)
  {
   return(log(1.0 / num + sqrt(1.0 / (num * num) - 1.0)));
  }

/*******************************************/
/* genacsch: Generic routine for computing */
/*   the hyperbolic arccosecant.           */
/*******************************************/
static double genacsch(
  double num)
  {
   return(log(1.0 / num + sqrt(1.0 / (num * num) + 1.0)));
  }

/*******************************************/
/* genacoth: Generic routine for computing */
/*   the hyperbolic arccotangent.          */
/*******************************************/
static double genacoth(
  double num)
  {
   return((0.5) * log((num + 1.0) / (num - 1.0)));
  }

#endif

