/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dataviewwidget.h"

namespace LC
{
	DataViewWidget::DataViewWidget (Options opts, QWidget *parent)
	: QWidget { parent }
	, Autoresize_ { static_cast<bool> (opts & Option::Autoresize) }
	{
		Ui_.setupUi (this);
		Ui_.Add_->setProperty ("ActionIcon", "list-add");
		Ui_.Modify_->setProperty ("ActionIcon", "configure");
		Ui_.Remove_->setProperty ("ActionIcon", "list-remove");

		setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		if (opts & Option::NoAdd)
			Ui_.Add_->setEnabled (false);
		if (opts & Option::NoModify)
			Ui_.Modify_->setEnabled (false);
		if (opts & Option::NoRemove)
			Ui_.Remove_->setEnabled (false);

		connect (Ui_.Add_,
				&QPushButton::released,
				this,
				&DataViewWidget::addRequested);
		connect (Ui_.Modify_,
				&QPushButton::released,
				this,
				&DataViewWidget::modifyRequested);
		connect (Ui_.Remove_,
				&QPushButton::released,
				this,
				&DataViewWidget::removeRequested);
	}

	void DataViewWidget::AddCustomButton (const QByteArray& id, const QString& text)
	{
		auto button = new QPushButton (text);
		Ui_.ButtonsLayout_->insertWidget (Ui_.ButtonsLayout_->count () - 1, button);

		connect (button,
				&QPushButton::released,
				this,
				[this, id] { emit customButtonReleased (id); });
	}

	void DataViewWidget::SetModel (QAbstractItemModel *model)
	{
		if (const auto prevModel = Ui_.View_->model ())
			disconnect (prevModel, nullptr, this, nullptr);

		Ui_.View_->setModel (model);
		if (Autoresize_)
		{
			connect (model,
					&QAbstractItemModel::rowsInserted,
					this,
					&DataViewWidget::resizeColumns);
			connect (model,
					&QAbstractItemModel::dataChanged,
					this,
					&DataViewWidget::resizeColumns);
		}
		resizeColumns ();
	}

	QAbstractItemModel* DataViewWidget::GetModel () const
	{
		return Ui_.View_->model ();
	}

	QModelIndex DataViewWidget::GetCurrentIndex () const
	{
		return Ui_.View_->currentIndex ();
	}

	QModelIndexList DataViewWidget::GetSelectedRows () const
	{
		return Ui_.View_->selectionModel ()->selectedRows ();
	}

	void DataViewWidget::resizeColumns ()
	{
		Ui_.View_->expandAll ();
		for (auto i = 0; i < GetModel ()->columnCount (); ++i)
			Ui_.View_->resizeColumnToContents (i);
	}
}
