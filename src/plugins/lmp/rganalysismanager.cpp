/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rganalysismanager.h"
#include "localcollection.h"
#include "localcollectionstorage.h"
#include "engine/rganalyser.h"
#include "engine/rgfilter.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace LMP
{
	RgAnalysisManager::RgAnalysisManager (LocalCollection *coll, QObject *parent)
	: QObject { parent }
	, Coll_ { coll }
	{
		connect (Coll_,
				SIGNAL (scanFinished ()),
				this,
				SLOT (handleScanFinished ()));

		XmlSettingsManager::Instance ().RegisterObject ("AutobuildRG",
				this, "handleScanFinished");
	}

	namespace
	{
		bool IsScanAllowed ()
		{
			return XmlSettingsManager::Instance ().property ("AutobuildRG").toBool ();
		}
	}

	void RgAnalysisManager::handleAnalysed ()
	{
		const auto& result = CurrentAnalyser_->GetResult ();

		for (const auto& track : result.Tracks_)
		{
			const auto id = Coll_->FindTrack (track.TrackPath_);
			if (id == -1)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot find track"
						<< track.TrackPath_;
				continue;
			}

			Coll_->GetStorage ()->SetRgTrackInfo (id,
					{
						track.TrackGain_,
						track.TrackPeak_,
						result.AlbumGain_,
						result.AlbumPeak_
					});
		}

		CurrentAnalyser_.reset ();
		rotateQueue ();
	}

	void RgAnalysisManager::rotateQueue ()
	{
		if (AlbumsQueue_.isEmpty ())
			return;

		if (!IsScanAllowed ())
		{
			AlbumsQueue_.clear ();
			return;
		}

		QStringList paths;
		for (const auto& track : AlbumsQueue_.takeFirst ()->Tracks_)
			paths << track.FilePath_;

		CurrentAnalyser_ = std::make_shared<RgAnalyser> (paths);
		connect (CurrentAnalyser_.get (),
				SIGNAL (finished ()),
				this,
				SLOT (handleAnalysed ()));
	}

	void RgAnalysisManager::handleScanFinished ()
	{
		qDebug () << Q_FUNC_INFO << IsScanAllowed ();
		if (!IsScanAllowed ())
			return;

		const bool wasEmpty = AlbumsQueue_.isEmpty ();

		for (auto albumId : Coll_->GetStorage ()->GetOutdatedRgAlbums ())
			if (const auto& album = Coll_->GetAlbum (albumId))
				AlbumsQueue_ << album;

		qDebug () << AlbumsQueue_.size ()
				<< "albums to rescan";
		if (wasEmpty)
			rotateQueue ();
	}
}
}
