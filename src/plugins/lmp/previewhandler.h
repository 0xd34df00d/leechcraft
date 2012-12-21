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

namespace Media
{
	class IAudioPile;
	struct AudioSearchRequest;
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
	public:
		PreviewHandler (Player*, QObject*);
	public slots:
		void previewArtist (const QString&);
		void previewTrack (const QString&, const QString&);
		void previewTrack (const QString&, const QString&, int);
	private:
		void RequestPreview (const Media::AudioSearchRequest&);
	private slots:
		void handlePendingReady ();
	};
}
}
