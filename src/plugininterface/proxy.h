#ifndef PLUGININTERFACE_PROXY_H
#define PLUGININTERFACE_PROXY_H
#include <QObject>
#include <QStringList>
#include <QTime>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		/*! @brief Provides some common features.
		 *
		 * Feature versions of Proxy class may include some sort of
		 * communications with MainWindow class as it was before removing of
		 * LogShower in main LeechCraft application.
		 *
		 */
		class Proxy : public QObject
		{
			Q_OBJECT

			Proxy ();
			~Proxy ();

			static Proxy *Instance_;
			QStringList Strings_;
		public:
			PLUGININTERFACE_API static Proxy *Instance ();
			PLUGININTERFACE_API void SetStrings (const QStringList&);
			PLUGININTERFACE_API QString GetApplicationName () const;
			PLUGININTERFACE_API QString GetOrganizationName () const;
			PLUGININTERFACE_API QString MakePrettySize (qint64) const;
			PLUGININTERFACE_API QString MakeTimeFromLong (ulong) const;
		};
	};
};

#endif

