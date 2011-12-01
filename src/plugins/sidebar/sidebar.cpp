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

#include "sidebar.h"
#include <QIcon>
#include <QAction>
#include <QMainWindow>
#include <QStatusBar>
#include <QTimer>
#include <interfaces/imwproxy.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavetabs.h>
#include "sbwidget.h"

namespace LeechCraft
{
namespace Sidebar
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		ActionUpdateScheduled_ = false;

		Proxy_ = proxy;

		Bar_ = new SBWidget;

		Proxy_->GetMWProxy ()->AddSideWidget (Bar_);
		Proxy_->GetMainWindow ()->statusBar ()->hide ();

		const auto& hasTabs = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveTabs*> ();
		Q_FOREACH (QObject *ihtObj, hasTabs)
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

		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		Q_FOREACH (QObject *actObj, hasActions)
			connect (actObj,
					SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));
	}

	void Plugin::SecondInit ()
	{
		auto hasTabs = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveTabs*> ();
		Q_FOREACH (QObject *ihtObj, hasTabs)
		{
			IHaveTabs *iht = qobject_cast<IHaveTabs*> (ihtObj);

			Q_FOREACH (const TabClassInfo& tc, iht->GetTabClasses ())
			{
				if (!(tc.Features_ & TabFeature::TFOpenableByRequest) ||
						(tc.Features_ & TabFeature::TFSingle))
					continue;

				if (tc.Icon_.isNull ())
					continue;

				QAction *act = new QAction (tc.Icon_,
						tc.VisibleName_, this);
				act->setProperty ("Sidebar/Object",
						QVariant::fromValue<QObject*> (ihtObj));
				act->setProperty ("Sidebar/TabClass", tc.TabClass_);
				connect (act,
						SIGNAL (triggered (bool)),
						this,
						SLOT (openNewTab ()));

				Bar_->AddTabOpenAction (act);
			}
		}

		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IActionsExporter*> ();
		Q_FOREACH (IActionsExporter *exp, hasActions)
		{
			const auto& acts = exp->GetActions (AEPLCTray);
			if (!acts.isEmpty ())
				AddToLCTray (acts);
		}
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Sidebar";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Sidebar";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A nice sidebar with quick launch area, tabs and tray-like area.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void Plugin::ScheduleUpdate ()
	{
		if (ActionUpdateScheduled_)
			return;

		ActionUpdateScheduled_ = true;
		QTimer::singleShot (700,
				this,
				SLOT (handleUpdates ()));
	}

	void Plugin::AddToQuickLaunch (const QList<QAction*>& actions)
	{
		Q_FOREACH (QAction *action, actions)
		{
			Proxy_->RegisterSkinnable (action);
			Bar_->AddQLAction (action);
		}
	}

	void Plugin::AddToLCTray (const QList<QAction*>& actions)
	{
		Q_FOREACH (QAction *action, actions)
		{
			Proxy_->RegisterSkinnable (action);
			Bar_->AddTrayAction (action);
		}
	}

	void Plugin::hookGonnaFillQuickLaunch (IHookProxy_ptr proxy)
	{
		proxy->CancelDefault ();

		auto exporters = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IActionsExporter*> ();

		Q_FOREACH (IActionsExporter *exp, exporters)
		{
			QList<QAction*> actions = exp->GetActions (AEPQuickLaunch);
			if (actions.isEmpty ())
				continue;

			AddToQuickLaunch (actions);
		}
	}

	namespace
	{
		QIcon GetDefIcon ()
		{
			return QIcon (":/resources/images/defaultpluginicon.svg");
		}
	}

	void Plugin::handleUpdates ()
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

	void Plugin::handleGotActions (QList<QAction*> actions, ActionsEmbedPlace aep)
	{
		if (aep == AEPQuickLaunch)
			AddToQuickLaunch (actions);
		else if (aep == AEPLCTray)
			AddToLCTray (actions);
	}

	void Plugin::handleNewTab (const QString& name, QWidget *w)
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

	void Plugin::handleChangeTabName (QWidget *w, const QString& name)
	{
		if (!TabActions_.contains (w))
			return;

		ActionTextUpdates_ [TabActions_ [w]] = name;
		ScheduleUpdate ();
	}

	void Plugin::handleChangeTabIcon (QWidget *w, const QIcon& icon)
	{
		if (!TabActions_.contains (w))
			return;

		ActionIconUpdates_ [TabActions_ [w]] = icon;
		ScheduleUpdate ();
	}

	void Plugin::handleRemoveTab (QWidget *w)
	{
		QAction *act = TabActions_.take (w);
		Bar_->RemoveCurTabAction (act, w);
		ActionIconUpdates_.remove (act);
		ActionTextUpdates_.remove (act);
		delete act;
	}

	void Plugin::handleSelectTab ()
	{
		QWidget *tw = qobject_cast<QWidget*> (sender ()->
					property ("Sidebar/Widget").value<QObject*> ());

		Proxy_->GetTabWidget ()->setCurrentWidget (tw);
	}

	void Plugin::openNewTab ()
	{
		QObject *pluginObj = sender ()->
				property ("Sidebar/Object").value<QObject*> ();
		const QByteArray& tc = sender ()->
				property ("Sidebar/TabClass").toByteArray ();

		IHaveTabs *iht = qobject_cast<IHaveTabs*> (pluginObj);
		iht->TabOpenRequested (tc);
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_sidebar, LeechCraft::Sidebar::Plugin);
