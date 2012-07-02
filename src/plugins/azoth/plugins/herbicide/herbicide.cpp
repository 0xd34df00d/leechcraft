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
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "confwidget.h"

uint qHash (const QRegExp& rx)
{
	return qHash (rx.pattern ());
}

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

		handleWhitelistChanged ();
		handleBlacklistChanged ();
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
		static QIcon icon (":/plugins/azoth/plugins/herbicide/resources/images/herbicide.svg");
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

		const auto& id = entry->GetHumanReadableID ();

		Q_FOREACH (const auto& rx, Whitelist_)
			if (rx.exactMatch (id))
				return true;

		if (XmlSettingsManager::Instance ().property ("AskOnlyBL").toBool ())
		{
			Q_FOREACH (const auto& rx, Blacklist_)
				if (rx.exactMatch (id))
					return false;

			return true;
		}

		return false;
	}

	void Plugin::ChallengeEntry (IHookProxy_ptr proxy, QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);
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

	void Plugin::GreetEntry (QObject *entryObj)
	{
		AllowedEntries_ << entryObj;

		AskedEntries_.remove (entryObj);

		auto entry = qobject_cast<ICLEntry*> (entryObj);

		const QString& text = tr ("Nice, seems like you've answered "
				"correctly. Please write again now what you wanted "
				"to write.");
		QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage, QString (), text);
		OurMessages_ << msgObj;
		qobject_cast<IMessage*> (msgObj)->Send ();

		if (DeniedAuth_.contains (entryObj))
			QMetaObject::invokeMethod (entry->GetParentAccount (),
					"authorizationRequested",
					Q_ARG (QObject*, entryObj),
					Q_ARG (QString, DeniedAuth_.take (entryObj)));
	}

	void Plugin::hookGotAuthRequest (IHookProxy_ptr proxy, QObject *entry, QString msg)
	{
		if (!IsConfValid ())
			return;

		if (!XmlSettingsManager::Instance ().property ("EnableForAuths").toBool ())
			return;

		if (IsEntryAllowed (entry))
			return;

		if (!AskedEntries_.contains (entry))
		{
			ChallengeEntry (proxy, entry);
			DeniedAuth_ [entry] = msg;
		}
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
			ChallengeEntry (proxy, entryObj);
		else if (ConfWidget_->GetAnswers ().contains (msg->GetBody ().toLower ()))
			GreetEntry (entryObj);
		else
		{
			const QString& text = tr ("Sorry, you are wrong. Try again.");
			QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage, QString (), text);
			OurMessages_ << msgObj;
			qobject_cast<IMessage*> (msgObj)->Send ();

			proxy->CancelDefault ();
		}
	}

	namespace
	{
		QSet<QRegExp> GetRegexps (const QByteArray& prop)
		{
			QSet<QRegExp> result;

			const auto& strings = XmlSettingsManager::Instance ().property (prop).toStringList ();
			Q_FOREACH (auto string, strings)
			{
				string = string.trimmed ();
				if (string.isEmpty ())
					continue;
				result << QRegExp (string);
			}

			return result;
		}
	}

	void Plugin::handleWhitelistChanged ()
	{
		Whitelist_ = GetRegexps ("WhitelistRegexps");
	}

	void Plugin::handleBlacklistChanged ()
	{
		Blacklist_ = GetRegexps ("BlacklistRegexps");
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_herbicide, LeechCraft::Azoth::Herbicide::Plugin);
