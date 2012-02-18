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

#include "liznoo.h"
#include <cmath>
#include <QIcon>
#include <QAction>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "batteryhistorydialog.h"

#if defined(Q_OS_LINUX)
	#include "platformupower.h"
#elif defined(Q_OS_WIN32)
	#include "platformwinapi.h"
#else
	#pragma message ("Unsupported system")
#endif

namespace LeechCraft
{
namespace Liznoo
{
	const int HistSize = 300;

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		qRegisterMetaType<BatteryInfo> ("Liznoo::BatteryInfo");

		Util::InstallTranslator ("liznoo");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (XmlSettingsManager::Instance (), "liznoosettings.xml");

#if defined(Q_OS_LINUX)
		PL_ = new PlatformUPower (this);
#elif defined(Q_OS_WIN32)
		PL_ = new PlatformWinAPI (this);
#else
		PL_ = 0;
#endif

		connect (PL_,
				SIGNAL (started ()),
				this,
				SLOT (handlePlatformStarted ()));

		Suspend_ = new QAction (tr ("Suspend"), this);
		connect (Suspend_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSuspendRequested ()));
		Suspend_->setProperty ("ActionIcon", "system-suspend");

		Hibernate_ = new QAction (tr ("Hibernate"), this);
		connect (Hibernate_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleHibernateRequested ()));
		Hibernate_->setProperty ("ActionIcon", "system-suspend-hibernate");
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
		if (PL_)
			PL_->Stop ();
	}

	QString Plugin::GetName () const
	{
		return "Liznoo";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("UPower/WinAPI-based power manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/liznoo/resources/images/liznoo.svg");
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;
		return result;
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		QMap<QString, QList<QAction*>> result;
		result ["System"] << Suspend_;
		result ["System"] << Hibernate_;
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

			return name;
		}
	}

	void Plugin::UpdateAction (const BatteryInfo& info)
	{
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

	void Plugin::CheckNotifications (const BatteryInfo& info)
	{
		auto check = [&info, &Battery2LastInfo_] (std::function<bool (const BatteryInfo&)> f)
		{
			if (!Battery2LastInfo_.contains (info.ID_))
				return f (info);

			return f (info) && !f (Battery2LastInfo_ [info.ID_]);
		};

		auto checkPerc = [] (const BatteryInfo& b, const QByteArray& prop)
			{ return b.Percentage_ <= XmlSettingsManager::Instance ()->property (prop).toInt (); };

		const bool isExtremeLow = check ([checkPerc] (const BatteryInfo& b)
				{ return checkPerc (b, "NotifyOnExtremeLowPower"); });
		const bool isLow = check ([checkPerc] (const BatteryInfo& b)
				{ return checkPerc (b, "NotifyOnLowPower"); });

		if (isExtremeLow || isLow)
			emit gotEntity (Util::MakeNotification ("Liznoo",
						tr ("Battery charge level is below %1.")
							.arg (info.Percentage_),
						isLow ? PWarning_ : PCritical_));

		if (XmlSettingsManager::Instance ()->property ("NotifyOnPowerTransitions").toBool ())
		{
			const bool startedCharging = check ([] (const BatteryInfo& b)
					{ return b.TimeToFull_ && !b.TimeToEmpty_; });
			const bool startedDischarging = check ([] (const BatteryInfo& b)
					{ return !b.TimeToFull_ && b.TimeToEmpty_; });

			if (startedCharging)
				emit gotEntity (Util::MakeNotification ("Liznoo",
							tr ("The device started charging."),
							PInfo_));
			else if (startedDischarging)
				emit gotEntity (Util::MakeNotification ("Liznoo",
							tr ("The device started discharging."),
							PWarning_));
		}
	}

	void Plugin::handleBatteryInfo (BatteryInfo info)
	{
		if (!Battery2Action_.contains (info.ID_))
		{
			QAction *act = new QAction (tr ("Battery status"), this);
			act->setProperty ("WatchActionIconChange", true);
			act->setProperty ("Liznoo/BatteryID", info.ID_);

			act->setProperty ("Action/Class", GetUniqueID () + "/BatteryAction");
			act->setProperty ("Action/ID", GetUniqueID () + "/" + info.ID_);

			emit gotActions (QList<QAction*> () << act, AEPLCTray);
			Battery2Action_ [info.ID_] = act;

			connect (act,
					SIGNAL (triggered ()),
					this,
					SLOT (handleHistoryTriggered ()));
		}

		UpdateAction (info);
		CheckNotifications (info);

		Battery2LastInfo_ [info.ID_] = info;
	}

	void Plugin::handleUpdateHistory ()
	{
		Q_FOREACH (const QString& id, Battery2LastInfo_.keys ())
		{
			auto& hist = Battery2History_ [id];
			hist << BatteryHistory (Battery2LastInfo_ [id]);
			if (hist.size () > HistSize)
				hist.removeFirst ();
		}

		Q_FOREACH (const QString& id, Battery2Dialog_.keys ())
			Battery2Dialog_ [id]->UpdateHistory (Battery2History_ [id]);
	}

	void Plugin::handleHistoryTriggered ()
	{
		const QString& id = sender ()->
				property ("Liznoo/BatteryID").toString ();
		if (!Battery2History_.contains (id) ||
				Battery2Dialog_.contains (id))
			return;

		auto dialog = new BatteryHistoryDialog (HistSize);
		dialog->UpdateHistory (Battery2History_ [id]);
		dialog->setAttribute (Qt::WA_DeleteOnClose);
		Battery2Dialog_ [id] = dialog;
		connect (dialog,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleBatteryDialogDestroyed ()));
		dialog->show ();
	}

	void Plugin::handleBatteryDialogDestroyed ()
	{
		auto dia = static_cast<BatteryHistoryDialog*> (sender ());
		Battery2Dialog_.remove (Battery2Dialog_.key (dia));
	}

	void Plugin::handlePlatformStarted ()
	{
		connect (PL_,
				SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)),
				this,
				SLOT (handleBatteryInfo (Liznoo::BatteryInfo)));
		connect (PL_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));

		QTimer *timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (handleUpdateHistory ()));
		timer->start (3000);
	}

	void Plugin::handleSuspendRequested ()
	{
		PL_->ChangeState (PlatformLayer::PowerState::Suspend);
	}

	void Plugin::handleHibernateRequested ()
	{
		PL_->ChangeState (PlatformLayer::PowerState::Hibernate);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_liznoo, LeechCraft::Liznoo::Plugin);
