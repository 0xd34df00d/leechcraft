/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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

#include "vlcwrapper.h"
#include <memory>
#include <QAction>
#include <QString>
#include <QTime>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QUrl>
#include <util/util.h>

namespace LeechCraft
{
namespace Laure
{
	const char * const vlc_args[] = {
			"-I", "dummy"         // Don't use any interface
			, "--ignore-config"      // Don't use VLC's config
			, "--verbose=-1"
			, "--quiet"
	};

	namespace
	{
		void ListEventCallback (const libvlc_event_t *event, void *data)
		{
			VLCWrapper *wrapper = static_cast<VLCWrapper*> (data);
			switch (event->type)
			{
			case libvlc_MediaListPlayerNextItemSet:
				wrapper->HandleNextItemSet ();
				break;
			case libvlc_MediaPlayerPlaying:
				wrapper->HandlePlayed ();
				break;
			}
		}
	}

	QVariantMap MediaMeta::ToVariantMap () const
	{
		QVariantMap map;
		map ["Artist"] = Artist_;
		map ["Album"] = Album_;
		map ["Title"] = Title_;
		map ["Genre"] = Genre_;
		map ["Date"] = Date_;
		map ["TrackNumber"] = TrackNumber_;
		map ["Length"] = Length_;
		map ["Type"] = Type_ == libvlc_track_audio ? "Audio" : "Video";
		map ["Location"] = Location_;
		return map;
	}
	
	VLCWrapper::VLCWrapper (QObject *parent)
	: QObject (parent)
	, CurrentItem_ (-1)
	, IsPlayedFromQueue_ (false)
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

		auto listEventManager = libvlc_media_list_player_event_manager (LPlayer_.get ());
		libvlc_event_attach (listEventManager, libvlc_MediaListPlayerNextItemSet,
				ListEventCallback, this);
		
		auto playerEventManager = libvlc_media_player_event_manager (Player_.get ());
		libvlc_event_attach (playerEventManager, libvlc_MediaPlayerPlaying,
				ListEventCallback, this);
	}

	int VLCWrapper::PlayQueue ()
	{
		if (IsPlayedFromQueue_)
			QueueListIndex_.pop_front ();

		IsPlayedFromQueue_ = !QueueListIndex_.empty ();
		if (IsPlayedFromQueue_)
		{
			int index = QueueListIndex_.first ();
			libvlc_media_list_player_play_item_at_index (LPlayer_.get (), index);
			CurrentItemMeta_ = GetItemMeta (index);
			return index;
		}
		
		return -1;
	}
	
	void VLCWrapper::HandleNextItemSet ()
	{
		int index = PlayQueue ();
		if (index < 0)
		{
			auto list = List_.get ();
			index = libvlc_media_list_index_of_item (list,
					libvlc_media_player_get_media (Player_.get ()));
			CurrentItemMeta_ = GetItemMeta (index);
		}
			
		emit gotEntity (Util::MakeNotification ("Laure",
				tr ("%1 - %2").arg (CurrentItemMeta_.Artist_).arg (CurrentItemMeta_.Title_),
					PInfo_));

		CurrentItem_ = index;
		emit (itemPlayed (CurrentItem_));
	}

	void VLCWrapper::HandlePlayed ()
	{
		Entity scrobbleEntity;
		scrobbleEntity.Additional_ = CurrentItemMeta_.ToVariantMap ();
		scrobbleEntity.Mime_ = "x-leechcraft/now-playing-track-info";

		emit gotEntity (scrobbleEntity);
		emit currentItemMeta (CurrentItemMeta_);
	}
	
	void VLCWrapper::addToQueue (int index)
	{
		QueueListIndex_ << index;
	}
		
	void VLCWrapper::removeFromQueue (int index)
	{
		QueueListIndex_.removeOne (index);
	}

	MediaMeta VLCWrapper::GetItemMeta (int row, const QString& location) const
	{
		MediaMeta meta;
		auto m = libvlc_media_list_item_at_index (List_.get (), row);
		if (!m)
			return meta;

		if (!QUrl (location).scheme ().isEmpty ())
		{
			meta.Artist_ = tr ("Internet stream");
			meta.Title_ = location;
			return meta;
		}

		libvlc_media_parse (m);

		meta.Artist_ = libvlc_media_get_meta (m, libvlc_meta_Artist);
		meta.Album_ = libvlc_media_get_meta (m, libvlc_meta_Album);
		meta.Title_ = libvlc_media_get_meta (m, libvlc_meta_Title);
		meta.Genre_ = libvlc_media_get_meta (m, libvlc_meta_Genre);
		meta.Date_ = libvlc_media_get_meta (m, libvlc_meta_Date);
		meta.Length_ = libvlc_media_get_duration (m) / 1000;
		
		if (location.isNull ())
			meta.Location_ = QUrl (libvlc_media_get_mrl (m));
		else
			meta.Location_ = QUrl (location);
		
		libvlc_media_track_info_t *pTrackInfo;
		int numOfStream = libvlc_media_get_tracks_info (m, &pTrackInfo);
		
		if (numOfStream >= 1)
			meta.Type_ = pTrackInfo->i_type;

		meta.TrackNumber_ = QString (libvlc_media_get_meta (m,
						libvlc_meta_TrackNumber))
				.toInt ();
		return meta;
	}

