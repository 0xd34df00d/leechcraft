/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QCoreApplication>
#include "ui_annwidget.h"

namespace LC::Monocle
{
	class AnnManager;

	class AnnWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::AnnWidget)

		Ui::AnnWidget Ui_;
		AnnManager& Mgr_;
	public:
		explicit AnnWidget (AnnManager&, QWidget* = nullptr);
	private:
		void ShowContextMenu (QPoint);
		void FocusOnAnnotation (const QModelIndex&);
	};
}
