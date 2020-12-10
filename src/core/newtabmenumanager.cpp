/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newtabmenumanager.h"
#include <algorithm>
#include <QMenu>
#include <QtDebug>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/iplugin2.h>
#include <util/sll/qtutil.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
{
	NewTabMenuManager::NewTabMenuManager (QObject *parent)
	: QObject (parent)
	, NewTabMenu_ (new QMenu (tr ("New tab")))
	, AdditionalTabMenu_ (new QMenu (tr ("Additional")))
	{
	}

	void NewTabMenuManager::AddObject (QObject *obj)
	{
		const auto imt = qobject_cast<IHaveTabs*> (obj);
		if (!imt || RegisteredMultiTabs_.contains (obj))
			return;

		const auto ii = qobject_cast<IInfo*> (obj);

		for (const auto& info : imt->GetTabClasses ())
		{
			if (!(info.Features_ & TFOpenableByRequest))
				continue;

			const auto newAct = new QAction (info.Icon_,
					AccelerateName (info.VisibleName_),
					this);
			connect (newAct,
					&QAction::triggered,
					this,
					[this, newAct] { OpenTab (newAct); });
			newAct->setProperty ("PluginObj", QVariant::fromValue<QObject*> (obj));
			newAct->setProperty ("TabClass", info.TabClass_);
			newAct->setProperty ("Single", static_cast<bool> (info.Features_ & TFSingle));
			newAct->setStatusTip (info.Description_);
			newAct->setToolTip (info.Description_);

			InsertAction (newAct);

			if (info.Features_ & TFByDefault)
			{
				const auto& id = ii->GetUniqueID () + '|' + info.TabClass_;
				const bool hide = XmlSettingsManager::Instance ()->Property (Util::AsStringView ("Hide" + id), false).toBool ();
				if (!hide)
				{
					OpenTab (newAct);
					XmlSettingsManager::Instance ()->setProperty ("Hide" + id, true);
				}
			}
		}
	}

	void NewTabMenuManager::SetToolbarActions (QList<QList<QAction*>> lists)
	{
		QList<QAction*> ones;
		for (const auto& list : decltype (lists) { lists })
			if (list.size () == 1)
			{
				ones += list;
				lists.removeAll (list);
			}

		if (ones.size ())
			lists.prepend (ones);

		for (const auto& list : lists)
		{
			if (!list.size ())
				continue;

			AdditionalTabMenu_->addSeparator ();
			AdditionalTabMenu_->addActions (list);
		}
	}

	void NewTabMenuManager::SingleRemoved (ITabWidget *itw)
	{
		const auto& tabClass = itw->GetTabClassInfo ().TabClass_;
		const auto act = HiddenActions_ [itw->ParentMultiTabs ()] [tabClass];
		if (!act)
		{
			qWarning () << Q_FUNC_INFO
					<< "no hidden action for"
					<< itw->GetTabClassInfo ().TabClass_
					<< "in"
					<< itw->ParentMultiTabs ();
			return;
		}
		InsertAction (act);
		ToggleHide (itw, true);
	}

	QMenu* NewTabMenuManager::GetNewTabMenu () const
	{
		return NewTabMenu_;
	}

	QMenu* NewTabMenuManager::GetAdditionalMenu ()
	{
		if (!AdditionalTabMenu_->actions ().isEmpty ())
			AdditionalTabMenu_->insertMenu (AdditionalTabMenu_->actions ().first (),
					NewTabMenu_);
		else
			AdditionalTabMenu_->addMenu (NewTabMenu_);

		return AdditionalTabMenu_;
	}

	void NewTabMenuManager::ToggleHide (ITabWidget *itw, bool hide)
	{
		ToggleHide (itw->ParentMultiTabs (), itw->GetTabClassInfo ().TabClass_, hide);
	}

	void NewTabMenuManager::HideAction (ITabWidget *itw)
	{
		const auto pObj = itw->ParentMultiTabs ();
		const auto tabClass = itw->GetTabClassInfo ().TabClass_;
		for (auto action : NewTabMenu_->actions ())
			if (action->property ("TabClass").toByteArray () == tabClass &&
					action->property ("PluginObj").value<QObject*> () == pObj)
			{
				NewTabMenu_->removeAction (action);
				HiddenActions_ [pObj] [tabClass] = action;
			}
	}

	QString NewTabMenuManager::AccelerateName (QString name)
	{
		for (int i = 0, length = name.length ();
				i < length; ++i)
		{
			QChar c = name.at (i);
			if (UsedAccelerators_.contains (c))
				continue;

			UsedAccelerators_ << c;
			name.insert (i, '&');
			break;
		}
		return name;
	}

	void NewTabMenuManager::ToggleHide (QObject *obj,
			const QByteArray& tabClass, bool hide)
	{
		if (!hide)
			return;

		const auto ii = qobject_cast<IInfo*> (obj);
		if (!ii)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't implement IInfo";
			return;
		}

		const auto& id = ii->GetUniqueID () + '|' + tabClass;
		XmlSettingsManager::Instance ()->setProperty ("Hide" + id, hide);
	}

	void NewTabMenuManager::OpenTab (QAction *action)
	{
		const auto pObj = action->property ("PluginObj").value<QObject*> ();
		const auto tabs = qobject_cast<IHaveTabs*> (pObj);
		if (!tabs)
		{
			qWarning () << Q_FUNC_INFO
					<< pObj
					<< "doesn't implement IHaveTabs";
			return;
		}

		const auto& tabClass = action->property ("TabClass").toByteArray ();
		tabs->TabOpenRequested (tabClass);

		const auto& classes = tabs->GetTabClasses ();
		if (action->property ("Single").toBool ())
		{
			NewTabMenu_->removeAction (action);
			HiddenActions_ [pObj] [tabClass] = action;
			ToggleHide (pObj, tabClass, false);
		}
		else
		{
			const auto pos = std::find_if (classes.begin (), classes.end (),
					[&tabClass] (const auto& item) { return item.TabClass_ == tabClass; });
			if (pos != classes.end () && pos->Features_ & TFByDefault)
				ToggleHide (pObj, tabClass, false);
		}
	}

	namespace
	{
		QAction* FindActionBefore (const QString& name, QMenu *menu)
		{
			for (auto otherAct : menu->actions ())
				if (otherAct->isSeparator () ||
						QString::localeAwareCompare (otherAct->text (), name) > 0)
					return otherAct;
			return nullptr;
		}
	}

	void NewTabMenuManager::InsertAction (QAction *act)
	{
		auto pObj = act->property ("PluginObj").value<QObject*> ();

		auto ip2 = qobject_cast<IPlugin2*> (pObj);
		if (!ip2)
		{
			InsertActionWParent (act, pObj, false);
			return;
		}

		const auto pm = Core::Instance ().GetPluginManager ();
		for (auto plugin : pm->GetFirstLevels (ip2->GetPluginClasses ()))
			InsertActionWParent (act, plugin, true);
	}

	void NewTabMenuManager::InsertActionWParent (QAction *act, QObject *pObj, bool sub)
	{
		const auto& tabClasses = qobject_cast<IHaveTabs*> (pObj)->GetTabClasses ();
		const auto& tcCount = std::count_if (tabClasses.begin (), tabClasses.end (),
				[] (const TabClassInfo& tc) { return tc.Features_ & TFOpenableByRequest; });

		const auto ii = qobject_cast<IInfo*> (pObj);
		const auto& name = ii->GetName ();

		auto rootMenu = NewTabMenu_;
		if (sub || tcCount > 1)
		{
			bool menuFound = false;
			for (auto menuAct : rootMenu->actions ())
				if (menuAct->menu () && menuAct->text () == name)
				{
					rootMenu = menuAct->menu ();
					menuFound = true;
					break;
				}

			if (!menuFound)
			{
				auto menu = new QMenu (name, rootMenu);
				menu->setIcon (ii->GetIcon ());
				rootMenu->insertMenu (FindActionBefore (name, rootMenu), menu);
				rootMenu = menu;
			}
		}

		rootMenu->insertAction (FindActionBefore (act->text (), rootMenu), act);
	}
}
