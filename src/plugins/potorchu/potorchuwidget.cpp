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
#include <QLabel>
#include <QAction>
#include <QFileDialog>
#include <QDebug>

#include "potorchu.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		QObject *PotorchuWidget::S_ParentMultiTabs_ = 0;
		PotorchuWidget::PotorchuWidget (QWidget *parent, Qt::WindowFlags f)
		: QWidget (parent, f)
		, ToolBar_ (new QToolBar)
		, Ui_ (new Ui::PotorchuWidget)
		{
			Ui_->setupUi (this);
			QAction *actionOpen = new QAction ("Open", this);
			ToolBar_->addAction (actionOpen);
			
			connect (actionOpen,
					SIGNAL (triggered (bool)),
					this,
					SLOT (handleOpenFile ()));
			
			connect (Ui_->StopButton_,
					SIGNAL (clicked (bool)),
					Ui_->Player_,
					SLOT (stop ()));
			connect (Ui_->Player_,
					SIGNAL (timeout ()),
					this,
					SLOT (updateInterface ()));
		}
		
		void PotorchuWidget::updateInterface ()
		{
			Ui_->VolumeSlider_->setValue (Ui_->Player_->getVolume ());
			Ui_->PositionSlider_->setValue (Ui_->Player_->getPosition ());
		}

		
		PotorchuWidget::~PotorchuWidget()
		{
			delete ToolBar_;
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
		
		void PotorchuWidget::handleOpenFile ()
		{
			const QString& fileName = QFileDialog::getOpenFileName (this,
					tr ("Choose file"), QDir::homePath ());
			if (!fileName.isEmpty ())
			{
				Ui_->Player_->playFile (fileName);
				Ui_->Player_->show ();
			}
		}
		
		void PotorchuWidget::handleStop ()
		{
			Ui_->Player_->stop ();
		}

	}
}