/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playeradaptor.h"
#include <QMetaObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <util/sll/unreachable.h>
#include "../player.h"
#include "../engine/sourceobject.h"
#include "../engine/output.h"
#include "fdopropsadaptor.h"

namespace LC
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
				&Player::songChanged,
				[this] { Notify ("Metadata"); });
		connect (Player_,
				&Player::playModeChanged,
				[this]
				{
					Notify ("LoopStatus");
					Notify ("Shuffle");
				});
		connect (Player_->GetSourceObject (),
				&SourceObject::stateChanged,
				[this] { Notify ("PlaybackStatus"); });
		connect (Player_->GetAudioOutput (),
				&Output::volumeChanged,
				[this] { Notify ("Volume"); });
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
		return Player_->GetSourceObject ()->IsSeekable ();
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
		case Player::PlayMode::ShuffleAlbums:
		case Player::PlayMode::ShuffleArtists:
			return "None";
		}

		Util::Unreachable ();
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
		result ["mpris:length"] = info.Length_ * 1000000;
		result ["mpris:artUrl"] = QUrl (Player_->GetCurrentAAPath ()).toLocalFile ();
		result ["xesam:album"] = info.Album_;
		result ["xesam:artist"] = QStringList { info.Artist_ };
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
		switch (Player_->GetSourceObject ()->GetState ())
		{
		case SourceState::Paused:
			return "Paused";
		case SourceState::Stopped:
		case SourceState::Error:
			return "Stopped";
		default:
			return "Playing";
		}
	}

	qlonglong PlayerAdaptor::GetPosition () const
	{
		return Player_->GetSourceObject ()->GetCurrentTime () * 1000;
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
		return Player_->GetAudioOutput ()->GetVolume ();
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
		Player_->GetSourceObject ()->Seek (offset / 1000);
	}

	void PlayerAdaptor::SetPosition (const QDBusObjectPath&, qlonglong)
	{
	}

	void PlayerAdaptor::Stop ()
	{
		Player_->stop ();
	}
}
}
}
