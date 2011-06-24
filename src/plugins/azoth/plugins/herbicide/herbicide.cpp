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

#include "herbicide.h"
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <plugininterface/util.h>
#include <interfaces/iclentry.h>
#include <interfaces/imessage.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "confwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Herbicide
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_herbicide"));
		
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothherbicidesettings.xml");
		
		ConfWidget_ = new ConfWidget ();
		SettingsDialog_->SetCustomWidget ("ConfWidget", ConfWidget_);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Herbicide";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Herbicide";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple antispam plugin for Azoth.");
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
	
	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}
	
	void Plugin::hookGonnaAppendMsg (LeechCraft::IHookProxy_ptr proxy,
				QObject *message)
	{
	}

	void Plugin::hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message)
	{
		IMessage *msg = qobject_cast<IMessage*> (message);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< message
					<< "doesn't implement IMessage";
			return;
		}
		
		QObject *entryObj = msg->OtherPart ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (entry->GetEntryFeatures () & ICLEntry::FPermanentEntry)
			return;
	}
	
	void Plugin::hookShouldCountUnread (IHookProxy_ptr proxy,
				QObject *message)
	{
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_herbicide, LeechCraft::Azoth::Herbicide::Plugin);
