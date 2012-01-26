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

#include "bookmarkeditwidget.h"
#include <QTextCodec>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	BookmarkEditWidget::BookmarkEditWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Q_FOREACH (const QByteArray& codec,
				QTextCodec::availableCodecs ())
			Ui_.Encoding_->addItem (QString::fromUtf8 (codec));
		Ui_.Encoding_->model ()->sort (0);
	}

	QVariantMap BookmarkEditWidget::GetIdentifyingData () const
	{
		QVariantMap result;
		result ["HumanReadableName"] = QString ("%1@%2 (%3)")
			.arg (Ui_.Channel_->text ())
			.arg (Ui_.Server_->text ())
			.arg (Ui_.Nickname_->text ());
		result ["StoredName"] = Ui_.Name_->text ();
		result ["Server"] = Ui_.Server_->text ();
		result ["Port"] = Ui_.Port_->value ();
		result ["Encoding"] = Ui_.Encoding_->currentText ();
		result ["Channel"] = Ui_.Channel_->text ();
		result ["Password"] = Ui_.Password_->text ();
		result ["Nickname"] = Ui_.Nickname_->text ();
		result ["SSL"] = Ui_.SSL_->checkState () == Qt::Checked;
		result ["Autojoin"] = Ui_.AutoJoin_->checkState () == Qt::Checked;

		return result;
	}

	void BookmarkEditWidget::SetIdentifyingData (const QVariantMap& map)
	{
		Ui_.HumanReadable_->setText (map.value ("HumanReadableName").toString ());
		Ui_.Name_->setText (map.value ("StoredName").toString ());
		Ui_.Server_->setText (map.value ("Server").toString ());
		Ui_.Port_->setValue (map.value ("Port").toInt ());
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->
				findText (map.value ("Encoding").toString ()));
		Ui_.Channel_->setText (map.value ("Channel").toString ());
		Ui_.Password_->setText (map.value ("Password").toString ());
		Ui_.Nickname_->setText (map.value ("Nickname").toString ());
		Ui_.SSL_->setCheckState (map.value ("SSL").toBool () ? Qt::Checked : Qt::Unchecked);
		Ui_.AutoJoin_->setCheckState (map.value ("Autojoin").toBool () ? Qt::Checked : Qt::Unchecked);
	}
}
}
}

