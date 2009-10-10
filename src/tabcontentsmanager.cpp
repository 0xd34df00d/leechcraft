/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "tabcontentsmanager.h"
#include <QToolBar>
#include "tabcontents.h"
#include "core.h"
#include "mainwindow.h"
#include "viewreemitter.h"
#include "toolbarguard.h"
#include "interfaces/imultitabs.h"

namespace LeechCraft
{
	TabContentsManager::TabContentsManager ()
	: Reemitter_ (new ViewReemitter (this))
	{
		TabWidget *tw = Core::Instance ()
			.GetReallyMainWindow ()->GetTabWidget ();
		connect (tw,
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleCurrentChanged (int)));
	}

	TabContentsManager& TabContentsManager::Instance ()
	{
		static TabContentsManager tcm;
		return tcm;
	}

	void TabContentsManager::SetDefault (TabContents *tc)
	{
		Default_ = tc;
		Current_ = Default_;

		Connect (tc);
		Reemitter_->Connect (Default_);
	}

	QList<TabContents*> TabContentsManager::GetTabs () const
	{
		QList<TabContents*> result = Others_;
		result.prepend (Default_);
		return result;
	}

	void TabContentsManager::AddNewTab (const QStringList& query)
	{
		TabWidget *tw = Core::Instance ()
			.GetReallyMainWindow ()->GetTabWidget ();
		IMultiTabsWidget *imtw =
			qobject_cast<IMultiTabsWidget*> (tw->currentWidget ());
		if (!imtw ||
				query.size ())
		{
			TabContents *tc = new TabContents ();
			Connect (tc);
			Reemitter_->Connect (tc);
			Others_ << tc;
			emit addNewTab (tr ("Summary"), tc);
			emit changeTabIcon (tc,
					QIcon (":/resources/images/leechcraft.svg"));

			tc->AllowPlugins ();
			tc->SetQuery (query);
		}
		else
			imtw->NewTabRequested ();
	}

	void TabContentsManager::RemoveTab (TabContents *tc)
	{
		emit removeTab (tc);
		Others_.removeAll (tc);
		delete tc;
	}

	void TabContentsManager::MadeCurrent (TabContents *tc)
	{
		Current_ = tc;

		Q_FOREACH (TabContents *tab, GetTabs () + (QList<TabContents*> () << Default_))
			if (tab != tc)
				tab->SmartDeselect (tc);

		if (tc)
		{
			QItemSelectionModel *selectionModel = tc->
				GetUi ().PluginsTasksTree_->selectionModel ();
			QModelIndex ri;
			if (selectionModel)
				ri = selectionModel->currentIndex ();

			if (ri.isValid ())
			{
				QToolBar *controls = Core::Instance ()
							.GetControls (ri);
				QWidget *addiInfo = Core::Instance ()
							.GetAdditionalInfo (ri);

				Core::Instance ().GetReallyMainWindow ()->
					GetGuard ()->AddToolbar (controls);
				if (controls)
				{
					controls->setFloatable (true);
					controls->setMovable (true);
					controls->show ();
				}
				if (addiInfo)
				{
					if (addiInfo->parent () != this)
						addiInfo->setParent (tc);
					tc->GetUi ().ControlsDockWidget_->setWidget (addiInfo);
					tc->GetUi ().ControlsDockWidget_->show ();
				}
			}
			else
			{
				Core::Instance ().GetReallyMainWindow ()->
					GetGuard ()->AddToolbar (0);
				tc->GetUi ().ControlsDockWidget_->hide ();
			}
		}
	}

	TabContents* TabContentsManager::GetCurrent () const
	{
		return Current_;
	}

	QObject* TabContentsManager::GetReemitter () const
	{
		return Reemitter_;
	}

	void TabContentsManager::Connect (TabContents *tc)
	{
		connect (tc,
				SIGNAL (filterUpdated ()),
				this,
				SLOT (handleFilterUpdated ()));
		connect (tc,
				SIGNAL (queryUpdated (const QString&)),
				this,
				SLOT (handleQueryUpdated (const QString&)));
	}

	void TabContentsManager::handleFilterUpdated ()
	{
		TabContents *tc = qobject_cast<TabContents*> (sender ());
		if (!tc)
			return;

		Reemitter_->ConnectModelSpecific (tc);
	}

	void TabContentsManager::handleQueryUpdated (const QString& query)
	{
		if (!query.size ())
		{
			emit changeTabName (static_cast<QWidget*> (sender ()),
					tr ("Summary"));
			return;
		}

		QStringList splittedList = query.split (' ', QString::SkipEmptyParts);
		QStringList categories;
		Q_FOREACH (QString splitted, splittedList)
			if (splitted.startsWith ("ca:") ||
					splitted.startsWith ("category:"))
			{
				QStringList parsed = splitted.split (':');
				if (parsed.size () < 2)
					continue;
				QString category = parsed.at (1);
				if (category.endsWith (')'))
					category.chop (1);
				categories << category;
			}

		if (categories.size ())
			emit changeTabName (static_cast<QWidget*> (sender ()),
					categories.join ("; "));
		else
			emit changeTabName (static_cast<QWidget*> (sender ()),
					query);
	}

	void TabContentsManager::handleCurrentChanged (int index)
	{
		TabContents *tc = index ?
			qobject_cast<TabContents*> (Core::Instance ()
					.GetReallyMainWindow ()->GetTabWidget ()->widget (index)) :
			Default_;

		if (!tc)
		{
			QToolBar *nt = Core::Instance ().GetToolBar (index);
			Core::Instance ().GetReallyMainWindow ()->
				GetGuard ()->AddToolbar (nt);
		}

		MadeCurrent (tc);

		QTreeView *nv = 0;
		if (tc)
			nv = tc->GetUi ().PluginsTasksTree_;
		emit currentViewChanged (nv);
	}
};

