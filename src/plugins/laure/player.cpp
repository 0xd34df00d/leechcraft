/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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

#include "player.h"
#include <QDebug>
#include <QSlider>
#include <QTimer>
#include <QPushButton>
#include <QTime>
#include "separateplayerwidget.h"
#include "playlistview.h"

namespace LeechCraft
{
namespace Laure
{
	const int pos_slider_max = 10000;
		
	Player::Player (QWidget *parent, Qt::WindowFlags f)
	: QFrame (parent, f)
	, Poller_ (new QTimer (this))
	, PlayListView_ (NULL)
	{
		const char * const vlc_args[] = {
				"-I", "dummy",
				"--ignore-config",
				"--extraintf=logger",
				"--verbose=2"
		};

		VLCInstance_ = libvlc_instance_ptr (libvlc_new (sizeof (vlc_args)
				/ sizeof (vlc_args[0]), vlc_args), libvlc_release);
		MLP_ = libvlc_media_list_player_ptr (libvlc_media_list_player_new (VLCInstance_.get ()),
				libvlc_media_list_player_release);
		
		MP_ = libvlc_media_player_ptr (libvlc_media_player_new (VLCInstance_.get ()),
				libvlc_media_player_release);
		libvlc_media_list_player_set_media_player (MLP_.get (), MP_.get ());
		
		ML_ = libvlc_media_list_ptr (libvlc_media_list_new (VLCInstance_.get ()),
				libvlc_media_list_release);
		
		libvlc_media_list_player_set_media_list (MLP_.get (), ML_.get ());
		libvlc_media_player_set_xwindow (MP_.get (), winId ());
		
		connect (Poller_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (timeout ()));
		connect (Poller_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTimeout ()));
		Poller_->start (300);
	}
	
	libvlc_media_list_t* Player::PlayList ()
	{
		return ML_.get ();
	}
	
	libvlc_instance_t* Player::Instance ()
	{
		return VLCInstance_.get ();
	}
	
	libvlc_media_t* Player::Media ()
	{
		return libvlc_media_player_get_media (MP_.get ());
	}
	
	void Player::SetPlayListView (PlayListView *playListView)
	{
		PlayListView_ = playListView;
		playListView->SetInstance (VLCInstance_.get ());
		playListView->SetPlayList (ML_.get ());
	}
	
	bool Player::IsPlaying () const
	{
		return libvlc_media_list_player_is_playing (MLP_.get ());
	}

	int Player::Volume () const
	{
		return libvlc_audio_get_volume (MP_.get ());
	}
	
	int Player::Position () const
	{
		if (!IsPlaying ())
			return -1;
		
		float pos = libvlc_media_player_get_position (MP_.get ());
		return pos * static_cast<float> (pos_slider_max);
	}
	
	float Player::MediaPosition () const
	{
		if (!IsPlaying ())
			return -1;
		return libvlc_media_player_get_position (MP_.get ());
	}
	
	QTime Player::Time () const
	{
		int i;
		QTime time = QTime (0, 0);
		return (i = libvlc_media_player_get_time (MP_.get ())) < 0 ?
				time : time.addMSecs (i);
	}
	
	QTime Player::Length () const
	{
		int i;
		QTime time = QTime (0, 0);
		return (i = libvlc_media_player_get_length (MP_.get ())) < 0 ?
				time : time.addMSecs (i);
	}
	
	void Player::pause ()
	{
		libvlc_media_list_player_pause (MLP_.get ());
	}
	
	void Player::play ()
	{
		libvlc_media_list_player_play (MLP_.get ());
	}

	void Player::stop ()
	{
		libvlc_media_list_player_stop (MLP_.get ());
	}

	void Player::next ()
	{
		libvlc_media_list_player_next (MLP_.get ());
		PlayListView_->SetCurrentIndex (PlayListView_->CurrentIndex () + 1);
	}

	void Player::prev ()
	{
		libvlc_media_list_player_previous (MLP_.get ());
		PlayListView_->SetCurrentIndex (PlayListView_->CurrentIndex () - 1);
	}
	
	void Player::setVolume (int vol)
	{
		libvlc_audio_set_volume (MP_.get (), vol);
	}
	
	void Player::setPosition (int pos)
	{
		float poss = (float) pos / (float) pos_slider_max;
		libvlc_media_player_set_position (MP_.get (), poss);
	}
	
	void Player::playItem (int item)
	{
		libvlc_media_list_player_play_item_at_index (MLP_.get (), item);
		PlayListView_->SetCurrentIndex (item);
	}
	
	void Player::separateDialog ()
	{
		SeparatePlayerWidget *w = new SeparatePlayerWidget (MP_.get (), this);
		w->show ();
	}
	
	void Player::handleTimeout ()
	{
		int time = libvlc_media_player_get_time (MP_.get ());
		int length = libvlc_media_player_get_length (MP_.get ());
		if (length - time < 200 && IsPlaying ())
			next ();
	}
}
}

