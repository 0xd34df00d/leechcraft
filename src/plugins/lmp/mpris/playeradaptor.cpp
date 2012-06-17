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

#include "playeradaptor.h"
#include <QMetaObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include "../player.h"
#include "fdopropsadaptor.h"

namespace LeechCraft
{
namespace LMP
{
namespace MPRIS
{
	PlayerAdaptor::PlayerAdaptor (FDOPropsAdaptor *fdo, Player *player)
	: QDBusAbstractAdaptor (player)
	, Props_ (fdo)
	, Player_ (player)
	{
		setAutoRelaySignals (true);

		connect (Player_,
				SIGNAL (songChanged (MediaInfo)),
				this,
				SLOT (handleSongChanged ()));
		connect (Player_,
				SIGNAL (playModeChanged (Player::PlayMode)),
				this,
				SLOT (handlePlayModeChanged ()));
		connect (Player_->GetSourceObject (),
				SIGNAL (stateChanged (Phonon::State, Phonon::State)),
				this,
				SLOT (handleStateChanged ()));
		connect (Player_->GetAudioOutput (),
				SIGNAL (volumeChanged (qreal)),
				this,
				SLOT (handleVolumeChanged ()));
	}

	PlayerAdaptor::~PlayerAdaptor ()
	{
	}

	bool PlayerAdaptor::GetCanControl () const
	{
		return true;
	}

	bool PlayerAdaptor::GetCanGoNext () const
	{
		return true;
	}

	bool PlayerAdaptor::GetCanGoPrevious () const
	{
		return true;
	}

	bool PlayerAdaptor::GetCanPause () const
	{
		return true;
	}

	bool PlayerAdaptor::GetCanPlay () const
	{
		return true;
	}

	bool PlayerAdaptor::GetCanSeek () const
	{
		return Player_->GetSourceObject ()->isSeekable ();
	}

	QString PlayerAdaptor::GetLoopStatus () const
	{
		switch (Player_->GetPlayMode ())
		{
		case Player::PlayMode::RepeatTrack:
			return "Track";
		case Player::PlayMode::RepeatAlbum:
		case Player::PlayMode::RepeatWhole:
			return "Playlist";
		case Player::PlayMode::Sequential:
		case Player::PlayMode::Shuffle:
			return "None";
		}
		return "None";
	}

	void PlayerAdaptor::SetLoopStatus (const QString& value)
	{
		if (value == "Track")
			Player_->SetPlayMode (Player::PlayMode::RepeatTrack);
		else if (value == "Playlist")
			Player_->SetPlayMode (Player::PlayMode::RepeatWhole);
		else
			Player_->SetPlayMode (Player::PlayMode::Sequential);
	}

	double PlayerAdaptor::GetMaximumRate () const
	{
		return 1;
	}

	QVariantMap PlayerAdaptor::GetMetadata () const
	{
		auto info = Player_->GetCurrentMediaInfo ();
		if (info.LocalPath_.isEmpty () && info.Title_.isEmpty ())
			return QVariantMap ();

		QVariantMap result;
		result ["mpris:trackid"] = info.LocalPath_.isEmpty () ?
					QString ("/nonlocal/%1_/%2_/%3_/%4_")
						.arg (info.Artist_)
						.arg (info.Album_)
						.arg (info.TrackNumber_)
						.arg (info.Title_) :
					QString ("/local/%1")
						.arg (info.LocalPath_);
		result ["mpris:length"] = info.Length_ * 1000;
		result ["mpris:artUrl"] = QUrl (Player_->GetCurrentAAPath ()).toLocalFile ();
		result ["xesam:album"] = info.Album_;
		result ["xesam:artist"] = info.Artist_;
		result ["xesam:genre"] = info.Genres_.join (" / ");
		result ["xesam:title"] = info.Title_;
		result ["xesam:trackNumber"] = info.TrackNumber_;
		return result;
	}

	double PlayerAdaptor::GetMinimumRate () const
	{
		return 1;
	}

	QString PlayerAdaptor::GetPlaybackStatus () const
	{
		switch (Player_->GetSourceObject ()->state ())
		{
		case Phonon::PausedState:
			return "Paused";
		case Phonon::StoppedState:
		case Phonon::ErrorState:
			return "Stopped";
		default:
			return "Playing";
		}
	}

	qlonglong PlayerAdaptor::GetPosition () const
	{
		return Player_->GetSourceObject ()->currentTime () * 1000;
	}

	double PlayerAdaptor::GetRate () const
	{
		return 1.0;
	}

	void PlayerAdaptor::SetRate (double)
	{
	}

	bool PlayerAdaptor::GetShuffle () const
	{
		return Player_->GetPlayMode () == Player::PlayMode::Shuffle;
	}

	void PlayerAdaptor::SetShuffle (bool value)
	{
		Player_->SetPlayMode (value ?
				Player::PlayMode::Shuffle :
				Player::PlayMode::Sequential);
	}

	double PlayerAdaptor::GetVolume () const
	{
		return Player_->GetAudioOutput ()->volume ();
	}

	void PlayerAdaptor::SetVolume (double value)
	{
		Player_->GetAudioOutput ()->setVolume (value);
	}

	void PlayerAdaptor::Notify (const QString& propName)
	{
		Props_->Notify ("org.mpris.MediaPlayer2.Player",
				propName, property (propName.toUtf8 ()));
	}

	void PlayerAdaptor::Next ()
	{
		Player_->nextTrack ();
	}

	void PlayerAdaptor::OpenUri (const QString& uri)
	{
		const auto& url = QUrl (uri);
		if (url.scheme () == "file")
			Player_->Enqueue (QStringList (uri));
		else
			Player_->Enqueue ({ url });
	}

	void PlayerAdaptor::Pause ()
	{
		Player_->setPause ();
	}

	void PlayerAdaptor::Play ()
	{
		if (GetPlaybackStatus () == "Playing")
			return;

		Player_->togglePause ();
	}

	void PlayerAdaptor::PlayPause ()
	{
		Player_->togglePause ();
	}

	void PlayerAdaptor::Previous ()
	{
		Player_->previousTrack ();
	}

	void PlayerAdaptor::Seek (qlonglong offset)
	{
		Player_->GetSourceObject ()->seek (offset / 1000);
	}

	void PlayerAdaptor::SetPosition (const QDBusObjectPath& TrackId, qlonglong Position)
	{
	}

	void PlayerAdaptor::Stop ()
	{
		Player_->stop ();
	}

	void PlayerAdaptor::handleSongChanged ()
	{
		Notify ("Metadata");
	}

	void PlayerAdaptor::handlePlayModeChanged ()
	{
		Notify ("LoopStatus");
		Notify ("Shuffle");
	}

	void PlayerAdaptor::handleStateChanged ()
	{
		Notify ("PlaybackStatus");
	}

	void PlayerAdaptor::handleVolumeChanged ()
	{
		Notify ("Volume");
	}
}
}
}
