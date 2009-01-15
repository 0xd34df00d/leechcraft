#include "customcookiejar.h"
#include <QtDebug>

using namespace LeechCraft;

CustomCookieJar::CustomCookieJar (QObject *parent)
: QNetworkCookieJar (parent)
{
}

CustomCookieJar::~CustomCookieJar ()
{
}

QByteArray CustomCookieJar::Save () const
{
	QList<QNetworkCookie> cookies = allCookies ();
	QByteArray result;
	for (QList<QNetworkCookie>::const_iterator i = cookies.begin (),
			end = cookies.end (); i != end; ++i)
	{
		result += i->toRawForm ();
		result += "\n";
	}
	return result;
}

void CustomCookieJar::Load (const QByteArray& data)
{
	QList<QByteArray> spcookies = data.split ('\n');
	QList<QNetworkCookie> cookies;
	for (QList<QByteArray>::const_iterator i = spcookies.begin (),
			end = spcookies.end (); i != end; ++i)
		cookies += QNetworkCookie::parseCookies (*i);
	setAllCookies (cookies);
}

