/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "documentactions.h"
#include <QAction>
#include "interfaces/monocle/idocument.h"
#include "docinfodialog.h"

namespace LC::Monocle
{
	DocumentActions::DocumentActions (IDocument& doc, const Deps& deps)
	: Deps_ { deps }
	, Doc_ { doc }
	{
		Entries_ << Separator {};

		auto infoAction = new QAction (tr ("Document info..."), this);
		infoAction->setProperty ("ActionIcon", "dialog-information");
		connect (infoAction,
				&QAction::triggered,
				this,
				[this]
				{
					auto dia = new DocInfoDialog { Doc_, &Deps_.DocTabWidget_ };
					dia->setAttribute (Qt::WA_DeleteOnClose);
					dia->show ();
				});
		Entries_ << infoAction;
	}

	const QVector<DocumentActions::Entry>& DocumentActions::GetEntries () const
	{
		return Entries_;
	}
}
