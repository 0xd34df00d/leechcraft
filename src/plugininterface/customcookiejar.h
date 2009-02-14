#ifndef CUSTOMCOOKIEJAR_H
#define CUSTOMCOOKIEJAR_H
#include <QNetworkCookieJar>
#include <QByteArray>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class LEECHCRAFT_API CustomCookieJar : public QNetworkCookieJar
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

