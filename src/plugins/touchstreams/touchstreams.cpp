/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "touchstreams.h"
#include <QIcon>
#include <QStandardItem>
#include <QFuture>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/queuemanager.h>
#include <util/sll/prelude.h>
#include <util/sll/functional.h>
#include <util/threads/futures.h>
#include <util/svcauth/vkauthmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "audiosearch.h"
#include "albumsmanager.h"
#include "friendsmanager.h"
#include "authclosehandler.h"
#include "recsmanager.h"
#include "tracksrestorehandler.h"

namespace LC
{
namespace TouchStreams
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "touchstreamssettings.xml");

		Queue_ = new Util::QueueManager (1000);

		AuthMgr_ = new Util::SvcAuth::VkAuthManager ("TouchStreams",
				"3298289",
				{ "audio", "friends" },
				XmlSettingsManager::Instance ().property ("Cookies").toByteArray (),
				proxy,
				Queue_);
		connect (AuthMgr_,
				SIGNAL (cookiesChanged (QByteArray)),
				this,
				SLOT (saveCookies (QByteArray)));

		AlbumsMgr_ = new AlbumsManager (AuthMgr_, proxy, this);
		FriendsMgr_ = new FriendsManager (AuthMgr_, Queue_, proxy, this);
		RecsManager_ = new RecsManager ({}, AuthMgr_, Queue_, proxy, this);

		Model_ = new QStandardItemModel;
		Model_->appendRow (AlbumsMgr_->GetRootItem ());
		Model_->appendRow (FriendsMgr_->GetRootItem ());
		Model_->appendRow (RecsManager_->GetRootItem ());
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TouchStreams";
	}

	void Plugin::Release ()
	{
		delete Queue_;
	}

	QString Plugin::GetName () const
	{
		return "TouchStreams";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("VK.com music streamer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QString Plugin::GetServiceName () const
	{
		return tr ("VKontakte");
	}

	QIcon Plugin::GetServiceIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QFuture<Media::IAudioPile::Result_t> Plugin::Search (const Media::AudioSearchRequest& req)
	{
		auto realReq = req;
		if (realReq.FreeForm_.isEmpty ())
		{
			QStringList parts { req.Artist_, req.Album_, req.Title_ };
			parts.removeAll ({});
			realReq.FreeForm_ = parts.join (" - ");
		}

		return (new AudioSearch (Proxy_, realReq, AuthMgr_, Queue_))->GetFuture ();
	}

	QList<QAbstractItemModel*> Plugin::GetRadioListItems () const
	{
		return { Model_ };
	}

	Media::IRadioStation_ptr Plugin::GetRadioStation (const QModelIndex& index, const QString&)
	{
		if (index.data (Media::RadioItemRole::RadioID).toString () == "auth")
		{
			AuthMgr_->ClearAuthData ();
			AuthMgr_->Reauth ();
		}

		return {};
	}

	void Plugin::RefreshItems (const QList<QModelIndex>& indices)
	{
		auto items = indices.isEmpty () ?
				Model_->findItems ({}) :
				Util::Map (indices, Util::BindMemFn (&QStandardItemModel::itemFromIndex, Model_));

		AlbumsMgr_->RefreshItems (items);
		FriendsMgr_->RefreshItems (items);
		RecsManager_->RefreshItems (items);
	}

	void Plugin::saveCookies (const QByteArray& cookies)
	{
		XmlSettingsManager::Instance ().setProperty ("Cookies", cookies);
	}

	QFuture<Media::RadiosRestoreResult_t> Plugin::RestoreRadioStations (const QStringList& ids)
	{
		const auto nam = Proxy_->GetNetworkAccessManager ();
		const auto handler = new TracksRestoreHandler { ids, nam, AuthMgr_, Queue_ };
		return handler->GetFuture ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_touchstreams, LC::TouchStreams::Plugin);
