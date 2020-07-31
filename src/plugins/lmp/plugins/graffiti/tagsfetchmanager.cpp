/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsfetchmanager.h"
#include <QFuture>
#include <util/threads/futures.h>
#include <util/sll/delayedexecutor.h>
#include <interfaces/lmp/mediainfo.h>
#include <interfaces/media/itagsfetcher.h>
#include "filesmodel.h"

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	TagsFetchManager::TagsFetchManager (const QStringList& paths,
			Media::ITagsFetcher *prov, FilesModel *filesModel, QObject *parent)
	: QObject (parent)
	, FilesModel_ (filesModel)
	, FetchedTags_ (0)
	, TotalTags_ (paths.size ())
	{
		for (const auto& path : paths)
			Util::Sequence (this, prov->FetchTags (path)) >>
					[this, path] (const Media::AudioInfo& result) { handleTagsFetched (path, result); };

		Util::ExecuteLater ([this] { emit tagsFetchProgress (0, TotalTags_, this); });
	}

	namespace
	{
		template<typename T>
		bool IsEmptyData (const T& val)
		{
			if constexpr (std::is_default_constructible<T> {})
				return val == T {};
			else
				static_assert (std::is_same<T, struct Error> {}, "not a default-constructible data type");
		}

		template<typename F>
		void UpgradeInfo (MediaInfo& info, MediaInfo& other, F getter)
		{
			static_assert (std::is_lvalue_reference<std::result_of_t<F (MediaInfo&)>> {},
					"functor doesn't return an lvalue reference");

			auto& data = std::invoke (getter, info);
			const auto& otherData = std::invoke (getter, other);
			if (!IsEmptyData (otherData) && IsEmptyData (data))
				data = otherData;
		}
	}

	void TagsFetchManager::handleTagsFetched (const QString& filename, const Media::AudioInfo& result)
	{
		emit tagsFetchProgress (++FetchedTags_, TotalTags_, this);

		const auto& index = FilesModel_->FindIndex (filename);
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

		emit tagsFetched (filename);

		if (FetchedTags_ == TotalTags_)
			emit finished (true);
	}
}
}
}
