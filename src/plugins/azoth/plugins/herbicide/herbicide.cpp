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

#include "herbicide.h"
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <util/util.h>
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
		Util::InstallTranslator ("azoth_herbicide");

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
		return QIcon (":/plugins/azoth/plugins/herbicide/resources/images/herbicide.svg");
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

	bool Plugin::IsConfValid () const
	{
		if (!XmlSettingsManager::Instance ()
				.property ("EnableQuest").toBool ())
			return false;

		if (ConfWidget_->GetQuestion ().isEmpty () ||
				ConfWidget_->GetAnswers ().isEmpty ())
			return false;

		return true;
	}

	bool Plugin::IsEntryAllowed (QObject *entryObj) const
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
			return true;

		if ((entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) == ICLEntry::FPermanentEntry)
			return true;

		if (AllowedEntries_.contains (entryObj))
			return true;

		return false;
	}

	void Plugin::hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message)
	{
		if (!IsConfValid ())
			return;

		if (OurMessages_.contains (message))
		{
			OurMessages_.remove (message);
			proxy->CancelDefault ();
			return;
		}

		IMessage *msg = qobject_cast<IMessage*> (message);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< message
					<< "doesn't implement IMessage";
			return;
		}

		if (msg->GetMessageType () != IMessage::MTChatMessage)
			return;

		QObject *entryObj = msg->OtherPart ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		if (IsEntryAllowed (entryObj))
			return;

		if (!AskedEntries_.contains (entryObj))
		{
			AskedEntries_ << entryObj;
			const QString& text = tr ("Please answer to the following "
					"question to verify you are not a bot and is welcome "
					"to communicate with me:\n%1")
						.arg (ConfWidget_->GetQuestion ());
			QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage, QString (), text);
			OurMessages_ << msgObj;
			qobject_cast<IMessage*> (msgObj)->Send ();

			proxy->CancelDefault ();
		}
		else if (ConfWidget_->GetAnswers ().contains (msg->GetBody ().toLower ()))
		{
			AllowedEntries_ << entryObj;
			AskedEntries_.remove (entryObj);
			const QString& text = tr ("Nice, seems like you've answered "
					"correctly. Please write again now what you wanted "
					"to write.");
			QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage, QString (), text);
			OurMessages_ << msgObj;
			qobject_cast<IMessage*> (msgObj)->Send ();
		}
		else
		{
			const QString& text = tr ("Sorry, you are wrong. Try again.");
			QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage, QString (), text);
			OurMessages_ << msgObj;
			qobject_cast<IMessage*> (msgObj)->Send ();

			proxy->CancelDefault ();
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_herbicide, LeechCraft::Azoth::Herbicide::Plugin);
