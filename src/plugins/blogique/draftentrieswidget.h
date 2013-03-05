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
#include "interfaces/blogique/iaccount.h"
#include "ui_draftentrieswidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
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
		explicit DraftEntriesWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
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
