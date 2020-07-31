/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_subscriptionsmanagerwidget.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	class Core;
	class SubscriptionsModel;

	class SubscriptionsManagerWidget : public QWidget
	{
		Q_OBJECT

		Ui::SubscriptionsManagerWidget Ui_;

		Core * const Core_;
		SubscriptionsModel * const Model_;
	public:
		SubscriptionsManagerWidget (Core*, SubscriptionsModel*, QWidget* = 0);
	private:
		void AddCustom (const QString&, const QString&);
	private slots:
		void on_AddButton__released ();
		void on_RemoveButton__released ();
	};
}
}
}
