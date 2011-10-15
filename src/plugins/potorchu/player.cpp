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
#include <QString>
#include <QDebug>

#include "separateplayerwidget.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		const int pos_slider_max = 10000;
			
		Player::Player (QWidget *parent, Qt::WindowFlags f)
		: QFrame (parent, f)
		, PlayListView_ (NULL)
		{
			const char * const vlc_args[] = {
					"-I", "dummy",
					"--ignore-config",
					"--extraintf=logger",
					"--verbose=2"
			};
			Poller_ = new QTimer (this);
			VLCInstance_ = libvlc_new (sizeof (vlc_args) / sizeof (vlc_args[0]), vlc_args);
			MLP_ = libvlc_media_list_player_new (VLCInstance_);
			MP_ = libvlc_media_player_new (VLCInstance_);
			libvlc_media_list_player_set_media_player (MLP_, MP_);
			
			ML_ = libvlc_media_list_new (VLCInstance_);
			libvlc_media_list_player_set_media_list (MLP_, ML_);
			libvlc_media_player_set_xwindow (MP_, winId ());
			
			connect (Poller_,
					SIGNAL (timeout ()),
					this,
					SIGNAL (timeout ()));
			Poller_->start (100);
		}
		
		libvlc_media_list_t *Player::PlayList ()
		{
			return ML_;
		}
		
		libvlc_instance_t *Player::Instance ()
		{
			return VLCInstance_;
		}
		
		void Player::SetPlayListView (PlayListView *playListView)
		{
			PlayListView_ = playListView;
			playListView->SetInstance (VLCInstance_);
			playListView->SetPlayList (ML_);
		}
		
		Player::~Player ()
		{
			libvlc_media_list_player_stop (MLP_);
			libvlc_media_list_player_release (MLP_);
			libvlc_media_player_release (MP_);
			libvlc_media_list_release (ML_);
			libvlc_release (VLCInstance_);
		}
		
		bool Player::IsPlaying () const
		{
			return libvlc_media_list_player_is_playing (MLP_);
		}

		int Player::Volume () const
		{
			return libvlc_audio_get_volume (MP_);
		}
		
		int Player::Position () const
		{
			if (!IsPlaying ())
				return -1;
			float pos = libvlc_media_player_get_position (MP_);
			return (int)(pos * (float)(pos_slider_max));
		}
		
		float Player::MediaPosition () const
		{
			if (!IsPlaying ())
				return -1;
			return libvlc_media_player_get_position (MP_);
		}
		
		QTime Player::Time () const
		{
			return QTime (0, 0, 0).addMSecs (libvlc_media_player_get_time (MP_));
		}
		
		QTime Player::Length () const
		{
			return QTime (0, 0, 0).addMSecs (libvlc_media_player_get_length (MP_));
		}
		
		void Player::pause ()
		{
			libvlc_media_list_player_pause (MLP_);
		}
		
		void Player::play ()
		{
			libvlc_media_list_player_play (MLP_);
		}

		void Player::stop ()
		{
			libvlc_media_list_player_stop (MLP_);
		}

		void Player::next ()
		{
			libvlc_media_list_player_next (MLP_);
			PlayListView_->SetCurrentIndex (PlayListView_->CurrentIndex () + 1);
		}

		void Player::prev ()
		{
			libvlc_media_list_player_previous (MLP_);
			PlayListView_->SetCurrentIndex (PlayListView_->CurrentIndex () - 1);
		}
		
		void Player::setVolume (int vol)
		{
			libvlc_audio_set_volume (MP_, vol);
		}
		
		void Player::setPosition (int pos)
		{
			libvlc_media_t *m = libvlc_media_list_media (ML_);
			if (m == NULL)
				return;
			float poss = (float) pos / (float) pos_slider_max;
			libvlc_media_player_set_position (MP_, poss);
		}
		
		void Player::playItem (int item)
		{
			libvlc_media_list_player_play_item_at_index (MLP_, item);
			PlayListView_->SetCurrentIndex (item);
		}
		
		void Player::separateDialog ()
		{
			SeparatePlayerWidget *w = new SeparatePlayerWidget (MP_, this);
			w->show ();
		}

	}
}

