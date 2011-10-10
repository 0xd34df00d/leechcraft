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

#include "autopaste.h"
#include <QIcon>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/imessage.h>
#include <interfaces/iclentry.h>
#include <interfaces/core/icoreproxy.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_autopaste"));

		Proxy_ = proxy;

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothautopastesettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Autopaste";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Autopaste";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Detects long messages and suggests pasting them to a pastebin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/autopaste/resources/images/autopaste.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void Plugin::Paste (const QString& text, QObject *entry)
	{
		QNetworkRequest req (QUrl ("http://paste.pocoo.org/"));
		req.setHeader (QNetworkRequest::ContentTypeHeader,
				"application/x-www-form-urlencoded");
		req.setRawHeader ("Referer", "http://paste.pocoo.org/");

		QByteArray data = "code=";
		data += text.toUtf8 ().toPercentEncoding ();
		data += "&language=text&webpage=";
		req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());

		QNetworkReply *reply = Proxy_->GetNetworkAccessManager ()->post (req, data);
		connect (reply,
				SIGNAL (metaDataChanged ()),
				this,
				SLOT (handleMetadata ()));

		Reply2Entry_ [reply] = entry;
	}

	void Plugin::hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
			QObject *chatTab, QObject *entry, int, QString)
	{
		ICLEntry *other = qobject_cast<ICLEntry*> (entry);
		if (!other)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< entry
				<< "to ICLEntry";
			return;
		}

		QString text = proxy->GetValue ("text").toString ();

		const int maxLines = XmlSettingsManager::Instance ()
				.property ("LineCount").toInt ();
		if (text.split ('\n').size () < maxLines)
			return;

		QByteArray propName;
		switch (other->GetEntryType ())
		{
		case ICLEntry::ETChat:
			propName = "EnableForNormalChats";
			break;
		case ICLEntry::ETMUC:
			propName = "EnableForMUCChats";
			break;
		case ICLEntry::ETPrivateChat:
			propName = "EnableForPrivateChats";
			break;
		default:
			return;
		}

		if (!XmlSettingsManager::Instance ()
				.property (propName).toBool ())
			return;

		const bool shouldConfirm = XmlSettingsManager::Instance ()
				.property ("ConfirmPasting").toBool ();
		if (shouldConfirm &&
			QMessageBox::question (qobject_cast<QWidget*> (chatTab),
					tr ("Confirm pasting"),
					tr ("This message is too long according to current "
						"settings. Would you like to paste it on a "
						"pastebin?"),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;

		Paste (text, entry);
		proxy->CancelDefault ();
	}

	void Plugin::handleMetadata ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a QNetworkReply:"
					<< sender ();
			return;
		}

		const QString& pasteUrl = reply->header (QNetworkRequest::LocationHeader).toString ();
		QPointer<QObject> entryObj = Reply2Entry_ [reply];
		if (!entryObj)
		{
			QApplication::clipboard ()->setText (pasteUrl, QClipboard::Clipboard);
			QApplication::clipboard ()->setText (pasteUrl, QClipboard::Selection);
			const Entity& e = Util::MakeNotification (tr ("Text pasted"),
					tr ("Your text has been pasted: %1. The URL has "
						"been copied to the clipboard."),
					PInfo_);
			emit gotEntity (e);
			return;
		}

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< entryObj
					<< "to ICLEntry";
			return;
		}

		IMessage::MessageType type =
				entry->GetEntryType () == ICLEntry::ETMUC ?
						IMessage::MTMUCMessage :
						IMessage::MTChatMessage;
		QObject *msgObj = entry->CreateMessage (type, QString (), pasteUrl);
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< msgObj
					<< "to IMessage";
			return;
		}
		msg->Send ();
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_autopaste, LeechCraft::Azoth::Autopaste::Plugin);
