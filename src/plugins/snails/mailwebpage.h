/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebEnginePage>
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

	class MailWebPage : public QWebEnginePage
	{
		MessagePageContext Ctx_;
	public:
		explicit MailWebPage (QObject* = nullptr);

		void SetMessageContext (const MessagePageContext&);
	protected:
		bool acceptNavigationRequest (const QUrl&, NavigationType, bool) override;
	};
}
}
