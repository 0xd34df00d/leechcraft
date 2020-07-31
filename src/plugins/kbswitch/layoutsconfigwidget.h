/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_layoutsconfigwidget.h"

class QStandardItemModel;

namespace LC
{
namespace KBSwitch
{
	class LayoutsConfigWidget : public QWidget
	{
		Q_OBJECT

		Ui::LayoutsConfigWidget Ui_;
		QStandardItemModel *AvailableModel_;
		QStandardItemModel *EnabledModel_;

		QList<QStringList> Layouts_;
	public:
		enum EnabledColumn
		{
			EnabledCode,
			EnabledDesc,
			EnabledVariant
		};

		LayoutsConfigWidget (QWidget* = 0);
	private:
		void FillModels ();
	public slots:
		void accept ();
		void reject ();
	private slots:
		void on_Enable__released ();
		void on_Disable__released ();

		void updateActionsState ();
	};
}
}
