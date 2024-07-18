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
	NavigationHistory::NavigationHistory (DocumentTab& docTab)
	: QObject { &docTab }
	, DocTab_ { docTab }
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

	void NavigationHistory::SaveCurrentPos ()
	{
		CurrentAction_.reset ();

		const auto action = MakeCurrentPositionAction ();

		const auto& backActions = BackwardMenu_->actions ();
		if (backActions.isEmpty ())
		{
			BackwardMenu_->addAction (action);
			emit backwardHistoryAvailabilityChanged (true);
		}
		else
			BackwardMenu_->insertAction (backActions.front (), action);

		if (!ForwardMenu_->actions ().isEmpty ())
		{
			ForwardMenu_->clear ();
			emit forwardHistoryAvailabilityChanged (false);
		}
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

	QAction* NavigationHistory::MakeCurrentPositionAction ()
	{
		const auto& entry = DocTab_.GetNavigationHistoryEntry ();
		const auto action = new QAction { GetEntryText (entry), this };
		connect (action,
				&QAction::triggered,
				this,
				[entry, action, this] { GoTo (action, entry); });
		return action;
	}

	void NavigationHistory::GoTo (QAction *action, const ExternalNavigationAction& entry)
	{
		auto backActions = BackwardMenu_->actions ();
		auto fwdActions = ForwardMenu_->actions ();

		const auto isBack = backActions.contains (action);
		auto& from = isBack ? backActions : fwdActions;
		auto& to = isBack ? fwdActions : backActions;

		auto fromIdx = from.indexOf (action);
		std::copy (from.begin (), from.begin () + fromIdx, std::front_inserter (to));
		from.erase (from.begin (), from.begin () + fromIdx + 1);

		if (CurrentAction_)
			to.push_front (*CurrentAction_);
		else
			to.push_front (MakeCurrentPositionAction ());

		BackwardMenu_->clear ();
		BackwardMenu_->addActions (backActions);
		ForwardMenu_->clear ();
		ForwardMenu_->addActions (fwdActions);

		emit backwardHistoryAvailabilityChanged (!backActions.isEmpty ());
		emit forwardHistoryAvailabilityChanged (!fwdActions.isEmpty ());

		CurrentAction_ = action;

		emit navigationRequested (entry);
	}
}
