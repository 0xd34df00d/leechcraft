/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "transcodingparamswidget.h"
#include <QThread>
#include "transcodingparams.h"

namespace LeechCraft
{
namespace LMP
{
	TranscodingParamsWidget::TranscodingParamsWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		const int idealThreads = QThread::idealThreadCount ();
		if (idealThreads > 0)
		{
			Ui_.ThreadsSlider_->setMaximum (idealThreads * 2);
			Ui_.ThreadsSlider_->setValue (idealThreads > 1 ? idealThreads - 1 : 1);
		}
		else
			Ui_.ThreadsSlider_->setMaximum (4);
	}

	TranscodingParams TranscodingParamsWidget::GetParams () const
	{
		const bool transcode = Ui_.TranscodingBox_->isChecked ();
		return
		{
			Ui_.FilenameMask_->text (),
			transcode ? Ui_.TranscodingFormat_->currentText () : QString (),
			Ui_.QualitySlider_->value (),
			Ui_.ThreadsSlider_->value ()
		};
	}
}
}
