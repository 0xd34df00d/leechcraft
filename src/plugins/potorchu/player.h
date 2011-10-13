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

#ifndef PLAYER_H
#define PLAYER_H

#include <QSlider>
#include <QTimer>
#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <vlc/vlc.h>
#include <vlc/libvlc_media_list.h>
#include <vlc/libvlc_media_list_player.h>

#include "playlistview.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		class Player : public QFrame
		{
			Q_OBJECT
			QTimer *Poller_;
			bool IsPlaying_;
			libvlc_instance_t *VLCInstance_;
			libvlc_media_list_player_t *MLP_;
			libvlc_media_list_t *ML_;
			libvlc_media_player_t *MP_;
			PlayListView *PlayListView_;
		public:
			Player (QWidget *parent = 0, Qt::WindowFlags f = 0);
			virtual ~Player ();
			
			libvlc_instance_t *Instance ();
			libvlc_media_list_t *PlayList ();
			void SetPlayListView (PlayListView *playListView);
			
			QString GetMeta (libvlc_meta_t meta) const;
			int Volume () const;
			int Position () const;
			float MediaPosition () const;
			bool IsPlaying () const;
		public slots:
			void setVolume (int vol);
			void setPosition (int pos);
			void stop ();
			void pause ();
			void play ();
			void playItem (int item);
			void next ();
			void prev ();
		signals:
			void timeout ();
		};
	}
}
#endif // PLAYER_H
