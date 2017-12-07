#include "Build.h"
#include "../Buildnumber.h"

//TODO: Cleanup and remove once tested.
//old build was 5058
//const int Build::NUMBER = 5061;
const int Build::NUMBER = kBuildNumber;
//const std::string Build::TIME = "Thu Jun 16 09:11:15 2011";

const std::string Build::TIME = __DATE__ " " __TIME__;
