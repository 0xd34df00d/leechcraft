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

#include "laurewidget.h"
#include <QFileInfo>
#include <QFileInfoList>
#include <QAction>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QToolBar>
#include <QUrl>
#include <QTime>
#include <QKeyEvent>
#include <QTimer>
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include "laure.h"
#include "playpauseaction.h"
#include "chooseurldialog.h"
#include "playlistview.h"
#include "player.h"
#include "playlistwidget.h"
#include "xmlsettingsmanager.h"
#include "separateplayer.h"


namespace LeechCraft
{
namespace Laure
{
	QObject *LaureWidget::S_ParentMultiTabs_ = 0;
	
	LaureWidget::LaureWidget (QWidget *parent, Qt::WindowFlags f)
	: QWidget (parent, f)
	, ToolBar_ (new QToolBar (this))
	, VLCWrapper_ (new VLCWrapper)
	, SeparatePlayer_ (new SeparatePlayer)
	, DetachedVideo_ (new QAction (tr ("Detached video"), this))
	, PlayListAction_ (new QAction (tr ("Playlist"), this))
	, SubtitleAction_ (new QAction (tr ("Subtitle"), this))
	, SubtitleMenu_ (new QMenu (this))
	{
		Ui_.setupUi (this);
		
		Ui_.PlayListWidget_->Init (VLCWrapper_);
		Ui_.Player_->SetVLCWrapper (VLCWrapper_);
		
		connect (SeparatePlayer_.get (),
				SIGNAL (closed ()),
				this,
				SLOT (handleSeparatePlayerClosed ()));
		
		VLCWrapper *wrapper = VLCWrapper_.get ();

		connect (Ui_.PlayListWidget_,
				SIGNAL (gotEntity (Entity)),
				this,
				SIGNAL (gotEntity (Entity)));
		connect (Ui_.PlayListWidget_,
				SIGNAL (delegateEntity (Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (Entity, int*, QObject**)));
		
		connect (Ui_.Player_,
				SIGNAL (timeout ()),
				this,
				SLOT (updateInterface ()));
		connect (Ui_.PositionSlider_,
				SIGNAL (sliderMoved (int)),
				Ui_.Player_,
				SLOT (setPosition (int)));
		connect (Ui_.VolumeSlider_,
				SIGNAL (valueChanged (int)),
				wrapper,
				SLOT (setVolume (int)));
				
		connect (wrapper,
				SIGNAL (gotEntity (Entity)),
				this,
				SIGNAL (gotEntity (Entity)));
		
		connect (wrapper,
				SIGNAL (delegateEntity (Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (Entity, int*, QObject**)));
		connect (wrapper,
				SIGNAL (currentItemMeta (MediaMeta)),
				this,
				SLOT (updateSubtitleMenu (MediaMeta)));
		
		connect (this,
				SIGNAL (addItem (QString)),
				wrapper,
				SLOT (addRow (QString)));
		
		const int playlistWidth = XmlSettingsManager::Instance ()
				.property ("PlayListWidgetWidth").toInt ();
		
		Ui_.Splitter_->setSizes (QList<int> () << size ().width () << playlistWidth);
		
		InitToolBar ();
		InitCommandFrame ();
	}
	
	LaureWidget::~LaureWidget ()
	{
		XmlSettingsManager::Instance ().setProperty ("PlayListWidgetWidth",
				Ui_.PlayListWidget_->size ().width ());
	}
	
	void LaureWidget::InitToolBar ()
	{
		QAction *actionOpenFile = new QAction (tr ("Open File"), this);
		QAction *actionOpenURL = new QAction (tr ("Open URL"), this);
		PlayListAction_ = new QAction (tr ("Playlist"), this);
		QAction *videoMode = new QAction (tr ("Video mode"), this);
		DetachedVideo_ = new QAction (tr ("Detached video"), this);
		SubtitleAction_ = new QAction (tr ("Subtitle"), this);
		
		actionOpenFile->setProperty ("ActionIcon", "folder");
		actionOpenURL->setProperty ("ActionIcon", "document-open-remote");
		PlayListAction_->setProperty ("ActionIcon", "format-list-unordered");
		videoMode->setProperty ("ActionIcon", "tool-animator");
		DetachedVideo_->setProperty ("ActionIcon", "view-fullscreen");
		SubtitleAction_->setProperty ("ActionIcon", "preferences-desktop-font");
		
		PlayListAction_->setCheckable (true);
		videoMode->setCheckable (true);
		DetachedVideo_->setCheckable (true);
		
		videoMode->setChecked (true);
		DetachedVideo_->setChecked (false);
		
		ToolBar_->addAction (actionOpenFile);
		ToolBar_->addAction (actionOpenURL);
		ToolBar_->addAction (PlayListAction_);
		ToolBar_->addAction (videoMode);
		ToolBar_->addAction (DetachedVideo_);
		ToolBar_->addAction (SubtitleAction_);
		
		SubtitleAction_->setMenu (SubtitleMenu_);
		SubtitleAction_->setMenuRole (QAction::ApplicationSpecificRole);
		
		connect (actionOpenFile,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleOpenFile ()));
		connect (actionOpenURL,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleOpenURL ()));
		connect (PlayListAction_,
				SIGNAL (triggered (bool)),
				Ui_.PlayListWidget_,
				SLOT (setVisible (bool)));
		connect (videoMode,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleVideoMode (bool)));
		connect (DetachedVideo_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleDetachPlayer (bool)));
		connect (SubtitleAction_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (showSubtitleMenu ()));
	}

	void LaureWidget::showSubtitleMenu ()
	{
		SubtitleMenu_->popup (QCursor::pos ());
	}
	
	void LaureWidget::updateSubtitleMenu (const MediaMeta& meta)
	{
		const bool isVideo = meta.Type_ == libvlc_track_video;
		SubtitleMenu_->clear ();
		SubtitleAction_->setVisible (isVideo);
		if (!isVideo)
			return;
		
		const QFileInfo metaFileInfo = QFileInfo (meta.Location_.path ());
		const QString& baseName = metaFileInfo.baseName ();
		QDir dir (metaFileInfo.dir ());
		
		dir.setFilter (QDir::Files | QDir::NoSymLinks);
		dir.setSorting (QDir::Reversed);
		dir.setNameFilters (QStringList ()
				<< baseName + "*.srt"
				<< baseName + "*.ass");
		const QFileInfoList& list = dir.entryInfoList ();
		
		VLCWrapper *wrapper = VLCWrapper_.get ();
		Q_FOREACH (const QFileInfo& file, list)
		{
			QAction *subtitleMenuAction = new QAction (file.fileName (), SubtitleMenu_);
			subtitleMenuAction->setData (file.absoluteFilePath ());
			SubtitleMenu_->addAction (subtitleMenuAction);
			
			connect (subtitleMenuAction,
					SIGNAL (triggered (bool)),
					wrapper,
					SLOT (setSubtitle ()));
		}
		
		QAction *otherSubtitle = new QAction (tr ("Other subtitle..."), SubtitleMenu_);
		SubtitleMenu_->addAction (otherSubtitle);
		
		connect (otherSubtitle,
				SIGNAL (triggered (bool)),
				this,
				SLOT (subtitleDialog ()));
	}
	
	void LaureWidget::subtitleDialog ()
	{
		const QString& fileName = QFileDialog::getOpenFileName (this,
				tr ("Open subtitles"), QDir::homePath (),
				tr ("Subtitles (*.srt *.ass);;All (*)"));
		
		if (fileName.isEmpty ())
			return;
		
		VLCWrapper_->setSubtitle (fileName);
	}
	
	void LaureWidget::InitCommandFrame ()
	{
		QToolBar *bar = new QToolBar (Ui_.CommandFrame_);
		bar->setToolButtonStyle (Qt::ToolButtonIconOnly);
		bar->setIconSize (QSize (32, 32));
		
		auto actionPlay = new PlayPauseAction (tr ("Play"), Ui_.CommandFrame_);
		QAction *actionStop = new QAction (tr ("Stop"), Ui_.CommandFrame_);
		QAction *actionNext = new QAction (tr ("Next"), Ui_.CommandFrame_);
		QAction *actionPrev = new QAction (tr ("Previous"), Ui_.CommandFrame_);
		
		actionStop->setProperty ("ActionIcon", "media-playback-stop");
		actionNext->setProperty ("ActionIcon", "media-skip-forward");
		actionPrev->setProperty ("ActionIcon", "media-skip-backward");
		
		bar->addAction (actionPrev);
		bar->addAction (actionPlay);
		bar->addAction (actionStop);
		bar->addAction (actionNext);
		
		VLCWrapper *wrapper = VLCWrapper_.get ();
		connect (wrapper,
				SIGNAL (paused ()),
				actionPlay,
				SLOT (handlePause ()));
		connect (wrapper,
				SIGNAL (itemPlayed (int)),
				actionPlay,
				SLOT (handlePlay ()));
		connect (actionStop,
				SIGNAL (triggered (bool)),
				actionPlay,
				SLOT (handlePause ()));
		connect (actionStop,
				SIGNAL (triggered (bool)),
				wrapper,
				SLOT (stop ()));
		connect (actionNext,
				SIGNAL (triggered (bool)),
				wrapper,
				SLOT (next ()));
		connect (actionPrev,
				SIGNAL (triggered (bool)),
				wrapper,
				SLOT (prev ()));
		connect (actionPlay,
				SIGNAL (play ()),
				wrapper,
				SLOT (play ()));
		connect (actionPlay,
				SIGNAL (pause ()),
				wrapper,
				SLOT (pause ()));
		connect (this,
				SIGNAL (playPause ()),
				actionPlay,
				SLOT (handleTriggered ()));
	}
	
	void LaureWidget::updateInterface ()
	{
		Ui_.VolumeSlider_->setValue (VLCWrapper_->GetVolume ());
		Ui_.PositionSlider_->setValue (Ui_.Player_->GetPosition ());
		const QTime& currTime = Ui_.Player_->GetTime ();
		const QTime& length = Ui_.Player_->GetLength ();
		Ui_.TimeStamp_->setText ("[" + currTime.toString () + "/" + length.toString () + "]");
	}
	
	void LaureWidget::SetParentMultiTabs (QObject* parent)
	{
		S_ParentMultiTabs_ = parent;
	}

	TabClassInfo LaureWidget::GetTabClassInfo () const
	{
		return qobject_cast<Plugin*> (S_ParentMultiTabs_)->GetTabClasses ().first ();
	}
	
	QObject* LaureWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}
	
	void LaureWidget::Remove ()
	{
		emit needToClose ();
	}
	
	QToolBar* LaureWidget::GetToolBar () const
	{
		return ToolBar_;
	}
	
	void LaureWidget::keyPressEvent (QKeyEvent *event)
	{
		switch (event->key ())
		{
		case Qt::Key_Space:
			emit playPause ();
		}
	}
	
	void LaureWidget::handleOpenURL ()
	{
		auto dialog = new ChooseURLDialog (this);
		if (dialog->exec () == QDialog::Accepted && dialog->IsUrlValid ())
			emit addItem (dialog->GetUrl ());
	}
	
	void LaureWidget::handleOpenMediaContent (const QString& location)
	{
		emit addItem (location);
	}
	
	void LaureWidget::handleOpenFile ()
	{
		const QString& fileName = QFileDialog::getOpenFileName (this,
				tr ("Choose file"), QDir::homePath ());
		if (!fileName.isEmpty ())
			emit addItem (fileName);
	}
	
	void LaureWidget::handleVideoMode (bool checked)
	{
		if (checked)
		{
			const int playlistWidth = XmlSettingsManager::Instance ()
					.property ("PlayListWidgetWidth").toInt ();
			Ui_.Splitter_->addWidget (Ui_.PlayListWidget_);
			PlayListAction_->setChecked (false);
			Ui_.Splitter_->setSizes (QList<int> () << size ().width () << playlistWidth);
		}
		else
		{
			XmlSettingsManager::Instance ().setProperty ("PlayListWidgetWidth",
					Ui_.PlayListWidget_->size ().width ());
			Ui_.GlobalGridLayout_->addWidget (Ui_.PlayListWidget_, 0, 1, 1, 4);
		}
		
		PlayListAction_->setEnabled (checked);
		Ui_.Player_->setVisible (checked);
		Ui_.PlayListWidget_->setVisible (!checked);
	}
	
	void LaureWidget::handleDetachPlayer (bool checked)
	{
		if (checked)
		{
			VLCWrapper_->setWindow (SeparatePlayer_->winId ());
			SeparatePlayer_->show ();
		}
		else
		{
			VLCWrapper_->setWindow (Ui_.Player_->winId ());
			SeparatePlayer_->hide ();
		}
	}
	
	void LaureWidget::handleSeparatePlayerClosed ()
	{
		DetachedVideo_->setChecked (false);
		handleDetachPlayer (false);
	}
}
}