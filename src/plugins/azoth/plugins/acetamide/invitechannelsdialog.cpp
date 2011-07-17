/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#include "invitechannelsdialog.h"
#include <QStandardItemModel>
#include "xmlsettingsmanager.h"
#include <QtDebug>
namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	InviteChannelsDialog::InviteChannelsDialog (const QString& channel,
			const QString& nick, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Model_ = new QStandardItemModel (this);
		ActionsModel_ = new QStandardItemModel (this);

		Ui_.InviteChannels_->setModel (Model_);
		Ui_.AlwaysAction_->setModel (ActionsModel_);

		Model_->setColumnCount (3);
		Model_->setHorizontalHeaderLabels (QStringList () << tr ("Join")
				<< tr ("Channel name")
				<< tr ("Nickname"));

		QStandardItem *joinItem = new QStandardItem;
		joinItem->setCheckable (true);
		joinItem->setEditable (false);
		QStandardItem *channelItem = new QStandardItem (channel);
		channelItem->setEditable (false);
		QStandardItem *nickItem = new QStandardItem (nick);
		nickItem->setEditable (false);
		Model_->appendRow (QList<QStandardItem*> () << joinItem
				<< channelItem
				<< nickItem);

		ActionsModel_->clear ();
		QStandardItem *askItem = new QStandardItem (tr ("Always ask"));
		askItem->setEditable (false);
		askItem->setData (Ask, ActionRole);
		ActionsModel_->appendRow (askItem);
		QStandardItem *joinAllItem =
				new QStandardItem (tr ("Always join"));
		joinAllItem->setEditable (false);
		joinAllItem->setData (JoinAll, ActionRole);
		ActionsModel_->appendRow (joinAllItem);
		QStandardItem *ignoreAllItem =
				new QStandardItem (tr ("Always ignore"));
		ignoreAllItem->setEditable (false);
		ignoreAllItem->setData (IgnoreAll, ActionRole);
		ActionsModel_->appendRow (ignoreAllItem);

		Ui_.AlwaysAction_->setCurrentIndex (
				XmlSettingsManager::Instance ().Property (
					"InviteActionByDefault",
					ActionsModel_->item (
						Ui_.AlwaysAction_->currentIndex ())->
					data (ActionRole)).toInt ());
	}

	void InviteChannelsDialog::AddInvitation (const QString& channel,
			const QString& nick)
	{
		if (Model_->findItems (channel, Qt::MatchExactly, 1).count ())
			return;

		QStandardItem *joinItem = new QStandardItem;
		joinItem->setCheckable (true);
		joinItem->setEditable (false);
		QStandardItem *channelItem = new QStandardItem (channel);
		channelItem->setEditable (false);
		QStandardItem *nickItem = new QStandardItem (nick);
		nickItem->setEditable (false);
		Model_->appendRow (QList<QStandardItem*> () << joinItem
				<< channelItem
				<< nickItem);
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
		XmlSettingsManager::Instance ()
				.setProperty ("InviteActionByDefault",
				ActionsModel_->item (Ui_.AlwaysAction_->
					currentIndex ())->data (ActionRole).toInt ());
		QDialog::accept ();
	}

};
};
};
