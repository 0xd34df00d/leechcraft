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

#pragma once

#include <QObject>
#include <QPair>
#include <QHash>
#include <QStringList>

namespace Media
{
	struct AudioSearchRequest;
	class IAudioPile;
	class IPendingAudioSearch;
}

namespace LeechCraft
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

		struct PendingTrackInfo
		{
			QString Artist_;
			QString Album_;
			QString Track_;
		};
		QHash<Media::IPendingAudioSearch*, PendingTrackInfo> Pending2Track_;
	public:
		PreviewHandler (Player*, QObject*);
	public slots:
		void previewArtist (const QString& artist);
		void previewTrack (const QString& track, const QString& artist);
		void previewTrack (const QString& track, const QString& artist, int length);
		void previewAlbum (const QString& artist, const QString& album,
				const QList<QPair<QString, int>>& tracks);
	private:
		QList<Media::IPendingAudioSearch*> RequestPreview (const Media::AudioSearchRequest&);
		void CheckPendingAlbum (Media::IPendingAudioSearch*);
	private slots:
		void handlePendingReady ();
	};
}
}
