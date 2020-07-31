/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xtazy.h"
#include <QIcon>
#include <QMessageBox>
#include <QUrl>
#include <util/util.h>
#include <util/sll/delayedexecutor.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iwebfilestorage.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/media/icurrentsongkeeper.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/iproxyobject.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "tracksharedialog.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xtazy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_xtazy");

		Proxy_ = proxy;

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothxtazysettings.xml");

		Commands_.append ({
				{ "/np" },
				[this] (ICLEntry *entry, QString& text) { return SendCurrentSong (entry, text); },
				tr ("Sends the metadata of the currently plaing tune to the chat."),
				{}
			});
		Commands_.append ({
				{ "/sharesong" },
				[this] (ICLEntry *entry, QString& text) { return HandleShare (entry, text); },
				tr ("Uploads the current track file to a web file storage and sends the link to the chat."),
				{}
			});
	}

	void Plugin::SecondInit ()
	{
		auto keeperObjs = Proxy_->GetPluginsManager ()->GetAllCastableRoots<decltype (Keeper_)> ();
		if (keeperObjs.isEmpty ())
			return;

		Keeper_ = qobject_cast<decltype (Keeper_)> (keeperObjs.at (0));
		connect (keeperObjs.at (0),
				SIGNAL (currentSongChanged (Media::AudioInfo)),
				this,
				SLOT (publish (Media::AudioInfo)));

		XmlSettingsManager::Instance ().RegisterObject ("AutoPublishTune",
				this, "handleAutoPublishChanged");
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Xtazy";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Xtazy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Publishes current tune.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
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

	StaticCommands_t Plugin::GetStaticCommands (ICLEntry*)
	{
		if (!Keeper_)
			return {};

		return Commands_;
	}

	bool Plugin::SendCurrentSong (ICLEntry *entry, QString& text)
	{
		const auto& song = Keeper_->GetCurrentSong ();

		QString songStr;
		if (!song.Title_.isEmpty () &&
				!song.Artist_.isEmpty ())
		{
			songStr = !song.Album_.isEmpty () ?
					XmlSettingsManager::Instance ().property ("NPCmdSubst").toString () :
					XmlSettingsManager::Instance ().property ("NPCmdSubstWOAlbum").toString ();
			songStr.replace ("$artist", song.Artist_);
			songStr.replace ("$album", song.Album_);
			songStr.replace ("$title", song.Title_);
		}
		else
			songStr = XmlSettingsManager::Instance ().property ("NPCmdNoPlaying").toString ();

		if (XmlSettingsManager::Instance ().property ("SendTextImmediately").toBool ())
		{
			text = songStr;
			return false;
		}
		else
		{
			const auto& eId = entry->GetEntryID ();
			const auto& accId = entry->GetParentAccount ()->GetAccountID ();
			new Util::DelayedExecutor
			{
				[this, eId, accId, songStr] { AzothProxy_->OpenChat (eId, accId, songStr); },
				0
			};

			return true;
		}
	}

	bool Plugin::HandleShare (ICLEntry *entry, QString& text)
	{
		const auto& song = Keeper_->GetCurrentSong ();
		const auto& url = song.Other_ ["URL"].toUrl ();
		if (!url.isValid ())
			return true;

		if (url.scheme () != "file")
		{
			text = QString::fromUtf8 (url.toEncoded ());
			return false;
		}

		auto sharers = Proxy_->GetPluginsManager ()->GetAllCastableRoots<IWebFileStorage*> ();
		QMap<QString, QObject*> variants;
		for (const auto sharerObj : sharers)
		{
			auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
			for (const auto& var : sharer->GetServiceVariants ())
				variants [var] = sharerObj;
		}

		if (sharers.isEmpty ())
		{
			QMessageBox::critical (0,
					"LeechCraft",
					tr ("No web share plugins are installed. Try installing NetStoreManager, for example."));
			return true;
		}

		const auto& localPath = url.toLocalFile ();

		TrackShareDialog dia (localPath, variants.keys (), entry->GetQObject ());
		if (dia.exec () != QDialog::Accepted)
			return true;

		const auto& selectedVar = dia.GetVariantName ();
		auto sharerObj = variants [selectedVar];

		auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
		sharer->UploadFile (localPath, selectedVar);

		PendingUploads_ [localPath].push_back ({ entry->GetQObject (), {} });

		connect (sharerObj,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SLOT (handleFileUploaded (QString, QUrl)),
				Qt::UniqueConnection);

		return true;
	}

	void Plugin::SendAudioInfo (const Media::AudioInfo& info)
	{
		QVariantMap map;
		map ["artist"] = info.Artist_;
		map ["source"] = info.Album_;
		map ["title"] = info.Title_;
		map ["length"] = info.Length_;
		map ["track"] = info.TrackNumber_;

		for (auto accObj : AzothProxy_->GetAllAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
				continue;
			if (acc->GetState ().State_ == SOffline)
				continue;

			if (auto tune = qobject_cast<ISupportTune*> (accObj))
				tune->PublishTune (map);
		}
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::publish (const Media::AudioInfo& info)
	{
		if (XmlSettingsManager::Instance ().property ("AutoPublishTune").toBool ())
			SendAudioInfo (info);
	}

	void Plugin::handleFileUploaded (const QString& filename, const QUrl& url)
	{
		if (!PendingUploads_.contains (filename))
			return;

		const auto& encoded = url.toEncoded ();

		for (const auto& notifee : PendingUploads_.take (filename))
		{
			auto entry = qobject_cast<ICLEntry*> (notifee.first);
			if (!entry)
				continue;

			const auto msgType = entry->GetEntryType () == ICLEntry::EntryType::MUC ?
					IMessage::Type::MUCMessage :
					IMessage::Type::ChatMessage;
			entry->CreateMessage (msgType, notifee.second, encoded)->Send ();
		}
	}

	void Plugin::handleAutoPublishChanged ()
	{
		if (!XmlSettingsManager::Instance ().property ("AutoPublishTune").toBool ())
			SendAudioInfo ({});
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_xtazy, LC::Azoth::Xtazy::Plugin);
