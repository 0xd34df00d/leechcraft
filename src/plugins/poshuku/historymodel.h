/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vector>
#include <QStringList>
#include <QDateTime>
#include <QStandardItemModel>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/poshuku/poshukutypes.h>

class QTimer;
class QAction;

namespace LC
{
namespace Poshuku
{
	class HistoryModel : public QStandardItemModel
	{
		Q_OBJECT

		QTimer * const GarbageTimer_;
		history_items_t Items_;
	public:
		enum Columns
		{
			ColumnTitle
			, ColumnURL
			, ColumnDate
		};

		enum Roles
		{
			URL = Qt::UserRole + 1
		};

		HistoryModel (QObject* = nullptr);

		void HandleStorageReady ();
	public slots:
		void addItem (QString title, QString url, QDateTime datetime);
		QList<QMap<QString, QVariant>> getItemsMap () const;
	private:
		void Add (const HistoryItem&, int section);
	private slots:
		void loadData ();
		void collectGarbage ();
		void handleItemAdded (const HistoryItem&);
	signals:
		// Hook support signals
		/** @brief Called when an entry is going to be added to
			* history.
			*
			* If the proxy is cancelled, no addition takes place
			* at all. If it is not, the return value from the proxy
			* is considered as a list of QVariants. First element
			* (if any) would be converted to string and replace
			* title, second element (if any) would be converted to
			* string and replace url, third element (if any) would
			* be converted to QDateTime and replace the date.
			*/
		void hookAddingToHistory (LC::IHookProxy_ptr proxy,
				QString title, QString url, QDateTime date);
	};
}
}

Q_DECLARE_METATYPE (LC::Poshuku::HistoryItem)
