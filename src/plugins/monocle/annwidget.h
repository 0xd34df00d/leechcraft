/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_annwidget.h"

namespace LC
{
namespace Monocle
{
	class AnnManager;

	class AnnWidget : public QWidget
	{
		Q_OBJECT

		Ui::AnnWidget Ui_;
		AnnManager * const Mgr_;
	public:
		AnnWidget (AnnManager*, QWidget* = nullptr);
	private slots:
		void on_AnnTree__customContextMenuRequested (const QPoint&);
		void focusOnAnnotation (const QModelIndex&);
	};
}
}
