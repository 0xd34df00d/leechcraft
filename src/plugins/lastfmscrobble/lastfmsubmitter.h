/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LASTFMSCROBBLE_LASTFMSUBMITTER_H
#define PLUGINS_LASTFMSCROBBLE_LASTFMSUBMITTER_H
#include <memory>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>

namespace lastfm
{
	class Audioscrobbler;
}

class QNetworkAccessManager;

namespace LeechCraft
{
namespace Lastfmscrobble
{
	struct MediaMeta
	{
		explicit MediaMeta (const QMap<QString, QVariant>& tagMap);
		QString Artist_, Album_, Title_, Genre_, Date_;
		int TrackNumber_;
		int Length_;
	};
	
	class LastFMSubmitter : public QObject
	{
		Q_OBJECT
		
		std::shared_ptr<lastfm::Audioscrobbler> Scrobbler_;
		QString Password_;
	public:
		LastFMSubmitter (QObject *parent = 0);
		
		void Init (QNetworkAccessManager *manager);
		void SetUsername (const QString& username);
		void SetPassword (const QString& password);
		bool IsConnected () const;
	public slots:
		void sendTrack (const MediaMeta& info);
		void submit ();
	private slots:
		void getSessionKey ();
	signals:
		void status (int code);
	};
}
}

#endif // PLUGINS_LASTFMSCROBBLE_LASTFMSUBMITTER_H
