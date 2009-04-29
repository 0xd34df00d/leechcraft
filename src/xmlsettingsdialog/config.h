#ifndef XMLSETTINGSDIALOG_CONFIG_H
#define XMLSETTINGSDIALOG_CONFIG_H
#include <QtGlobal>

#if defined(Q_CC_GNU)

# if defined(xmlsettingsdialog_EXPORTS)
#  define XMLSETTINGSMANAGER_API __attribute__ ((visibility("default")))
# else
#  define XMLSETTINGSMANAGER_API
# endif

#elif defined(Q_CC_MSVC)

# if defined(xmlsettingsdialog_EXPORTS)
#  define XMLSETTINGSMANAGER_API __declspec(dllexport)
# else
#  define XMLSETTINGSMANAGER_API __declspec(dllimport)
# endif

#else
# define XMLSETTINGSMANAGER_API
#endif

#endif

