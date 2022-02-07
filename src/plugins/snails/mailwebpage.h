/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebPage>
#include <interfaces/core/icoreproxy.h>
#include "messageinfo.h"

namespace LC
{
namespace Snails
{
	class Account;

	struct MessagePageContext
	{
		Account *Acc_ = nullptr;
		MessageInfo MsgInfo_;
	};

	class MailWebPage : public QWebPage
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		MessagePageContext Ctx_;
	public:
		MailWebPage (const ICoreProxy_ptr&, QObject* = nullptr);

		void SetMessageContext (const MessagePageContext&);
	protected:
		bool acceptNavigationRequest (QWebFrame*, const QNetworkRequest&, NavigationType) override;
	};
}
}
