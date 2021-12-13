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
#include <util/sll/qtutil.h>
#include "localtypes.h"

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

	QVariantMap BookmarkEditWidget::GetIdentifyingData () const
	{
		const auto& name = u"%1@%2 (%3)"_qsv
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
			{ Lits::HumanReadableName, name },
			{ Lits::StoredName, Ui_.Name_->text () },
			{ Lits::Server, Ui_.Server_->text () },
			{ Lits::Port, Ui_.Port_->value () },
			{ Lits::ServerPassword, Ui_.ServerPassword_->text () },
			{ Lits::Encoding, Ui_.Encoding_->currentText () },
			{ Lits::Channel, channel },
			{ Lits::ChannelPassword, Ui_.Password_->text () },
			{ Lits::Nickname, Ui_.Nickname_->text () },
			{ Lits::SSL, Ui_.SSL_->checkState () == Qt::Checked },
			{ Lits::Autojoin, Ui_.AutoJoin_->checkState () == Qt::Checked },
		};
	}

	void BookmarkEditWidget::SetIdentifyingData (const QVariantMap& map)
	{
		Ui_.HumanReadable_->setText (map [Lits::HumanReadableName].toString ());
		Ui_.Name_->setText (map [Lits::StoredName].toString ());
		Ui_.Server_->setText (map [Lits::Server].toString ());
		Ui_.Port_->setValue (map [Lits::Port].toInt ());
		Ui_.ServerPassword_->setText (map [Lits::ServerPassword].toString ());
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText (map [Lits::Encoding].toString ()));
		Ui_.Channel_->setText (map [Lits::Channel].toString ());
		Ui_.Password_->setText (map [Lits::ChannelPassword].toString ());
		Ui_.Nickname_->setText (map [Lits::Nickname].toString ());
		Ui_.SSL_->setCheckState (map [Lits::SSL].toBool () ? Qt::Checked : Qt::Unchecked);
		Ui_.AutoJoin_->setCheckState (map [Lits::Autojoin].toBool () ? Qt::Checked : Qt::Unchecked);
	}
}
