#ifndef CUSTOMCOOKIEJAR_H
#define CUSTOMCOOKIEJAR_H
#include <QNetworkCookieJar>
#include <QByteArray>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class CustomCookieJar : public QNetworkCookieJar
		{
			Q_OBJECT
		public:
			LEECHCRAFT_API CustomCookieJar (QObject* = 0);
			LEECHCRAFT_API virtual ~CustomCookieJar ();

			LEECHCRAFT_API QByteArray Save () const;
			LEECHCRAFT_API void Load (const QByteArray&);
		};
	};
};

#endif

