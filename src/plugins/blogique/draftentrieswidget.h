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
#include "ui_draftentrieswidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Blogique
{
	class IBloggingPlatform;
	class EntriesFilterProxyModel;

	class DraftEntriesWidget : public QWidget
	{
		Q_OBJECT

		Ui::DraftEntriesWidget Ui_;

		QStandardItemModel *DraftEntriesModel_;
		EntriesFilterProxyModel *FilterProxyModel_;
		QHash<QStandardItem*, Entry> Item2Entry_;

	public:
		explicit DraftEntriesWidget (QWidget *parent = 0);
		QString GetName () const;
	private:
		void FillView (const QList<Entry>& entries);
		void FillStatistic ();
		void FillCurrentTab (const QModelIndex& index = QModelIndex ());

		void RemoveDraftEntry (qint64 id);

	public slots:
		void clear ();
		void loadDraftEntries ();
	private slots:
		void saveSplitterPosition (int pos, int index);
		void loadDraftsByDate (const QDate& date);
		void handleOpenDraftEntryInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenDraftEntryInNewTab (const QModelIndex& index = QModelIndex ());
		void on_DraftEntriesFilter__textChanged (const QString& text);
		void on_RemoveDraftEntry__released ();
		void on_PublishDraftEntry__released ();
		void on_DraftEntriesView__doubleClicked (const QModelIndex& index);
		void handleCalendarVisibilityChanged (bool visible);

	signals:
		void fillCurrentWidgetWithDraftEntry (const Entry& e);
		void fillNewWidgetWithDraftEntry (const Entry& e, const QByteArray& array = QByteArray ());
	};
}
}
