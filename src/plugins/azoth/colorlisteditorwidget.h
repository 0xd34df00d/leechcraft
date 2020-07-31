/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_colorlisteditorwidget.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
	class ColorListEditorWidget : public QWidget
	{
		Q_OBJECT

		Ui::ColorListEditorWidget Ui_;
		QStandardItemModel *Model_;
	public:
		ColorListEditorWidget (QWidget* = 0);
	private:
		void AddColor (const QColor&);
	public slots:
		void accept ();
		void reject ();
	private slots:
		void on_AddColorButton__released ();
		void on_RemoveColorButton__released ();
	};
}
}
