   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*            CLIPS Version 6.40  01/06/16             */
   /*                                                     */
   /*                                                     */
   /*******************************************************/

/*************************************************************/
/* Purpose: Definstances Construct Compiler Code             */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.30: Added support for path name argument to        */
/*            constructs-to-c.                               */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*************************************************************/

/* =========================================
   *****************************************
               EXTERNAL DEFINITIONS
   =========================================
   ***************************************** */
#include "setup.h"

#if DEFINSTANCES_CONSTRUCT && CONSTRUCT_COMPILER && (! RUN_TIME)

#include "conscomp.h"
#include "defins.h"
#include "envrnmnt.h"

#include "dfinscmp.h"

/* =========================================
   *****************************************
      INTERNALLY VISIBLE FUNCTION HEADERS
   =========================================
   ***************************************** */

static void ReadyDefinstancesForCode(void *);
static bool DefinstancesToCode(void *,const char *,const char *,char *,int,FILE *,int,int);
static void CloseDefinstancesFiles(void *,FILE *,FILE *,int);
static void DefinstancesModuleToCode(void *,FILE *,struct defmodule *,int,int);
static void SingleDefinstancesToCode(void *,FILE *,DEFINSTANCES *,int,int,int);

/* =========================================
   *****************************************
          EXTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/***************************************************
  NAME         : SetupDefinstancesCompiler
  DESCRIPTION  : Initializes the construct compiler
                   item for definstances
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : Code generator item initialized
  NOTES        : None
 ***************************************************/
void SetupDefinstancesCompiler(
  void *theEnv)
  {
   DefinstancesData(theEnv)->DefinstancesCodeItem = AddCodeGeneratorItem(theEnv,"definstances",0,ReadyDefinstancesForCode,
                                               NULL,DefinstancesToCode,2);
  }


/****************************************************
  NAME         : DefinstancesCModuleReference
  DESCRIPTION  : Prints out a reference to a
                 definstances module
  INPUTS       : 1) The output file
                 2) The id of the module item
                 3) The id of the image
                 4) The maximum number of elements
                    allowed in an array
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances module reference printed
  NOTES        : None
 ****************************************************/
void DefinstancesCModuleReference(
  void *theEnv,
  FILE *theFile,
  int count,
  int imageID,
  int maxIndices)
  {
   fprintf(theFile,"MIHS &%s%d_%d[%d]",
                      ModulePrefix(DefinstancesData(theEnv)->DefinstancesCodeItem),
                      imageID,
                      (count / maxIndices) + 1,
                      (count % maxIndices));
  }

/* =========================================
   *****************************************
          INTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/***************************************************
  NAME         : ReadyDefinstancesForCode
  DESCRIPTION  : Sets index of deffunctions
                   for use in compiled expressions
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : BsaveIndices set
  NOTES        : None
 ***************************************************/
static void ReadyDefinstancesForCode(
  void *theEnv)
  {
   MarkConstructBsaveIDs(theEnv,DefinstancesData(theEnv)->DefinstancesModuleIndex);
  }

/*******************************************************
  NAME         : DefinstancesToCode
  DESCRIPTION  : Writes out static array code for
                   definstances
  INPUTS       : 1) The base name of the construct set
                 2) The base id for this construct
                 3) The file pointer for the header file
                 4) The base id for the construct set
                 5) The max number of indices allowed
                    in an array
  RETURNS      : 0 on errors,
                  1 if definstances written
  SIDE EFFECTS : Code written to files
  NOTES        : None
 *******************************************************/
