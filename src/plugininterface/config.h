#ifndef PLUGININTERFACE_CONFIG_H
#define PLUGININTERFACE_CONFIG_H
#include <QtGlobal>

#if defined(Q_CC_GNU)

# if defined(plugininterface_EXPORTS)
#  define PLUGININTERFACE_API __attribute__ ((visibility("default")))
# else
#  define PLUGININTERFACE_API
# endif

#elif defined(Q_CC_MSVC)

# if defined(plugininterface_EXPORTS)
#  define PLUGININTERFACE_API __declspec(dllexport)
# else
#  define PLUGININTERFACE_API __declspec(dllimport)
# endif

#else
# define PLUGININTERFACE_API
#endif

#endif

