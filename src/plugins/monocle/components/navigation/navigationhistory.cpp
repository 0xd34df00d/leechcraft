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
#include "interfaces/monocle/ilink.h"
#include "documenttab.h"

namespace LC::Monocle
{
	NavigationHistory::Actions::Actions ()
	: Back_ { NavigationHistory::tr ("Go back") }
	, Forward_ { NavigationHistory::tr ("Go forward") }
	{
		Back_.setProperty ("ActionIcon", "go-previous");
		Forward_.setProperty ("ActionIcon", "go-next");
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

	NavigationHistory::NavigationHistory (PositionGetter posGetter, QObject *parent)
	: QObject { parent }
	, Actions_ { std::make_unique<Actions> () }
	, PosGetter_ { std::move (posGetter) }
	{
		Actions_->Back_.setEnabled (false);
		connect (&Actions_->Back_,
				&QAction::triggered,
				this,
				[this]
				{
					if (!CurrentAction_)
						CurrentAction_ = MakeCurrentPositionAction ();
					GoSingleAction (Actions_->BackMenu_);
				});

		Actions_->Forward_.setEnabled (false);
		connect (&Actions_->Forward_,
				&QAction::triggered,
				this,
				[this] { GoSingleAction (Actions_->ForwardMenu_); });
	}

	NavigationHistory::Actions& NavigationHistory::GetActions () const
	{
		return *Actions_;
	}

	void NavigationHistory::SaveCurrentPos ()
	{
		const auto& backActions = Actions_->BackMenu_.actions ();
		Actions_->BackMenu_.insertAction (backActions.value (0), MakeCurrentPositionAction ());
		Actions_->Back_.setEnabled (true);

		if (CurrentAction_)
		{
			delete *CurrentAction_;
			CurrentAction_.reset ();
		}

		const auto& fwdActions = Actions_->ForwardMenu_.actions ();
		Actions_->ForwardMenu_.clear ();
		qDeleteAll (fwdActions);
		Actions_->Forward_.setEnabled (false);
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
		const auto& entry = PosGetter_ ();

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
		const auto& allActions = Actions_->BackMenu_.actions () + MaybeToList (CurrentAction_) + Actions_->ForwardMenu_.actions ();
		const auto actIdx = allActions.indexOf (action);

		const auto& backActions = allActions.mid (0, actIdx);
		const auto& fwdActions = allActions.mid (actIdx + 1);

		Actions_->BackMenu_.clear ();
		Actions_->BackMenu_.addActions (backActions);
		Actions_->ForwardMenu_.clear ();
		Actions_->ForwardMenu_.addActions (fwdActions);

		CurrentAction_ = action;

		Actions_->Back_.setEnabled (!backActions.isEmpty ());
		Actions_->Forward_.setEnabled (!fwdActions.isEmpty ());
		emit navigationRequested (entry);
	}
}
