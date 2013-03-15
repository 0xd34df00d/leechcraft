/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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
#include <QUrl>
#include <interfaces/media/iradiostation.h>

class QNetworkAccessManager;

namespace LeechCraft
{
namespace HotStreams
{
	class RadioStation : public QObject
					   , public Media::IRadioStation
	{
		Q_OBJECT
		Q_INTERFACES (Media::IRadioStation)

		QUrl StreamUrl_;
		QString Name_;
		QString PlaylistFormat_;
	public:
		RadioStation (const QUrl&, const QString&, QNetworkAccessManager*, const QString&);

		QObject* GetQObject ();
		QString GetRadioName () const;
		void RequestNewStream ();
	private slots:
		void handlePlaylistFetched ();
	signals:
		void gotNewStream (const QUrl&, const Media::AudioInfo&);
		void gotPlaylist (const QString&, const QString&);
		void gotError (const QString&);
	};
}
}
