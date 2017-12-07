/*
 * PrintInConstructor.cpp
 *
 *  Created on: May 26, 2011
 *      Author: David
 */
#include <stdio.h>
#include "PrintInConstructor.h"

//Print different message in ctor/dtor.  If arg is NULL, then nothing will print.
//dtor_message is copied.  It does not have to be static.
PrintInConstructor::PrintInConstructor(const char* ctor_message, const char* dtor_message)
{
    Initialize(ctor_message, dtor_message);
}

//print same message in contructor/destructor
PrintInConstructor::PrintInConstructor(const char* ctor_dtor_message)
{
    Initialize(ctor_dtor_message, ctor_dtor_message);
}


//Needed an intermediate function, because a single arg constructor() gets optimized and
//mmediately destructed.
void PrintInConstructor::Initialize(const char* ctor_message, const char* dtor_message)
{
    if(ctor_message)
    {
        printf("ctor:");
        printf(ctor_message);
        printf("\n");
    }
    if(dtor_message) //save a copy with "dtor:" prepended, otherwise we won't print anything.
        destructorMessage_.assign("dtor:").append(dtor_message).append("\n");
}

PrintInConstructor::~PrintInConstructor()
{
    printf(destructorMessage_.c_str());
}
