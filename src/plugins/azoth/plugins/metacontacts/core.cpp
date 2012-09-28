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

#include "core.h"
#include <QSettings>
#include <QTimer>
#include <QCoreApplication>
#include <QUuid>
#include <QtDebug>
#include "metaaccount.h"
#include "metaentry.h"
#include "addtometacontactsdialog.h"
#include <QInputDialog>

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	Core::Core ()
	: SaveEntriesScheduled_ (false)
	, Account_ (0)
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

			Q_FOREACH (const QString& id, reals)
				UnavailRealEntries_ [id] = entry;
		}
		settings.endArray ();

		if (!Entries_.isEmpty ())
			emit accountAdded (Account_);
	}

	QList<QObject*> Core::GetEntries () const
	{
		QList<QObject*> result;
		Q_FOREACH (MetaEntry *entry, Entries_)
			result << entry;
		return result;
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

		QList<MetaEntry*> allowed = Entries_;
		Q_FOREACH (MetaEntry *entry, allowed)
			if (entry->GetRealEntries ().contains (real->GetEntryID ()))
				allowed.removeAll (entry);

		AddToMetacontactsDialog dia (real, Entries_);
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

		if (!source || !target || source == target)
			return false;

		MetaEntry *targetME = qobject_cast<MetaEntry*> (targetObj);
		if (targetME)
		{
			MetaEntry *sourceME = qobject_cast<MetaEntry*> (sourceObj);

			if (!sourceME)
				AddRealToMeta (targetME, source);
			else
			{
				const QObjectList& reals = sourceME->GetAvailEntryObjs ();

				RemoveEntry (sourceME);

				Q_FOREACH (QObject *real, reals)
					AddRealToMeta (targetME, qobject_cast<ICLEntry*> (real));
			}

			return true;
		}

		const QString& name = QInputDialog::getText (0,
				"LeechCraft",
				tr ("Enter the name of the new metacontact uniting %1 and %2:")
					.arg (source->GetEntryName ())
					.arg (target->GetEntryName ()),
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
		emit removedCLItems (QObjectList () << entry);

		handleEntriesRemoved (entry->GetAvailEntryObjs ());

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

	void Core::AddRealToMeta (MetaEntry *existingMeta, ICLEntry *real)
	{
		existingMeta->AddRealObject (real);
		QMetaObject::invokeMethod (this,
				"removedCLItems",
				Qt::QueuedConnection,
				Q_ARG (QList<QObject*>, QList<QObject*> () << real->GetObject ()));

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

		emit gotCLItems (QList<QObject*> () << result);

		return result;
	}

	void Core::ConnectSignals (MetaEntry *entry)
	{
		connect (entry,
				SIGNAL (entriesRemoved (const QList<QObject*>&)),
				this,
				SLOT (handleEntriesRemoved (const QList<QObject*>&)));
		connect (entry,
				SIGNAL (shouldRemoveThis ()),
				this,
				SLOT (handleEntryShouldBeRemoved ()));
	}

	void Core::handleEntriesRemoved (const QList<QObject*>& entries)
	{
		Q_FOREACH (QObject *entryObj, entries)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

			AvailRealEntries_.remove (entry->GetEntryID ());

			QObject *accObj = entry->GetParentAccount ();
			QMetaObject::invokeMethod (accObj,
					"gotCLItems",
					Q_ARG (QList<QObject*>, QList<QObject*> () << entryObj));
		}

		ScheduleSaveEntries ();
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
		Q_FOREACH (MetaEntry *entry, Entries_)
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