static bool DefinstancesToCode(
  void *theEnv,
  const char *fileName,
  const char *pathName,
  char *fileNameBuffer,
  int fileID,
  FILE *headerFP,
  int imageID,
  int maxIndices)
  {
   int fileCount = 1;
   struct defmodule *theModule;
   DEFINSTANCES *theDefinstances;
   int moduleCount = 0, moduleArrayCount = 0, moduleArrayVersion = 1;
   int definstancesArrayCount = 0, definstancesArrayVersion = 1;
   FILE *moduleFile = NULL, *definstancesFile = NULL;

   /* ================================================
      Include the appropriate definstances header file
      ================================================ */
   fprintf(headerFP,"#include \"defins.h\"\n");

   /* =============================================================
      Loop through all the modules and all the definstances writing
      their C code representation to the file as they are traversed
      ============================================================= */
   theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,NULL);

   while (theModule != NULL)
     {
      EnvSetCurrentModule(theEnv,(void *) theModule);

      moduleFile = OpenFileIfNeeded(theEnv,moduleFile,fileName,pathName,fileNameBuffer,fileID,imageID,&fileCount,
                                    moduleArrayVersion,headerFP,
                                    "DEFINSTANCES_MODULE",ModulePrefix(DefinstancesData(theEnv)->DefinstancesCodeItem),
                                    false,NULL);

      if (moduleFile == NULL)
        {
         CloseDefinstancesFiles(theEnv,moduleFile,definstancesFile,maxIndices);
         return(false);
        }

      DefinstancesModuleToCode(theEnv,moduleFile,theModule,imageID,maxIndices);
      moduleFile = CloseFileIfNeeded(theEnv,moduleFile,&moduleArrayCount,&moduleArrayVersion,
                                     maxIndices,NULL,NULL);

      theDefinstances = (DEFINSTANCES *) EnvGetNextDefinstances(theEnv,NULL);

      while (theDefinstances != NULL)
        {
         definstancesFile = OpenFileIfNeeded(theEnv,definstancesFile,fileName,pathName,fileNameBuffer,fileID,imageID,&fileCount,
                                             definstancesArrayVersion,headerFP,
                                             "DEFINSTANCES",ConstructPrefix(DefinstancesData(theEnv)->DefinstancesCodeItem),
                                             false,NULL);
         if (definstancesFile == NULL)
           {
            CloseDefinstancesFiles(theEnv,moduleFile,definstancesFile,maxIndices);
            return(false);
           }

         SingleDefinstancesToCode(theEnv,definstancesFile,theDefinstances,imageID,
                                  maxIndices,moduleCount);
         definstancesArrayCount++;
         definstancesFile = CloseFileIfNeeded(theEnv,definstancesFile,&definstancesArrayCount,
                                              &definstancesArrayVersion,maxIndices,NULL,NULL);

         theDefinstances = (DEFINSTANCES *) EnvGetNextDefinstances(theEnv,theDefinstances);
        }

      theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,theModule);
      moduleCount++;
      moduleArrayCount++;
     }

   CloseDefinstancesFiles(theEnv,moduleFile,definstancesFile,maxIndices);

   return(true);
  }

/***************************************************
  NAME         : CloseDefinstancesFiles
  DESCRIPTION  : Closes construct compiler files
                  for definstances structures
  INPUTS       : 1) The definstances module file
                 2) The definstances structure file
                 3) The maximum number of indices
                    allowed in an array
  RETURNS      : Nothing useful
  SIDE EFFECTS : Files closed
  NOTES        : None
 ***************************************************/
static void CloseDefinstancesFiles(
  void *theEnv,
  FILE *moduleFile,
  FILE *definstancesFile,
  int maxIndices)
  {
   int count = maxIndices;
   int arrayVersion = 0;

   if (definstancesFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,definstancesFile,&count,&arrayVersion,
                                         maxIndices,NULL,NULL);
     }

   if (moduleFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,moduleFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }
  }

/***************************************************
  NAME         : DefinstancesModuleToCode
  DESCRIPTION  : Writes out the C values for a
                 definstances module item
  INPUTS       : 1) The output file
                 2) The module for the definstances
                 3) The compile image id
                 4) The maximum number of elements
                    in an array
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances module item written
  NOTES        : None
 ***************************************************/
static void DefinstancesModuleToCode(
  void *theEnv,
  FILE *theFile,
  struct defmodule *theModule,
  int imageID,
  int maxIndices)
  {
   fprintf(theFile,"{");
   ConstructModuleToCode(theEnv,theFile,theModule,imageID,maxIndices,
                         DefinstancesData(theEnv)->DefinstancesModuleIndex,ConstructPrefix(DefinstancesData(theEnv)->DefinstancesCodeItem));
   fprintf(theFile,"}");
  }

/***************************************************
  NAME         : SingleDefinstancesToCode
  DESCRIPTION  : Writes out a single definstances'
                 data to the file
  INPUTS       : 1) The output file
                 2) The definstances
                 3) The compile image id
                 4) The maximum number of
                    elements in an array
                 5) The module index
  RETURNS      : Nothing useful
  SIDE EFFECTS : Definstances data written
  NOTES        : None
 ***************************************************/
static void SingleDefinstancesToCode(
  void *theEnv,
  FILE *theFile,
  DEFINSTANCES *theDefinstances,
  int imageID,
  int maxIndices,
  int moduleCount)
  {
   /* ===================
      Definstances Header
      =================== */

   fprintf(theFile,"{");
   ConstructHeaderToCode(theEnv,theFile,&theDefinstances->header,imageID,maxIndices,moduleCount,
                         ModulePrefix(DefinstancesData(theEnv)->DefinstancesCodeItem),
                         ConstructPrefix(DefinstancesData(theEnv)->DefinstancesCodeItem));

   /* ==========================
      Definstances specific data
      ========================== */
   fprintf(theFile,",0,");
   ExpressionToCode(theEnv,theFile,theDefinstances->mkinstance);
   fprintf(theFile,"}");
  }

#endif
