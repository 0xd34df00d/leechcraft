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

#include "presenterwidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTimer>

namespace LeechCraft
{
namespace Monocle
{
	PresenterWidget::PresenterWidget (IDocument_ptr doc)
	: QWidget (0, Qt::Window | Qt::WindowStaysOnTopHint)
	, PixmapLabel_ (new QLabel)
	, Doc_ (doc)
	, CurrentPage_ (0)
	{
		setStyleSheet ("background-color: black;");

		auto lay = new QHBoxLayout ();
		lay->setSpacing (0);
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (PixmapLabel_, 0, Qt::AlignVCenter | Qt::AlignHCenter);
		setLayout (lay);

		showFullScreen ();

		QTimer::singleShot (50,
				this,
				SLOT (delayedShowInit ()));
	}

	void PresenterWidget::closeEvent (QCloseEvent *event)
	{
		deleteLater ();
		QWidget::closeEvent (event);
	}

	void PresenterWidget::keyPressEvent (QKeyEvent *event)
	{
		switch (event->key ())
		{
		case Qt::Key_Escape:
		case Qt::Key_Enter:
			deleteLater ();
			return;
		case Qt::Key_Backspace:
		case Qt::Key_PageUp:
			NavigateTo (CurrentPage_ - 1);
			break;
		case Qt::Key_PageDown:
		case Qt::Key_Space:
			NavigateTo (CurrentPage_ + 1);
			break;
		case Qt::Key_Home:
			NavigateTo (0);
			break;
		case Qt::Key_End:
			NavigateTo (Doc_->GetNumPages () - 1);
			break;
		}

		QWidget::keyPressEvent (event);
	}

	void PresenterWidget::NavigateTo (int page)
	{
		if (page < 0 || page >= Doc_->GetNumPages ())
			return;

		CurrentPage_ = page;

		const auto& pageSize = Doc_->GetPageSize (page);

		auto scale = std::min (static_cast<double> (width ()) / pageSize.width (),
				static_cast<double> (height ()) / pageSize.height ());

		const auto& img = Doc_->RenderPage (page, scale, scale);

		PixmapLabel_->setFixedSize (img.size ());
		PixmapLabel_->setPixmap (QPixmap::fromImage (img));
	}

	void PresenterWidget::delayedShowInit ()
	{
		NavigateTo (CurrentPage_);
	}
}
}
