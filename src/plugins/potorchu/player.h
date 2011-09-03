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
			libvlc_media_player_t *MP_;
			libvlc_media_t *M_;
		public:
			Player (QWidget *parent = 0, Qt::WindowFlags f = 0);
			virtual ~Player ();
			
			QString GetMeta (libvlc_meta_t meta) const;
			
			int GetVolume () const;
			int GetPosition () const;
			bool IsPlayed () const;
		public slots:
			void playFile (const QString& file);
			void changeVolume (int newVolume);
			void changePosition (int newPosition);
			void stop ();
			void pause ();
			void play ();
		signals:
			void timeout ();
		};
	}
}
#endif // PLAYER_H
