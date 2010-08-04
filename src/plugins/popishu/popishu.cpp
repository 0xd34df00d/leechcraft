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

#include "popishu.h"
#include <QTranslator>
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/util.h>
#include "core.h"
#include "editorpage.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Popishu
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				EditorPage::SetParentMultiTabs (this);

				Translator_.reset (Util::InstallTranslator ("popishu"));

				XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"popishusettings.xml");

				Core::Instance ().SetProxy (proxy);

				connect (&Core::Instance (),
						SIGNAL (addNewTab (const QString&, QWidget*)),
						this,
						SIGNAL (addNewTab (const QString&, QWidget*)));
				connect (&Core::Instance (),
						SIGNAL (removeTab (QWidget*)),
						this,
						SIGNAL (removeTab (QWidget*)));
				connect (&Core::Instance (),
						SIGNAL (raiseTab (QWidget*)),
						this,
						SIGNAL (raiseTab (QWidget*)));
				connect (&Core::Instance (),
						SIGNAL (changeTabName (QWidget*, const QString&)),
						this,
						SIGNAL (changeTabName (QWidget*, const QString&)));
				connect (&Core::Instance (),
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
						this,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
				connect (&Core::Instance (),
						SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
						this,
						SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));
				connect (&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.Popishu";
			}

			QString Plugin::GetName () const
			{
				return "Popishu";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Plain text editor with syntax highlighting and stuff.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/resources/images/popishu.svg");
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

			bool Plugin::CouldHandle (const Entity& entity) const
			{
				return entity.Mime_ == "x-leechcraft/plain-text-document" &&
						entity.Entity_.canConvert<QString> ();
			}

			void Plugin::Handle (Entity entity)
			{
				Core::Instance ().Handle (entity);
			}

			boost::shared_ptr<Util::XmlSettingsDialog> Plugin::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}

			void Plugin::newTabRequested ()
			{
				Core::Instance ().NewTabRequested ();
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_popishu, LeechCraft::Plugins::Popishu::Plugin);

