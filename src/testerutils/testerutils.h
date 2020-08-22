#ifndef __TESTERUTILS_H__
#define __TESTERUTILS_H__

//
//    ______          __            __  ____  _ __    
//   /_  __/__  _____/ /____  _____/ / / / /_(_) /____
//    / / / _ \/ ___/ __/ _ \/ ___/ / / / __/ / / ___/
//   / / /  __(__  ) /_/  __/ /  / /_/ / /_/ / (__  ) 
//  /_/  \___/____/\__/\___/_/   \____/\__/_/_/____/  
//                                                    
// Auxiliary module for Tester utilities such as
// serialization and deserialization of variables.
//

#include <aa/posixstring.h>
#include <stdio.h>

// Macro for comparing two strings in an if statement
// Usage: if matches(command, "create") { ... }

#define matches(a, b) (!strcmp(a, b))
#define nmatches(a, b) (strcmp(a, b))

//  ============================ ===================================== 
//   TesterUtilsSerializeString            Serializes string           
//  ============================ ===================================== 
//   file                         File pointer opened in writing mode  
//   string                       string to be serialized              
//  ============================ ===================================== 
//  [!] Returns 0 on success and 1 on failure

int TesterUtilsSerializeString(FILE *file, const char *string);

//  ============================== ===================================== 
//   TesterUtilsDeserializeString           Deserializes string          
//  ============================== ===================================== 
//   file                           File pointer opened in reading mode  
//  ============================== ===================================== 
//  [!] Returns string on success and NULL on failure

char *TesterUtilsDeserializeString(FILE *file);

//  ============================ =================================== 
//   TesterGetYesOrNoFromString   Obtain boolean value from string   
//  ============================ =================================== 
//   string                       string containg YES or NO          
//  ============================ =================================== 
//  [!] Returns boolean value. If neither YES or NO, NO is assumed.

int TesterGetYesOrNoFromString(const char *string);

#endif
