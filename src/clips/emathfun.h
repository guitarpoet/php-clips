   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*          EXTENDED MATH FUNCTIONS HEADER FILE        */
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
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
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
/*************************************************************/

#ifndef _H_emathfun

#pragma once

#define _H_emathfun

   void                           ExtendedMathFunctionDefinitions(void *theEnv);
#if EXTENDED_MATH_FUNCTIONS
   void                           CosFunction(UDFContext *,CLIPSValue *);
   void                           SinFunction(UDFContext *,CLIPSValue *);
   void                           TanFunction(UDFContext *,CLIPSValue *);
   void                           SecFunction(UDFContext *,CLIPSValue *);
   void                           CscFunction(UDFContext *,CLIPSValue *);
   void                           CotFunction(UDFContext *,CLIPSValue *);
   void                           AcosFunction(UDFContext *,CLIPSValue *);
   void                           AsinFunction(UDFContext *,CLIPSValue *);
   void                           AtanFunction(UDFContext *,CLIPSValue *);
   void                           AsecFunction(UDFContext *,CLIPSValue *);
   void                           AcscFunction(UDFContext *,CLIPSValue *);
   void                           AcotFunction(UDFContext *,CLIPSValue *);
   void                           CoshFunction(UDFContext *,CLIPSValue *);
   void                           SinhFunction(UDFContext *,CLIPSValue *);
   void                           TanhFunction(UDFContext *,CLIPSValue *);
   void                           SechFunction(UDFContext *,CLIPSValue *);
   void                           CschFunction(UDFContext *,CLIPSValue *);
   void                           CothFunction(UDFContext *,CLIPSValue *);
   void                           AcoshFunction(UDFContext *,CLIPSValue *);
   void                           AsinhFunction(UDFContext *,CLIPSValue *);
   void                           AtanhFunction(UDFContext *,CLIPSValue *);
   void                           AsechFunction(UDFContext *,CLIPSValue *);
   void                           AcschFunction(UDFContext *,CLIPSValue *);
   void                           AcothFunction(UDFContext *,CLIPSValue *);
   void                           RoundFunction(UDFContext *,CLIPSValue *);
   void                           ModFunction(UDFContext *,CLIPSValue *);
   void                           ExpFunction(UDFContext *,CLIPSValue *);
   void                           LogFunction(UDFContext *,CLIPSValue *);
   void                           Log10Function(UDFContext *,CLIPSValue *);
   void                           SqrtFunction(UDFContext *,CLIPSValue *);
   void                           PiFunction(UDFContext *,CLIPSValue *);
   void                           DegRadFunction(UDFContext *,CLIPSValue *);
   void                           RadDegFunction(UDFContext *,CLIPSValue *);
   void                           DegGradFunction(UDFContext *,CLIPSValue *);
   void                           GradDegFunction(UDFContext *,CLIPSValue *);
   void                           PowFunction(UDFContext *,CLIPSValue *);
#endif

#endif /* _H_emathfun */



