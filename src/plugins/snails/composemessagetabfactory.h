/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include "account.h"

namespace LC
{
namespace Snails
{
	class ComposeMessageTab;
	class MsgTemplatesManager;
	class AccountsManager;

	enum class MsgType;

	class ComposeMessageTabFactory : public QObject
	{
		Q_OBJECT

		const AccountsManager * const AccsMgr_;
		const MsgTemplatesManager * const TemplatesMgr_;
	public:
		ComposeMessageTabFactory (const AccountsManager*,
				const MsgTemplatesManager*, QObject* = nullptr);

		ComposeMessageTab* MakeTab () const;

		void PrepareComposeTab (const Account_ptr&);
		void PrepareLinkedTab (MsgType, const Account_ptr&, const MessageInfo&,
				const std::variant<MessageBodies, Account::FetchWholeMessageResult_t>&);
	signals:
		void gotTab (const QString&, QWidget*);
	};
}
}
