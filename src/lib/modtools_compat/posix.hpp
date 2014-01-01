#ifndef MODTOOLS_COMPAT_POSIX_HPP
#define MODTOOLS_COMPAT_POSIX_HPP

/*
 * Windows includes many functions from POSIX, however it prepends an underscore.
 * This header provides a compatibility layer so they also work under Unix.
 */

#ifdef _MSC_VER
# define stat _stat
#else
# define _stat stat
#endif

#endif
