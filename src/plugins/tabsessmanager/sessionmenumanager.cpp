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
#include <util/sll/slotclosure.h>
#include <interfaces/iinfo.h>
#include "sessionsmanager.h"
#include "recinfo.h"

namespace LC
{
namespace TabSessManager
{
	SessionMenuManager::SessionMenuManager (SessionsManager *sessMgr, QObject *parent)
	: QObject { parent }
	, SessMgr_ { sessMgr }
	, SessMgrMenu_ { new QMenu { tr ("Sessions") } }
	{
		const auto saveAct = SessMgrMenu_->addAction (tr ("Save current session..."),
				this,
				SIGNAL (saveCustomSessionRequested ()));
		saveAct->setProperty ("ActionIcon", "document-save-all");

		SessMgrMenu_->menuAction ()->setProperty ("ActionIcon",
				"preferences-system-session-services");

		SessMgrMenu_->addSeparator ();
	}

	QAction* SessionMenuManager::GetSessionsAction () const
	{
		return SessMgrMenu_->menuAction ();
	}

	void SessionMenuManager::DeleteSession (const QString& name)
	{
		Session2Menu_.remove (name);
		emit deleteRequested (name);
	}

	void SessionMenuManager::addCustomSession (const QString& name)
	{
		if (Session2Menu_.contains (name))
			return;

		const auto& menu = std::shared_ptr<QMenu> (SessMgrMenu_->addMenu (name),
				[this] (QMenu *menu) { SessMgrMenu_->removeAction (menu->menuAction ()); });
		Session2Menu_ [name] = menu;

		const auto loadAct = menu->addAction (tr ("Load"));
		loadAct->setProperty ("ActionIcon", "edit-find-replace");
		loadAct->setToolTip (tr ("Load the session, replacing all currently opened tabs."));
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this, name] { emit loadRequested (name); },
			loadAct,
			SIGNAL (triggered ()),
			loadAct
		};

		const auto addAct = menu->addAction (tr ("Add"));
		addAct->setProperty ("ActionIcon", "list-add");
		loadAct->setToolTip (tr ("Add the tabs from the session to the currently open ones."));
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this, name] { emit addRequested (name); },
			addAct,
			SIGNAL (triggered ()),
			addAct
		};

		const auto deleteAct = menu->addAction (tr ("Delete"));
		deleteAct->setProperty ("ActionIcon", "list-remove");
		deleteAct->setToolTip (tr ("Delete the session."));
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this, name] { DeleteSession (name); },
			deleteAct,
			SIGNAL (triggered ()),
			deleteAct
		};

		menu->addSeparator ();

		for (const auto& pair : Util::Stlize (SessMgr_->GetTabsInSession (name)))
		{
			const auto pluginObj = pair.first;
			const auto ii = qobject_cast<IInfo*> (pluginObj);

			const auto submenu = menu->addMenu (ii->GetIcon (), ii->GetName ());

			for (const auto& info : pair.second)
			{
				const auto action = submenu->addAction (info.Icon_, info.Name_);
				new Util::SlotClosure<Util::NoDeletePolicy>
				{
					[pluginObj, info, this]
					{
						QHash<QObject*, QList<RecInfo>> toOpen;
						toOpen [pluginObj] = { info };
						SessMgr_->OpenTabs (toOpen);
					},
					action,
					SIGNAL (triggered ()),
					action
				};
			}
		}
	}
}
}
