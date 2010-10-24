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

#include "shellopen.h"
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>
#include <QMainWindow>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace ShellOpen
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (Util::InstallTranslator ("shellopen"));

				Proxy_ = proxy;
			}

			void Plugin::SecondInit ()
			{
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.ShellOpen";
			}

			void Plugin::Release ()
			{
			}

			QString Plugin::GetName () const
			{
				return "ShellOpen";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Allows to open files and otherwise handle entities with external applications.");
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

			bool Plugin::CouldHandle (const LeechCraft::Entity& e) const
			{
				if (!(e.Parameters_ & FromUserInitiated))
					return false;

				if (!e.Entity_.canConvert<QUrl> ())
					return false;

				if (e.Mime_.startsWith ("x-leechcraft/"))
					return false;

				QUrl url = e.Entity_.toUrl ();
				if (url.scheme () != "file")
					return false;

				if (!QFileInfo (url.toLocalFile ()).exists ())
					return false;

				return true;
			}

			void Plugin::Handle (LeechCraft::Entity e)
			{
				QUrl url = e.Entity_.toUrl ();
				if (e.Parameters_ & FromUserInitiated &&
						e.Parameters_ & IsDownloaded &&
						QMessageBox::question (Proxy_->GetMainWindow (),
								"LeechCraft",
								tr ("Do you want to open %1?")
									.arg (url.toLocalFile ()),
								QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					return;

				QDesktopServices::openUrl (url);
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_shellopen, LeechCraft::Plugins::ShellOpen::Plugin);

