/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "composemessagetabfactory.h"
#include <QMessageBox>
#include <util/sll/visitor.h>
#include "composemessagetab.h"
#include "core.h"
#include "structures.h"
#include "messageinfo.h"
#include "messagebodies.h"

namespace LC
{
namespace Snails
{
	ComposeMessageTabFactory::ComposeMessageTabFactory (const AccountsManager *accsMgr,
			const MsgTemplatesManager *templatesMgr, QObject *parent)
	: QObject { parent }
	, AccsMgr_ { accsMgr }
	, TemplatesMgr_ { templatesMgr }
	{
	}

	ComposeMessageTab* ComposeMessageTabFactory::MakeTab () const
	{
		return new ComposeMessageTab { AccsMgr_, TemplatesMgr_ };
	}

	void ComposeMessageTabFactory::PrepareComposeTab (const Account_ptr& account)
	{
		const auto cmt = MakeTab ();

		cmt->SelectAccount (account);
		cmt->PrepareLinked (MsgType::New, {}, {});

		emit gotTab (cmt->GetTabClassInfo ().VisibleName_, cmt);
	}

	void ComposeMessageTabFactory::PrepareLinkedTab (MsgType type,
			const Account_ptr& account, const MessageInfo& msgInfo,
			const std::variant<MessageBodies, Account::FetchWholeMessageResult_t>& bodiesVar)
	{
		const auto cmt = MakeTab ();

		cmt->SelectAccount (account);

		Util::Visit (bodiesVar,
				[cmt, type, msgInfo] (const MessageBodies& bodies) { cmt->PrepareLinked (type, msgInfo, bodies); },
				[cmt, type, msgInfo] (const Account::FetchWholeMessageResult_t& future)
				{
					cmt->setEnabled (false);

					Util::Sequence (cmt, future) >>
							[cmt, type, msgInfo] (const auto& result)
							{
								Util::Visit (result.AsVariant (),
										[cmt, type, msgInfo] (const MessageBodies& bodies)
										{
											cmt->setEnabled (true);
											cmt->PrepareLinked (type, msgInfo, bodies);
										},
										[cmt] (const auto& err)
										{
											QMessageBox::critical (cmt,
													"LeechCraft",
													tr ("Unable to fetch message: %1.")
														.arg (Util::Visit (err,
																[] (auto err) { return err.what (); })));

											cmt->Remove ();
										});
							};
				});

		emit gotTab (cmt->GetTabClassInfo ().VisibleName_, cmt);
	}
}
}
