/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "statuschangemenumanager.h"
#include <QMenu>
#include "core.h"
#include "customstatusesmanager.h"
#include "resourcesmanager.h"

namespace LC
{
namespace Azoth
{
	StatusChangeMenuManager::StatusChangeMenuManager (QObject *parent)
	: QObject (parent)
	{
	}

	QMenu* StatusChangeMenuManager::CreateMenu (QObject* obj, const char* slot, QWidget *parent, bool autoupdate)
	{
		QMenu *result = new QMenu (tr ("Change status"), parent);
		result->addAction (ResourcesManager::Instance ().GetIconForState (SOnline),
				tr ("Online"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOnline));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SChat),
				tr ("Free to chat"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SChat));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SAway),
				tr ("Away"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SAway));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SDND),
				tr ("DND"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SDND));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SXA),
				tr ("Not available"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SXA));
		result ->addAction (ResourcesManager::Instance ().GetIconForState (SOffline),
				tr ("Offline"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOffline));

		result->addSeparator ();
		auto customAct = result->addAction (QString (), obj, slot);

		if (autoupdate)
			connect (result,
					SIGNAL (aboutToShow ()),
					this,
					SLOT (updateCustomStatuses ()));
		connect (result,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleMenuDestroyed ()));

		Infos_ [result] = { obj, slot, customAct };

		return result;
	}

	void StatusChangeMenuManager::UpdateCustomStatuses (QMenu *rootMenu)
	{
		if (!Infos_.contains (rootMenu))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown menu"
					<< rootMenu
					<< Infos_.keys ();
			return;
		}

		const auto& info = Infos_ [rootMenu];

		std::shared_ptr<QMenu> oldMenu (info.CustomAction_->menu ());

		const auto& customs = Core::Instance ().GetCustomStatusesManager ()->GetStates ();
		if (customs.isEmpty ())
		{
			info.CustomAction_->setText (tr ("Custom..."));
			info.CustomAction_->setMenu (nullptr);
			return;
		}

		auto menu = new QMenu;
		for (const auto& custom : customs)
		{
			auto action = menu->addAction (custom.Name_, info.Obj_, info.Slot_);
			action->setIcon (ResourcesManager::Instance ().GetIconForState (custom.State_));
			action->setProperty ("Azoth/TargetState", QVariant::fromValue<State> (custom.State_));
			action->setProperty ("Azoth/TargetText", custom.Text_);
		}

		menu->addSeparator ();

		menu->addAction (tr ("Other..."), info.Obj_, info.Slot_);

		info.CustomAction_->setText (tr ("Custom"));
		info.CustomAction_->setMenu (menu);
	}

	void StatusChangeMenuManager::updateCustomStatuses ()
	{
		UpdateCustomStatuses (static_cast<QMenu*> (sender ()));
	}

	void StatusChangeMenuManager::handleMenuDestroyed ()
	{
		const auto& info = Infos_.take (sender ());
		if (info.CustomAction_)
		{
			auto menu = info.CustomAction_->menu ();
			info.CustomAction_->setMenu (nullptr);
			delete menu;
		}
	}
}
}
