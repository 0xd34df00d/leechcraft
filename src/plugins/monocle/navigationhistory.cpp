/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "navigationhistory.h"
#include <QFileInfo>
#include <QMenu>
#include "documenttab.h"

namespace LC::Monocle
{
	NavigationHistory::NavigationHistory (QObject *parent)
	: QObject { parent }
	, BackwardMenu_ { new QMenu }
	, ForwardMenu_ { new QMenu }
	{
	}

	QMenu* NavigationHistory::GetBackwardMenu () const
	{
		return BackwardMenu_;
	}

	QMenu* NavigationHistory::GetForwardMenu () const
	{
		return ForwardMenu_;
	}

	namespace
	{
		void GoSingleAction (QMenu& menu)
		{
			const auto& actions = menu.actions ();
			if (!actions.isEmpty ())
				actions.front ()->trigger ();
		}
	}

	void NavigationHistory::GoBack () const
	{
		GoSingleAction (*BackwardMenu_);
	}

	void NavigationHistory::GoForward () const
	{
		GoSingleAction (*ForwardMenu_);
	}

	void NavigationHistory::SaveCurrentPos (const ExternalNavigationAction& entry)
	{
		const auto& backActions = BackwardMenu_->actions ();
		BackwardMenu_->insertAction (backActions.value (0), MakeCurrentPositionAction (entry));
		emit backwardHistoryAvailabilityChanged (true);

		if (CurrentAction_)
		{
			delete *CurrentAction_;
			CurrentAction_.reset ();
		}

		const auto& fwdActions = ForwardMenu_->actions ();
		ForwardMenu_->clear ();
		qDeleteAll (fwdActions);
		emit forwardHistoryAvailabilityChanged (false);
	}

	namespace
	{
		QString GetEntryText (const ExternalNavigationAction& entry)
		{
			return NavigationHistory::tr ("Page %1 (%2)")
					.arg (entry.DocumentNavigation_.PageNumber_)
					.arg (QFileInfo { entry.TargetDocument_ }.fileName ());
		}
	}

	QAction* NavigationHistory::MakeCurrentPositionAction (const ExternalNavigationAction& entry)
	{
		const auto action = new QAction { GetEntryText (entry), this };
		connect (action,
				&QAction::triggered,
				this,
				[entry, action, this] { GoTo (action, entry); });
		return action;
	}

	namespace
	{
		template<typename T>
		QList<T> MaybeToList (const std::optional<T>& maybe)
		{
			return maybe ? QList { *maybe } : QList<T> {};
		}
	}

	void NavigationHistory::GoTo (QAction *action, const ExternalNavigationAction& entry)
	{
		const auto& allActions = BackwardMenu_->actions () + MaybeToList (CurrentAction_) + ForwardMenu_->actions ();
		const auto actIdx = allActions.indexOf (action);

		const auto& backActions = allActions.mid (0, actIdx);
		const auto& fwdActions = allActions.mid (actIdx + 1);

		BackwardMenu_->clear ();
		BackwardMenu_->addActions (backActions);
		ForwardMenu_->clear ();
		ForwardMenu_->addActions (fwdActions);

		CurrentAction_ = action;

		emit backwardHistoryAvailabilityChanged (!backActions.isEmpty ());
		emit forwardHistoryAvailabilityChanged (!fwdActions.isEmpty ());
		emit navigationRequested (entry);
	}
}
