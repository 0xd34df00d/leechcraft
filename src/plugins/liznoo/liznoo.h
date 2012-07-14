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

#pragma once

#include <QObject>
#include <QLinkedList>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iactionsexporter.h>
#include "batteryhistory.h"
#include "batteryinfo.h"

namespace LeechCraft
{
namespace Liznoo
{
	class BatteryHistoryDialog;
	class PlatformLayer;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IActionsExporter)

		ICoreProxy_ptr Proxy_;

		Util::XmlSettingsDialog_ptr XSD_;

		PlatformLayer *PL_;
		QMap<QString, QAction*> Battery2Action_;
		QMap<QString, BatteryInfo> Battery2LastInfo_;
		QMap<QString, BatteryHistoryDialog*> Battery2Dialog_;
		QMap<QString, QLinkedList<BatteryHistory>> Battery2History_;

		QAction *Suspend_;
		QAction *Hibernate_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
		QMap<QString, QList<QAction*>> GetMenuActions () const;
	private:
		void UpdateAction (const BatteryInfo&);
		void CheckNotifications (const BatteryInfo&);
	private slots:
		void handleBatteryInfo (Liznoo::BatteryInfo);
		void handleUpdateHistory ();
		void handleHistoryTriggered ();
		void handleBatteryDialogDestroyed ();
		void handlePlatformStarted ();

		void handleSuspendRequested ();
		void handleHibernateRequested ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}

