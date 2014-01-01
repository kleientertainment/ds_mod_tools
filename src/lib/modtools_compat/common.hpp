#ifndef MODTOOLS_COMPAT_COMMON_HPP
#define MODTOOLS_COMPAT_COMMON_HPP

#if defined(_WIN32) || defined(WIN32)
# define IS_WINDOWS 1
#elif defined(__linux__)
# define IS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
# define IS_MAC 1
#else
# error "Unknown operating system"
#endif

#if defined(IS_LINUX) || defined(IS_MAC)
# define IS_UNIX
#endif

#endif
