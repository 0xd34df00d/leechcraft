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

#include "xtazy.h"
#include <QIcon>
#include <QMessageBox>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iwebfilestorage.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/iproxyobject.h>
#include "tunesourcebase.h"
#include "xmlsettingsmanager.h"
#include "filesource.h"
#include "lcsource.h"
#include "tracksharedialog.h"

#ifdef HAVE_DBUS
#include "mprissource.h"
#endif

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_xtazy"));

		AzothProxy_ = 0;
		Proxy_ = proxy;

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothxtazysettings.xml");

		LCSource_ = new LCSource (this);

#ifdef HAVE_DBUS
		TuneSources_ << new MPRISSource (this);
#endif
		TuneSources_ << new FileSource (this);
		TuneSources_ << LCSource_;
	}

	void Plugin::SecondInit ()
	{
		Q_FOREACH (TuneSourceBase *base, TuneSources_)
			connect (base,
					SIGNAL (tuneInfoChanged (const QMap<QString, QVariant>&)),
					this,
					SLOT (publish (const QMap<QString, QVariant>&)));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Xtazy";
	}

	void Plugin::Release ()
	{
		qDeleteAll (TuneSources_);
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
		static QIcon icon (":/plugins/azoth/plugins/xtazy/resources/images/xtazy.svg");
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

	QString Plugin::GetServiceName () const
	{
		return GetName ();
	}

	void Plugin::NowPlaying (const Media::AudioInfo& audio)
	{
		LCSource_->NowPlaying (audio);
	}

	void Plugin::PlaybackStopped ()
	{
		LCSource_->Stopped ();
	}

	void Plugin::LoveCurrentTrack ()
	{
	}

	void Plugin::HandleShare (LeechCraft::IHookProxy_ptr proxy, QObject *entryObj, const QString& variant, const QUrl& url)
	{
		proxy->CancelDefault ();
		if (!url.isValid ())
			return;

		if (url.scheme () != "file")
		{
			proxy->SetValue ("text", QString::fromUtf8 (url.toEncoded ()));
			return;
		}

		auto sharers = Proxy_->GetPluginsManager ()->GetAllCastableRoots<IWebFileStorage*> ();
		QMap<QString, QObject*> variants;
		Q_FOREACH (auto sharerObj, sharers)
		{
			auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
			Q_FOREACH (const auto& var, sharer->GetServiceVariants ())
				variants [var] = sharerObj;
		}

		if (sharers.isEmpty ())
		{
			QMessageBox::critical (0,
					"LeechCraft",
					tr ("No web share plugins are installed. Try installing NetStoreManager, for example."));
			return;
		}

		const auto& localPath = url.toLocalFile ();

		TrackShareDialog dia (localPath, variants.keys (), entryObj);
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& selectedVar = dia.GetVariantName ();
		auto sharerObj = variants [selectedVar];

		auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
		sharer->UploadFile (localPath, selectedVar);

		PendingUploads_ [localPath] << UploadNotifee_t (entryObj, variant);

		connect (sharerObj,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SLOT (handleFileUploaded (QString, QUrl)),
				Qt::UniqueConnection);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
			QObject*,
			QObject *entryObj,
			int,
			QString variant)
	{
		if (!XmlSettingsManager::Instance ().property ("NPCmdEnabled").toBool ())
			return;

		auto text = proxy->GetValue ("text").toString ();
		if (text == "/np")
		{
			if (!Previous_.isEmpty ())
			{
				text = XmlSettingsManager::Instance ().property ("NPCmdSubst").toString ();
				text.replace ("$artist", Previous_ ["artist"].toString ());
				text.replace ("$album", Previous_ ["source"].toString ());
				text.replace ("$title", Previous_ ["title"].toString ());
			}
			else
				text = XmlSettingsManager::Instance ().property ("NPCmdNoPlaying").toString ();
			proxy->SetValue ("text", text);
		}
		else if (text == "/sharesong" && Previous_.contains ("URL"))
			HandleShare (proxy, entryObj, variant, Previous_ ["URL"].toUrl ());
	}

	void Plugin::publish (const QMap<QString, QVariant>& info)
	{
		if (info == Previous_)
			return;

		const QByteArray& objName = sender ()->objectName ().toLatin1 ();

		if (!XmlSettingsManager::Instance ()
				.property ("Enable" + objName).toBool ())
			return;

		Previous_ = info;

		Q_FOREACH (QObject *accObj, AzothProxy_->GetAllAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
				continue;
			if (acc->GetState ().State_ == SOffline)
				continue;

			ISupportTune *tune = qobject_cast<ISupportTune*> (accObj);
			if (tune)
				tune->PublishTune (info);
		}
	}

	void Plugin::handleFileUploaded (const QString& filename, const QUrl& url)
	{
		if (!PendingUploads_.contains (filename))
			return;

		const auto& encoded = url.toEncoded ();

		const auto& notifees = PendingUploads_.take (filename);
		Q_FOREACH (const auto& notifee, notifees)
		{
			auto entry = qobject_cast<ICLEntry*> (notifee.first);
			if (!entry)
				continue;

			const auto msgType = entry->GetEntryType () == ICLEntry::ETMUC ?
					IMessage::MTMUCMessage :
					IMessage::MTChatMessage;
			auto msgObj = entry->CreateMessage (msgType, notifee.second, encoded);
			auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Send ();
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_xtazy, LeechCraft::Azoth::Xtazy::Plugin);

