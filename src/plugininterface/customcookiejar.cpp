/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "customcookiejar.h"
#include <QtDebug>

using namespace LeechCraft::Util;

CustomCookieJar::CustomCookieJar (QObject *parent)
: QNetworkCookieJar (parent)
, FilterTrackingCookies_ (false)
{
}

CustomCookieJar::~CustomCookieJar ()
{
}

void CustomCookieJar::SetFilterTrackingCookies (bool filter)
{
	FilterTrackingCookies_ = filter;
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
	QList<QNetworkCookie> cookies, filteredCookies;
	for (QList<QByteArray>::const_iterator i = spcookies.begin (),
			end = spcookies.end (); i != end; ++i)
		cookies += QNetworkCookie::parseCookies (*i);
	Q_FOREACH (QNetworkCookie cookie, cookies)
		if (!(FilterTrackingCookies_ &&
					cookie.name ().startsWith ("__utm")))
			filteredCookies << cookie;
	setAllCookies (filteredCookies);
}

