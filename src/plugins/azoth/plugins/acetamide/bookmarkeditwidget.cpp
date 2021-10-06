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

namespace LC::Azoth::Acetamide
{
	BookmarkEditWidget::BookmarkEditWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		for (const auto& codec : QTextCodec::availableCodecs ())
			Ui_.Encoding_->addItem (QString::fromUtf8 (codec));
		Ui_.Encoding_->model ()->sort (0);
	}

	namespace
	{
#define DEFINE_LIT(a) const auto a = QStringLiteral (#a);

		DEFINE_LIT (HumanReadableName)
		DEFINE_LIT (StoredName)
		DEFINE_LIT (Server)
		DEFINE_LIT (Port)
		DEFINE_LIT (ServerPassword)
		DEFINE_LIT (Encoding)
		DEFINE_LIT (Channel)
		DEFINE_LIT (Password)
		DEFINE_LIT (Nickname)
		DEFINE_LIT (SSL)
		DEFINE_LIT (Autojoin)
	}

	QVariantMap BookmarkEditWidget::GetIdentifyingData () const
	{
		const auto& name = QStringLiteral ("%1@%2 (%3)")
				.arg (Ui_.Channel_->text (),
					  Ui_.Server_->text (),
					  Ui_.Nickname_->text ());

		auto channel = Ui_.Channel_->text ();
		if (!channel.startsWith ('#') &&
				!channel.startsWith ('&') &&
				!channel.startsWith ('+') &&
				!channel.startsWith ('!'))
			channel.prepend ('#');

		return
		{
			{ HumanReadableName, name },
			{ StoredName, Ui_.Name_->text () },
			{ Server, Ui_.Server_->text () },
			{ Port, Ui_.Port_->value () },
			{ ServerPassword, Ui_.ServerPassword_->text () },
			{ Encoding, Ui_.Encoding_->currentText () },

			{ Channel, channel },

			{ Password, Ui_.Password_->text () },
			{ Nickname, Ui_.Nickname_->text () },
			{ SSL, Ui_.SSL_->checkState () == Qt::Checked },
			{ Autojoin, Ui_.AutoJoin_->checkState () == Qt::Checked },
		};
	}

	void BookmarkEditWidget::SetIdentifyingData (const QVariantMap& map)
	{
		Ui_.HumanReadable_->setText (map [HumanReadableName].toString ());
		Ui_.Name_->setText (map [StoredName].toString ());
		Ui_.Server_->setText (map [Server].toString ());
		Ui_.Port_->setValue (map [Port].toInt ());
		Ui_.ServerPassword_->setText (map [ServerPassword].toString ());
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText (map [Encoding].toString ()));
		Ui_.Channel_->setText (map [Channel].toString ());
		Ui_.Password_->setText (map [Password].toString ());
		Ui_.Nickname_->setText (map [Nickname].toString ());
		Ui_.SSL_->setCheckState (map [SSL].toBool () ? Qt::Checked : Qt::Unchecked);
		Ui_.AutoJoin_->setCheckState (map [Autojoin].toBool () ? Qt::Checked : Qt::Unchecked);
	}
}
