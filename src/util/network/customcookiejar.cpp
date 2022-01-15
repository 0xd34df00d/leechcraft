/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customcookiejar.h"
#include <set>
#include <algorithm>
#include <QNetworkCookie>
#include <QtDebug>
#include <QDateTime>
#include <QtConcurrentRun>
#include <util/sll/util.h>
#include <util/threads/futures.h>

namespace LC::Util
{
	CustomCookieJar::CustomCookieJar (QObject *parent)
	: QNetworkCookieJar (parent)
	{
	}

	void CustomCookieJar::SetFilterTrackingCookies (bool filter)
	{
		FilterTrackingCookies_ = filter;
	}

	void CustomCookieJar::SetEnabled (bool enabled)
	{
		Enabled_ = enabled;
	}

	void CustomCookieJar::SetExactDomainMatch (bool enabled)
	{
		MatchDomainExactly_ = enabled;
	}

	void CustomCookieJar::SetWhitelist (const QList<QRegExp>& list)
	{
		WL_ = list;
	}

	void CustomCookieJar::SetBlacklist (const QList<QRegExp>& list)
	{
		BL_ = list;
	}

	QByteArray CustomCookieJar::Save () const
	{
		auto cookies = allCookies ();
		QByteArray result;
		for (const auto& cookie : cookies)
		{
			result += cookie.toRawForm ();
			result += "\n";
		}
		return result;
	}

	namespace
	{
		bool IsExpired (const QNetworkCookie& cookie, const QDateTime& now)
		{
			return !cookie.isSessionCookie () && cookie.expirationDate () < now;
		}
	}

	void CustomCookieJar::Load (const QByteArray& data)
	{
		QList<QNetworkCookie> cookies, filteredCookies;
		for (const auto& ba : data.split ('\n'))
			cookies << QNetworkCookie::parseCookies (ba);

		const auto& now = QDateTime::currentDateTime ();
		for (const auto& cookie : cookies)
		{
			if (FilterTrackingCookies_ &&
					cookie.name ().startsWith ("__utm"))
				continue;

			if (IsExpired (cookie, now))
				continue;

			filteredCookies << cookie;
		}
		emit cookiesAdded (filteredCookies);
		setAllCookies (filteredCookies);
	}

	void CustomCookieJar::CollectGarbage ()
	{
		const auto& cookies = allCookies ();
		QList<QNetworkCookie> result;
		const auto& now = QDateTime::currentDateTime ();
		for (const auto& cookie : cookies)
		{
			if (IsExpired (cookie, now))
				continue;

			if (result.contains (cookie))
				continue;

			result << cookie;
		}
		qDebug () << Q_FUNC_INFO << cookies.size () << result.size ();
		setAllCookies (result);
	}

	QList<QNetworkCookie> CustomCookieJar::cookiesForUrl (const QUrl& url) const
	{
		if (!Enabled_)
			return {};

		QList<QNetworkCookie> filtered;
		for (const auto& cookie : QNetworkCookieJar::cookiesForUrl (url))
			if (!filtered.contains (cookie))
				filtered << cookie;
		return filtered;
	}

	namespace
	{
		bool MatchDomain (const QString& rawDomain, const QString& rawCookieDomain)
		{
			auto normalize = [] (QStringView s)
			{
				return s.startsWith ('.') ? s.mid (1) : s;
			};
			const auto& domain = normalize (rawDomain);
			const auto& cookieDomain = normalize (rawCookieDomain);

			if (domain == cookieDomain)
				return true;

			const auto idx = domain.indexOf (cookieDomain);
			return idx > 0 && domain.at (idx - 1) == '.';
		}

		bool Check (const QList<QRegExp>& list, const QString& str)
		{
			return std::any_of (list.begin (), list.end (),
					[&str] (const auto& rx) { return str == rx.pattern () || rx.exactMatch (str); });
		}

		struct CookiesDiff
		{
			QList<QNetworkCookie> Added_;
			QList<QNetworkCookie> Removed_;
		};

		auto CookieToTuple (const QNetworkCookie& c)
		{
			return std::make_tuple (c.isHttpOnly (),
					c.isSecure (),
					c.isSessionCookie (),
					c.name (),
					c.domain (),
					c.path (),
					c.value (),
					c.expirationDate ());
		}

		struct CookieLess
		{
			bool operator() (const QNetworkCookie& left, const QNetworkCookie& right) const
			{
				return CookieToTuple (left) < CookieToTuple (right);
			}
		};

		CookiesDiff CheckDifferences (const QList<QNetworkCookie>& previousList,
				const QList<QNetworkCookie>& currentList)
		{
			using Set_t = std::set<QNetworkCookie, CookieLess>;
			Set_t previous { previousList.begin (), previousList.end () };
			Set_t current { currentList.begin (), currentList.end () };

			CookiesDiff diff;
			std::set_difference (previous.begin (), previous.end (),
					current.begin (), current.end (),
					std::back_inserter (diff.Removed_),
					CookieLess {});
			std::set_difference (current.begin (), current.end (),
					previous.begin (), previous.end (),
					std::back_inserter (diff.Added_),
					CookieLess {});
			return diff;
		}
	}

	bool CustomCookieJar::setCookiesFromUrl (const QList<QNetworkCookie>& cookieList, const QUrl& url)
	{
		if (!Enabled_)
			return false;

		QList<QNetworkCookie> filtered;
		filtered.reserve (cookieList.size ());
		for (auto cookie : cookieList)
		{
			if (cookie.domain ().isEmpty ())
				cookie.setDomain (url.host ());

			bool checkWhitelist = false;
			const auto wlGuard = Util::MakeScopeGuard ([&]
					{
						if (checkWhitelist && Check (WL_, cookie.domain ()))
							filtered << cookie;
					});

			if (MatchDomainExactly_ && !MatchDomain (url.host (), cookie.domain ()))
			{
				checkWhitelist = true;
				continue;
			}

			if (FilterTrackingCookies_ &&
					cookie.name ().startsWith ("__utm"))
			{
				checkWhitelist = true;
				continue;
			}

			if (!Check (BL_, cookie.domain ()))
				filtered << cookie;
		}

		const auto& existing = cookiesForUrl (url);
		if (existing.isEmpty ())
			emit cookiesAdded (filtered);
		else
			Util::Sequence (this, QtConcurrent::run (CheckDifferences, existing, filtered)) >>
					[this] (const CookiesDiff& diff)
					{
						if (!diff.Removed_.isEmpty ())
							emit cookiesRemoved (diff.Removed_);
						if (!diff.Added_.isEmpty ())
							emit cookiesAdded (diff.Added_);
					};

		return QNetworkCookieJar::setCookiesFromUrl (filtered, url);
	}
}
