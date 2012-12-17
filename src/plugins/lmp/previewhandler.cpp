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

#include "previewhandler.h"
#include <interfaces/media/iaudiopile.h>
#include <interfaces/core/ipluginsmanager.h>
#include "player.h"
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	PreviewHandler::PreviewHandler (Player *player, QObject *parent)
	: QObject (parent)
	, Player_ (player)
	{
		Providers_ = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<Media::IAudioPile*> ();
	}

	void PreviewHandler::previewArtist (const QString& artist)
	{
		Media::AudioSearchRequest req;
		req.Artist_ = artist;
		RequestPreview (req);
	}

	void PreviewHandler::previewTrack (const QString& track, const QString& artist)
	{
		Media::AudioSearchRequest req;
		req.Title_ = track;
		req.Artist_ = artist;
		RequestPreview (req);
	}

	void PreviewHandler::RequestPreview (const Media::AudioSearchRequest& req)
	{
		for (auto prov : Providers_)
		{
			auto pending = prov->Search (req);
			connect (pending->GetObject (),
					SIGNAL (ready ()),
					this,
					SLOT (handlePendingReady ()));
		}
	}

	namespace
	{
		struct SamenessCheckInfo
		{
			QString Artist_;
			QString Title_;
			qint32 Length_;
		};

		bool operator== (const SamenessCheckInfo& i1, const SamenessCheckInfo& i2)
		{
			return i1.Artist_ == i2.Artist_ &&
					i1.Title_ == i2.Title_ &&
					i1.Length_ == i2.Length_;
		}

		uint qHash (const SamenessCheckInfo& info)
		{
			return (qHash (info.Artist_) << 20) + (qHash (info.Title_) << 8) + info.Length_;
		}
	}

	void PreviewHandler::handlePendingReady ()
	{
		auto pending = qobject_cast<Media::IPendingAudioSearch*> (sender ());

		QList<Phonon::MediaSource> sources;
		QSet<SamenessCheckInfo> infos;
		for (const auto& res : pending->GetResults ())
		{
			const SamenessCheckInfo checkInfo { res.Info_.Album_, res.Info_.Title_, res.Info_.Length_ };
			if (infos.contains (checkInfo))
				continue;

			infos << checkInfo;

			Player_->PrepareURLInfo (res.Source_, MediaInfo::FromAudioInfo (res.Info_));
			sources << Phonon::MediaSource (res.Source_);
		}

		if (sources.isEmpty ())
			return;

		Player_->Enqueue (sources, false);
	}
}
}