	int VLCWrapper::RowCount () const
	{
		return libvlc_media_list_count (List_.get ());
	}

	int VLCWrapper::GetCurrentIndex () const
	{
		return CurrentItem_;
	}

	bool VLCWrapper::IsPlaying () const
	{
		return libvlc_media_player_is_playing (Player_.get ());
	}

	int VLCWrapper::GetVolume () const
	{
		return libvlc_audio_get_volume (Player_.get ());
	}

	float VLCWrapper::GetMediaPosition () const
	{
		return libvlc_media_player_get_position (Player_.get ());
	}

	int VLCWrapper::GetTime () const
	{
		return libvlc_media_player_get_time (Player_.get ());
	}

	int VLCWrapper::GetLength () const
	{
		return libvlc_media_player_get_length (Player_.get ());
	}

	void VLCWrapper::setWindow (WId winId)
	{
		libvlc_media_player_t *m = Player_.get ();
		int time = libvlc_media_player_get_time (m);
		libvlc_media_player_stop (m);

#ifdef Q_OS_WIN32
		libvlc_media_player_set_hwnd (m, winId);
#endif
#ifdef Q_WS_X11
		libvlc_media_player_set_xwindow (m, winId);
#endif
		libvlc_media_player_play (m);
		libvlc_media_player_set_time (m, time);
	}

	bool VLCWrapper::removeRow (int pos)
	{
		const bool res = !libvlc_media_list_remove_index (List_.get (), pos);
		QueueListIndex_.removeOne (pos);
		
		auto itr = QueueListIndex_.begin ();
		for (; itr != QueueListIndex_.end (); ++itr)
			if (*itr > pos)
				--(*itr);
		
		return res;
	}
	
	void VLCWrapper::setSubtitle (const QString& location) const
	{
		if (location.isEmpty ())
		{
			QAction *senderAction = qobject_cast<QAction*> (sender ());
			if (!senderAction)
				return;
		
			libvlc_video_set_subtitle_file (Player_.get (),
					senderAction->data ().toString ().toUtf8 ().constData ());
		}
		else
			libvlc_video_set_subtitle_file (Player_.get (),
					location.toUtf8 ().constData ());
	}
	
	QList<int> VLCWrapper::GetQueueListIndexes () const
	{
		return QueueListIndex_;
	}

	void VLCWrapper::addRow (const QString& location)
	{
		libvlc_media_t *m = libvlc_media_new_path (Instance_.get (),
				location.toUtf8 ());

		if (!libvlc_media_list_add_media (List_.get (), m))
			emit itemAdded (GetItemMeta (RowCount () - 1, location), location);
		else
			libvlc_media_release (m);
	}

	void VLCWrapper::setPlaybackMode (PlaybackMode mode)
	{
		auto list = LPlayer_.get ();
		switch (mode)
		{
		case PlaybackModeDefault:
			libvlc_media_list_player_set_playback_mode (list,
					libvlc_playback_mode_default);
			break;
		case PlaybackModeLoop:
			libvlc_media_list_player_set_playback_mode (list,
					libvlc_playback_mode_loop);
			break;
		case PlaybackModeRepeat:
			libvlc_media_list_player_set_playback_mode (list,
					libvlc_playback_mode_repeat);
			break;
		}
	}

	void VLCWrapper::setMeta (libvlc_meta_t type, const QString& value, int index)
	{
		auto m = libvlc_media_list_item_at_index (List_.get (), index);
		libvlc_media_set_meta (m, type, value.toUtf8 ());
		libvlc_media_save_meta (m);
	}

	void VLCWrapper::playItem (int val)
	{
		const int count = RowCount ();
		if (val == count)
			CurrentItem_ = 0;
		else if (val == -1)
			CurrentItem_ = count - 1;
		else
			CurrentItem_ = val;
		CurrentItemMeta_ = GetItemMeta (val);
		libvlc_media_list_player_play_item_at_index (LPlayer_.get (),
				CurrentItem_);
	}

	void VLCWrapper::stop ()
	{
		libvlc_media_player_stop (Player_.get ());
	}

	void VLCWrapper::pause ()
	{
		libvlc_media_player_pause (Player_.get ());
	}

	void VLCWrapper::play ()
	{
		int index = 0;
		if (IsPlaying () || (index = PlayQueue ()) < 0)
			libvlc_media_list_player_play (LPlayer_.get ());
	}

	void VLCWrapper::next ()
	{
		int index = PlayQueue ();
		if (index < 0)
			libvlc_media_list_player_next (LPlayer_.get ());
	}

	void VLCWrapper::prev ()
	{
		libvlc_media_list_player_previous (LPlayer_.get ());
	}

	void VLCWrapper::setVolume (int vol)
	{
		libvlc_audio_set_volume (Player_.get (), vol);
	}

	void VLCWrapper::setPosition (float pos)
	{
		libvlc_media_player_set_position (Player_.get (), pos);
	}
}
}
