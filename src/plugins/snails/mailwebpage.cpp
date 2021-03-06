/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mailwebpage.h"
#include <QNetworkRequest>
#include <QtDebug>
#include <QUrlQuery>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "mailwebpagenam.h"

namespace LC
{
namespace Snails
{
	MailWebPage::MailWebPage (const ICoreProxy_ptr& proxy, QObject *parent)
	: QWebPage { parent }
	, Proxy_ { proxy }
	{
		setNetworkAccessManager (new MailWebPageNAM { [this] { return Ctx_; }, this });
	}

	void MailWebPage::SetMessageContext (const MessagePageContext& ctx)
	{
		Ctx_ = ctx;
	}

	bool MailWebPage::acceptNavigationRequest (QWebFrame*, const QNetworkRequest& req, QWebPage::NavigationType type)
	{
		const auto& url = req.url ();
		if (type == NavigationTypeLinkClicked &&
				url.scheme () != "snails")
		{
			const auto& e = Util::MakeEntity (url, {}, FromUserInitiated);
			Proxy_->GetEntityManager ()->HandleEntity (e);
			return false;
		}

		if (url.scheme () == "snails")
		{
			if (url.host () == "attachment")
				HandleAttachment (url);
		}

		return false;
	}

	void MailWebPage::HandleAttachment (const QUrl& url)
	{
		const QUrlQuery queryable { url };
		const auto& msgId = queryable.queryItemValue ("msgId").toUtf8 ();
		const auto& folder = queryable.queryItemValue ("folderId").split ('/');
		const auto& attName = queryable.queryItemValue ("attName");

		emit attachmentSelected (msgId, folder, attName);
	}
}
}
