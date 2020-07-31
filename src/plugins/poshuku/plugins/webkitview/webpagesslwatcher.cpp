/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webpagesslwatcher.h"
#include <cstring>
#include <QSslConfiguration>
#include <QNetworkReply>
#include <qwebview.h>
#include <qwebpage.h>
#include <qwebframe.h>

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	WebPageSslWatcher::WebPageSslWatcher (QWebView *view)
	: QObject { view }
	, Page_ { view->page () }
	{
		connect (view,
				SIGNAL (navigateRequested (QUrl)),
				this,
				SLOT (resetStats ()));
		connect (view,
				SIGNAL (urlChanged (QUrl)),
				this,
				SLOT (resetStats ()));

		const auto page = view->page ();
		connect (page->networkAccessManager (),
				SIGNAL (requestCreated (QNetworkAccessManager::Operation,
						QNetworkRequest, QNetworkReply*)),
				this,
				SLOT (handleReplyCreated (QNetworkAccessManager::Operation,
						QNetworkRequest, QNetworkReply*)));
		connect (page,
				SIGNAL (hookAcceptNavigationRequest (LC::IHookProxy_ptr,
						QWebPage*,
						QWebFrame*,
						const QNetworkRequest&,
						QWebPage::NavigationType)),
				this,
				SLOT (handleNavigationRequest (LC::IHookProxy_ptr,
						QWebPage*,
						QWebFrame*,
						const QNetworkRequest&,
						QWebPage::NavigationType)));
	}

	WebPageSslWatcher::State WebPageSslWatcher::GetPageState () const
	{
		if (!ErrSslResources_.isEmpty ())
			return State::SslErrors;
		else if (SslResources_.isEmpty ())
			return State::NoSsl;
		else if (!NonSslResources_.isEmpty ())
			return State::UnencryptedElems;
		else
			return State::FullSsl;
	}

	const QSslConfiguration& WebPageSslWatcher::GetPageConfiguration () const
	{
		return PageConfig_;
	}

	QList<QUrl> WebPageSslWatcher::GetNonSslUrls () const
	{
		return NonSslResources_;
	}

	QMap<QUrl, QList<QSslError>> WebPageSslWatcher::GetErrSslUrls () const
	{
		return ErrSslResources_;
	}

	void WebPageSslWatcher::handleReplyFinished ()
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		const auto& url = reply->url ();

		if (url.scheme () == "data")
			return;

		if (reply->attribute (QNetworkRequest::SourceIsFromCacheAttribute).toBool ())
			return;

		const auto& sslConfig = reply->sslConfiguration ();
		if (sslConfig.peerCertificate ().isNull ())
		{
			if (!reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).isNull ())
				NonSslResources_ << url;
		}
		else
		{
			SslResources_ << url;

			const auto& frameUrl = Page_->mainFrame ()->url ();
			if (url.host () == frameUrl.host ())
			{
				qDebug () << Q_FUNC_INFO
						<< "detected main frame cert for URL"
						<< url;
				PageConfig_ = sslConfig;
			}
		}

		emit sslStateChanged (this);
	}

	void WebPageSslWatcher::handleSslErrors (const QList<QSslError>& errors)
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		ErrSslResources_ [reply->url ()] += errors;
	}

	namespace
	{
		bool CheckReplyFrame (QObject *original, QWebFrame *mainFrame)
		{
			if (!original ||
					std::strcmp (original->metaObject ()->className (), "QWebFrame"))
				return false;

			auto webFrame = qobject_cast<QWebFrame*> (original);
			while (const auto parent = webFrame->parentFrame ())
				webFrame = parent;

			return webFrame == mainFrame;
		}
	}

	void WebPageSslWatcher::handleReplyCreated (QNetworkAccessManager::Operation,
			const QNetworkRequest& req, QNetworkReply *reply)
	{
		if (!CheckReplyFrame (req.originatingObject (), Page_->mainFrame ()))
			return;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (sslErrors (QList<QSslError>)),
				this,
				SLOT (handleSslErrors (QList<QSslError>)));
	}

	void WebPageSslWatcher::resetStats ()
	{
		qDebug () << Q_FUNC_INFO;
		SslResources_.clear ();
		NonSslResources_.clear ();
		ErrSslResources_.clear ();

		PageConfig_ = {};

		emit sslStateChanged (this);
	}

	void WebPageSslWatcher::handleNavigationRequest (IHookProxy_ptr,
			QWebPage*, QWebFrame *frame, const QNetworkRequest&, QWebPage::NavigationType type)
	{
		if (frame != Page_->mainFrame () ||
				type == QWebPage::NavigationTypeOther)
			return;

		resetStats ();
	}
}
}
}
