/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "birthdaynotifier.h"
#include <QTimer>
#include <QIcon>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/threads/futures.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/an/constants.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/azoth/iextselfinfoaccount.h>

namespace LC
{
namespace Azoth
{
namespace BirthdayNotifier
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"Azoth_BirthdayNotifier">;

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("azoth_birthdaynotifier");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothbirthdaynotifiersettings.xml");
		XmlSettingsManager::Instance ().RegisterObject ("NotifyNTimesPerDay", this, "notifyNTimesPerDaySettingsChanged");

		CheckTimer_ = new QTimer (this);

		connect (CheckTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkDates ()));

		notifyNTimesPerDaySettingsChanged ();
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (10000,
				this,
				SLOT (checkDates ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.BirthdayNotifier";
	}

	void Plugin::Release ()
	{
		CheckTimer_->stop ();
	}

	QString Plugin::GetName () const
	{
		return "Azoth Birthday Notifier";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Notifies about birthdays of your buddies.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::NotifyBirthday (ICLEntry *entry, int days)
	{
		const auto& hrId = entry->GetEntryName ();

		const QString& notify = days ?
				tr ("%1 has birthday in %n day(s)!", 0, days).arg (hrId) :
				tr ("%1 has birthday today!").arg (hrId);
		auto e = Util::MakeNotification (tr ("Birthday reminder"), notify, Priority::Info);

		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = GetUniqueID ();
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = AN::CatOrganizer;
		e.Additional_ ["org.LC.AdvNotifications.EventID"] = "org.LC.Plugins.Azoth.BirthdayNotifier.Birthday/" + entry->GetEntryID ();
		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (hrId);

		e.Additional_ ["org.LC.AdvNotifications.EventType"] = AN::TypeOrganizerEventDue;
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;

		const auto avatarsMgr = AzothProxy_->GetAvatarsManager ();
		const QPointer<QObject> entryObj { entry->GetQObject () };

		const auto avatarGetter = [avatarsMgr, entryObj] () -> Util::LazyNotificationPixmap_t::result_type
		{
			if (entryObj)
				return avatarsMgr->GetAvatar (entryObj, IHaveAvatars::Size::Thumbnail);
			else
				return {};
		};
		e.Additional_ ["NotificationPixmap"] = QVariant::fromValue<Util::LazyNotificationPixmap_t> (avatarGetter);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::checkDates ()
	{
		if (!XmlSettingsManager::Instance ().property ("NotifyEnabled").toBool ())
			return;

		const auto& rangesStr = XmlSettingsManager::Instance ().property ("NotificationDays").toString ();
		const auto& ranges = QStringView { rangesStr }.split (',', Qt::SkipEmptyParts);

		QList<int> allowedDays;
		for (const auto& range : ranges)
		{
			if (!range.contains ('-'))
			{
				bool ok = false;
				const int day = range.toInt (&ok);
				if (ok)
					allowedDays << day;
				continue;
			}

			const auto& ends = range.split ('-', Qt::SkipEmptyParts);
			if (ends.size () != 2)
				continue;

			bool bOk = false, eOk = false;
			const int begin = ends.at (0).toInt (&bOk);
			const int end = ends.at (1).toInt (&eOk);
			if (!bOk || !eOk)
				continue;

			for (int i = begin; i <= end; ++i)
				allowedDays << i;
		}

		const auto& today = QDate::currentDate ();

		for (auto accObj : AzothProxy_->GetAllAccounts ())
		{
			auto acc = qobject_cast<IAccount*> (accObj);
			auto extSelf = qobject_cast<IExtSelfInfoAccount*> (accObj);
			for (auto entryObj : acc->GetCLEntries ())
			{
				auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry || entry->GetEntryType () != ICLEntry::EntryType::Chat)
					continue;

				if (extSelf && extSelf->GetSelfContact () == entryObj)
					continue;

				auto meta = qobject_cast<IMetaInfoEntry*> (entryObj);
				if (!meta)
					continue;

				auto dt = meta->GetMetaInfo (IMetaInfoEntry::DataField::BirthDate).toDate ();
				if (!dt.isValid ())
					continue;

				dt.setDate (today.year (), dt.month (), dt.day ());
				if (!dt.isValid ())
					continue;

				int days = today.daysTo (dt);
				if (days < 0)
				{
					dt.setDate (today.year () + 1, dt.month (), dt.day ());
					if (!dt.isValid ())
						continue;
					days = today.daysTo (dt);
				}

				if (allowedDays.contains (days))
					NotifyBirthday (entry, days);
			}
		}
	}

	void Plugin::notifyNTimesPerDaySettingsChanged ()
	{
		static const int kMSecPerDay = 24 * 60 * 60 * 1000;

		CheckTimer_->stop ();
		const int interval = XmlSettingsManager::Instance ().property ("NotifyNTimesPerDay").toInt ();
		const int timeOutMSec = kMSecPerDay / (interval ? interval : 1);
		CheckTimer_->start (timeOutMSec);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_birthdaynotifier, LC::Azoth::BirthdayNotifier::Plugin);
