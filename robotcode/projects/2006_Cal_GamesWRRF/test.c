void ordinary_procedure(void)
{
}

char returnsValue(void)
{
	return 0x78;
}
void takesArg(char x)
{

}
void hasLocals(void)
{
	int i=0x1234;
}
void hasGlobals(void)
{
	static int i;
	i=0x2345;

}


#pragma interruptlow fooA
void fooA(void)
{
}


#pragma interruptlow fooB save=PROD
void fooB(void)
{
}

#pragma interruptlow fooC save=PROD,section(".tmpdata")
void fooC(void)
{
}

#pragma interruptlow fooD save=section(".tmpdata")
void fooD(void)
{
}
