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

		for (auto prov : Providers_)
		{
			auto pending = prov->Search (req);
			connect (pending->GetObject (),
					SIGNAL (ready ()),
					this,
					SLOT (handlePendingReady ()));
		}
	}

	void PreviewHandler::handlePendingReady ()
	{
		auto pending = qobject_cast<Media::IPendingAudioSearch*> (sender ());

		QList<Phonon::MediaSource> sources;
		for (const auto& res : pending->GetResults ())
		{
			Player_->PrepareURLInfo (res.Source_, MediaInfo::FromAudioInfo (res.Info_));
			sources << Phonon::MediaSource (res.Source_);
		}

		if (sources.isEmpty ())
			return;

		Player_->Enqueue (sources, false);
	}
}
}
