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

#include "birthdaynotifier.h"
#include <QTimer>
#include <QIcon>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/an/constants.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace BirthdayNotifier
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_birthdaynotifier");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothbirthdaynotifiersettings.xml");

		CheckTimer_ = new QTimer (this);

		connect (CheckTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkDates ()));

		CheckTimer_->start (3600 * 2 * 1000);
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
		static QIcon icon (":/azoth/birthdaynotifier/resources/images/birthdaynotifier.svg");
		return icon;
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
		auto e = Util::MakeNotification (tr ("Birthday reminder"), notify, PInfo_);

		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = GetUniqueID ();
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = AN::CatOrganizer;
		e.Additional_ ["org.LC.AdvNotifications.EventID"] = "org.LC.Plugins.Azoth.BirthdayNotifier.Birthday/" + entry->GetEntryID ();
		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (hrId);

		e.Additional_ ["org.LC.AdvNotifications.EventType"] = AN::TypeOrganizerEventDue;
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;

		const auto& px = QPixmap::fromImage (entry->GetAvatar ());
		if (!px.isNull ())
			e.Additional_ ["NotificationPixmap"] = px;

		emit gotEntity (e);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::checkDates ()
	{
		if (!XmlSettingsManager::Instance ().property ("NotifyEnabled").toBool ())
			return;

		const auto& ranges = XmlSettingsManager::Instance ()
				.property ("NotificationDays").toString ().split (',', QString::SkipEmptyParts);

		QList<int> allowedDays;
		Q_FOREACH (const auto& range, ranges)
		{
			if (!range.contains ('-'))
			{
				bool ok = false;
				const int day = range.toInt (&ok);
				if (ok)
					allowedDays << day;
				continue;
			}

			const auto& ends = range.split ('-', QString::SkipEmptyParts);
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

		auto accs = AzothProxy_->GetAllAccounts ();
		Q_FOREACH (auto accObj, AzothProxy_->GetAllAccounts ())
		{
			auto acc = qobject_cast<IAccount*> (accObj);
			auto extSelf = qobject_cast<IExtSelfInfoAccount*> (accObj);
			Q_FOREACH (auto entryObj, acc->GetCLEntries ())
			{
				auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry || entry->GetEntryType () != ICLEntry::ETChat)
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
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_birthdaynotifier, LeechCraft::Azoth::BirthdayNotifier::Plugin);
