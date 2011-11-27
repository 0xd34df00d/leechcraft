/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "liznoo.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>
#include "dbusconnector.h"
#include "dbusthread.h"
#include <cmath>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Liznoo
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		qRegisterMetaType<BatteryInfo> ("Liznoo::BatteryInfo");
		Thread_ = new DBusThread;
		connect (Thread_,
				SIGNAL(started ()),
				this,
				SLOT (handleThreadStarted ()));
		Thread_->start (QThread::LowestPriority);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Liznoo";
	}

	void Plugin::Release ()
	{
		if (!Thread_->wait (1000))
			Thread_->terminate ();
	}

	QString Plugin::GetName () const
	{
		return "Liznoo";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("UPower-based power manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;
		return result;
	}
	
	namespace
	{
		QString GetBattIconName (BatteryInfo info)
		{
			const bool isCharging = info.TimeToFull_ && !info.TimeToEmpty_;
			
			QString name = "battery-";
			if (isCharging)
				name += "charging-";
			
			if (info.Percentage_ < 15)
				name += "low";
			else if (info.Percentage_ < 30)
				name += "caution";
			else if (info.Percentage_ < 50)
				name += "040";
			else if (info.Percentage_ < 70)
				name += "060";
			else if (info.Percentage_ < 90)
				name += "080";
			else if (isCharging)
				name.chop (1);
			else
				name += "100";
			
			qDebug () << name;
			return name;
		}
	}
	
	void Plugin::handleBatteryInfo (BatteryInfo info)
	{
		if (!Battery2Action_.contains (info.ID_))
		{
			QAction *act = new QAction (QString (), this);
			act->setProperty ("WatchActionIconChange", true);
			emit gotActions (QList<QAction*> () << act, AEPLCTray);
			Battery2Action_ [info.ID_] = act;
		}
		
		QAction *act = Battery2Action_ [info.ID_];

		const bool isDischarging = info.TimeToEmpty_ && !info.TimeToFull_;
		const bool isCharging = info.TimeToFull_ && !info.TimeToEmpty_;

		QString text = QString::number (info.Percentage_) + '%';
		if (isCharging)
			text += " " + tr ("(charging)");
		else if (isDischarging)
			text += " " + tr ("(discharging)");
		act->setText (text);
		
		QString tooltip = QString ("%1: %2<br />")
				.arg (tr ("Battery"))
				.arg (text);
		if (isCharging)
			tooltip += QString ("%1 until charged")
					.arg (Util::MakeTimeFromLong (info.TimeToFull_));
		else if (isDischarging)
			tooltip += QString ("%1 until discharged")
					.arg (Util::MakeTimeFromLong (info.TimeToEmpty_));
		if (isCharging || isDischarging)
			tooltip += "<br /><br />";
		
		tooltip += tr ("Battery technology: %1")
				.arg (info.Technology_);
		tooltip += "<br />";
		if (info.EnergyRate_)
		{
			tooltip += tr ("Energy rate: %1 W")
					.arg (std::abs (info.EnergyRate_));
			tooltip += "<br />";
		}
		if (info.Energy_)
		{
			tooltip += tr ("Remaining energy: %1 Wh")
					.arg (info.Energy_);
			tooltip += "<br />";
		}
		if (info.EnergyFull_)
		{
			tooltip += tr ("Full energy capacity: %1 Wh")
					.arg (info.EnergyFull_);
			tooltip += "<br />";
		}

		act->setToolTip (tooltip);
		act->setProperty ("ActionIcon", GetBattIconName (info));
	}
	
	void Plugin::handleThreadStarted ()
	{
		connect (Thread_->GetConnector (),
				SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)),
				this,
				SLOT (handleBatteryInfo (Liznoo::BatteryInfo)));
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_liznoo, LeechCraft::Liznoo::Plugin);
