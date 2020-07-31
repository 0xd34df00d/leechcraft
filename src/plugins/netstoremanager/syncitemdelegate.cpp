/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncitemdelegate.h"
#include <QComboBox>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QtDebug>
#include <QDir>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "accountsmanager.h"
#include "directorywidget.h"
#include "utils.h"

namespace LC
{
namespace NetStoreManager
{
	SyncItemDelegate::SyncItemDelegate (AccountsManager *am,
			QStandardItemModel *model, QObject *parent)
	: QItemDelegate (parent)
	, AM_ (am)
	, Model_ (model)
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
		case LocalDirectory:
		{
			DirectoryWidget *dw = new DirectoryWidget (DirectoryWidget::Type::Local,
					QByteArray (), 0, parent);
			dw->setAttribute (Qt::WA_DeleteOnClose);
			connect (dw,
					SIGNAL (finished (QWidget*)),
					this,
					SLOT (handleCloseDirectoryEditor (QWidget*)));
			return dw;
		}
		case RemoteDirectory:
		{
			DirectoryWidget *dw = new DirectoryWidget (DirectoryWidget::Type::Remote,
					index.sibling (index.row (), Account)
						.data (SyncItemDelegateRoles::AccountId).toByteArray (),
					AM_, parent);
			dw->setAttribute (Qt::WA_DeleteOnClose);
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
		case LocalDirectory:
		case RemoteDirectory:
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
		case LocalDirectory:
		case RemoteDirectory:
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
			const QStyleOptionViewItem& option, const QModelIndex&) const
	{
		editor->setGeometry (option.rect);
	}

	void SyncItemDelegate::FillAccounts (QComboBox *box) const
	{
		for (auto acc : AM_->GetAccounts ())
		{
			auto isp = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
			if (!isp)
				continue;

			box->addItem (Utils::GetStorageIcon (isp),
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
