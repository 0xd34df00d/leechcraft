/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "statuschange.h"
#include <QMenu>
#include <QMainWindow>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/azoth/iaccount.h>
#include "components/dialogs/setstatusdialog.h"
#include "core.h"
#include "util.h"
#include "customstatusesmanager.h"
#include "resourcesmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth::StatusChange
{
	using StatusSetter_f = std::function<void (State, QString)>;

	namespace
	{
		void UpdateCustomStatuses (QAction *customAct, const StatusSetter_f& handler)
		{
			std::unique_ptr<QMenu> oldMenu { customAct->menu () };
			customAct->setMenu (nullptr);

			const auto& customs = Core::Instance ().GetCustomStatusesManager ()->GetStates ();
			if (customs.isEmpty ())
			{
				customAct->setText (QObject::tr ("Custom...", "the label for the 'custom status' action"));
				return;
			}

			auto menu = new QMenu;

			for (const auto& custom : customs)
			{
				auto action = menu->addAction (custom.Name_, [=] { handler (custom.State_, custom.Text_); });
				action->setIcon (ResourcesManager::Instance ().GetIconForState (custom.State_));
			}

			menu->addSeparator ();

			menu->addAction (QObject::tr ("Other...", "other custom status"), [=] { handler (SInvalid, {}); });

			customAct->setText (QObject::tr ("Custom", "the label for the 'custom status' menu"));
			customAct->setMenu (menu);
		}
	}

	QMenu* CreateMenu (QObject *context, const StatusSetter_f& handler, QWidget *parent)
	{
		const auto& rm = ResourcesManager::Instance ();

		const auto result = new QMenu (QObject::tr ("Change status"), parent);
		const auto addAction = [&] (State state)
		{
			result->addAction (rm.GetIconForState (state), StateToString (state), context, [=] { handler (state, {}); });
		};
		addAction (SOnline);
		addAction (SChat);
		addAction (SAway);
		addAction (SDND);
		addAction (SXA);
		addAction (SOffline);

		result->addSeparator ();
		auto customAct = result->addAction ({}, context, [=] { handler (SInvalid, {}); });

		QObject::connect (result,
				&QMenu::aboutToShow,
				[=] { UpdateCustomStatuses (customAct, handler); });
		QObject::connect (result,
				&QObject::destroyed,
				customAct,
				[customAct]
				{
					auto menu = customAct->menu ();
					customAct->setMenu (nullptr);
					delete menu;
				});

		return result;
	}

	QString GetStatusText (State state, const QString& override)
	{
		if (!override.isEmpty ())
			return override;

		const auto& propName = "DefaultStatus" + QString::number (state);
		return XmlSettingsManager::Instance ()
				.property (propName.toLatin1 ()).toString ();
	}

	void ChangeAllAccountsStatus (State state, const QString& text)
	{
		EntryStatus status;
		if (state != SInvalid)
			status = EntryStatus { state, GetStatusText (state, text) };
		else
		{
			SetStatusDialog ssd { "global", GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow () };
			if (ssd.exec () != QDialog::Accepted)
				return;

			status = EntryStatus { ssd.GetState (), ssd.GetStatusText () };
		}

		for (const auto acc : Core::Instance ().GetAccounts ())
			if (acc->IsShownInRoster ())
				acc->ChangeState (status);
	}
}
