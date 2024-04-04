#ifndef _Debug_H_
#define _Debug_H_

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <string.h>

#define DBG		0
#if DBG
#define AR_DBG()	printf("[%s %s %d]\n", strrchr(__FILE__, '\\') + 1, __func__, __LINE__)
#else
#define AR_DBG()
#endif

#endif
#endif
