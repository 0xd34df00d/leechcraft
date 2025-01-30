/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QMap>
#include <util/threads/coro/taskfwd.h>
#include <interfaces/media/audiostructs.h>

class QStandardItem;

namespace LC::LMP::MP3Tunes
{
	class AuthManager;
	class AccountsManager;

	struct Playlist;

	class PlaylistManager : public QObject
	{
		Q_OBJECT

		AuthManager *AuthMgr_;
		AccountsManager *AccMgr_;
		QStandardItem *Root_;

		QMap<QString, QMap<QString, QStandardItem*>> AccPlaylists_;

		QHash<QUrl, Media::AudioInfo> Infos_;
	public:
		PlaylistManager (AuthManager*, AccountsManager*, QObject* = nullptr);

		QStandardItem* GetRoot () const;
		Util::ContextTask<> Update ();

		std::optional<Media::AudioInfo> GetMediaInfo (const QUrl&) const;
	private:
		void HandlePlaylists (QStandardItem&, const QList<Playlist>&);
	};
}
