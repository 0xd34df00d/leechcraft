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
#include <interfaces/media/audiostructs.h>

class QStandardItem;
class QNetworkAccessManager;

namespace LC
{
namespace LMP
{
namespace MP3Tunes
{
	class AuthManager;
	class AccountsManager;

	class PlaylistManager : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;

		AuthManager *AuthMgr_;
		AccountsManager *AccMgr_;
		QStandardItem *Root_;

		QMap<QString, QStandardItem*> AccItems_;
		QMap<QString, QMap<QString, QStandardItem*>> AccPlaylists_;

		QHash<QUrl, Media::AudioInfo> Infos_;
	public:
		PlaylistManager (QNetworkAccessManager*, AuthManager*, AccountsManager*, QObject* = 0);

		QStandardItem* GetRoot () const;
		void Update ();

		std::optional<Media::AudioInfo> GetMediaInfo (const QUrl&) const;
	private slots:
		void requestPlaylists (const QString&);
		void handleGotPlaylists ();
		void handleGotPlaylistContents ();
		void handleAccountsChanged ();
	};
}
}
}
