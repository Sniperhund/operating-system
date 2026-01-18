#pragma once

#include "stdint.h"

static uint8_t s_error = 0;

#define E_KILL      255 /* The kernel should immediately kill the offending process */

#define E_NOENT     1   /* No such file or directory */
#define E_INVAL     2   /* Invalid or missing argument */
#define E_NOTDIR    3   /* Not a directory */
#define E_ISDIR     4   /* Is a directory */
#define E_NOTUNIQ   5   /* Name not unique */
#define E_MOUNTNPOS 6   /* Mount not possible */
#define E_NOMEM     7   /* Cannot allocate memory */
#define E_MFILE     8   /* Too many open files */