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


namespace LeechCraft
{
	namespace Potorchu
	{
		const int pos_slider_max = 10000;
		Player::Player (QWidget *parent, Qt::WindowFlags f)
		: QFrame (parent, f)
		, IsPlaying_ (false)
		{
			const char * const vlc_args[] = {
					"-I", "dummy",
					"--ignore-config",
					"--extraintf=logger",
					"--verbose=2"};
			Poller_ = new QTimer (this);
			
			VLCInstance_ = libvlc_new (sizeof (vlc_args) / sizeof (vlc_args[0]), vlc_args);
			MP_ = libvlc_media_player_new (VLCInstance_);
			connect (Poller_,
					SIGNAL (timeout ()),
					this,
					SIGNAL (timeout ()));

			Poller_->start (100);
		}
		
		Player::~Player ()
		{
			libvlc_media_player_stop (MP_);
			libvlc_media_player_release (MP_);
			libvlc_release (VLCInstance_);
		}
		
		void Player::playFile (const QString& file)
		{
			M_ = libvlc_media_new_path (VLCInstance_, file.toAscii ());
			libvlc_media_player_set_media (MP_, M_);
			
			libvlc_media_player_set_xwindow (MP_, winId ());
			
			libvlc_media_player_play (MP_);
			IsPlaying_ = true;
		}
		
		bool Player::IsPlayed () const
		{
			if(!IsPlaying_)
				return false;
			libvlc_media_t *curMedia = libvlc_media_player_get_media (MP_);
			return (curMedia != NULL);
		}

		
		int Player::GetVolume () const
		{
			if (!IsPlayed ())
				return -1;
			int volume = libvlc_audio_get_volume (MP_);
			return volume;
		}
		
		int Player::GetPosition() const
		{
			if (!IsPlayed ())
				return -1;
			float pos = libvlc_media_player_get_position (MP_);
			int siderPos = (int)(pos * (float)(pos_slider_max));
			return siderPos;
		}
		
		void Player::pause ()
		{
			libvlc_media_player_pause (MP_);
		}
		
		void Player::play ()
		{
			libvlc_media_player_play (MP_);
		}

		
		void Player::stop ()
		{
			libvlc_media_player_stop (MP_);
		}

		
		void Player::changeVolume (int newVolume)
		{
			libvlc_audio_set_volume (MP_, newVolume);
		}
		
		void Player::changePosition (int newPosition)
		{
			libvlc_media_t *curMedia = libvlc_media_player_get_media (MP_);
			if (curMedia == NULL)
				return;
			
			float pos = (float)(newPosition) / (float) pos_slider_max;
			libvlc_media_player_set_position (MP_, pos);
		}
	}
}

