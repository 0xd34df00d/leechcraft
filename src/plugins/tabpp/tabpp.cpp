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

#include "tabpp.h"
#include <QIcon>
#include <QAction>
#include <QMainWindow>
#include <QTranslator>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "core.h"
#include "tabppwidget.h"
#include "xmlsettingsmanager.h"

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

			QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
			{
				QList<QAction*> result;

				if (place == AEPCommonContextMenu)
					result << Dock_->GetActivatorAction ();
				return result;
			}

			std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Plugin::GetSettingsDialog () const
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
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_tabpp, LeechCraft::Plugins::TabPP::Plugin);

