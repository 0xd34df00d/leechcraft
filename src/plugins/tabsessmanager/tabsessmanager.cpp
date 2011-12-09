/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Georg Rudoy
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

#include "tabsessmanager.h"
#include <QIcon>
#include <QTimer>
#include <QSettings>
#include <QCoreApplication>
#include <QMenu>
#include <QInputDialog>
#include <QMainWindow>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ihavetabs.h>
#include "restoresessiondialog.h"
#include "recinfo.h"

namespace LeechCraft
{
namespace TabSessManager
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		IsRecovering_ = true;

		const auto& roots = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveRecoverableTabs*> ();
		Q_FOREACH (QObject *root, roots)
			connect (root,
					SIGNAL (addNewTab (const QString&, QWidget*)),
					this,
					SLOT (handleNewTab (const QString&, QWidget*)));

		SessMgrMenu_ = new QMenu (tr ("Sessions"));
		SessMgrMenu_->addAction (tr ("Save current session..."),
				this,
				SLOT (saveCustomSession ()));
		SessMgrMenu_->addSeparator ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		Q_FOREACH (const auto& group, settings.childGroups ())
			AddCustomSession (group);
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (5000,
				this,
				SLOT (recover ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TabSessManager";
	}

	void Plugin::Release ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		settings.setValue ("CleanShutdown", true);
	}

	QString Plugin::GetName () const
	{
		return "TabSessManager";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Manages sessions of tabs in LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;
		if (place == AEPToolsMenu)
			result << SessMgrMenu_->menuAction ();
		return result;
	}

	QByteArray Plugin::GetCurrentSession () const
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);
		Q_FOREACH (auto tab, Tabs_)
		{
			ITabWidget *tw = qobject_cast<ITabWidget*> (tab);
			if (!tw)
				continue;

			IInfo *plugin = qobject_cast<IInfo*> (tw->ParentMultiTabs ());
			if (!plugin)
				continue;

			auto rec = qobject_cast<IRecoverableTab*> (tab);
			const auto& data = rec->GetTabRecoverData ();
			if (data.isEmpty ())
				continue;

			str << plugin->GetUniqueID ()
					<< data
					<< rec->GetTabRecoverName ()
					<< rec->GetTabRecoverIcon ();

			qDebug () << "appended data for" << plugin->GetUniqueID () << rec->GetTabRecoverName ();
		}

		return result;
	}

	void Plugin::AddCustomSession (const QString& name)
	{
		QAction *act = SessMgrMenu_->addAction (name,
				this,
				SLOT (loadCustomSession ()));
		act->setProperty ("TabSessManager/SessName", name);
	}

	void Plugin::handleNewTab (const QString&, QWidget *widget)
	{
		auto tab = qobject_cast<IRecoverableTab*> (widget);
		if (!tab)
			return;

		Tabs_ << widget;

		connect (widget,
				SIGNAL (tabRecoverDataChanged ()),
				this,
				SLOT (handleTabRecoverDataChanged ()));
		connect (widget,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleTabDestroyed ()));

		if (!tab->GetTabRecoverData ().isEmpty ())
			handleTabRecoverDataChanged ();
	}

	void Plugin::handleTabDestroyed ()
	{
		Tabs_.remove (sender ());

		handleTabRecoverDataChanged ();
	}

	namespace
	{
		QHash<QObject*, QList<RecInfo>> GetTabsFromStream (QDataStream& str, ICoreProxy_ptr proxy)
		{
			QHash<QByteArray, QObject*> pluginCache;
			QHash<QObject*, QList<RecInfo>> tabs;

			while (!str.atEnd ())
			{
				QByteArray pluginId;
				QByteArray recData;
				QString name;
				QIcon icon;

				str >> pluginId >> recData >> name >> icon;
				if (!pluginCache.contains (pluginId))
				{
					QObject *obj = proxy->GetPluginsManager ()->
							GetPluginByID (pluginId);
					pluginCache [pluginId] = obj;
				}

				QObject *plugin = pluginCache [pluginId];
				tabs [plugin] << RecInfo { recData, name, icon };

				qDebug () << "got restore data for" << pluginId << name << plugin;
			}

			Q_FOREACH (QObject *obj, tabs.keys (QList<RecInfo> ()))
				tabs.remove (obj);

			return tabs;
		}

		void AskTabs (QHash<QObject*, QList<RecInfo>>& tabs)
		{
			if (tabs.isEmpty ())
				return;

			RestoreSessionDialog dia;
			dia.SetPages (tabs);

			if (dia.exec () != QDialog::Accepted)
			{
				tabs.clear ();
				return;
			}

			tabs = dia.GetPages ();
		}

		void OpenTabs (const QHash<QObject*, QList<RecInfo>>& tabs)
		{
			Q_FOREACH (QObject *plugin, tabs.keys ())
			{
				auto ihrt = qobject_cast<IHaveRecoverableTabs*> (plugin);
				if (!ihrt)
					continue;

				QList<QByteArray> datas;
				const auto& infos = tabs [plugin];
				std::transform (infos.begin (), infos.end (), std::back_inserter (datas),
						[] (const RecInfo& rec) { return rec.Data_; });
				qDebug () << "recovering" << plugin << infos.size ();
				ihrt->RecoverTabs (datas);
			}
		}
	}

	void Plugin::recover ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");

		QDataStream str (settings.value ("Data").toByteArray ());
		auto tabs = GetTabsFromStream (str, Proxy_);

		if (!settings.value ("CleanShutdown", false).toBool ())
			AskTabs (tabs);

		OpenTabs (tabs);

		IsRecovering_ = false;
		settings.setValue ("CleanShutdown", false);
	}

	void Plugin::handleTabRecoverDataChanged ()
	{
		qDebug () << Q_FUNC_INFO << IsRecovering_ << Proxy_->IsShuttingDown ();
		if (IsRecovering_ || Proxy_->IsShuttingDown ())
			return;

		qDebug () << "saving restore data";

		const auto& result = GetCurrentSession ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		settings.setValue ("Data", result);
	}

	void Plugin::saveCustomSession ()
	{
		const QString& name = QInputDialog::getText (Proxy_->GetMainWindow (),
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

		AddCustomSession (name);
	}

	void Plugin::loadCustomSession ()
	{
		const QString& name = sender ()->
				property ("TabSessManager/SessName").toString ();
		if (name.isEmpty ())
			return;

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_TabSessManager");
		settings.beginGroup (name);
		QDataStream str (settings.value ("Data").toByteArray ());
		settings.endGroup ();

		auto tabs = GetTabsFromStream (str, Proxy_);
		OpenTabs (tabs);
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_tabsessmanager, LeechCraft::TabSessManager::Plugin);
