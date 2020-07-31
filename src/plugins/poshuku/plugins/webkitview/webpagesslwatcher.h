/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QNetworkAccessManager>
#include <QSslConfiguration>
#include <QWebPage>
#include <interfaces/core/ihookproxy.h>

class QNetworkReply;
class QUrl;
class QWebView;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class WebPageSslWatcher : public QObject
	{
		Q_OBJECT

		QWebPage * const Page_;

		QList<QUrl> SslResources_;
		QList<QUrl> NonSslResources_;
		QMap<QUrl, QList<QSslError>> ErrSslResources_;

		QSslConfiguration PageConfig_;
	public:
		WebPageSslWatcher (QWebView*);

		enum class State
		{
			NoSsl,
			SslErrors,
			UnencryptedElems,
			FullSsl
		};
		State GetPageState () const;

		const QSslConfiguration& GetPageConfiguration () const;

		QList<QUrl> GetNonSslUrls () const;
		QMap<QUrl, QList<QSslError>> GetErrSslUrls () const;
	public slots:
		void resetStats ();
	private slots:
		void handleReplyFinished ();
		void handleSslErrors (const QList<QSslError>&);

		void handleReplyCreated (QNetworkAccessManager::Operation,
				const QNetworkRequest&, QNetworkReply*);

		void handleNavigationRequest (LC::IHookProxy_ptr,
				QWebPage*,
				QWebFrame*,
				const QNetworkRequest&,
				QWebPage::NavigationType);
	signals:
		void sslStateChanged (WebPageSslWatcher*);
	};
}
}
}
