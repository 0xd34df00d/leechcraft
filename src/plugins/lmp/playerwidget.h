/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_LMP_PLAYERWIDGET_H
#define PLUGINS_LMP_PLAYERWIDGET_H
#include <memory>
#include <QWidget>
#include <interfaces/imediaplayer.h>
#include "ui_playerwidget.h"
#include "phonon.h"

class QToolBar;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class PlayerWidget : public QWidget
							   , public IVideoWidget
			{
				Q_OBJECT
				Q_INTERFACES (IVideoWidget)

				Ui::PlayerWidget Ui_;

				Phonon::Path VideoPath_;
				Phonon::Path AudioPath_;
				std::auto_ptr<Phonon::MediaObject> MediaObject_;
				std::auto_ptr<Phonon::AudioOutput> AudioOutput_;
				QAction *Play_;
				QAction *Pause_;
				QAction *ViewerSettings_;
				
				QAction *FullScreen_;
				QAction *TogglePause_;
				QAction *VolumeUp_;
				QAction *VolumeDown_;
			public:
				enum SkipAmount
				{
					SkipLittle = 10
					, SkipMedium = 60
					, SkipALot = 600
				};

				PlayerWidget (QWidget* = 0);
				void Play ();
				void Pause ();
				void Stop ();
				void Clear ();
				void Enqueue (const QUrl&);
				void Enqueue (QIODevice*);
				QWidget* Widget ();

				void Enqueue (const Phonon::MediaSource&);
				void Forward (SkipAmount);
				void Rewind (SkipAmount);
				Phonon::State GetState () const;
				Phonon::MediaObject* GetMediaObject () const;
			public slots:
				void play ();
				void pause ();
				void toggleFullScreen ();
				void togglePause ();
				void incrementVolume ();
				void decrementVolume ();
			private:
				QToolBar* SetupToolbar ();
				void SetupContextMenu ();
				void ApplyVideoSettings (qreal, qreal, qreal, qreal);
			private slots:
				void handleHasVideoChanged (bool);
				void updateState ();
				void changeViewerSettings ();
				void handleStateUpdated (const QString&);
			signals:
				void stateUpdated (const QString&);
				void error (const QString&);
			};
		};
	};
};

#endif

