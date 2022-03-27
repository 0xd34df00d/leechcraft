/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sessionsmanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QStringList>
#include <QTimer>
#include <QInputDialog>
#include <QMainWindow>
#include <QtDebug>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <util/sll/qtutil.h>
#include "recinfo.h"
#include "restoresessiondialog.h"
#include "util.h"
#include "tabspropsmanager.h"

namespace LC
{
namespace TabSessManager
{
	SessionsManager::SessionsManager (TabsPropsManager *tpm, QObject *parent)
	: QObject { parent }
	, TabsPropsMgr_ { tpm }
	{
		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		connect (rootWM->GetQObject (),
				SIGNAL (tabAdded (int, QWidget*)),
				this,
				SLOT (handleNewTab (int, QWidget*)),
				Qt::QueuedConnection);

		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (rootWM->GetQObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
	}

	QStringList SessionsManager::GetCustomSessions () const
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager" };
		return settings.childGroups ();
	}

	namespace
	{
		QHash<QObject*, QList<RecInfo>> GetTabsFromStream (QDataStream& str)
		{
			QHash<QByteArray, QObject*> pluginCache;
			QHash<QObject*, QList<RecInfo>> tabs;

			int order = 0;
			while (!str.atEnd ())
			{
				QByteArray pluginId;
				QByteArray recData;
				QString name;
				QIcon icon;
				QList<QPair<QByteArray, QVariant>> props;
				int winId = 0;

				str >> pluginId >> recData >> name >> icon >> props >> winId;
				if (!pluginCache.contains (pluginId))
				{
					const auto obj = GetProxyHolder ()->GetPluginsManager ()->GetPluginByID (pluginId);
					pluginCache [pluginId] = obj;
				}

				const auto plugin = pluginCache [pluginId];
				if (!plugin)
				{
					qWarning () << "null plugin for" << pluginId;
					continue;
				}

				tabs [plugin] << RecInfo { order++, recData, props, name, icon, winId };

				qDebug () << Q_FUNC_INFO << "got restore data for"
						<< pluginId << name << plugin << "; window" << winId;
			}

			return tabs;
		}

		void AskTabs (QHash<QObject*, QList<RecInfo>>& tabs)
		{
			if (tabs.isEmpty ())
				return;

			RestoreSessionDialog dia;
			dia.SetTabs (tabs);

			if (dia.exec () != QDialog::Accepted)
			{
				tabs.clear ();
				return;
			}

			tabs = dia.GetTabs ();
		}

		QHash<QObject*, QList<RecInfo>> GetSession (const QString& name)
		{
			QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager" };
			settings.beginGroup (name);
			QDataStream str { settings.value ("Data").toByteArray () };
			settings.endGroup ();

			return GetTabsFromStream (str);
		}
	}

	QHash<QObject *, QList<RecInfo>> SessionsManager::GetTabsInSession (const QString& name) const
	{
		return GetSession (name);
	}

	bool SessionsManager::HasTab (QObject *tab)
	{
		return std::any_of (Tabs_.begin (), Tabs_.end (),
				[tab] (const QList<QObject*>& list) { return list.indexOf (tab) != -1; });
	}

	bool SessionsManager::eventFilter (QObject*, QEvent *e)
	{
		if (e->type () != QEvent::DynamicPropertyChange)
			return false;

		auto propEvent = static_cast<QDynamicPropertyChangeEvent*> (e);
		if (propEvent->propertyName ().startsWith ("SessionData/"))
			handleTabRecoverDataChanged ();

		return false;
	}

	namespace
	{
		void WriteRecoverableTab (QDataStream& str, int windowIndex,
				QObject *tab, IRecoverableTab *rec, IInfo *plugin)
		{
			const auto& data = rec->GetTabRecoverData ();
			if (data.isEmpty ())
				return;

			const QIcon forRecover { rec->GetTabRecoverIcon ().pixmap (32, 32) };

			str << plugin->GetUniqueID ()
					<< data
					<< rec->GetTabRecoverName ()
					<< forRecover
					<< GetSessionProps (tab)
					<< windowIndex;
		}

