/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "openedtabmanager.h"
#include <QTimer>
#include <interfaces/core/icoretabwidget.h>
#include "sbwidget.h"

namespace LeechCraft
{
namespace Sidebar
{
	namespace
	{
		QIcon GetDefIcon ()
		{
			return QIcon (":/resources/images/defaultpluginicon.svg");
		}
	}

	OpenedTabManager::OpenedTabManager (SBWidget *w,
			ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Bar_ (w)
	, ActionUpdateScheduled_ (false)
	{
	}

	void OpenedTabManager::ManagePlugin (QObject *ihtObj)
	{
		connect (ihtObj,
				SIGNAL (addNewTab (const QString&, QWidget*)),
				this,
				SLOT (handleNewTab (const QString&, QWidget*)));
		connect (ihtObj,
				SIGNAL (removeTab (QWidget*)),
				this,
				SLOT (handleRemoveTab (QWidget*)));
		connect (ihtObj,
				SIGNAL (changeTabName (QWidget*, const QString&)),
				this,
				SLOT (handleChangeTabName (QWidget*, const QString&)));
		connect (ihtObj,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				this,
				SLOT (handleChangeTabIcon (QWidget*, const QIcon&)));
	}

	void OpenedTabManager::ScheduleUpdate ()
	{
		if (ActionUpdateScheduled_)
			return;

		ActionUpdateScheduled_ = true;
		QTimer::singleShot (700,
				this,
				SLOT (handleUpdates ()));
	}

	void OpenedTabManager::handleNewTab (const QString& name, QWidget *w)
	{
		if (TabActions_.contains (w))
			return;

		QAction *act = new QAction (GetDefIcon (), name, this);
		act->setProperty ("Sidebar/Widget", QVariant::fromValue<QObject*> (w));
		TabActions_ [w] = act;

		Bar_->AddCurTabAction (act, w);

		connect (act,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleSelectTab ()));
	}

	void OpenedTabManager::handleChangeTabName (QWidget *w, const QString& name)
	{
		if (!TabActions_.contains (w))
			return;

		ActionTextUpdates_ [TabActions_ [w]] = name;
		ScheduleUpdate ();
	}

	void OpenedTabManager::handleChangeTabIcon (QWidget *w, const QIcon& icon)
	{
		if (!TabActions_.contains (w))
			return;

		ActionIconUpdates_ [TabActions_ [w]] = icon;
		ScheduleUpdate ();
	}

	void OpenedTabManager::handleRemoveTab (QWidget *w)
	{
		QAction *act = TabActions_.take (w);
		Bar_->RemoveCurTabAction (act, w);
		ActionIconUpdates_.remove (act);
		ActionTextUpdates_.remove (act);
		delete act;
	}

	void OpenedTabManager::handleUpdates ()
	{
		ActionUpdateScheduled_ = false;

		Q_FOREACH (QAction *act, ActionTextUpdates_.keys ())
			act->setText (ActionTextUpdates_ [act]);

		Q_FOREACH (QAction *act, ActionIconUpdates_.keys ())
		{
			const QIcon& icon = ActionIconUpdates_ [act];
			QIcon toSet = GetDefIcon ();
			if (!icon.isNull ())
				toSet = QIcon (icon.pixmap (48, 48).scaled (48, 48,
							Qt::KeepAspectRatio, Qt::SmoothTransformation));

			act->setIcon (toSet);
		}
	}

	void OpenedTabManager::handleSelectTab ()
	{
		QWidget *tw = qobject_cast<QWidget*> (sender ()->
					property ("Sidebar/Widget").value<QObject*> ());

		Proxy_->GetTabWidget ()->setCurrentWidget (tw);
	}
}
}
