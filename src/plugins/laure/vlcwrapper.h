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

#pragma once
#include <memory>
#include <QObject>
#include <QUrl>
#include <vlc/vlc.h>
#include <interfaces/ientityhandler.h>

class QTime;

namespace LeechCraft
{
namespace Laure
{
	typedef std::shared_ptr<libvlc_instance_t> libvlc_instance_ptr;
	typedef std::shared_ptr<libvlc_media_list_t> libvlc_media_list_ptr;
	typedef std::shared_ptr<libvlc_media_list_player_t> libvlc_media_list_player_ptr;
	typedef std::shared_ptr<libvlc_media_player_t> libvlc_media_player_ptr;
	typedef std::shared_ptr<libvlc_media_t> libvlc_media_ptr;
	typedef QMap<QString, QVariant> QVariantMap;

	/** @brief Provides a structure for storing media meta info
	 */
	struct MediaMeta
	{
		QVariantMap ToVariantMap () const;

		QString Artist_, Album_, Title_, Genre_, Date_;
		QUrl Location_;
		int TrackNumber_;
		int Length_;
		libvlc_track_type_t Type_;
	};

	/** @brief Defines playback modes for playlist.
	 */
	enum  PlaybackMode
	{
		PlaybackModeDefault,
		PlaybackModeRepeat,
		PlaybackModeLoop
	};

	/** @brief Provides a wrapper around libvlc functions.
	 *
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class VLCWrapper : public QObject
	{
		Q_OBJECT

		int CurrentItem_;
		libvlc_instance_ptr Instance_;
		libvlc_media_list_ptr List_;
		libvlc_media_list_player_ptr LPlayer_;
		libvlc_media_player_ptr Player_;
		QList<int> QueueListIndex_;
		bool IsPlayedFromQueue_;
		
		MediaMeta CurrentItemMeta_;
	public:
		/** @brief Constructs a new VLCWrapper class
		 * with the given parent.
		 */
		VLCWrapper (QObject* = 0);

		/** @brief Returns libvlc playlist item count.
		 */
		int RowCount () const;

		/** @brief Returns currently playing item index.
		 */
		int GetCurrentIndex () const;

		/** @brief Returns playback state.
		 *
		 * @return True if it is playing, false otherwise.
		 */
		bool IsPlaying () const;

		/** @brief Returns current volume.
		 */
		int GetVolume () const;

		/** @brief Get the current track time (in ms).
		 *
		 * @return The track time (in ms), or  a negative value if there
		 * is no media.
		 */
		int GetTime () const;

		/** @brief Get the current track length (in ms).
		 *
		 * @return The track length (in ms), or  a negative value if there
		 * is no media.
		 */
		int GetLength () const;

		/** @brief Returns current media position in the [0; 1] interval.
		 *
		 * @return Track position, or a negative value. in case of error.
		 */
		float GetMediaPosition () const;

		/** @brief Returns media meta info for the item in the given row.
		 *
		 * @param[in] row Item index.
		 *
		 * @return Media meta info.
		 */
		MediaMeta GetItemMeta (int row, const QString& location = QString ()) const;
		
		void HandlePlayed ();
		void HandleStopped ();
		
		/** @brief Is called when the next item is chosen.
		 */
		void HandleNextItemSet ();
	public slots:
		void addToQueue (int index);
		void removeFromQueue (int index);
		
		/** @brief Adds media file to the libvlc media list.
		 *
		 * @param[in] location Media file location.
		 */
		void addRow (const QString& location);

		/** @brief Sets the video frame window.
		 *
		 * Set an X Window System drawable where the media player should render its
		 * video output. If libvlc was built without X11 output support, then this has
		 * no effects.
		 *
		 * @param[in] winId Window identifier.
		 */
		void setWindow (WId winId);

		/** @brief Removes the media item in the pos row.
		 *
		 * @param[in] pos Item index.
		 */
		bool removeRow (int pos);

		/** @brief Plays the media item.
		 *
		 * @param[in] val Item index.
		 */
		void playItem (int val);

		/** @brief Stop playing.
		 *
		 * No effect if there is no media.
		 *
		 * @sa play()
		 * @sa pause()
		 */
		void stop ();

		/** @brief Pause.
		 *
		 * No effect if there is no media.
		 *
		 * @sa stop()
		 * @sa play()
		 */
		void pause ();

		/** @brief Play.
		 *
		 * @sa stop()
		 * @sa pause()
		 */
		void play ();

		/** @brief Play the next list media item.
		 *
		 * @sa prev()
		 */
		void next ();

		/** @brief Play the previous list media item.
		 *
		 * @sa next()
		 */
		void prev ();

		/** @brief Sets a volume.
		 */
		void setVolume (int vol);

		/** @brief Sets media position in the [0; 1] interval.
		 */
		void setPosition (float);

		/** @brief Sets the playback mode for the playlist.
		 *
		 * @param mode Playback mode.
		 *
		 * @sa PlaybackMode
		 */
		void setPlaybackMode (PlaybackMode mode);

		/** @brief Sets and save the meta of the media.
		 *
		 * @param[in] type Media type.
		 * @param[in] value New media meta info.
		 * @param[in] index Playlist item index.
		 */
		void setMeta (libvlc_meta_t type, const QString& value, int index);
		
		QList<int> GetQueueListIndexes () const;
		
		void setSubtitle (const QString& location = QString ()) const;
	private:
		int PlayQueue ();

	signals:
		/** @brief Is emitted when the item index is played.
		 *
		 * @param[out] index Item index.
		 */
		void itemPlayed (int index);
		
		void currentItemMeta (const MediaMeta& meta);

		/** @brief Is emitted when the media file is added to
		 * libvlc media list.
		 *
		 * @param[out] meta Media meta info.
		 * @param[out] location Media file location.
		 */
		void itemAdded (const MediaMeta& meta, const QString& location);

		/** @brief Is emitted when playback is paused.
		 */
		void paused ();

		void gotEntity (const Entity&);
		void delegateEntity (const Entity&, int*, QObject**);
	};
}
}