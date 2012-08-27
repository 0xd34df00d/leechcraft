/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "syncitemdelegate.h"
#include <QComboBox>
#include <QtDebug>
#include "accountsmanager.h"
#include "directorywidget.h"
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	SyncItemDelegate::SyncItemDelegate (AccountsManager *am, QObject *parent)
	: QItemDelegate (parent)
	, AM_ (am)
	{
	}

	QWidget* SyncItemDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column ())
		{
			case Account:
			{
				QComboBox *box = new QComboBox (parent);
				FillAccounts (box);
				return box;
			}
			case Directory:
			{
				DirectoryWidget *dw = new DirectoryWidget (parent);
				connect (dw,
						SIGNAL (finished (QWidget*)),
						this,
						SLOT (handleCloseDirectoryEditor (QWidget*)));
				return dw;
			}
			default:
				return QItemDelegate::createEditor (parent, option, index);
		}
	}

	void SyncItemDelegate::setEditorData (QWidget *editor,
			const QModelIndex& index) const
	{
		switch (index.column ())
		{
			case Account:
			{
				auto box = static_cast<QComboBox*> (editor);
				QString accText = index.data (Qt::EditRole).toString ();
				box->setCurrentIndex (box->findText (accText, Qt::MatchExactly));
				break;
			}
			case Directory:
			{
				auto dw = static_cast<DirectoryWidget*> (editor);
				dw->SetPath (index.data (Qt::EditRole).toString ());
				break;
			}
			default:
				QItemDelegate::setEditorData (editor, index);
		}
	}

	void SyncItemDelegate::setModelData (QWidget *editor,
			QAbstractItemModel *model, const QModelIndex& index) const
	{
		switch (index.column ())
		{
			case Account:
			{
				auto box = static_cast<QComboBox*> (editor);

				model->setData (index, box->currentText (), Qt::EditRole);
				model->setData (index,
						box->itemData (box->currentIndex (),
								SyncItemDelegateRoles::AccountId),
						SyncItemDelegateRoles::AccountId);
				break;
			}
			case Directory:
			{
				auto dw = static_cast<DirectoryWidget*> (editor);

				model->setData (index, dw->GetPath (), Qt::EditRole);
				break;
			}
			default:
				QItemDelegate::setModelData (editor, model, index);
		}
	}

	void SyncItemDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		editor->setGeometry (option.rect);
	}

	void SyncItemDelegate::FillAccounts (QComboBox *box) const
	{
		const auto& accounts = AM_->GetAccounts ();
		for (auto acc : accounts)
		{
			auto isp = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
			if (!isp)
				continue;

			box->addItem (isp->GetStorageIcon (),
					isp->GetStorageName() + ": " + acc->GetAccountName ());
			box->setItemData (box->count () - 1,
					acc->GetUniqueID (),
					SyncItemDelegateRoles::AccountId);
		}
	}

	void SyncItemDelegate::handleCloseDirectoryEditor (QWidget *w)
	{
		emit commitData (w);
		emit closeEditor (w);
	}

}
}