		void WriteSingleTab (QDataStream& str, int windowIndex,
				QObject *tab, const TabClassInfo& tc, IInfo *plugin)
		{
			str << plugin->GetUniqueID ()
					<< tc.TabClass_
					<< tc.VisibleName_
					<< tc.Icon_.pixmap (32, 32)
					<< GetSessionProps (tab)
					<< windowIndex;
		}
	}

	QByteArray SessionsManager::GetCurrentSession () const
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);

		int windowIndex = 0;
		for (const auto& list : Tabs_)
		{
			for (auto tab : list)
			{
				auto tw = qobject_cast<ITabWidget*> (tab);
				if (!tw)
					continue;

				auto plugin = qobject_cast<IInfo*> (tw->ParentMultiTabs ());
				if (!plugin)
					continue;

				if (const auto rec = qobject_cast<IRecoverableTab*> (tab))
					WriteRecoverableTab (str, windowIndex, tab, rec, plugin);
				else
				{
					const auto& tc = tw->GetTabClassInfo ();
					if (IsGoodSingleTC (tc))
						WriteSingleTab (str, windowIndex, tab, tc, plugin);
				}
			}

			++windowIndex;
		}

		return result;
	}

	void SessionsManager::OpenTabs (const QHash<QObject*, QList<RecInfo>>& tabs)
	{
		QList<QPair<QObject*, RecInfo>> ordered;
		for (auto i = tabs.begin (); i != tabs.end (); ++i)
			for (const auto& info : i.value ())
				ordered.append ({ i.key (), info });

		std::sort (ordered.begin (), ordered.end (),
				[] (const auto& left, const auto& right)
					{ return left.second.Order_ < right.second.Order_; });

		for (const auto& pair : ordered)
		{
			const auto winGuard = TabsPropsMgr_->AppendWindow (pair.second.WindowID_);
			const auto propsGuard = TabsPropsMgr_->AppendProps (pair.second.Props_);
			if (const auto ihrt = qobject_cast<IHaveRecoverableTabs*> (pair.first))
				ihrt->RecoverTabs ({ TabRecoverInfo { pair.second.Data_, {} } });
			else if (const auto iht = qobject_cast<IHaveTabs*> (pair.first))
				iht->TabOpenRequested (pair.second.Data_);
		}
	}

	void SessionsManager::recover ()
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager" };

		QDataStream str (settings.value ("Data").toByteArray ());
		auto tabs = GetTabsFromStream (str);

		if (!settings.value ("CleanShutdown", false).toBool ())
			AskTabs (tabs);

		OpenTabs (tabs);

		IsRecovering_ = false;
		settings.setValue ("CleanShutdown", false);
	}

	void SessionsManager::handleTabRecoverDataChanged ()
	{
		if (IsRecovering_ || GetProxyHolder ()->IsShuttingDown ())
			return;

		if (IsScheduled_)
			return;

		IsScheduled_ = true;
		QTimer::singleShot (2000,
				this,
				SLOT (saveDefaultSession ()));
	}

	void SessionsManager::saveDefaultSession ()
	{
		IsScheduled_ = false;

		const auto& result = GetCurrentSession ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		settings.setValue ("Data", result);
	}

	void SessionsManager::saveCustomSession ()
	{
		auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		const QString& name = QInputDialog::getText (rootWM->GetPreferredWindow (),
				tr ("Custom session"),
				tr ("Enter the name of the session:"));
		if (name.isEmpty ())
			return;

		const auto& result = GetCurrentSession ();
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		settings.beginGroup (name);
		settings.setValue ("Data", result);
		settings.endGroup ();

		emit gotCustomSession (name);
	}

	void SessionsManager::loadCustomSession (const QString& name)
	{
		const auto rootMgr = GetProxyHolder ()->GetRootWindowsManager ();
		for (int i = rootMgr->GetWindowsCount () - 1; i >= 0; --i)
		{
			const auto tabWidget = rootMgr->GetTabWidget (i);
			for (int j = tabWidget->WidgetCount () - 1; j >= 0; --j)
			{
				const auto tab = tabWidget->Widget (j);
				const auto itw = qobject_cast<ITabWidget*> (tab);
				itw->Remove ();
			}

			if (i)
				rootMgr->GetMainWindow (i)->close ();
		}

		OpenTabs (GetSession (name));
	}

	void SessionsManager::addCustomSession (const QString& name)
	{
		auto tabs = GetSession (name);

		QHash<QObject*, QList<QByteArray>> plugin2recoveries;
		for (const auto& window : Tabs_)
			for (const auto tab : window)
			{
				const auto tw = qobject_cast<ITabWidget*> (tab);
				const auto rec = qobject_cast<IRecoverableTab*> (tab);
				if (!tw || !rec)
					continue;

				plugin2recoveries [tw->ParentMultiTabs ()] << rec->GetTabRecoverData ();
			}

		for (const auto& pair : Util::Stlize (tabs))
		{
			const auto& present = plugin2recoveries.value (pair.first);

			const auto ihrt = qobject_cast<IHaveRecoverableTabs*> (pair.first);
			if (!ihrt)
				continue;

			auto& recList = pair.second;
			recList.erase (std::remove_if (recList.begin (), recList.end (),
						[&present, ihrt] (const RecInfo& info)
						{
							return present.contains (info.Data_) ||
									ihrt->HasSimilarTab (info.Data_, present);
						}),
					recList.end ());
		}

		OpenTabs (tabs);
	}

	void SessionsManager::deleteCustomSession (const QString& name)
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager" };
		settings.remove (name);
	}

	void SessionsManager::handleRemoveTab (QWidget *widget)
	{
		for (auto& list : Tabs_)
			if (list.removeOne (widget))
				break;

		handleTabRecoverDataChanged ();
	}

	void SessionsManager::handleNewTab (int, QWidget *widget)
	{
		if (HasTab (widget))
			return;

		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		const auto itw = qobject_cast<ITabWidget*> (widget);
		const auto windowIndex = rootWM->GetWindowForTab (itw);

		if (windowIndex < 0 || windowIndex >= Tabs_.size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown window index"
					<< windowIndex
					<< "of"
					<< Tabs_.size ()
					<< "for"
					<< widget;
			return;
		}

		Tabs_ [windowIndex] << widget;

		const auto irt = qobject_cast<IRecoverableTab*> (widget);
		if (!irt && !IsGoodSingleTC (itw->GetTabClassInfo ()))
			return;

		if (irt)
			connect (widget,
					SIGNAL (tabRecoverDataChanged ()),
					this,
					SLOT (handleTabRecoverDataChanged ()));

		widget->installEventFilter (this);

		if (!irt || !irt->GetTabRecoverData ().isEmpty ())
			handleTabRecoverDataChanged ();

		const auto& posProp = widget->property ("TabSessManager/Position");
		if (posProp.isValid ())
		{
			const auto prevPos = posProp.toInt ();

			const auto tabWidget = rootWM->GetTabWidget (windowIndex);
			const auto currentIdx = tabWidget->IndexOf (widget);
			if (prevPos < tabWidget->WidgetCount () && currentIdx != prevPos)
				tabWidget->MoveTab (currentIdx, prevPos);
		}
	}

	void SessionsManager::handleTabMoved (int from, int to)
	{
		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		const auto tabWidget = qobject_cast<ICoreTabWidget*> (sender ());
		const auto pos = rootWM->GetTabWidgetIndex (tabWidget);

		auto& tabs = Tabs_ [pos];

		if (std::max (from, to) >= tabs.size () ||
			std::min (from, to) < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid"
					<< from
					<< "->"
					<< to
					<< "; total tabs:"
					<< Tabs_.size ();
			return;
		}

		auto tab = tabs.takeAt (from);
		tabs.insert (to, tab);

		handleTabRecoverDataChanged ();
	}

	void SessionsManager::handleWindow (int index)
	{
		Tabs_ << QList<QObject*> {};
		connect (GetProxyHolder ()->GetRootWindowsManager ()->GetTabWidget (index)->GetQObject (),
				SIGNAL (tabWasMoved (int, int)),
				this,
				SLOT (handleTabMoved (int, int)));
	}

	void SessionsManager::handleWindowRemoved (int index)
	{
		Tabs_.removeAt (index);
		handleTabRecoverDataChanged ();
	}
}
}
