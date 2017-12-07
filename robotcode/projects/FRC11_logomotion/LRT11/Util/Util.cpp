/*
 * Util.cpp
 *
 *  Created on: Jun 2, 2011
 *      Author: David
 */

#include "Util.h"
#include <stdio.h>


void Util::Die()
{
    exit(1); //this should really kill the current process, so things get properly cleaned up. -dg TODO
}
void Util::Die(const char* message)
{
    if(message) printf(message); //should be puts() but it doesn't add the newline.  -dg
    putchar('\n');
    Die();
}
bool Util::Assert(bool test, const char* message)
{
    if(test == false)
    {
        puts(message);
        putchar('\n'); //puts isn't working; manually append newline.
    }
    return test;
}
