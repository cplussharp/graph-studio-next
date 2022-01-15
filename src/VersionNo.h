#pragma once

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               0
#define VERSION_MINOR               7
#define VERSION_REVISION            3

#if ENV_BUILD_NR > 0
#define VERSION_BUILD               ENV_BUILD_NR
#else
#define VERSION_BUILD               0
#endif

#define VERSION_MODIFIER

#define VER_FILE_DESCRIPTION_STR    "Built " __DATE__ " " __TIME__ " "

#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
                                    "." STRINGIZE(VERSION_BUILD)

