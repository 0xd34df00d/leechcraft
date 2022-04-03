/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sessionmenumanager.h"
#include <QMenu>
#include <util/sll/qtutil.h>
#include <interfaces/iinfo.h>
#include "sessionsmanager.h"
#include "recinfo.h"

namespace LC::TabSessManager
{
	SessionMenuManager::SessionMenuManager (SessionsManager *sessMgr, QObject *parent)
	: QObject { parent }
	, SessMgr_ { sessMgr }
	, SessMgrMenu_ { new QMenu { tr ("Sessions") } }
	{
		const auto saveAct = SessMgrMenu_->addAction (tr ("Save current session..."),
				sessMgr,
				&SessionsManager::SaveCustomSession);
		saveAct->setProperty ("ActionIcon", "document-save-all");

		SessMgrMenu_->menuAction ()->setProperty ("ActionIcon",
				"preferences-system-session-services");

		SessMgrMenu_->addSeparator ();
	}

	QAction* SessionMenuManager::GetSessionsAction () const
	{
		return SessMgrMenu_->menuAction ();
	}

	void SessionMenuManager::AddCustomSession (const QString& name)
	{
		if (Session2Menu_.contains (name))
			return;

		const auto& menu = std::shared_ptr<QMenu> (SessMgrMenu_->addMenu (name),
				[this] (QMenu *menu) { SessMgrMenu_->removeAction (menu->menuAction ()); });
		Session2Menu_ [name] = menu;

		const auto loadAct = menu->addAction (tr ("Load"),
				this,
				[=] { SessMgr_->LoadCustomSession (name); });
		loadAct->setProperty ("ActionIcon", "edit-find-replace");
		loadAct->setToolTip (tr ("Load the session, replacing all currently opened tabs."));

		const auto addAct = menu->addAction (tr ("Add"),
				this,
				[=] { SessMgr_->AddCustomSession (name); });
		addAct->setProperty ("ActionIcon", "list-add");
		loadAct->setToolTip (tr ("Add the tabs from the session to the currently open ones."));

		const auto deleteAct = menu->addAction (tr ("Delete"),
				this,
				[=]
				{
					Session2Menu_.remove (name);
					SessMgr_->DeleteCustomSession (name);
				});
		deleteAct->setProperty ("ActionIcon", "list-remove");
		deleteAct->setToolTip (tr ("Delete the session."));

		menu->addSeparator ();

		for (const auto& [obj, infos] : Util::Stlize (SessMgr_->GetTabsInSession (name)))
		{
			const auto ii = qobject_cast<IInfo*> (obj);
			const auto submenu = menu->addMenu (ii->GetIcon (), ii->GetName ());

			for (const auto& info : infos)
				submenu->addAction (info.Icon_,
						info.Name_,
						this,
						[=, obj = obj] { SessMgr_->OpenTabs ({ { obj, { info } } }); });
		}
	}
}
