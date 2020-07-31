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
	DataViewWidget::DataViewWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.Add_->setProperty ("ActionIcon", "list-add");
		Ui_.Modify_->setProperty ("ActionIcon", "configure");
		Ui_.Remove_->setProperty ("ActionIcon", "list-remove");

		setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		connect (Ui_.Add_,
				SIGNAL (released ()),
				this,
				SIGNAL (addRequested ()));
		connect (Ui_.Modify_,
				SIGNAL (released ()),
				this,
				SIGNAL (modifyRequested ()));
		connect (Ui_.Remove_,
				SIGNAL (released ()),
				this,
				SIGNAL (removeRequested ()));
	}

	void DataViewWidget::DisableAddition ()
	{
		Ui_.Add_->setEnabled (false);
	}

	void DataViewWidget::DisableModification ()
	{
		Ui_.Modify_->setEnabled (false);
	}

	void DataViewWidget::DisableRemoval ()
	{
		Ui_.Remove_->setEnabled (false);
	}

	void DataViewWidget::AddCustomButton (const QByteArray& id, const QString& text)
	{
		auto button = new QPushButton (text);
		button->setProperty ("XSD/Id", id);
		Ui_.ButtonsLayout_->insertWidget (Ui_.ButtonsLayout_->count () - 1, button);

		connect (button,
				SIGNAL (released ()),
				this,
				SLOT (handleCustomButtonReleased ()));
	}

	void DataViewWidget::SetModel (QAbstractItemModel *model)
	{
		Ui_.View_->setModel (model);
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

	void DataViewWidget::handleCustomButtonReleased ()
	{
		auto button = qobject_cast<QPushButton*> (sender ());
		emit customButtonReleased (button->property ("XSD/Id").toByteArray ());
	}
}
