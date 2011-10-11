/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "deliciousauthwidget.h"

namespace LeechCraft
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