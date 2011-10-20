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

#include "laurecore.h"
#include <QString>
#include <QTime>

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
	
	MediaInfo::MediaInfo (const MediaInfo& info)
	: meta (info.meta)
	, state (info.state)
	, length (info.length)
	{
	}

	MediaMeta::MediaMeta (const MediaMeta& meta)
	: artist (meta.artist)
	, album (meta.album)
	, title (meta.title)
	, genre (meta.genre)
	, date (meta.date)
	, duration (meta.duration)
	{
	}

	AddRowState::AddRowState (const AddRowState& state)
	: meta (state.meta)
	, state (state.state)
	{
	}
	
	LaureCore::LaureCore (QObject* parent)
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
	
	int LaureCore::rowCount () const
	{
		return libvlc_media_list_count (List_.get ());
	}
	
	int LaureCore::CurrentItem () const
	{
		return CurrentItem_;
	}
	
	bool LaureCore::IsPlaying () const
	{
		return libvlc_media_list_player_is_playing (LPlayer_.get ());
	}
	
	int LaureCore::Volume () const
	{
		return libvlc_audio_get_volume (Player_.get ());
	}
	
	float LaureCore::MediaPosition () const
	{
		return IsPlaying ()
				? libvlc_media_player_get_position (Player_.get ())
				: -1;
	}
	
	MediaInfo LaureCore::MediaItemInfo ()
	{
		MediaInfo info;
		if (IsPlaying ())
		{
			info.state = true;
			info.meta = GetMeta (libvlc_media_player_get_media (Player_.get ()));
			info.length = libvlc_media_player_get_length (Player_.get ());
		}
		else
			info.state = false;
		return info;
	}
	
	QTime LaureCore::Time ()
	{
		return IntToQTime (libvlc_media_player_get_time (Player_.get ()));
	}
	
	QTime LaureCore::Length ()
	{
		return IntToQTime (libvlc_media_player_get_length (Player_.get ()));
	}
	
	void LaureCore::setWindow (int winId)
	{
		libvlc_media_player_set_xwindow (Player_.get (), winId);
	}

	bool LaureCore::removeRow (int pos)
	{
		return !libvlc_media_list_remove_index (List_.get (), pos);
	}
	
	AddRowState LaureCore::addRow (const QString& item)
	{
		AddRowState st;
		libvlc_media_t *m = libvlc_media_new_path (Instance_.get (), item.toAscii ());
		if (!libvlc_media_list_add_media (List_.get (), m))
		{
			st.state = true;
			libvlc_media_parse (m);
			st.meta = GetMeta (m);
		}
		else
			st.state = false;
		return st;
	}
	
	void LaureCore::playItem (int val)
	{
		const int count = rowCount ();
		if (val == count)
			CurrentItem_ = 0;
		else if (val == -1)
			CurrentItem_ = count - 1;
		else
			CurrentItem_ = val;
		libvlc_media_list_player_play_item_at_index (LPlayer_.get (), val);
	}
	
	void LaureCore::stop ()
	{
		libvlc_media_list_player_stop (LPlayer_.get ());
	}
	
	void LaureCore::pause ()
	{
		libvlc_media_list_player_pause (LPlayer_.get ());
	}
	
	void LaureCore::play ()
	{
		libvlc_media_list_player_play (LPlayer_.get ());
	}
	
	void LaureCore::next ()
	{
		libvlc_media_list_player_next (LPlayer_.get ());
	}
	
	void LaureCore::prev ()
	{
		libvlc_media_list_player_previous (LPlayer_.get ());
	}
	
	void LaureCore::setVolume (int vol)
	{
		libvlc_audio_set_volume (Player_.get (), vol);
	}
	
	void LaureCore::setPosition (float pos)
	{
		libvlc_media_player_set_position (Player_.get (), pos);
	}
	
	MediaMeta LaureCore::GetMeta (libvlc_media_t *m)
	{
		MediaMeta meta;
		meta.artist = libvlc_media_get_meta (m, libvlc_meta_Artist);
		meta.album = libvlc_media_get_meta (m, libvlc_meta_Album);
		meta.title = libvlc_media_get_meta (m, libvlc_meta_Title);
		meta.genre = libvlc_media_get_meta (m, libvlc_meta_Genre);
		meta.date = libvlc_media_get_meta (m, libvlc_meta_Date);
		return meta;
	}
	
	QTime LaureCore::IntToQTime (int val)
	{
		QTime time = QTime (0, 0);
		return val < 0 ? time : time.addMSecs (val);
	}
}
}
