#ifndef PLUGININTERFACE_CUSTOMCOOKIEJAR_H
#define PLUGININTERFACE_CUSTOMCOOKIEJAR_H
#include <QNetworkCookieJar>
#include <QByteArray>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class PLUGININTERFACE_API CustomCookieJar : public QNetworkCookieJar
		{
			Q_OBJECT
		public:
			CustomCookieJar (QObject* = 0);
			virtual ~CustomCookieJar ();

			QByteArray Save () const;
			void Load (const QByteArray&);

			using QNetworkCookieJar::allCookies;
			using QNetworkCookieJar::setAllCookies;
		};
	};
};

#endif

