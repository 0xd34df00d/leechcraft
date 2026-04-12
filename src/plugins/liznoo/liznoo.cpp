/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "liznoo.h"
#include <cmath>
#include <limits>
#include <QIcon>
#include <QAction>
#include <QMessageBox>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <util/xpc/util.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "batteryhistorydialog.h"
#include "quarkmanager.h"
#include "platformobjects.h"

namespace LC
{
namespace Liznoo
{
	const int HistSize = 300;
	const auto UpdateMsecs = 3000;

	Plugin::Plugin () = default;

	Plugin::~Plugin () = default;

	void Plugin::Init (ICoreProxy_ptr)
	{
		qRegisterMetaType<BatteryInfo> ("Liznoo::BatteryInfo");

		const auto qm = new QuarkManager;
		PlatformInitTask_ = InitializePlatform (qm);

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "liznoosettings.xml");

		const auto battTimer = new QTimer { this };
		connect (battTimer,
				SIGNAL (timeout ()),
				this,
				SLOT (handleUpdateHistory ()));
		battTimer->start (UpdateMsecs);

		Suspend_ = new QAction (tr ("Suspend"), this);
		connect (Suspend_,
				&QAction::triggered,
				this,
				[this] { HandleStateRequested (PowerActions::Platform::State::Suspend); });
		Suspend_->setProperty ("ActionIcon", "system-suspend");

		Hibernate_ = new QAction (tr ("Hibernate"), this);
		connect (Hibernate_,
				&QAction::triggered,
				this,
				[this] { HandleStateRequested (PowerActions::Platform::State::Hibernate); });
		Hibernate_->setProperty ("ActionIcon", "system-suspend-hibernate");

		connect (XSD_.get (),
				&Util::XmlSettingsDialog::pushButtonClicked,
				this,
				&Plugin::HandleSettingsButton);

		LiznooQuark_ = std::make_shared<QuarkComponent> ("liznoo", "LiznooQuark.qml");
		LiznooQuark_->DynamicProps_.append ({ "Liznoo_proxy", qm });

		connect (qm,
				SIGNAL (batteryHistoryDialogRequested (QString)),
				this,
				SLOT (handleHistoryTriggered (QString)));
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
		Platform_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Liznoo";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Battery, power and sleep manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		return entity.Mime_ == "x-leechcraft/power-management" ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity entity)
	{
		const auto& context = entity.Entity_.toString ();
		if (context != "ScreensaverProhibition"_qs)
			return;

		[] (Entity e, Plugin *pThis) -> Util::ContextTask<void>
		{
			co_await Util::AddContextObject { *pThis };
			co_await pThis->EnsurePlatformReady ();
			pThis->Platform_->ProhibitScreensaver (e.Additional_ ["Enable"].toBool (), e.Additional_ ["ContextID"].toString ());
		} (entity, this);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		return { { "System", { Suspend_, Hibernate_ } } };
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { LiznooQuark_ };
	}

	void Plugin::CheckNotifications (const BatteryInfo& info)
	{
		auto check = [&info, this] (const std::function<bool (BatteryInfo)>& f)
		{
			if (!Battery2LastInfo_.contains (info.ID_))
				return f (info);

			return f (info) && !f (Battery2LastInfo_ [info.ID_]);
		};

		auto& xsm = XmlSettingsManager::Instance ();

		auto checkPerc = [&] (const BatteryInfo& b, const QByteArray& prop)
		{
			if (!xsm.property ("NotifyOn" + prop).toBool ())
				return false;

			return b.Percentage_ <= xsm.property (prop + "Level").toInt ();
		};

		const bool isExtremeLow = check ([checkPerc] (const BatteryInfo& b)
				{ return checkPerc (b, "ExtremeLowPower"); });
		const bool isLow = check ([checkPerc] (const BatteryInfo& b)
				{ return checkPerc (b, "LowPower"); });

		const auto iem = GetProxyHolder ()->GetEntityManager ();
		if (isExtremeLow || isLow)
			iem->HandleEntity (Util::MakeNotification ("Liznoo",
						tr ("Battery charge level is %1%.")
							.arg (static_cast<int> (info.Percentage_)),
						isLow ? Priority::Info : Priority::Warning));

		if (xsm.property ("NotifyOnPowerTransitions").toBool ())
		{
			const bool startedCharging = check ([] (const BatteryInfo& b)
					{ return b.TimeToFull_ && !b.TimeToEmpty_; });
			const bool startedDischarging = check ([] (const BatteryInfo& b)
					{ return !b.TimeToFull_ && b.TimeToEmpty_; });

			if (startedCharging)
				iem->HandleEntity (Util::MakeNotification ("Liznoo",
							tr ("The device started charging."),
							Priority::Info));
			else if (startedDischarging)
				iem->HandleEntity (Util::MakeNotification ("Liznoo",
							tr ("The device started discharging."),
							Priority::Warning));
		}
	}

