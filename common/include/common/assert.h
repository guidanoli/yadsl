#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <assert.h>

// Necessary wrapping of assert function because MSVC implementation
// of assert simply ignores whatever is inside the assertion when the
// the application is compiled on Release.
#define _assert(x) do { int y = (int) (x); assert(y); } while(0)

#endif