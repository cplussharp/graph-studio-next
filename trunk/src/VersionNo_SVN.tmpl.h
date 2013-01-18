#pragma once

#ifndef _SVN_VERSION_H_
#define _SVN_VERSION_H_
 
#define SVN_LOCAL_MODIFICATIONS $WCMODS?1:0$  // 1 if there are modifications to the local working copy, 0 otherwise
#define SVN_REVISION            $WCREV$       // Highest committed revision number in the working copy
#define SVN_TIME_NOW            $WCNOWUTC=%Y-%m-%d %H:%M:%S$       // Current system date &amp; time
 
#endif