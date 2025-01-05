/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsfetchmanager.h"
#include <concepts>
#include <QTimer>
#include <util/threads/futures.h>
#include <interfaces/lmp/mediainfo.h>
#include <interfaces/media/itagsfetcher.h>
#include "filesmodel.h"

namespace LC::LMP::Graffiti
{
	namespace
	{
		template<std::default_initializable T>
		bool IsEmptyData (const T& val)
		{
			return val == T {};
		}

		template<typename F>
		void UpgradeInfo (MediaInfo& info, MediaInfo& other, F getter)
		{
			auto& data = std::invoke (getter, info);
			const auto& otherData = std::invoke (getter, other);
			if (!IsEmptyData (otherData) && IsEmptyData (data))
				data = otherData;
		}
	}

	TagsFetchManager::TagsFetchManager (const QStringList& paths,
			Media::ITagsFetcher *prov, FilesModel *filesModel, QObject *parent)
	: QObject { parent }
	, FilesModel_ { filesModel }
	, TotalTags_ { static_cast<int> (paths.size ()) }
	{
		for (const auto& path : paths)
			Util::Sequence (this, prov->FetchTags (path)) >>
					[this, path] (const Media::AudioInfo& result)
					{
						emit tagsFetchProgress (++FetchedTags_, TotalTags_, this);

						const auto& index = FilesModel_->FindIndex (path);
						if (!index.isValid ())
							return;

						auto newInfo = MediaInfo::FromAudioInfo (result);

						auto info = index.data (FilesModel::Roles::MediaInfoRole).value<MediaInfo> ();
						UpgradeInfo (info, newInfo, &MediaInfo::Title_);
						UpgradeInfo (info, newInfo, &MediaInfo::Artist_);
						UpgradeInfo (info, newInfo, &MediaInfo::Album_);
						UpgradeInfo (info, newInfo, &MediaInfo::Year_);
						UpgradeInfo (info, newInfo, &MediaInfo::TrackNumber_);
						UpgradeInfo (info, newInfo, &MediaInfo::Genres_);
						FilesModel_->UpdateInfo (index, info);

						emit tagsFetched (path);

						if (FetchedTags_ == TotalTags_)
								emit finished ();
					};

		QTimer::singleShot (0, this, [this] { emit tagsFetchProgress (0, TotalTags_, this); });
	}
}
