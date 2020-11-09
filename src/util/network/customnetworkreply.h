/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkReply>
#include "networkconfig.h"

namespace LC::Util
{
	/** @brief A network reply with customizable content and reply headers.
	 *
	 * This class provides a custom network reply with settable content
	 * and headers. It can be used, for example, in a plugin that renders
	 * local filesystem to QNetworkAccessManager-enabled plugins, or that
	 * just needs to provide a network reply with a predefined or
	 * runtime-generated string.
	 *
	 * @ingroup NetworkUtil
	 */
	class UTIL_NETWORK_API CustomNetworkReply : public QNetworkReply
	{
		QByteArray Content_;
		qint64 Offset_ = 0;
	public:
		/** @brief Creates the reply with the given url and parent.
		 *
		 * Sets the URL of this reply to be \em url. This URL will be
		 * returned by <code>QNetworkReply::url()</code>
		 *
		 * @param[in] url The URL this custom reply corresponds to.
		 * @param[in] parent The parent object of this object.
		 */
		explicit CustomNetworkReply (const QUrl& url, QObject *parent = nullptr);

		using QNetworkReply::setError;
		using QNetworkReply::setHeader;

		/** @brief Sets the content type of this reply.
		 *
		 * This function sets the <em>Content-Type</em> header to \em type.
		 *
		 * It is equivalent to
		 * \code
		 * SetHeader (QNetworkRequest::ContentType, type);
		 * \endcode
		 */
		void SetContentType (const QByteArray& type);

		/** @brief Sets content of this reply to the given string.
		 *
		 * This convenience function is equivalent to
		 * \code
		 * SetContent (string.toUtf8 ());
		 * \endcode
		 *
		 * @param[in] string The string to set.
		 */
		void SetContent (const QString& string);

		/** @brief Sets content of this reply to the given data.
		 *
		 * This function sets the content of this reply to the given
		 * \em data, updates the <em>Content-Length</em> header and
		 * schedules emission of the <code>readyRead()</code> and
		 * <code>finished()</code> signals next time control reaches the
		 * event loop.
		 *
		 * @param[in] data The data this network reply should contain.
		 */
		void SetContent (const QByteArray& data);

		/** @brief Reimplemented from QNetworkReply::abort().
		 *
		 * This function does nothing.
		 */
		void abort () override;

		/** @brief Reimplemented from QNetworkReply::bytesAvailable().
		 *
		 * This function returns the number of bytes left unread.
		 */
		qint64 bytesAvailable () const override;

		/** @brief Reimplemented from QNetworkReply::isSequential().
		 *
		 * This function always returns <code>true</code>.
		 */
		bool isSequential () const override;
	protected:
		qint64 readData (char*, qint64) override;
	};
}
