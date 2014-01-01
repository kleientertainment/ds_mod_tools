#ifndef MODTOOLS_COMPAT_POSIX_HPP
#define MODTOOLS_COMPAT_POSIX_HPP

/*
 * Windows includes many functions from POSIX, however it prepends an underscore.
 * This header provides a compatibility layer so they also work under Unix.
 */

extern "C" {
#	include <sys/stat.h>
}

#ifdef _MSC_VER
# define stat _stat
#else
# ifndef _stat
#  define _stat stat
# endif
#endif

#endif
