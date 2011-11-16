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
	
	struct MediaMeta
	{
		QString Artist_, Album_, Title_, Genre_, Date_;
		int TrackNumber_;
		int Length_;
	};
	
	class VLCWrapper : public QObject
	{
		Q_OBJECT
		
		int CurrentItem_;
		libvlc_instance_ptr Instance_;
		libvlc_media_list_ptr List_;
		libvlc_media_list_player_ptr LPlayer_;
		libvlc_media_player_ptr Player_;
	public:
		VLCWrapper (QObject* = 0);
		
		int RowCount () const;
		int CurrentItem () const;
		bool IsPlaying () const;
		int Volume () const;
		int Time () const;
		int Length () const;
		float MediaPosition () const;
		MediaMeta GetItemMeta (int row) const;
	public slots:
		void addRow (const QString&);
		void setWindow (int);
		bool removeRow (int);
		void playItem (int);
		void stop ();
		void pause ();
		void play ();
		void next ();
		void prev ();
		void setVolume (int);
		void setPosition (float);
		
		void handledHasPlayed ();
		void handleNextItemSet ();
	private slots:
		void nowPlaying ();
	signals:
		void currentTrackMeta (const MediaMeta&);
		void trackFinished ();
		void itemPlayed (int);
		void itemAdded (const MediaMeta&, const QString&);
		void paused ();
		
		void gotEntity (const Entity&);
		void delegateEntity (const Entity&, int*, QObject**);
	};
}
}
#endif // PLUGINS_LAURE_CORE_H
