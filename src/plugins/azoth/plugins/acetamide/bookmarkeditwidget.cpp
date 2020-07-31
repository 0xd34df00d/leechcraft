/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarkeditwidget.h"
#include <QTextCodec>
#include <QtDebug>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	BookmarkEditWidget::BookmarkEditWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		for (const auto& codec : QTextCodec::availableCodecs ())
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
		result ["ServerPassword"] = Ui_.ServerPassword_->text ();
		result ["Encoding"] = Ui_.Encoding_->currentText ();

		QString channel = Ui_.Channel_->text ();
		if (!channel.startsWith ('#') &&
				!channel.startsWith ('&') &&
				!channel.startsWith ('+') &&
				!channel.startsWith ('!'))
			channel.prepend ('#');
		result ["Channel"] = channel;

		result ["Password"] = Ui_.Password_->text ();
		result ["Nickname"] = Ui_.Nickname_->text ();
		result ["SSL"] = Ui_.SSL_->checkState () == Qt::Checked;
		result ["Autojoin"] = Ui_.AutoJoin_->checkState () == Qt::Checked;
		return result;
	}

	void BookmarkEditWidget::SetIdentifyingData (const QVariantMap& map)
	{
		Ui_.HumanReadable_->setText (map ["HumanReadableName"].toString ());
		Ui_.Name_->setText (map ["StoredName"].toString ());
		Ui_.Server_->setText (map ["Server"].toString ());
		Ui_.Port_->setValue (map ["Port"].toInt ());
		Ui_.ServerPassword_->setText (map ["ServerPassword"].toString ());
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->
		findText (map ["Encoding"].toString ()));
		Ui_.Channel_->setText (map ["Channel"].toString ());
		Ui_.Password_->setText (map ["Password"].toString ());
		Ui_.Nickname_->setText (map ["Nickname"].toString ());
		Ui_.SSL_->setCheckState (map ["SSL"].toBool () ? Qt::Checked : Qt::Unchecked);
		Ui_.AutoJoin_->setCheckState (map ["Autojoin"].toBool () ? Qt::Checked : Qt::Unchecked);
	}
}
}
}

