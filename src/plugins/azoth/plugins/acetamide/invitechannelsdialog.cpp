/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "invitechannelsdialog.h"
#include <QStandardItemModel>
#include "xmlsettingsmanager.h"

namespace LC::Azoth::Acetamide
{
	InviteChannelsDialog::InviteChannelsDialog (const QString& channel, const QString& nick, QWidget *parent)
	: QDialog { parent }
	, Model_ { new QStandardItemModel { this } }
	{
		Ui_.setupUi (this);

		Ui_.InviteChannels_->setModel (Model_);

		Model_->setColumnCount (3);
		Model_->setHorizontalHeaderLabels ({ tr ("Join"), tr ("Channel name"), tr ("Nickname") });

		AddInvitation (channel, nick);

		const auto curDefault = XmlSettingsManager::Instance ().Property ("InviteActionByDefault", 0).toInt ();
		Ui_.AlwaysAction_->setCurrentIndex (curDefault);
	}

	void InviteChannelsDialog::AddInvitation (const QString& channel, const QString& nick)
	{
		if (Model_->findItems (channel, Qt::MatchExactly, 1).count ())
			return;

		QList row
		{
			new QStandardItem,
			new QStandardItem { channel },
			new QStandardItem { nick },
		};
		row.first ()->setCheckable (true);
		for (auto item : row)
			item->setEditable (false);
		Model_->appendRow (row);
	}

	QStringList InviteChannelsDialog::GetChannels () const
	{
		QStringList channels;
		for (int i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i, 0)->checkState () == Qt::Checked)
				channels << Model_->item (i, 0)->text ();
		return channels;
	}

	void InviteChannelsDialog::accept ()
	{
		XmlSettingsManager::Instance ().setProperty ("InviteActionByDefault", Ui_.AlwaysAction_->currentIndex ());
		QDialog::accept ();
	}
}
