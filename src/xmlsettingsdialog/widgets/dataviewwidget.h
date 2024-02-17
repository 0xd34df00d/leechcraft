/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_dataviewwidget.h"

namespace LC
{
	class DataViewWidget : public QWidget
	{
		Q_OBJECT

		Ui::DataViewWidget Ui_;

		const bool Autoresize_;
	public:
		enum Option
		{
			None       = 0b0000,
			Autoresize = 0b0001,
			NoAdd      = 0b0010,
			NoModify   = 0b0100,
			NoRemove   = 0b1000,
		};
		Q_DECLARE_FLAGS (Options, Option)
		Q_FLAGS (Options)

		explicit DataViewWidget (Options opts, QWidget* = nullptr);

		void AddCustomButton (const QByteArray& id, const QString& text);

		void SetModel (QAbstractItemModel*);
		QAbstractItemModel* GetModel () const;
		QModelIndex GetCurrentIndex () const;
		QModelIndexList GetSelectedRows () const;
	public slots:
		void resizeColumns ();
	signals:
		void addRequested ();
		void modifyRequested ();
		void removeRequested ();

		void customButtonReleased (const QByteArray&);
	};
}
