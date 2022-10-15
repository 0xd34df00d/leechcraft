/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shareriexdialog.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iprotocol.h"

Q_DECLARE_METATYPE (LC::Azoth::ICLEntry*);

namespace LC
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

	QString ShareRIEXDialog::GetShareMessage () const
	{
		return Ui_.Message_->toPlainText ();
	}

	bool ShareRIEXDialog::ShouldSuggestGroups () const
	{
		return Ui_.SuggestGroupsBox_->checkState () == Qt::Checked;
	}

	namespace
	{
		QMap<QString, QList<ICLEntry*>> GetEntries (IAccount *acc)
		{
			QMap<QString, QList<ICLEntry*>> result;

			for (const auto entryObj : acc->GetCLEntries ())
			{
				ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry ||
						(entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
					continue;

				if (entry->Groups ().isEmpty ())
					result [""] << entry;
				else
					for (const auto& group : entry->Groups ())
						result [group] << entry;
			}

			return result;
		}
	}

	void ShareRIEXDialog::fillModel ()
	{
		Model_->clear ();

		Model_->setHorizontalHeaderLabels ({
					tr ("Name"),
					tr ("ID"),
					tr ("Account"),
					tr ("Groups")
				});

		const auto acc = Entry_->GetParentAccount ();

		auto entries = GetEntries (acc);
		if (Ui_.AllAccountsBox_->checkState () == Qt::Checked)
		{
			const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
			for (const auto accObj : proto->GetRegisteredAccounts ())
			{
				const auto otherAcc = qobject_cast<IAccount*> (accObj);
				if (!otherAcc || otherAcc == acc)
					continue;

				const auto others = GetEntries (otherAcc);
				for (const auto& key : others.keys ())
					entries [key] << others [key];
			}
		}

		for (const QString& group : entries.keys ())
		{
			const QString& title = group.isEmpty () ? tr ("General") : group;
			QStandardItem *groupItem = new QStandardItem (title);

			for (ICLEntry *entry : entries [group])
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
				row << new QStandardItem (entry->GetParentAccount ()->GetAccountName ());
				row << new QStandardItem (entry->Groups ().join ("; "));

				groupItem->appendRow (row);
			}

			Model_->appendRow (groupItem);
		}
	}
}
}
