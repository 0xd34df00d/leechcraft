/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QSettings>
#include <QTimer>
#include <QCoreApplication>
#include <QUuid>
#include <QInputDialog>
#include <QtDebug>
#include <util/sll/functional.h>
#include <util/sll/prelude.h>
#include <util/gui/util.h>
#include <interfaces/azoth/imucentry.h>
#include "metaaccount.h"
#include "metaentry.h"
#include "addtometacontactsdialog.h"

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	Core::Core ()
	{
		qRegisterMetaType<QList<QObject*>> ("QList<QObject*>");
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetMetaAccount (MetaAccount *acc)
	{
		if (!acc)
		{
			if (Account_)
				emit accountRemoved (Account_);
			Account_ = 0;
			return;
		}

		Account_ = acc;
		connect (this,
				SIGNAL (gotCLItems (const QList<QObject*>&)),
				acc,
				SIGNAL (gotCLItems (const QList<QObject*>&)));
		connect (this,
				SIGNAL (removedCLItems (const QList<QObject*>&)),
				acc,
				SIGNAL (removedCLItems (const QList<QObject*>&)));
		connect (this,
				SIGNAL (accountAdded (QObject*)),
				Account_->GetParentProtocol (),
				SIGNAL (accountAdded (QObject*)));
		connect (this,
				SIGNAL (accountRemoved (QObject*)),
				Account_->GetParentProtocol (),
				SIGNAL (accountRemoved (QObject*)));

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Metacontacts_Entries");
		const int numEntries = settings.beginReadArray ("Entries");
		for (int i = 0; i < numEntries; ++i)
		{
			settings.setArrayIndex (i);
			const QString& id = settings.value ("ID").toString ();
			if (id.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "empty ID";
				continue;
			}

			const QString& name = settings.value ("Name").toString ();
			if (name.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "empty name";
				continue;
			}

			const QStringList& reals = settings.value ("RealIDs").toStringList ();
			if (reals.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "empty real IDs list for"
						<< id
						<< name;
				continue;
			}

			MetaEntry *entry = new MetaEntry (id, acc);
			ConnectSignals (entry);
			entry->SetEntryName (name);
			entry->SetGroups (settings.value ("Groups").toStringList ());
			entry->SetRealEntries (reals);
			Entries_ << entry;

			for (const auto& id : reals)
				UnavailRealEntries_ [id] = entry;
		}
		settings.endArray ();

		if (!Entries_.isEmpty ())
			emit accountAdded (Account_);
	}

	QList<QObject*> Core::GetEntries () const
	{
		return Util::Map (Entries_, Util::Upcast<QObject*>);
	}

	bool Core::HandleRealEntryAddBegin (QObject *entryObj)
	{
		if (!qstrcmp (entryObj->metaObject ()->className (), "MetaEntry"))
			return false;

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry";
			return false;
		}

		const QString& id = entry->GetEntryID ();
		if (AvailRealEntries_.contains (id))
			return true;

		if (!UnavailRealEntries_.contains (id))
			return false;

		MetaEntry *metaEntry = UnavailRealEntries_.take (id);
		metaEntry->AddRealObject (entry);

		AvailRealEntries_ [id] = metaEntry;

		return true;
	}

	void Core::AddRealEntry (QObject *realObj)
	{
		ICLEntry *real = qobject_cast<ICLEntry*> (realObj);
		if (!real)
		{
			qWarning () << Q_FUNC_INFO
					<< realObj
					<< "doesn't implement ICLEntry";
			return;
		}

		const auto& allowed = Util::Filter (Entries_,
				[real] (MetaEntry *entry) { return !entry->GetRealEntries ().contains (real->GetEntryID ()); });

		AddToMetacontactsDialog dia (real, allowed);
		if (dia.exec () != QDialog::Accepted)
			return;

		MetaEntry *existingMeta = dia.GetSelectedMeta ();
		if (!existingMeta)
		{
			const QString& name = dia.GetNewMetaName ();
			if (name.isEmpty ())
				return;

			existingMeta = CreateMetaEntry ();
			existingMeta->SetEntryName (name);
		}

		AddRealToMeta (existingMeta, real);
	}

	bool Core::HandleDnDEntry2Entry (QObject *sourceObj, QObject *targetObj)
	{
		if (qobject_cast<MetaEntry*> (sourceObj))
			std::swap (sourceObj, targetObj);

		ICLEntry *source = qobject_cast<ICLEntry*> (sourceObj);
		ICLEntry *target = qobject_cast<ICLEntry*> (targetObj);

		if (!source ||
				!target ||
				source == target ||
				qobject_cast<IMUCEntry*> (sourceObj) ||
				qobject_cast<IMUCEntry*> (targetObj))
			return false;

		if (const auto targetME = qobject_cast<MetaEntry*> (targetObj))
		{
			if (const auto sourceME = qobject_cast<MetaEntry*> (sourceObj))
			{
				const auto& reals = sourceME->GetAvailEntryObjs ();
				RemoveEntry (sourceME);
				for (const auto real : reals)
					AddRealToMeta (targetME, qobject_cast<ICLEntry*> (real));
			}
			else
				AddRealToMeta (targetME, source);

			return true;
		}

		const QString& name = QInputDialog::getText (0,
				"LeechCraft",
				tr ("Enter the name of the new metacontact uniting %1 and %2:")
					.arg (Util::FormatName (source->GetEntryName ()))
					.arg (Util::FormatName (target->GetEntryName ())),
				QLineEdit::Normal,
				source->GetEntryName ());
		if (name.isEmpty ())
			return false;

		MetaEntry *entry = CreateMetaEntry ();
		entry->SetEntryName (name);

		AddRealToMeta (entry, source);
		AddRealToMeta (entry, target);

		return true;
	}

	void Core::RemoveEntry (MetaEntry *entry)
	{
		Entries_.removeAll (entry);
		emit removedCLItems ({ entry });

		HandleEntriesRemoved (entry->GetAvailEntryObjs (), true);

		entry->deleteLater ();

		if (Entries_.isEmpty ())
			emit accountRemoved (Account_);
	}

	void Core::ScheduleSaveEntries ()
	{
		if (SaveEntriesScheduled_)
			return;

		QTimer::singleShot (1000,
				this,
				SLOT (saveEntries ()));
		SaveEntriesScheduled_ = true;
	}

	void Core::HandleEntriesRemoved (const QList<QObject*>& entries, bool readd)
	{
		for (const auto entryObj : entries)
		{
			const auto entry = qobject_cast<ICLEntry*> (entryObj);
			AvailRealEntries_.remove (entry->GetEntryID ());
			if (readd)
				entry->GetParentAccount ()->gotCLItems ({ entryObj });
		}

		ScheduleSaveEntries ();
	}

	void Core::AddRealToMeta (MetaEntry *existingMeta, ICLEntry *real)
	{
		existingMeta->AddRealObject (real);
		QTimer::singleShot (0, this, [this, real] { emit removedCLItems ({ real->GetQObject () }); });
		ScheduleSaveEntries ();
	}

	MetaEntry* Core::CreateMetaEntry ()
	{
		const QString& id = QUuid::createUuid ().toString ();
		MetaEntry *result = new MetaEntry (id, Account_);
		ConnectSignals (result);

		if (Entries_.isEmpty ())
			emit accountAdded (Account_);

		Entries_ << result;

		emit gotCLItems ({ result });

		return result;
	}

	void Core::ConnectSignals (MetaEntry *entry)
	{
		connect (entry,
				SIGNAL (shouldRemoveThis ()),
				this,
				SLOT (handleEntryShouldBeRemoved ()));
	}

	void Core::handleEntryShouldBeRemoved ()
	{
		MetaEntry *entry = qobject_cast<MetaEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< sender ()
					<< "to MetaEntry*";
			return;
		}

		RemoveEntry (entry);
	}

	void Core::saveEntries ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Metacontacts_Entries");
		settings.remove ("Entries");
		settings.beginWriteArray ("Entries");
		int i = 0;
		for (const auto entry : Entries_)
		{
			settings.setArrayIndex (i++);
			settings.setValue ("ID", entry->GetEntryID ());
			settings.setValue ("Name", entry->GetEntryName ());
			settings.setValue ("Groups", entry->Groups ());
			settings.setValue ("RealIDs", entry->GetRealEntries ());
		}
		settings.endArray ();

		SaveEntriesScheduled_ = false;
	}
}
}
}
