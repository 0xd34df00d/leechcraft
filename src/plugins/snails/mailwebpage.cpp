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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "mailwebpagenam.h"

namespace LC
{
namespace Snails
{
	MailWebPage::MailWebPage (QObject *parent)
	: QWebPage { parent }
	{
		setNetworkAccessManager (new MailWebPageNAM { [this] { return Ctx_; }, this });
	}

	void MailWebPage::SetMessageContext (const MessagePageContext& ctx)
	{
		Ctx_ = ctx;
	}

	bool MailWebPage::acceptNavigationRequest (QWebFrame*, const QNetworkRequest& req, QWebPage::NavigationType type)
	{
		if (type == NavigationTypeLinkClicked)
		{
			const auto& e = Util::MakeEntity (req.url (), {}, FromUserInitiated);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		return false;
	}
}
}
