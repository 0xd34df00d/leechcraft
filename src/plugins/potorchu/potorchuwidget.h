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

#ifndef POTORCHUWIDGET_H
#define POTORCHUWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QUrl>
#include <interfaces/ihavetabs.h>
#include <interfaces/iinfo.h>
#include "player.h"
#include "playlistwidget.h"
#include "ui_potorchuwidget.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		class PlayPauseAction;
		class PotorchuWidget : public QWidget
					, public ITabWidget
		{
			Q_OBJECT
			Q_INTERFACES (ITabWidget)
			QToolBar *ToolBar_;
			static QObject *S_ParentMultiTabs_;
			Ui::PotorchuWidget *Ui_;
			PlayPauseAction *ActionPlay_;
		public:
			PotorchuWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
			void Init (ICoreProxy_ptr);
			virtual ~PotorchuWidget ();
			static void SetParentMultiTabs (QObject *parent);
			TabClassInfo GetTabClassInfo () const;
			QObject* ParentMultiTabs ();
			void Remove ();
			QToolBar* GetToolBar () const;
			
			void keyPressEvent (QKeyEvent *event);
		private:
			void InitCommandFrame (ICoreProxy_ptr);
			void InitToolBar (ICoreProxy_ptr);
		signals:
			void needToClose ();
			void playPause ();
		public slots:
			void handleOpenMediaContent (const QString& val);
		private slots:
			void handleOpenFile ();
			void handleOpenURL ();
			void handleStop ();
			void handlePlay ();
			void updateInterface ();
			void handlePlaylist ();
		};
	}
}

#endif // POTORCHUWIDGET_H
