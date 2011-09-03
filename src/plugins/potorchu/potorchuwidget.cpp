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

#include "potorchuwidget.h"
#include <QAction>
#include <QFileDialog>
#include <QDebug>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QMessageBox>

#include <interfaces/core/icoreproxy.h>

#include "potorchu.h"
#include "playpauseaction.h"
#include "chooseurldialog.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		QObject *PotorchuWidget::S_ParentMultiTabs_ = 0;
		PotorchuWidget::PotorchuWidget (QWidget *parent, Qt::WindowFlags f)
		: QWidget (parent, f)
		, ToolBar_ (new QToolBar (this))
		, Ui_ (new Ui::PotorchuWidget)
		{
			Ui_->setupUi (this);
			Ui_->Player_->setFrameStyle (QFrame::Box | QFrame::Sunken);
			Ui_->CommandFrame_->setFrameStyle (QFrame::NoFrame);
			Ui_->PlayListWidget_->setVisible (false);
			connect (Ui_->Player_,
					SIGNAL (timeout ()),
					this,
					SLOT (updateInterface ()));
			connect (Ui_->PositionSlider_,
					SIGNAL (sliderMoved (int)),
					Ui_->Player_,
					SLOT (changePosition (int)));
			connect (Ui_->VolumeSlider_,
					SIGNAL (sliderMoved (int)),
					Ui_->Player_,
					SLOT (changeVolume (int)));
			connect (Ui_->PlayListWidget_,
					SIGNAL (play (QString)),
					Ui_->Player_,
					SLOT (playFile (QString)));
			connect (Ui_->Player_,
					SIGNAL (timeout ()),
					Ui_->PlayListWidget_,
					SIGNAL (nextFile ()));
		}
		
		void PotorchuWidget::Init (ICoreProxy_ptr proxy)
		{
			Ui_->PlayListWidget_->Init (proxy);
			QToolBar *bar = new QToolBar (Ui_->CommandFrame_);
			bar->setToolButtonStyle (Qt::ToolButtonIconOnly);
			bar->setIconSize (QSize (32, 32));
			QAction *actionPlay = new PlayPauseAction (QPair<QIcon, QIcon> (proxy->GetIcon ("start"),
							proxy->GetIcon ("pause")),
					Ui_->CommandFrame_);
			QAction *actionStop = new QAction (proxy->GetIcon ("media_stop"),
					tr ("Stop"), Ui_->CommandFrame_);
			QAction *actionNext = new QAction (proxy->GetIcon ("media_skip_forward"),
					tr ("Next"), Ui_->CommandFrame_);
			QAction *actionPrev = new QAction (proxy->GetIcon ("media_skip_backward"),
					tr ("Previous"), Ui_->CommandFrame_);
			bar->addAction (actionPrev);
			bar->addAction (actionPlay);
			bar->addAction (actionStop);
			bar->addAction (actionNext);
	
			connect (actionStop,
					SIGNAL (triggered (bool)),
					Ui_->Player_,
					SLOT (stop ()));
			connect (actionPlay,
					SIGNAL (play ()),
					Ui_->Player_,
					SLOT (play ()));
			connect (actionPlay,
					SIGNAL (pause ()),
					Ui_->Player_,
					SLOT (pause ()));
			
			QAction *actionOpenFile = new QAction (proxy->GetIcon ("folder"),
					tr ("Open File"), this);
			QAction *actionOpenURL = new QAction (proxy->GetIcon ("networkmonitor_plugin"),
					tr ("Open URL"), this);
			QAction *playList = new QAction (proxy->GetIcon ("itemlist"),
					tr ("Playlist"), this);
			playList->setCheckable (true);
			
			ToolBar_->addAction (actionOpenFile);
			ToolBar_->addAction (actionOpenURL);
			ToolBar_->addAction (playList);
			
			connect (actionOpenFile,
					SIGNAL (triggered (bool)),
					this,
					SLOT (handleOpenFile ()));
			connect (actionOpenURL,
					SIGNAL (triggered (bool)),
					this,
					SLOT (handleOpenURL ()));
			connect (playList,
					SIGNAL (triggered (bool)),
					this,
					SLOT (handlePlaylist ()));
		}

		
		void PotorchuWidget::updateInterface ()
		{
			Ui_->VolumeSlider_->setValue (Ui_->Player_->GetVolume ());
			Ui_->PositionSlider_->setValue (Ui_->Player_->GetPosition ());
		}

		
		PotorchuWidget::~PotorchuWidget ()
		{
			delete Ui_;
		}
		
		void PotorchuWidget::SetParentMultiTabs (QObject* parent)
		{
			S_ParentMultiTabs_ = parent;
		}


		TabClassInfo PotorchuWidget::GetTabClassInfo () const
		{
			return qobject_cast<Potorchu *> (S_ParentMultiTabs_)->GetTabClasses ().first ();
		}
		
		QObject* PotorchuWidget::ParentMultiTabs ()
		{
			return S_ParentMultiTabs_;
		}
		
		void PotorchuWidget::Remove ()
		{
			emit needToClose ();
		}
		
		QToolBar* PotorchuWidget::GetToolBar () const
		{
			return ToolBar_;
		}
		
		void PotorchuWidget::handleOpenURL ()
		{
			ChooseURLDialog *dialog = new ChooseURLDialog (this);
			if (dialog->exec () == QDialog::Accepted)
			{
				if (dialog->IsUrlValid ())
					Ui_->Player_->playFile (dialog->GetUrl ().toAscii ());
				else
					QMessageBox::warning (this,
							tr ("The URL's not valid"),
							tr ("The URL's not valid"));
			}
		}

		
		void PotorchuWidget::handleOpenMediaContent (const QString& val)
		{
			Ui_->Player_->playFile (val.toAscii ());
		}

		
		void PotorchuWidget::handleOpenFile ()
		{
			const QString& fileName = QFileDialog::getOpenFileName (this,
					tr ("Choose file"), QDir::homePath ());
			if (!fileName.isEmpty ())
				Ui_->Player_->playFile (fileName);
		}
		
		void PotorchuWidget::handlePlaylist ()
		{
			Ui_->PlayListWidget_->setVisible (qobject_cast<QAction *> (sender ())->isChecked ());
		}


	}
}