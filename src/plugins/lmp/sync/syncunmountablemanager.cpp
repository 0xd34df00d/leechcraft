/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncunmountablemanager.h"
#include <interfaces/lmp/iunmountablesync.h>
#include "core.h"
#include "localcollection.h"
#include "localcollectionmodel.h"

namespace LC
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
		connect (CopyMgr_,
				SIGNAL (copyProgress (qint64, qint64)),
				this,
				SLOT (handleCopyProgress (qint64, qint64)));
		connect (CopyMgr_,
				SIGNAL (errorCopying (QString, QString)),
				this,
				SLOT (handleErrorCopying (QString, QString)));
	}

	void SyncUnmountableManager::AddFiles (const AddFilesParams& params)
	{
		auto coll = Core::Instance ().GetLocalCollection ();

		const auto& format = params.TCParams_.FormatID_;

		auto syncer = params.Syncer_;
		for (const auto& file : params.Files_)
		{
			const auto trackId = coll->FindTrack (file);
			if (trackId < 0)
				continue;

			const auto trackNumber = coll->
					GetTrackData (trackId, LocalCollectionModel::Role::TrackNumber).toInt ();
			const auto& trackTitle = coll->
					GetTrackData (trackId, LocalCollectionModel::Role::TrackTitle).toString ();

			const auto album = coll->GetTrackAlbum (trackId);
			if (!album)
				continue;

			const auto& artists = coll->GetAlbumArtists (album->ID_);
			if (artists.isEmpty ())
				continue;

			const auto& artist = coll->GetArtist (artists.at (0));

			syncer->SetFileInfo (file,
					{
						format.isEmpty () ?
							QFileInfo (file).suffix ().toLower () :
							format,
						trackNumber,
						trackTitle,
						artist.Name_,
						album->Name_,
						album->Year_,
						album->CoverPath_,
						QStringList ()
					});

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
