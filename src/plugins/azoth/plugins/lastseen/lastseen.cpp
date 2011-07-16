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

#include "lastseen.h"
#include <QTimer>
#include <QSettings>
#include <QCoreApplication>
#include <QIcon>
#include <QTranslator>
#include <util/util.h>
#include <interfaces/iclentry.h>

Q_DECLARE_METATYPE (LeechCraft::Azoth::LastSeen::LastHash_t);

namespace LeechCraft
{
namespace Azoth
{
namespace LastSeen
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		SaveScheduled_ = false;

		Translator_.reset (Util::InstallTranslator ("azoth_lastseen"));

		qRegisterMetaType<LastHash_t> ("LeechCraft::Azoth::LastSeen::LastHash_t");
		qRegisterMetaTypeStreamOperators<LastHash_t> ("LeechCraft::Azoth::LastSeen::LastHash_t");

		Load ();
	}

	void Plugin::SecondInit ()
	{
	}	

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.LastSeen";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth LastSeen";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth LastSeen displays when a contact has been online and available for the list time.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}
	
	namespace
	{
		bool IsGoodEntry (QObject *entryObj)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< entryObj
						<< "doesn't implement ICLEntry";
				return false;
			}
			
			if ((entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
				return false;
			
			return true;
		}
	}
	
	void Plugin::ScheduleSave ()
	{
		if (SaveScheduled_)
			return;
		
		QTimer::singleShot (3000,
				this,
				SLOT (save ()));
		SaveScheduled_ = true;
	}
	
	void Plugin::Load ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_LastSeen");
		LastAvailable_ = settings.value ("LastAvailable").value<LastHash_t> ();
		LastOnline_ = settings.value ("LastOnline").value<LastHash_t> ();
	}
	
	void Plugin::save ()
	{
		SaveScheduled_ = false;
		
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_LastSeen");
		settings.setValue ("LastAvailable",
				QVariant::fromValue<LastHash_t> (LastAvailable_));
		settings.setValue ("LastOnline",
				QVariant::fromValue<LastHash_t> (LastOnline_));
	}
	
	void Plugin::hookEntryStatusChanged (IHookProxy_ptr, QObject *entryObj, QString)
	{
		if (!IsGoodEntry (entryObj))
			return;
		
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		const QString& id = entry->GetEntryID ();
		const EntryStatus& status = entry->GetStatus ();

		if (!LastState_.contains (id))
		{
			LastState_ [id] = status.State_;
			return;
		}
		
		const State oldState = LastState_ [id];
		LastState_ [id] = status.State_;

		switch (oldState)
		{
		case SOffline:
		case SProbe:
		case SError:
		case SInvalid:
		case SConnecting:
			return;
		case SOnline:
			LastAvailable_ [id] = QDateTime::currentDateTime ();
		default:
			LastOnline_ [id] = QDateTime::currentDateTime ();
			ScheduleSave ();
		}
	}
	
	void Plugin::hookTooltipBeforeVariants (IHookProxy_ptr proxy, QObject *entryObj)
	{
		if (!IsGoodEntry (entryObj))
			return;
		
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		const QString& id = entry->GetEntryID ();

		QString addition;

		const State curState = entry->GetStatus ().State_;
		
		if (curState != SOnline)
		{
			const QDateTime& avail = LastAvailable_.value (id);
			if (avail.isValid ())
				addition += tr ("Was available: %1")
					.arg (avail.toString ());
		}

		if (curState == SOffline ||
				curState == SError ||
				curState == SInvalid)
		{
			const QDateTime& online = LastOnline_.value (id);
			if (LastOnline_.contains (id))
			{
				if (!addition.isEmpty ())
					addition += "<br/>";
				addition += tr ("Was online: %1")
					.arg (online.toString ());
			}
		}
		
		if (addition.isEmpty ())
			return;

		const QString& tip = proxy->GetValue ("tooltip").toString ();
		proxy->SetValue ("tooltip", tip + "<br/><br/>" + addition + "<br/>");
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_lastseen, LeechCraft::Azoth::LastSeen::Plugin);
