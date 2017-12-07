cd ..\Config
perl build.pl
cd ..
astyle "*.cpp" "*.h"
cd PPC603gnu
%makeprefix% make --no-print-directory %CmdCmdLine:~22%