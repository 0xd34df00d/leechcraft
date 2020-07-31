/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "interfaces/blogique/iaccount.h"
#include "ui_blogentrieswidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
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
		explicit BlogEntriesWidget (QWidget *parent = 0);
		QString GetName () const;
		void SetAccount (IAccount *account);
	private:
		Entry GetEntry (const QModelIndex& index = QModelIndex ());
		void FillCurrentTab (const QModelIndex& index = QModelIndex ());

	public slots:
		void clear ();
	private slots:
		void fillView (const QList<Entry>& entries);
		void fillStatistic (const QMap<QDate, int>& statistics);
		void saveSplitterPosition (int pos, int index);
		void loadPostsByDate (const QDate& date);
		void handleOpenBlogEntryInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenBlogEntryInNewTab (const QModelIndex& index = QModelIndex ());
		void on_BlogEntriesFilter__textChanged (const QString& text);
		void on_RemoveBlogEntry__released ();
		void on_BlogEntriesView__doubleClicked (const QModelIndex& index);
		void handleCalendarVisibilityChanged (bool visible);

	signals:
		void fillCurrentWidgetWithBlogEntry (const Entry& e);
		void fillNewWidgetWithBlogEntry (const Entry& e, const QByteArray& accId);

		void entryAboutToBeRemoved ();
		void entriesListUpdated ();
	};
}
}
