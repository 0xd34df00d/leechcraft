/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_SUBSCRIPTIONADDDIALOG_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_SUBSCRIPTIONADDDIALOG_H
#include <QDialog>
#include "ui_subscriptionadddialog.h"

class QStandardItem;
class QDomElement;

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
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
}
}

#endif
