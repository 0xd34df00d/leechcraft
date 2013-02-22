/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#pragma once

#include <QWidget>
#include "ui_blogentrieswidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Blogique
{
	class EntriesFilterProxyModel;

	class BlogEntriesWidget : public QWidget
	{
		Q_OBJECT

		Ui::BlogEntriesWidget Ui_;
		IAccount *Account_;
		QStandardItemModel *BlogEntriesModel_;
		EntriesFilterProxyModel *FilterProxyModel_;
		QHash<QStandardItem*, Entry> Item2Entry_;
		QList<QAction*> LoadActions_;

	public:
		explicit BlogEntriesWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
		QString GetName () const;
		void SetAccount (IAccount *account);
	private:
		Entry LoadFullEntry (qint64 id);
		void FillView (const QList<Entry>& entries);
		void FillStatistic ();
		void FillCurrentTab (const QModelIndex& index = QModelIndex ());

	public slots:
		void clear ();
	private slots:
		void saveSplitterPosition (int pos, int index);
		void loadPostsByDate (const QDate& date);
		void handleOpenBlogEntryInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenBlogEntryInNewTab (const QModelIndex& index = QModelIndex ());
		void on_BlogEntriesFilter__textChanged (const QString& text);
		void on_RemoveBlogEntry__released ();
		void handleGotEntries (QObject *acc, const QList<Entry>& entries);
		void on_BlogEntriesView__doubleClicked (const QModelIndex& index);

	signals:
		void fillCurrentWidgetWithBlogEntry (const Entry& e);
		void fillNewWidgetWithBlogEntry (const Entry& e, const QByteArray& accId);
	};
}
}
