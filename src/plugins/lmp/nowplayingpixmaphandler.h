/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <functional>
#include <QObject>

class QPixmap;
class QUrl;

namespace LeechCraft
{
namespace LMP
{
	struct MediaInfo;

	class NowPlayingPixmapHandler : public QObject
	{
		Q_OBJECT

		bool IsValidPixmap_;
		QString LastArtist_;
		QString LastCoverPath_;
	public:
		typedef std::function<void (QPixmap, QString)> PixmapSetter_f;
	private:
		QList<PixmapSetter_f> Setters_;
	public:
		NowPlayingPixmapHandler (QObject* = 0);

		void AddSetter (const PixmapSetter_f);

		void HandleSongChanged (const MediaInfo&, const QString&, const QPixmap&, bool);
	public slots:
		void handleGotArtistImage (const QString&, const QUrl&);
	private slots:
		void handleDownloadedImage ();
	};
}
}
