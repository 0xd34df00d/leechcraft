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

#include "shellopen.h"
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>
#include <QMainWindow>
#include <interfaces/entitytesthandleresult.h>
#include <util/util.h>
#include <util/notificationactionhandler.h>

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
				return tr ("Allows one to open files and otherwise handle entities with external applications.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
			}

			EntityTestHandleResult Plugin::CouldHandle (const LeechCraft::Entity& e) const
			{
				if (!(e.Parameters_ & FromUserInitiated))
					return EntityTestHandleResult ();

				if (!e.Entity_.canConvert<QUrl> ())
					return EntityTestHandleResult ();

				if (e.Mime_.startsWith ("x-leechcraft/"))
					return EntityTestHandleResult ();

				const QUrl& url = e.Entity_.toUrl ();
				if (url.scheme () != "file")
					return EntityTestHandleResult ();

				if (!QFileInfo (url.toLocalFile ()).exists ())
					return EntityTestHandleResult ();

				return EntityTestHandleResult (EntityTestHandleResult::PHigh);
			}

			void Plugin::Handle (LeechCraft::Entity e)
			{
				QUrl url = e.Entity_.toUrl ();

				Entity notif = Util::MakeNotification ("ShellOpen",
						tr ("%1 just finished downloading.")
							.arg (url.toLocalFile ()),
						PInfo_);
				Util::NotificationActionHandler *nh =
						new Util::NotificationActionHandler (notif);
				nh->AddFunction (tr ("Open"), [url] () { QDesktopServices::openUrl (url); });

				emit gotEntity (notif);
			}
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_shellopen, LeechCraft::Plugins::ShellOpen::Plugin);
