/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "newtabmenumanager.h"
#include <QMenu>
#include <QtDebug>
#include "interfaces/iinfo.h"
#include "interfaces/ihavetabs.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	NewTabMenuManager::NewTabMenuManager (QObject *parent)
	: QObject (parent)
	, NewTabMenu_ (new QMenu (tr ("New tab menu")))
	, AdditionalTabMenu_ (new QMenu (tr ("Additional tab menu")))
	{
	}

	void NewTabMenuManager::AddObject (QObject *obj)
	{
		IHaveTabs *imt = qobject_cast<IHaveTabs*> (obj);
		if (!imt || RegisteredMultiTabs_.contains (obj))
			return;
		
		IInfo *ii = qobject_cast<IInfo*> (obj);

		Q_FOREACH (const TabClassInfo& info, imt->GetTabClasses ())
		{
			if (!(info.Features_ & TFOpenableByRequest))
				continue;

			QAction *newAct = new QAction (info.Icon_,
					AccelerateName (info.VisibleName_),
					this);
			connect (newAct,
					SIGNAL (triggered ()),
					this,
					SLOT (handleNewTabRequested ()));
			newAct->setProperty ("PluginObj", QVariant::fromValue<QObject*> (obj));
			newAct->setProperty ("TabClass", info.TabClass_);
			newAct->setProperty ("Single",
					static_cast<bool> (info.Features_ & TFSingle));
			newAct->setStatusTip (info.Description_);
			newAct->setToolTip (info.Description_);
			
			InsertAction (newAct);
			
			if (info.Features_ & TFSingle ||
					info.Features_ & TFByDefault)
			{
				const QByteArray& id = ii->GetUniqueID () + '|' + info.TabClass_;
				const bool hide = XmlSettingsManager::Instance ()->
						Property ("Hide" + id, false).toBool ();
				if (!hide)
					OpenTab (newAct);
			}
		}
	}

	void NewTabMenuManager::HandleEmbedTabRemoved (QObject *obj)
	{
	}

	void NewTabMenuManager::SetToolbarActions (QList<QList<QAction*> > lists)
	{
		QList<QAction*> ones;
		Q_FOREACH (QList<QAction*> list, lists)
			if (list.size () == 1)
			{
				ones += list;
				lists.removeAll (list);
			}

		if (ones.size ())
			lists.prepend (ones);

		Q_FOREACH (QList<QAction*> list, lists)
		{
			if (!list.size ())
				continue;

			AdditionalTabMenu_->addSeparator ();
			AdditionalTabMenu_->addActions (list);
		}
	}
	
	void NewTabMenuManager::SingleRemoved (ITabWidget *itw)
	{
		const QByteArray& tabClass = itw->GetTabClassInfo ().TabClass_;
		QAction *act = HiddenActions_ [itw->ParentMultiTabs ()] [tabClass];
		if (!act)
		{
			qWarning () << Q_FUNC_INFO
					<< "no hidden action for"
					<< itw->GetTabClassInfo ().TabClass_
					<< "in"
					<< itw->ParentMultiTabs ();
			return;
		}
		
		ToggleHide (itw->ParentMultiTabs (), tabClass, true);
		
		InsertAction (act);
	}

	QMenu* NewTabMenuManager::GetNewTabMenu () const
	{
		return NewTabMenu_;
	}

	QMenu* NewTabMenuManager::GetAdditionalMenu ()
	{
		AdditionalTabMenu_->insertMenu (AdditionalTabMenu_->actions ().first (),
				NewTabMenu_);
		return AdditionalTabMenu_;
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
		IInfo *ii = qobject_cast<IInfo*> (obj);
		if (!ii)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "doesn't implement IInfo";
			return;
		}

		const QByteArray& id = ii->GetUniqueID () + '|' + tabClass;
		XmlSettingsManager::Instance ()->setProperty ("Hide" + id, hide);
	}
	
	void NewTabMenuManager::OpenTab (QAction *action)
	{
		QObject *pObj = action->property ("PluginObj").value<QObject*> ();
		IHaveTabs *tabs = qobject_cast<IHaveTabs*> (pObj);
		if (!tabs)
		{
			qWarning () << Q_FUNC_INFO
					<< pObj
					<< "doesn't implement IHaveTabs";
			return;
		}

		const QByteArray& tabClass = action->property ("TabClass").toByteArray ();
		tabs->TabOpenRequested (tabClass);
		if (action->property ("Single").toBool ())
		{
			NewTabMenu_->removeAction (action);
			HiddenActions_ [pObj] [tabClass] = action;
			ToggleHide (pObj, tabClass, false);
		}
	}
	
	void NewTabMenuManager::InsertAction (QAction *act)
	{
		bool inserted = false;
		Q_FOREACH (QAction *menuAct, NewTabMenu_->actions ())
			if (menuAct->isSeparator () ||
					QString::localeAwareCompare (menuAct->text (), act->text ()) > 0)
			{
				NewTabMenu_->insertAction (menuAct, act);
				inserted = true;
				break;
			}

		if (!inserted)
			NewTabMenu_->addAction (act);
	}

	void NewTabMenuManager::handleNewTabRequested ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an action";
			return;
		}

		OpenTab (action);
	}
}
