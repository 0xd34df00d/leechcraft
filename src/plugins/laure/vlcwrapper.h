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

#ifndef PLUGINS_LAURE_CORE_H
#define PLUGINS_LAURE_CORE_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include <vlc/vlc.h>
#include <interfaces/ientityhandler.h>

class QTime;

namespace LeechCraft
{
namespace Laure
{
	typedef boost::shared_ptr<libvlc_instance_t> libvlc_instance_ptr;
	typedef boost::shared_ptr<libvlc_media_list_t> libvlc_media_list_ptr;
	typedef boost::shared_ptr<libvlc_media_list_player_t> libvlc_media_list_player_ptr;
	typedef boost::shared_ptr<libvlc_media_player_t> libvlc_media_player_ptr;
	typedef boost::shared_ptr<libvlc_media_t> libvlc_media_ptr;
	
	/** @brief Provides a structure for storing media meta info
	 */
	struct MediaMeta
	{
		QString Artist_, Album_, Title_, Genre_, Date_;
		int TrackNumber_;
		int Length_;
	};
	
	typedef enum
	{
		PlaybackModeDefault,
		PlaybackModeRepeat,
		PlaybackModeLoop
		//...
	} PlaybackMode;
	
	/** @brief Provides a wrap around libvlc functions.
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
	public:
		/** @brief Constructs a new VLCWrapper class
		 * with the given parent.
		 */
		VLCWrapper (QObject* = 0);
		
		/** @brief Returns libvlc playlist item count.
		 */
		int RowCount () const;
		
		/** @brief Returns current played item.
		 */
		int GetCurrentIndex () const;
		
		/** @brief Returns playing state.
		 * 
		 * @return True if it's playing, false otherwise.
		 */
		bool IsPlaying () const;
		
		/** @brief Returns current volume.
		 */
		int GetVolume () const;
		
		/** @brief Returns current media playing time.
		 */
		int GetTime () const;
		
		/** @brief Returns current media playing length.
		 */
		int GetLength () const;
		
		/** @brief Returns current media position as a part of 1.
		 */
		float GetMediaPosition () const;
		
		/** @brief Returns media meta info of the item row.
		 * 
		 * @param[in] row Item index.
		 * 
		 * @return Media meta info.
		 * 
		 * @sa MediaMeta
		 */
		MediaMeta GetItemMeta (int row) const;
	public slots:
		/** @brief Adds media file to the libvlc media list.
		 * 
		 * @param[in] location Media file location.
		 */
		void addRow (const QString& location);
		
		/** @brief Sets the video frame window.
		 *
		 * @param[in] winId Window identifier.
		 */
		void setWindow (int winId);
		
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
		 */
		void stop ();
		
		/** @brief Pause.
		 */
		void pause ();
		
		/** @brief Play.
		 */
		void play ();
		
		/** @brief Play the next list media item.
		 */
		void next ();
		
		/** @brief Play the previous list media item.
		 */
		void prev ();
		
		/** @brief Sets a volume.
		 */
		void setVolume (int vol);
		
		/** @brief Sets media position as a part of 1.
		 */
		void setPosition (float);
		
		/** @brief Sets playback mode.
		 * 
		 * @param mode Playback mode.
		 * 
		 * @sa PlaybackMode
		 */
		void setPlaybackMode (PlaybackMode mode);
		
		/** @brief This slot's called when the track's finished.
		 */
		void handledHasPlayed ();
		
		/** @brief This slot's called when the next item's chosen.
		 */
		void handleNextItemSet ();
	private slots:
		void nowPlaying ();
	signals:
		/** @brief This signal is emmited to notify about the current track
		 * media meta info.
		 * 
		 * @param[out] meta Media meta info.
		 */
		void currentTrackMeta (const MediaMeta& meta);
		
		/** @brief This signal is emmited when the track's finished.
		 */
		void trackFinished ();
		
		/** @brief This signal is emmited when the item index is played.
		 *
		 * @param[out] index Item index.
		 */ 
		void itemPlayed (int index);
		
		/** @brief This signal is emmited when the media file's added to
		 * libvlc media list.
		 * 
		 * @param[out] meta Media meta info.
		 * @param[out] location Media file location.
		 *
		 * @sa MediaMeta
		 */
		void itemAdded (const MediaMeta& meta, const QString& location);
		
		/** @brief This signal is emmited when it's paused.
		 */
		void paused ();
		
		void gotEntity (const Entity&);
		void delegateEntity (const Entity&, int*, QObject**);
	};
}
}
#endif // PLUGINS_LAURE_CORE_H
