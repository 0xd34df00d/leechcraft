/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "readitlaterauthwidget.h"
#include <QtDebug>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterAuthWidget::ReadItLaterAuthWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	QVariantMap ReadItLaterAuthWidget::GetIdentifyingData () const
	{
		QVariantMap map;
		map ["Login"] = Ui_.Login_->text ();
		map ["Password"] = Ui_.Password_->text ();
		return map;
	}

	void ReadItLaterAuthWidget::SetIdentifyingData (const QVariantMap& map)
	{
		Ui_.Login_->setText (map ["Login"].toString ());
		Ui_.Password_->setText (map ["Password"].toString ());
	}
}
}
}
}
