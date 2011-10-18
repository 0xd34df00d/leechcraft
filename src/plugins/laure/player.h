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

#ifndef PLUGINS_LAURE_PLAYER_H
#define PLUGINS_LAURE_PLAYER_H

#include <QFrame>
#include <boost/shared_ptr.hpp>
#include <vlc/vlc.h>
#include <vlc/libvlc_media_list.h>
#include <vlc/libvlc_media_list_player.h>

class QTime;
class QPushButton;
class QTimer;
class QSlider;

namespace LeechCraft
{
namespace Laure
{
	class PlayListView;
	typedef boost::shared_ptr<libvlc_instance_t> libvlc_instance_ptr;
	typedef boost::shared_ptr<libvlc_media_list_t> libvlc_media_list_ptr;
	typedef boost::shared_ptr<libvlc_media_list_player_t> libvlc_media_list_player_ptr;
	typedef boost::shared_ptr<libvlc_media_player_t> libvlc_media_player_ptr;
	
	class Player : public QFrame
	{
		Q_OBJECT
		
		QTimer *Poller_;
		bool IsPlaying_;
		libvlc_instance_ptr VLCInstance_;
		libvlc_media_list_ptr ML_;
		libvlc_media_list_player_ptr MLP_;
		libvlc_media_player_ptr MP_;
		PlayListView *PlayListView_;
	public:
		Player (QWidget *parent = 0, Qt::WindowFlags f = 0);
		
		libvlc_instance_t* Instance ();
		libvlc_media_list_t* PlayList ();
		libvlc_media_t* Media ();
		void SetPlayListView (PlayListView*);
		
		int Volume () const;
		int Position () const;
		float MediaPosition () const;
		bool IsPlaying () const;
		QTime Time () const;
		QTime Length () const;
	public slots:
		void setVolume (int);
		void setPosition (int);
		void stop ();
		void pause ();
		void play ();
		void playItem (int);
		void next ();
		void prev ();
		void separateDialog ();
		void handleTimeout ();
	signals:
		void timeout ();
	};
}
}
#endif // PLUGINS_LAURE_PLAYER_H
