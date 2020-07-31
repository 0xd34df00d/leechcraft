/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "musiczombie.h"
#include <QIcon>
#include <QFuture>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/queuemanager.h>
#include <util/util.h>
#include "pendingdisco.h"

#ifdef WITH_CHROMAPRINT
#include "pendingtagsfetch.h"
#endif

namespace LC
{
namespace MusicZombie
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("musiczombie");

		Queue_ = new Util::QueueManager (1000);
		AcoustidQueue_ = new Util::QueueManager (1000);

		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.MusicZombie";
	}

	void Plugin::Release ()
	{
		delete Queue_;
	}

	QString Plugin::GetName () const
	{
		return "MusicZombie";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Client for the MusicBrainz.org service.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QString Plugin::GetServiceName () const
	{
		return "MusicBrainz.org";
	}

	QFuture<Plugin::Result_t> Plugin::GetDiscography (const QString& artist, const QStringList& hints)
	{
		const auto fetcher = new PendingDisco (Queue_, artist, {},
				hints, Proxy_->GetNetworkAccessManager (), this);
		return fetcher->GetFuture ();
	}

	QFuture<Plugin::Result_t> Plugin::GetReleaseInfo (const QString& artist, const QString& release)
	{
		const auto fetcher = new PendingDisco (Queue_, artist, release,
				{ release }, Proxy_->GetNetworkAccessManager (), this);
		return fetcher->GetFuture ();
	}

#ifdef WITH_CHROMAPRINT
	QFuture<Media::AudioInfo> Plugin::FetchTags (const QString& filename)
	{
		auto fetcher = new PendingTagsFetch (AcoustidQueue_, Proxy_->GetNetworkAccessManager (), filename);
		return fetcher->GetFuture ();
	}
#endif
}
}

LC_EXPORT_PLUGIN (leechcraft_musiczombie, LC::MusicZombie::Plugin);
