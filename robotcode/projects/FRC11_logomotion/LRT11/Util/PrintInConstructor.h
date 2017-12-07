#ifndef PRINT_IN_CONSTRUCTOR_H_
#define PRINT_IN_CONSTRUCTOR_H_

#include <string>
//Allows messages to be printed during the construction and destruction of objects. -dg
//Use as:   func_constructor(): print_brain("Constructing Brain"), brain(args) ... { }

class PrintInConstructor
{
public:
    PrintInConstructor(const char* ctor_dtor_message);
    PrintInConstructor(const char* ctor_message, const char* dtor_message);
    ~PrintInConstructor();

protected:
    void Initialize(const char* ctor_message, const char* dtor_message);

public:
    std::string destructorMessage_;
};
#endif //PRINT_IN_CONSTRUCTOR_H_
