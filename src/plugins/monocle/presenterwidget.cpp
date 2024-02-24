/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "presenterwidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTimer>
#include <util/threads/futures.h>

namespace LC::Monocle
{
	PresenterWidget::PresenterWidget (IDocument_ptr doc)
	: QWidget { nullptr, Qt::Window | Qt::WindowStaysOnTopHint }
	, PixmapLabel_ { *new QLabel }
	, Doc_ { std::move (doc) }
	{
		setStyleSheet ("background-color: black;");

		auto lay = new QHBoxLayout;
		lay->setSpacing (0);
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (&PixmapLabel_, 0, Qt::AlignVCenter | Qt::AlignHCenter);
		setLayout (lay);

		showFullScreen ();

		QTimer::singleShot (50,
				this,
				SLOT (delayedShowInit ()));
		QTimer::singleShot (500,
				this,
				SLOT (delayedShowInit ()));
	}

	void PresenterWidget::NavigateTo (int page)
	{
		if (page < 0 || page >= Doc_->GetNumPages ())
			return;

		CurrentPage_ = page;

		const auto& pageSize = Doc_->GetPageSize (page);

		auto scale = std::min (static_cast<double> (width ()) / pageSize.width (),
				static_cast<double> (height ()) / pageSize.height ());

		Util::Sequence (this, Doc_->RenderPage (page, scale, scale)) >>
				[&] (const QImage& img)
				{
					PixmapLabel_.setFixedSize (img.size ());
					PixmapLabel_.setPixmap (QPixmap::fromImage (img));
				};
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
		case Qt::Key_K:
		case Qt::Key_Backspace:
		case Qt::Key_PageUp:
		case Qt::Key_Left:
			NavigateTo (CurrentPage_ - 1);
			break;
		case Qt::Key_J:
		case Qt::Key_Space:
		case Qt::Key_PageDown:
		case Qt::Key_Right:
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

	void PresenterWidget::delayedShowInit ()
	{
		NavigateTo (CurrentPage_);
	}
}
