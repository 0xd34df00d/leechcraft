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

#include "lackman.h"
#include <QSortFilterProxyModel>
#include <QIcon>
#include <plugininterface/util.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (Util::InstallTranslator ("lackman"));

				Ui_.setupUi (this);

				FilterByTags_ = new QSortFilterProxyModel (this);
				FilterString_ = new QSortFilterProxyModel (this);
				FilterString_->setSourceModel (FilterByTags_);

				Ui_.Plugins_->setModel (FilterString_);

				Core::Instance ().SetProxy (proxy);

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
				IPluginsManager *pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
				QObjectList browsers = pm->Filter<IWebBrowser*> (pm->GetAllPlugins ());
				IWebBrowser *browser = browsers.size () ?
					qobject_cast<IWebBrowser*> (browsers.at (0)) :
					0;
				Ui_.Description_->Construct (browser);

				Core::Instance ().AddRepo (QUrl::fromLocalFile ("/home/d34df00d/Programming/lcpacks"));
			}

			void Plugin::Release ()
			{
				Core::Instance ().Release ();
			}

			QString Plugin::GetName () const
			{
				return "LackMan";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("LeechCraft Package Manager.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
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

			QWidget* Plugin::GetTabContents ()
			{
				return this;
			}

			QToolBar* Plugin::GetToolBar () const
			{
				return 0;
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_lackman, LeechCraft::Plugins::LackMan::Plugin);

