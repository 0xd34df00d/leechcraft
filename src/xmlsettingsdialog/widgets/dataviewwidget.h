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
	public:
		DataViewWidget (QWidget* = 0);

		void DisableAddition ();
		void DisableModification ();
		void DisableRemoval ();

		void AddCustomButton (const QByteArray& id, const QString& text);

		void SetModel (QAbstractItemModel*);
		QAbstractItemModel* GetModel () const;
		QModelIndex GetCurrentIndex () const;
		QModelIndexList GetSelectedRows () const;
	public slots:
		void resizeColumns ();
	private slots:
		void handleCustomButtonReleased ();
	signals:
		void addRequested ();
		void modifyRequested ();
		void removeRequested ();

		void customButtonReleased (const QByteArray&);
	};
}
