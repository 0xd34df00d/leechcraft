/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "messagelisteditormanager.h"
#include <QTreeView>
#include <QEvent>
#include <util/sll/lambdaeventfilter.h>
#include "common.h"
#include "mailmodel.h"

namespace LC
{
namespace Snails
{
	MessageListEditorManager::MessageListEditorManager (QTreeView *view, QObject *parent)
	: QObject { parent }
	, View_ { view }
	, Mode_ { MailListMode::Normal }
	{
		View_->setMouseTracking (true);

		connect (View_,
				&QTreeView::entered,
				[this] (const QModelIndex& index)
				{
					CloseCurrent ();

					View_->openPersistentEditor (index);
					LastEdited_ = index;
				});

		const auto ef = Util::MakeLambdaEventFilter ([this] (QEvent *event)
				{
					if (event->type () == QEvent::Leave)
						CloseCurrent ();
					return false;
				},
				this);
		View_->installEventFilter (ef);
	}

	void MessageListEditorManager::SetMailListMode (MailListMode mode)
	{
		if (mode == Mode_)
			return;

		Mode_ = mode;
		HandleMessageListUpdated ();
	}

	void MessageListEditorManager::HandleMessageListUpdated ()
	{
		if (Mode_ == MailListMode::MultiSelect)
			CloseCurrent ();
	}

	void MessageListEditorManager::CloseCurrent ()
	{
		if (LastEdited_.isValid ())
		{
			View_->closePersistentEditor (LastEdited_);
			LastEdited_ = {};
		}
	}
}
}
