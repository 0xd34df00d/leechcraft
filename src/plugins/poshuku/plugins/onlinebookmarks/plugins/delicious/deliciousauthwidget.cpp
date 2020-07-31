/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "deliciousauthwidget.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	DeliciousAuthWidget::DeliciousAuthWidget (QWidget *widget)
	: QWidget (widget)
	{
		Ui_.setupUi (this);
	}

	QVariantMap DeliciousAuthWidget::GetIdentifyingData () const
	{
		QVariantMap map;
		map ["Login"] = Ui_.Login_->text ();
		map ["Password"] = Ui_.Password_->text ();
		map ["OAuth"] = Ui_.YahooID_->isChecked ();
		return map;
	}

	void DeliciousAuthWidget::SetIdentifyingData (const QVariantMap& map)
	{
		Ui_.Login_->setText (map ["Login"].toString ());
		Ui_.Password_->setText (map ["Password"].toString ());
		bool oAuth = map.value ("OAuth", false).toBool ();
		Ui_.YahooID_->setChecked (oAuth);
	}
}
}
}
}
