/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPair>
#include <QHash>
#include <QStringList>
#include <interfaces/media/iaudiopile.h>

namespace LC
{
namespace LMP
{
	class Player;

	class PreviewHandler : public QObject
	{
		Q_OBJECT

		Player *Player_;

		QList<Media::IAudioPile*> Providers_;

		QHash<QString, QHash<QString, QHash<QString, int>>> Artist2Album2Tracks_;

		struct PendingTrackInfo;

		using FuturesList_t = QList<QFuture<Media::IAudioPile::Result_t>>;
	public:
		PreviewHandler (Player*, QObject* = nullptr);

		void InitWithPlugins ();
	public slots:
		void previewArtist (const QString& artist);
		void previewTrack (const QString& track, const QString& artist);
		void previewTrack (const QString& track, const QString& artist, int length);
		void previewAlbum (const QString& artist, const QString& album,
				const QList<QPair<QString, int>>& tracks);
	private:
		FuturesList_t RequestPreview (const Media::AudioSearchRequest&);
		void CheckPendingAlbum (const PendingTrackInfo&, bool);
		void HandlePendingReady (const Media::IAudioPile::Results_t&);
	};
}
}
