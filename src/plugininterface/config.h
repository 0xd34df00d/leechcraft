#ifndef CONFIG_H
#define CONFIG_H
#include <QtGlobal>

#if defined(Q_CC_GNU)

# if defined(plugininterface_EXPORTS)
#  define LEECHCRAFT_API __attribute__ ((visibility("default")))
# else
#  define LEECHCRAFT_API
# endif

#elif defined(Q_CC_MSVC)

# if defined(plugininterface_EXPORTS)
#  define LEECHCRAFT_API __declspec(dllexport)
# else
#  define LEECHCRAFT_API __declspec(dllimport)
# endif

#else
# define LEECHCRAFT_API
#endif

#endif

