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

namespace LC
{
namespace Monocle
{
	NavigationHistory::NavigationHistory (const EntryGetter_f& getter, QObject *parent)
	: QObject { parent }
	, EntryGetter_ { getter }
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

	void NavigationHistory::GoBack () const
	{
		GoSingleAction (BackwardMenu_);
	}

	void NavigationHistory::GoForward () const
	{
		GoSingleAction (ForwardMenu_);
	}

	void NavigationHistory::HandleSearchNavigationRequested ()
	{
		AppendHistoryEntry ();
	}

	void NavigationHistory::GoSingleAction (QMenu *menu) const
	{
		const auto& actions = menu->actions ();
		if (!actions.isEmpty ())
			actions.front ()->trigger ();
	}

	namespace
	{
		QString GetEntryText (const NavigationHistory::Entry& entry)
		{
			return NavigationHistory::tr ("Page %1 (%2)")
					.arg (entry.Position_.Page_)
					.arg (QFileInfo { entry.Document_ }.fileName ());
		}
	}

	void NavigationHistory::AppendHistoryEntry ()
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

	QAction* NavigationHistory::MakeCurrentPositionAction ()
	{
		const auto& entry = EntryGetter_ ();
		const auto action = new QAction { GetEntryText (entry), this };
		connect (action,
				&QAction::triggered,
				[entry, action, this] { GoTo (action, entry); });
		return action;
	}

	void NavigationHistory::GoTo (QAction *action, const Entry& entry)
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

		emit entryNavigationRequested (entry);
	}

	void NavigationHistory::handleDocumentNavigationRequested ()
	{
		AppendHistoryEntry ();
	}
}
}
