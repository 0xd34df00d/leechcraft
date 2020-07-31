/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "compparamswidget.h"
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QtDebug>
#include <util/sll/curry.h>
#include "compparamsmanager.h"

namespace LC::Fenet
{
	namespace
	{
		class EditDelegate : public QStyledItemDelegate
		{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
			void setEditorData (QWidget*, const QModelIndex&) const override;
			void setModelData (QWidget*, QAbstractItemModel*, const QModelIndex&) const override;
		};

		QWidget* EditDelegate::createEditor (QWidget *parent,
				const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			const auto& var = index.data (CompParamsManager::Role::Description);
			if (!var.canConvert<Param> ())
				return QStyledItemDelegate::createEditor (parent, option, index);

			return new QDoubleSpinBox (parent);
		}

		void EditDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
		{
			const auto& var = index.data (CompParamsManager::Role::Description);
			if (!var.canConvert<Param> ())
				return QStyledItemDelegate::setEditorData (editor, index);

			const auto& param = var.value<Param> ();

			auto box = qobject_cast<QDoubleSpinBox*> (editor);
			box->setValue (index.data (Qt::EditRole).toDouble ());
			box->setMinimum (param.Min_);
			box->setMaximum (param.Max_);
		}

		void EditDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
		{
			const auto& var = index.data (CompParamsManager::Role::Description);
			if (!var.canConvert<Param> ())
				return QStyledItemDelegate::setModelData (editor, model, index);

			auto box = qobject_cast<QDoubleSpinBox*> (editor);
			const auto value = box->value ();

			model->setData (index, value);
		}
	}

	CompParamsWidget::CompParamsWidget (QWidget *parent)
	: QTreeView (parent)
	{
		setItemDelegate (new EditDelegate (this));
		setRootIsDecorated (false);

		const auto& fm = fontMetrics ();
		setColumnWidth (0, fm.horizontalAdvance ("Average compositor option description length"));
		setColumnWidth (1, fm.horizontalAdvance ("average value"));
		setColumnWidth (2, fm.horizontalAdvance ("flag"));
	}

	void CompParamsWidget::accept ()
	{
		emit accepted ();
	}

	void CompParamsWidget::reject ()
	{
		emit rejected ();
	}
}
