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

#include "core.h"
#include <QString>
#include <QTime>
#include <QDebug>
#include <QTimer>

namespace LeechCraft
{
namespace Laure
{
	const char * const vlc_args[] = {
			"-I", "dummy",
			"--ignore-config",
			"--extraintf=logger",
			"--verbose=2"
	};
	
	Core::Core (QObject* parent)
	: QObject (parent)
	, CurrentItem_ (-1)
	{	
		Instance_ = libvlc_instance_ptr (libvlc_new (sizeof (vlc_args)
				/ sizeof (vlc_args[0]), vlc_args), libvlc_release);
		LPlayer_ = libvlc_media_list_player_ptr (libvlc_media_list_player_new (Instance_.get ()),
				libvlc_media_list_player_release);	
		Player_ = libvlc_media_player_ptr (libvlc_media_player_new (Instance_.get ()),
				libvlc_media_player_release);
		List_ = libvlc_media_list_ptr (libvlc_media_list_new (Instance_.get ()),
				libvlc_media_list_release);
		
		libvlc_media_list_player_set_media_player (LPlayer_.get (), Player_.get ());
		libvlc_media_list_player_set_media_list (LPlayer_.get (), List_.get ());
	}
	
	MediaMeta Core::ItemMeta (int row) const
	{
		MediaMeta meta;
		libvlc_media_t *m = libvlc_media_list_item_at_index (List_.get (), row);
		if (!m)
			return meta;
		
		libvlc_media_parse (m);
		meta.Artist_ = libvlc_media_get_meta (m, libvlc_meta_Artist);
		meta.Album_ = libvlc_media_get_meta (m, libvlc_meta_Album);
		meta.Title_ = libvlc_media_get_meta (m, libvlc_meta_Title);
		meta.Genre_ = libvlc_media_get_meta (m, libvlc_meta_Genre);
		meta.Date_ = libvlc_media_get_meta (m, libvlc_meta_Date);
		meta.TrackNumber_ = QString (libvlc_media_get_meta (m, libvlc_meta_TrackNumber)).toInt ();
		return meta;
	}
	
	int Core::RowCount () const
	{
		return libvlc_media_list_count (List_.get ());
	}
	
	int Core::CurrentItem () const
	{
		return CurrentItem_;
	}
	
	bool Core::IsPlaying () const
	{
		return libvlc_media_list_player_is_playing (LPlayer_.get ());
	}
	
	int Core::Volume () const
	{
		return libvlc_audio_get_volume (Player_.get ());
	}
	
	float Core::MediaPosition () const
	{
		return IsPlaying () ?
				libvlc_media_player_get_position (Player_.get ()) :
				-1;
	}
	
	int Core::Time () const
	{
		return libvlc_media_player_get_time (Player_.get ());
	}
	
	int Core::Length () const
	{
		return libvlc_media_player_get_length (Player_.get ());
	}
	
	void Core::setWindow (int winId)
	{
		libvlc_media_player_set_xwindow (Player_.get (), winId);
	}

	bool Core::removeRow (int pos)
	{
		return !libvlc_media_list_remove_index (List_.get (), pos);
	}
	
	void Core::addRow (const QString& item)
	{
		libvlc_media_ptr m (libvlc_media_new_path (Instance_.get (), item.toAscii ()),
				libvlc_media_release);
		
		libvlc_media_track_info_t *info = NULL;
		libvlc_media_get_tracks_info (m.get (), &info);
		
		if (!libvlc_media_list_add_media (List_.get (), m.get ()))
			emit itemAdded (ItemMeta (RowCount () - 1));
	}
	
	void Core::playItem (int val)
	{
		const int count = RowCount ();
		if (val == count)
			CurrentItem_ = 0;
		else if (val == -1)
			CurrentItem_ = count - 1;
		else
			CurrentItem_ = val;
		
		libvlc_media_list_player_play_item_at_index (LPlayer_.get (), CurrentItem_);
		emit (itemPlayed (CurrentItem_));
		QTimer::singleShot (5000, this, SLOT (nowPlaying ()));
		
	}
	
	void Core::nowPlaying ()
	{
		MediaMeta meta = ItemMeta (CurrentItem_);
		meta.Length_ = libvlc_media_player_get_length (Player_.get ()) / 1000;
		emit nowPlayed (meta);
	}
	
	void Core::stop ()
	{
		libvlc_media_player_stop (Player_.get ());
	}
	
	void Core::pause ()
	{
		libvlc_media_player_pause (Player_.get ());
	}
	
	void Core::play ()
	{
		libvlc_media_player_play (Player_.get ());
	}
	
	void Core::next ()
	{
		emit played ();
		playItem (CurrentItem_ + 1);
	}
	
	void Core::prev ()
	{
		emit played ();
		playItem (CurrentItem_ - 1);
	}
	
	void Core::setVolume (int vol)
	{
		libvlc_audio_set_volume (Player_.get (), vol);
	}
	
	void Core::setPosition (float pos)
	{
		libvlc_media_player_set_position (Player_.get (), pos);
	}
}
}
