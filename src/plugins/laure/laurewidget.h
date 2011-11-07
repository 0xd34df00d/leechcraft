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

#ifndef PLUGINS_LAURE_LAUREWIDGET_H
#define PLUGINS_LAURE_LAUREWIDGET_H
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/iinfo.h>
#include "ui_laurewidget.h"

class QToolBar;
class QUrl;

namespace LeechCraft
{
namespace Laure
{
	class Player;
	class PlayListWidget;
	class PlayPauseAction;
	
	class LaureWidget : public QWidget
				, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)
		
		static QObject *S_ParentMultiTabs_;
		
		QToolBar *ToolBar_;
		Ui::LaureWidget Ui_;
		VLCWrapper *VLCWrapper_;
	public:
		LaureWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
		
		void Init (ICoreProxy_ptr);
		static void SetParentMultiTabs (QObject*);
		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
		
		void keyPressEvent (QKeyEvent *);
	private:
		void InitCommandFrame ();
		void InitToolBar ();
	signals:
		void needToClose ();
		void playPause ();
		void currentTrackMeta (const MediaMeta&);
		void trackFinished ();
		void addItem (const QString&);
		void gotEntity (const Entity&);
		void delegateEntity (const Entity&,
				int*, QObject**);
		void playListMode (bool);
	public slots:
		void handleOpenMediaContent (const QString&);
	private slots:
		void handleOpenFile ();
		void handleOpenURL ();
		void updateInterface ();
		void handleVideoMode (bool);
	};
}
}

#endif // PLUGINS_LAURE_LAUREWIDGET_H
