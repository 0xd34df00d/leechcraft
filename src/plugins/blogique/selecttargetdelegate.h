/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QItemDelegate>

namespace LC
{
namespace Blogique
{
	class SubmitToDialog;

	class SelectTargetDelegate : public QItemDelegate
	{
		Q_OBJECT

		SubmitToDialog *Dlg_;

	public:
		enum Role
		{
			TargetRole = Qt::UserRole + 1
		};

		explicit SelectTargetDelegate (SubmitToDialog *dlg, QObject *parent = 0);

		QWidget* createEditor (QWidget *parent,
		const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void setEditorData (QWidget *editor, const QModelIndex& index) const;
		void setModelData (QWidget *editor, QAbstractItemModel *model,
		const QModelIndex& index) const;
		void updateEditorGeometry (QWidget *editor,
		const QStyleOptionViewItem& option, const QModelIndex& index) const;
	};
}
}
