/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarkeditwidget.h"

namespace LC
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
		const auto& hrName = QString ("%1@%2 (%3)")
				.arg (Ui_.Room_->text ())
				.arg (Ui_.Server_->text ())
				.arg (Ui_.Nickname_->text ());
		result ["HumanReadableName"] = hrName;

		auto name = Ui_.Name_->text ();
		if (name.isEmpty ())
			name = hrName;
		result ["StoredName"] = name;

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
