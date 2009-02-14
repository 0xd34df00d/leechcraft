#ifndef CUSTOMCOOKIEJAR_H
#define CUSTOMCOOKIEJAR_H
#include <QNetworkCookieJar>
#include <QByteArray>

namespace LeechCraft
{
	class CustomCookieJar : public QNetworkCookieJar
	{
		Q_OBJECT
	public:
		CustomCookieJar (QObject* = 0);
		virtual ~CustomCookieJar ();

		QByteArray Save () const;
		void Load (const QByteArray&);
	};
};

#endif

