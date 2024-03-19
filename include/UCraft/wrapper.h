#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef _MSC_VER
#include "wrapper_win.h"
#else
#include "wrapper_unix.h"
#endif
#endif