/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "tabpp.h"
#include <QIcon>
#include <QAction>
#include <QMainWindow>
#include <QTranslator>
#include <plugininterface/util.h>
#include "core.h"
#include "tabppwidget.h"
#include "xmlsettingsmanager.h"
#include "tabwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (Util::InstallTranslator ("tabpp"));

				XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"tabppsettings.xml");

				Core::Instance ().SetProxy (proxy);

				Dock_ = new TabPPWidget ("Tab++", proxy->GetMainWindow ());

				TabWidget::SetMultiTabsParent (this);
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
				Dock_->Release ();
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.Tab++";
			}

			QString Plugin::GetName () const
			{
				return "Tab++";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Enhances user experience with tabs.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/resources/images/tabpp.svg");
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ();
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

			QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
			{
				QList<QAction*> result;

				if (place == AEPCommonContextMenu ||
						place == AEPQuickLaunch)
					result << Dock_->GetActivatorAction ();
				return result;
			}

			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Plugin::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}

			void Plugin::SetShortcut (const QString& id, const QKeySequences_t& seqs)
			{
				if (id == "TabPPActivator")
					Dock_->GetActivatorAction ()->setShortcuts (seqs);
			}

			QMap<QString, ActionInfo> Plugin::GetActionInfo () const
			{
				QMap<QString, ActionInfo> result;
				result ["TabPPActivator"] = ActionInfo (tr ("Show tab switcher"),
						Dock_->GetActivatorAction ()->shortcut (),
						Dock_->GetActivatorAction ()->icon ());
				return result;
			}

			void Plugin::newTabRequested ()
			{
				TabWidget *w = new TabWidget ();
				connect (w,
						SIGNAL (removeTab (QWidget*)),
						this,
						SIGNAL (removeTab (QWidget*)));
				emit addNewTab (QString ("Tab++"), w);
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_tabpp, LeechCraft::Plugins::TabPP::Plugin);