	Util::Task<void> Plugin::EnsurePlatformReady ()
	{
		if (PlatformInitTask_)
			co_await *PlatformInitTask_;
	}

	Util::SharedContextTask<void> Plugin::InitializePlatform (QPointer<QuarkManager> qm)
	{
		co_await Util::AddContextObject { *this };
		Platform_ = co_await PlatformObjects::Create ();
		connect (Platform_.get (),
				&PlatformObjects::batteryInfoUpdated,
				this,
				[this, qm] (const BatteryInfo& info)
				{
					CheckNotifications (info);
					Battery2LastInfo_ [info.ID_] = info;
					if (qm)
						qm->handleBatteryInfo (info);
				});
		PlatformInitTask_.reset ();
	}

	namespace
	{
		QString GetReasonString (PlatformObjects::ChangeStateFailed::Reason reason)
		{
			switch (reason)
			{
			case PlatformObjects::ChangeStateFailed::Reason::Unavailable:
				return Plugin::tr ("No platform backend is available.");
			case PlatformObjects::ChangeStateFailed::Reason::PlatformFailure:
				return Plugin::tr ("Platform backend failed.");
			case PlatformObjects::ChangeStateFailed::Reason::Other:
				return Plugin::tr ("Unknown reason.");
			}
			std::unreachable ();
		}
	}

	Util::ContextTask<void> Plugin::HandleStateRequested (PowerActions::Platform::State state)
	{
		co_await Util::AddContextObject { *this };
		co_await EnsurePlatformReady ();
		Util::Visit (co_await Platform_->ChangeState (state),
				[] (PlatformObjects::ChangeStateSucceeded) {},
				[] (PlatformObjects::ChangeStateFailed failure)
				{
					auto msg = GetReasonString (failure.Reason_);
					if (!failure.ReasonString_.isEmpty ())
						msg += " " + failure.ReasonString_;
					const auto& entity = Util::MakeNotification ("Liznoo", msg, Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (entity);
				});
	}

	void Plugin::handleUpdateHistory ()
	{
		for (const auto& pair : Util::Stlize (Battery2LastInfo_))
		{
			const auto& id = pair.first;
			auto pos = Battery2History_.find (id);
			if (pos == Battery2History_.end ())
				pos = Battery2History_.insert (id, BatteryHistoryList { HistSize });
			pos->push_back (BatteryHistory { pair.second });
		}

		for (const auto& pair : Util::Stlize (Battery2Dialog_))
		{
			const auto& id = pair.first;
			pair.second->UpdateHistory (Battery2History_ [id], Battery2LastInfo_ [id]);
		}
	}

	void Plugin::handleHistoryTriggered ()
	{
		const auto& id = sender ()->property ("Liznoo/BatteryID").toString ();
		handleHistoryTriggered (id);
	}

	void Plugin::handleHistoryTriggered (const QString& id)
	{
		if (!Battery2History_.contains (id) ||
				Battery2Dialog_.contains (id))
		{
			if (auto dia = Battery2Dialog_.value (id))
				dia->close ();
			return;
		}

		auto dialog = new BatteryHistoryDialog (HistSize, UpdateMsecs / 1000.);
		dialog->UpdateHistory (Battery2History_ [id], Battery2LastInfo_ [id]);
		dialog->setAttribute (Qt::WA_DeleteOnClose);
		Battery2Dialog_ [id] = dialog;
		connect (dialog,
				&QObject::destroyed,
				this,
				[this, id] { Battery2Dialog_.remove (id); });
		dialog->show ();
		dialog->activateWindow ();
		dialog->raise ();
	}

	Util::ContextTask<void> Plugin::HandleSettingsButton (QString button)
	{
		co_await Util::AddContextObject { *this };
		co_await EnsurePlatformReady ();
		const auto res = [&button, this]
		{
			if (button == "TestSleep")
				return Platform_->EmitTestSleep ();
			if (button == "TestWake")
				return Platform_->EmitTestWakeup ();
			return true;
		} ();

		if (!res)
			QMessageBox::critical (nullptr,
					"LeechCraft",
					tr ("Unable to send test power events."));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_liznoo, LC::Liznoo::Plugin);
