/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_subscriptionadddialog.h"

class QStandardItem;
class QDomElement;

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	class SubscriptionAddDialog : public QDialog
	{
		Q_OBJECT

		Ui::SubscriptionAddDialog Ui_;
		QList<QStandardItem*> Items_;
	public:
		SubscriptionAddDialog (QWidget* = 0);

		QString GetURL () const;
		QString GetName () const;

		QList<QUrl> GetAdditionalSubscriptions () const;
	private:
		void Iterate (const QDomElement&, QStandardItem*);
	};
}
}
}
