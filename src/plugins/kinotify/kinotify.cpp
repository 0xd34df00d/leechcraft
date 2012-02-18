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

#include "kinotify.h"
#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <interfaces/entitytesthandleresult.h>
#include <util/resourceloader.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <interfaces/core/icoreproxy.h>
#include "kinotifywidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Kinotify
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;

				ThemeLoader_.reset (new Util::ResourceLoader ("kinotify/themes/notification"));
				ThemeLoader_->AddLocalPrefix ();
				ThemeLoader_->AddGlobalPrefix ();

				connect (ThemeLoader_.get (),
						SIGNAL (watchedDirectoriesChanged ()),
						this,
						SLOT (handleWatchedDirsChanged ()));

				SettingsDialog_.reset (new Util::XmlSettingsDialog ());
				SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"kinotifysettings.xml");

				SettingsDialog_->SetDataSource ("NotificatorStyle",
						ThemeLoader_->GetSubElemModel ());
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.Kinotify";
			}

			QString Plugin::GetName () const
			{
				return "Kinotify";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Fancy Kinetic notifications.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/plugins/kinotify/resources/images/kinotify.svg");
			}

			EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
			{
				const bool could = e.Mime_ == "x-leechcraft/notification" &&
						e.Additional_ ["Priority"].toInt () != PLog_ &&
						!e.Additional_ ["Text"].toString ().isEmpty ();
				return could ?
						EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
						EntityTestHandleResult ();
			}

			void Plugin::Handle (Entity e)
			{
				Priority prio = static_cast<Priority> (e.Additional_ ["Priority"].toInt ());
				if (prio == PLog_)
					return;

				QString header = e.Entity_.toString ();
				QString text = e.Additional_ ["Text"].toString ();

				int timeout = Proxy_->GetSettingsManager ()->
						property ("FinishedDownloadMessageTimeout").toInt () * 1000;

 				KinotifyWidget *notificationWidget = new KinotifyWidget (timeout);
				notificationWidget->SetThemeLoader (ThemeLoader_);
				notificationWidget->SetEntity (e);

				QStringList actionsNames = e.Additional_ ["NotificationActions"].toStringList ();
				if (!actionsNames.isEmpty ())
				{
					if (!e.Additional_ ["HandlingObject"].canConvert<QObject_ptr> ())
						qWarning () << Q_FUNC_INFO
								<< "value is not QObject_ptr"
								<< e.Additional_ ["HandlingObject"];
					else
					{
						QObject_ptr actionObject = e.Additional_ ["HandlingObject"].value<QObject_ptr> ();
						notificationWidget->SetActions (actionsNames, actionObject);
					}
				}

				connect (notificationWidget,
						SIGNAL (checkNotificationQueue ()),
						this,
						SLOT (pushNotification ()));
				connect (notificationWidget,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));

				QString mi = "dialog-information";
				switch (prio)
				{
					case PWarning_:
						mi = "dialog-warning";
						break;
					case PCritical_:
						mi = "dialog-error";
					default:
						break;
				}

				const QIcon& icon = Proxy_->GetIcon (mi);
				const QPixmap& px = icon.pixmap (QSize (128, 128));
				notificationWidget->SetContent (header, text, QString ());

				const QPixmap& notif = e.Additional_ ["NotificationPixmap"].value<QPixmap> ();
				notificationWidget->OverrideImage (notif.isNull () ? px : notif);

				if (!ActiveNotifications_.size ())
					notificationWidget->PrepareNotification ();

				ActiveNotifications_ << notificationWidget;
			}

			Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
			{
				return SettingsDialog_;
			}

			void Plugin::pushNotification ()
			{
				if (!ActiveNotifications_.size ())
					return;

				ActiveNotifications_.removeFirst ();
				if (ActiveNotifications_.size ())
					ActiveNotifications_.first ()->PrepareNotification ();
			}

			void Plugin::handleWatchedDirsChanged ()
			{
				KinotifyWidget::ClearThemeCache ();
			}
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_kinotify, LeechCraft::Plugins::Kinotify::Plugin);

