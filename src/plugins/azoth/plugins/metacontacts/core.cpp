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

#include "core.h"
#include <QSettings>
#include <QTimer>
#include <QCoreApplication>
#include <QUuid>
#include <QtDebug>
#include "metaaccount.h"
#include "metaentry.h"
#include "addtometacontactsdialog.h"

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
			entry->SetEntryName (name);
			entry->SetGroups (settings.value ("Groups").toStringList ());
			entry->SetRealEntries (reals);
			Entries_ << entry;
			
			Q_FOREACH (const QString& id, reals)
				UnavailRealEntries_ [id] = entry;
		}
		settings.endArray ();
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
		if (entryObj->metaObject ()->className () == "MetaEntry")
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
		if (!UnavailRealEntries_.contains (id))
			return false;
		
		UnavailRealEntries_.take (id)->AddRealObject (entry);
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
			
			const QString& id = QUuid::createUuid ().toString ();
			existingMeta = new MetaEntry (id, Account_);
			existingMeta->SetEntryName (name);
			
			Entries_ << existingMeta;
			
			emit gotCLItems (QList<QObject*> () << existingMeta);
		}
		
		existingMeta->AddRealObject (real);
		QMetaObject::invokeMethod (this,
				"removedCLItems",
				Qt::QueuedConnection,
				Q_ARG (QList<QObject*>, QList<QObject*> () << realObj));
		
		ScheduleSaveEntries ();
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
