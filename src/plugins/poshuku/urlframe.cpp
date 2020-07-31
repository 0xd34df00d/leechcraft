/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urlframe.h"
#include <QIcon>

namespace LC
{
namespace Poshuku
{
	URLFrame::URLFrame (QWidget *parent)
	: QFrame (parent)
	{
		Ui_.setupUi (this);
	}

	QLineEdit* URLFrame::GetEdit () const
	{
		return Ui_.URLEdit_;
	}

	ProgressLineEdit* URLFrame::GetEditAsProgressLine () const
	{
		return Ui_.URLEdit_;
	}

	void URLFrame::SetFavicon (const QIcon& icon)
	{
		QPixmap pixmap = icon.pixmap (Ui_.FaviconLabel_->size ());
		Ui_.FaviconLabel_->setPixmap (pixmap);
	}

	void URLFrame::AddWidget (QWidget *widget)
	{
		layout ()->addWidget (widget);
	}

	void URLFrame::RemoveWidget (QWidget *widget)
	{
		layout ()->removeWidget (widget);
	}

	void URLFrame::on_URLEdit__returnPressed ()
	{
		if (Ui_.URLEdit_->IsCompleting () ||
				Ui_.URLEdit_->text ().isEmpty ())
			return;

		emit load (Ui_.URLEdit_->text ());
	}
}
}
