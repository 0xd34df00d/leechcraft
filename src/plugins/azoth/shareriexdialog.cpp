/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "shareriexdialog.h"
#include <QStandardItemModel>
#include "interfaces/iclentry.h"
#include "interfaces/iaccount.h"
#include "interfaces/iprotocol.h"
#include <QSortFilterProxyModel>

Q_DECLARE_METATYPE (LeechCraft::Azoth::ICLEntry*);

namespace LeechCraft
{
namespace Azoth
{
	ShareRIEXDialog::ShareRIEXDialog (ICLEntry *entry, QWidget *parent)
	: QDialog (parent)
	, Entry_ (entry)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		const QString& name = entry->GetEntryName ();
		const QString& hrID = entry->GetHumanReadableID ();
		Ui_.MessageLabel_->setText (tr ("Select items to be shared with %1:")
					.arg (name.isEmpty () ? hrID : name + " (" + hrID + ")"));

		connect (Ui_.AllAccountsBox_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (fillModel ()));

		fillModel ();

		QSortFilterProxyModel *proxy = new QSortFilterProxyModel (this);
		proxy->setSourceModel (Model_);
		Ui_.ContactsTree_->setModel (proxy);

		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (const QString&)),
				proxy,
				SLOT (setFilterFixedString (const QString&)));
	}

	QList<ICLEntry*> ShareRIEXDialog::GetSelectedEntries () const
	{
		QList<ICLEntry*> result;

		for (int i = 0, iSize = Model_->rowCount ();
				i < iSize; ++i)
		{
			QStandardItem *groupItem = Model_->item (i);
			for (int j = 0, jSize = groupItem->rowCount ();
					j < jSize; ++j)
			{
				QStandardItem *entryItem = groupItem->child (j);
				if (entryItem->checkState () == Qt::Checked )
					result << entryItem->data ().value<ICLEntry*> ();
			}
		}

		return result;
	}

	QString ShareRIEXDialog::GetMessage () const
	{
		return Ui_.Message_->toPlainText ();
	}

	bool ShareRIEXDialog::ShouldSuggestGroups () const
	{
		return Ui_.SuggestGroupsBox_->checkState () == Qt::Checked;
	}

	namespace
	{
		QMap<QString, QList<ICLEntry*> > GetEntries (IAccount *acc)
		{
			QMap<QString, QList<ICLEntry*> > result;

			Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
			{
				ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry ||
						(entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
					continue;

				if (entry->Groups ().isEmpty ())
					result [""] << entry;
				else
					Q_FOREACH (const QString& group, entry->Groups ())
						result [group] << entry;
			}

			return result;
		}
	}

	void ShareRIEXDialog::fillModel ()
	{
		Model_->clear ();

		Model_->setHorizontalHeaderLabels (QStringList (tr ("Name"))
						<< tr ("ID")
						<< tr ("Account")
						<< tr ("Groups"));

		IAccount *acc = qobject_cast<IAccount*> (Entry_->GetParentAccount ());

		QMap<QString, QList<ICLEntry*> > entries = GetEntries (acc);
		if (Ui_.AllAccountsBox_->checkState () == Qt::Checked)
		{
			IProtocol *proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
			Q_FOREACH (QObject *accObj, proto->GetRegisteredAccounts ())
			{
				IAccount *otherAcc = qobject_cast<IAccount*> (accObj);
				if (!otherAcc || otherAcc == acc)
					continue;

				const QMap<QString, QList<ICLEntry*> > others = GetEntries (otherAcc);
				Q_FOREACH (const QString& key, others.keys ())
					entries [key] << others [key];
			}
		}

		Q_FOREACH (const QString& group, entries.keys ())
		{
			const QString& title = group.isEmpty () ? tr ("General") : group;
			QStandardItem *groupItem = new QStandardItem (title);

			Q_FOREACH (ICLEntry *entry, entries [group])
			{
				QList<QStandardItem*> row;

				QStandardItem *itemName = new QStandardItem;
				itemName->setCheckState (Qt::Unchecked);
				itemName->setCheckable (true);
				itemName->setText (entry->GetEntryName ().isEmpty () ?
							entry->GetHumanReadableID () :
							entry->GetEntryName ());
				itemName->setData (QVariant::fromValue<ICLEntry*> (entry));
				row << itemName;

				row << new QStandardItem (entry->GetHumanReadableID ());
				const QString& accName = qobject_cast<IAccount*> (entry->
							GetParentAccount ())->GetAccountName ();
				row << new QStandardItem (accName);
				row << new QStandardItem (entry->Groups ().join ("; "));

				groupItem->appendRow (row);
			}

			Model_->appendRow (groupItem);
		}
	}
}
}
