/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QByteArray>
#include <QRegularExpression>
#include "networkconfig.h"

namespace LC::Util
{
	/** @brief A customized cookie jar with additional features.
	 *
	 * Allows one to filter tracking cookies, filter duplicate cookies
	 * and has unlimited storage period.
	 *
	 * @ingroup NetworkUtil
	 */
	class UTIL_NETWORK_API CustomCookieJar : public QNetworkCookieJar
	{
		Q_OBJECT

		bool FilterTrackingCookies_ = false;
		bool Enabled_ = true;
		bool MatchDomainExactly_ = false;

		QList<QRegularExpression> WL_;
		QList<QRegularExpression> BL_;
	public:
		/** @brief Constructs the cookie jar.
		 *
		 * Filtering of tracking cookies is false by default, and
		 * cookies aren't restored.
		 *
		 * @param[in] parent The parent object.
		 */
		explicit CustomCookieJar (QObject *parent = nullptr);

		/** Enables or disables filtering tracking cookies.
		 *
		 * @param[in] filter Whether to filter tracking cookies.
		 */
		void SetFilterTrackingCookies (bool filter);

		/** @brief Enables or disables the cookies.
		 *
		 * If cookie jar is disabled, no new cookies will be saved and
		 * no cookies will be returned for any URL.
		 *
		 * @param[in] enabled Whether the cookie jar should be
		 * enabled.
		 */
		void SetEnabled (bool enabled);

		/** @brief Sets whether exact domain matching is enabled.
		 *
		 * @param[in] enabled Whether exact matching is enabled.
		 */
		void SetExactDomainMatch (bool enabled);

		/** @brief Sets the cookies whitelist.
		 *
		 * Cookies whose domains match regexps from the list will always
		 * be accepted even despite the SetFilterTrackingCookies() and
		 * SetExactDomainMatch() settings.
		 *
		 * If a cookie domain matches both a whitelist regexp and
		 * blacklist regexp, it is accepted.
		 *
		 * If cookies are disabled via SetEnabled(), this option has no
		 * effect.
		 *
		 * @param[in] list The whitelist.
		 *
		 * @sa SetBlacklist()
		 */
		void SetWhitelist (const QList<QRegularExpression>& list);

		/** @brief Sets the cookies blacklist.
		 *
		 * Cookies whose domains match regexps from the list will always
		 * be rejected until they are also present in the whitelist, in
		 * which case they are accepted.
		 *
		 * @param[in] list The blacklist.
		 *
		 * @sa SetWhitelist()
		 */
		void SetBlacklist (const QList<QRegularExpression>& list);

		/** Serializes the cookie jar contents into a QByteArray
		 * suitable for storage.
		 *
		 * @return The serialized cookies.
		 *
		 * @sa Load()
		 */
		[[nodiscard]] QByteArray Save () const;

		[[nodiscard]] static QByteArray Save (const QList<QNetworkCookie>&);

		/** Restores the cookies from the array previously obtained
		 * from Save().
		 *
		 * @param[in] data Serialized cookies.
		 * @sa Save()
		 */
		void Load (const QByteArray& data);

		/** Removes duplicate cookies.
		 */
		void CollectGarbage ();

		/** @brief Returns cookies for the given url.
		 *
		 * This function automatically filters out duplicate cookies.
		 *
		 * If the cookie jar is disabled, this function does nothing.
		 *
		 * @param[in] url The url to return cookies for.
		 * @return The list of cookies, dup-free.
		 */
		QList<QNetworkCookie> cookiesForUrl (const QUrl& url) const override;

		/** @brief Adds the cookieList for the given url to the jar.
		 *
		 * If the cookie jar is disabled, this function does nothing.
		 *
		 * @param[in] cookieList The list of cookies to add.
		 * @param[in] url The url to set cookies for.
		 * @return Whether the jar has been modified as the result.
		 */
		bool setCookiesFromUrl (const QList<QNetworkCookie>& cookieList, const QUrl& url) override;

		using QNetworkCookieJar::allCookies;
		using QNetworkCookieJar::setAllCookies;
	signals:
		void cookiesAdded (const QList<QNetworkCookie>&);
		void cookiesRemoved (const QList<QNetworkCookie>&);
	};
}
