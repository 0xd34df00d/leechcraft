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

#include "syncunmountablemanager.h"
#include <interfaces/lmp/iunmountablesync.h>
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	SyncUnmountableManager::SyncUnmountableManager (QObject *parent)
	: SyncManagerBase (parent)
	, CopyMgr_ (new CopyManager<CopyJob> (this))
	{
		connect (CopyMgr_,
				SIGNAL (startedCopying (QString)),
				this,
				SLOT (handleStartedCopying (QString)));
		connect (CopyMgr_,
				SIGNAL (finishedCopying ()),
				this,
				SLOT (handleFinishedCopying ()));
	}

	void SyncUnmountableManager::AddFiles (const AddFilesParams& params)
	{
		auto coll = Core::Instance ().GetLocalCollection ();

		auto syncer = params.Syncer_;
		for (auto i = 0, size = params.Files_.size (); i < size; ++i)
		{
			const auto& file = params.Files_.at (i);

			const auto trackId = coll->FindTrack (file);
			const auto album = coll->GetTrackAlbum (trackId);
			if (!album)
				continue;

			const auto& artists = coll->GetAlbumArtists (album->ID_);
			if (artists.isEmpty ())
				continue;

			const auto& artist = coll->GetArtist (artists.at (0));

			syncer->SetFileInfo (file,
					{ artist.Name_, album->Name_, album->Year_, album->CoverPath_, QStringList () });

			Source2Params_ [file] = params;
		}

		SyncManagerBase::AddFiles (params.Files_, params.TCParams_);
	}

	void SyncUnmountableManager::handleFileTranscoded (const QString& from, const QString& transcoded, QString)
	{
		SyncManagerBase::HandleFileTranscoded (from, transcoded);

		const auto& params = Source2Params_.take (from);
		if (!params.Syncer_)
		{
			qWarning () << Q_FUNC_INFO
					<< "no syncer for file"
					<< from;
			return;
		}

		const CopyJob copyJob
		{
			transcoded,
			from != transcoded,
			params.Syncer_,
			params.DevID_,
			params.StorageID_,
			from
		};
		CopyMgr_->Copy (copyJob);
	}
}
}
