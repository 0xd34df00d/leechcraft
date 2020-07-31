/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/poshuku/iurlcompletionmodel.h>
#include "historymodel.h"

class QTimer;

namespace LC
{
namespace Poshuku
{
	class URLCompletionModel : public QAbstractItemModel
							 , public IURLCompletionModel
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::IURLCompletionModel)

		mutable bool Valid_ = false;
		mutable history_items_t Items_;

		QString Base_;

		QTimer * const ValidateTimer_;
	public:
		enum
		{
			RoleURL = Qt::UserRole + 1
		};
		URLCompletionModel (QObject* = 0);

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;

		void AddItem (const QString& title, const QString& url, size_t pos) override;
	private:
		void PopulateNonHook ();
	private slots:
		void validate ();
	public slots:
		void setBase (const QString&);
		void handleItemAdded (const HistoryItem&);
	signals:
		// Plugin API
		void hookURLCompletionNewStringRequested (LC::IHookProxy_ptr proxy,
				QObject *model,
				const QString& string,
				int historyItems);
	};
}
}
