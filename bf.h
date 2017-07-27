#ifndef _INCLUDE_BF_H
#define _INCLUDE_BF_H

#include <jit/jit.h>

typedef void (*Function)(void);

Function get_compiled(jit_context_t context, char* program);

#endif
