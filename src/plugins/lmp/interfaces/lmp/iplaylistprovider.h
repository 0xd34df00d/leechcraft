/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QtPlugin>
#include <QUrl>
#include <interfaces/media/audiostructs.h>

class QStandardItem;

namespace LC
{
namespace LMP
{
	class IPlaylistProvider
	{
	public:
		enum ItemRoles
		{
			SourceURLs = Qt::UserRole + 1,
			Max
		};

		virtual ~IPlaylistProvider () {}

		virtual QStandardItem* GetPlaylistsRoot () const = 0;

		virtual void UpdatePlaylists () = 0;

		virtual std::optional<Media::AudioInfo> GetURLInfo (const QUrl&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::IPlaylistProvider, "org.LeechCraft.LMP.IPlaylistProvider/1.0")
