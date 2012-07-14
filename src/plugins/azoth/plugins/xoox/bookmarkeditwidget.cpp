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

#include "bookmarkeditwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	BookmarkEditWidget::BookmarkEditWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}
	
	QVariantMap BookmarkEditWidget::GetIdentifyingData() const
	{
		QVariantMap result;
		result ["HumanReadableName"] = QString ("%1@%2 (%3)")
			.arg (Ui_.Room_->text ())
			.arg (Ui_.Server_->text ())
			.arg (Ui_.Nickname_->text ());
		result ["StoredName"] = Ui_.Name_->text ();
		result ["Room"] = Ui_.Room_->text ();
		result ["Server"] = Ui_.Server_->text ();
		result ["Nick"] = Ui_.Nickname_->text ();
		result ["Autojoin"] = Ui_.Autojoin_->checkState () == Qt::Checked;
		return result;
	}
	
	void BookmarkEditWidget::SetIdentifyingData (const QVariantMap& map)
	{
		Ui_.HumanReadable_->setText (map.value ("HumanReadableName").toString ());
		Ui_.Name_->setText (map.value ("StoredName").toString ());
		Ui_.Room_->setText (map.value ("Room").toString ());
		Ui_.Server_->setText (map.value ("Server").toString ());
		Ui_.Nickname_->setText (map.value ("Nick").toString ());
		Ui_.Autojoin_->setCheckState (map.value ("Autojoin").toBool () ? Qt::Checked : Qt::Unchecked);
	}
}
}
}
