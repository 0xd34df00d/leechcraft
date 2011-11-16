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

#ifndef PLUGINS_SNAILS_MAILTAB_H
#define PLUGINS_SNAILS_MAILTAB_H
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_mailtab.h"
#include "account.h"

class QStandardItemModel;
class QStandardItem;
class QSortFilterProxyModel;

namespace LeechCraft
{
namespace Snails
{
	class MailTab : public QWidget
				  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::MailTab Ui_;

		QToolBar *TabToolbar_;

		TabClassInfo TabClass_;
		QObject *PMT_;

		QStandardItemModel *MailModel_;
		QHash<QByteArray, QStandardItem*> MailID2Item_;
		QSortFilterProxyModel *MailSortFilterModel_;
		Account_ptr CurrAcc_;

		enum Columns
		{
			From,
			Subj,
			Date,
			Size,
			Max
		};
	public:
		enum Roles
		{
			ID = Qt::UserRole + 1,
			Sort,
			ReadStatus
		};

		MailTab (const TabClassInfo&, QObject*, QWidget* = 0);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private slots:
		void handleCurrentAccountChanged (const QModelIndex&);
		void handleMailSelected (const QModelIndex&);
		void handleFetchNewMail ();
		void handleMessageBodyFetched (Message_ptr);
		void handleGotNewMessages (QList<Message_ptr>);
		void updateReadStatus (const QByteArray&, bool);
	signals:
		void removeTab (QWidget*);
	};
}
}

#endif
