   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*               I/O FUNCTIONS HEADER FILE             */
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
/*            Added a+, w+, rb, ab, r+b, w+b, and a+b modes  */
/*            for the open function.                         */
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
/*      6.40: Added print and println functions.             */
/*                                                           */
/*************************************************************/

#ifndef _H_iofun

#pragma once

#define _H_iofun

   void                           IOFunctionDefinitions(void *);
#if IO_FUNCTIONS
   bool                           SetFullCRLF(void *,bool);
   void                           PrintoutFunction(UDFContext *,CLIPSValue *);
   void                           PrintFunction(UDFContext *,CLIPSValue *);
   void                           PrintlnFunction(UDFContext *,CLIPSValue *);
   void                           ReadFunction(UDFContext *,CLIPSValue *);
   void                           OpenFunction(UDFContext *,CLIPSValue *);
   void                           CloseFunction(UDFContext *,CLIPSValue *);
   void                           GetCharFunction(UDFContext *,CLIPSValue *);
   void                           PutCharFunction(UDFContext *,CLIPSValue *);
   void                           ReadlineFunction(UDFContext *,CLIPSValue *);
   void                           FormatFunction(UDFContext *,CLIPSValue *);
   void                           RemoveFunction(UDFContext *,CLIPSValue *);
   void                           RenameFunction(UDFContext *,CLIPSValue *);
   void                           SetLocaleFunction(UDFContext *,CLIPSValue *);
   void                           ReadNumberFunction(UDFContext *,CLIPSValue *);
#endif

#endif /* _H_iofun */






