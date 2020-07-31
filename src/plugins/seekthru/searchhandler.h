/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QAbstractItemModel>
#include <QUrl>
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>
#include "description.h"

class QToolBar;
class QAction;

class IEntityManager;

namespace LC
{
namespace Util
{
	class SelectableBrowser;
};

namespace SeekThru
{
	class SearchHandler : public QAbstractItemModel
	{
		Q_OBJECT

		static const QString OS_;

		Description D_;

		IEntityManager * const IEM_;

		QString SearchString_;
		struct Result
		{
			enum Type
			{
				TypeRSS,
				TypeAtom,
				TypeHTML
			} Type_ = TypeHTML;

			int TotalResults_ = 0;
			int StartIndex_ = 0;
			int ItemsPerPage_ = 0;
			QString Response_;
			QUrl RequestURL_;
		};

		QList<Result> Results_;
		QMap<int, Result> Jobs_;
		std::shared_ptr<Util::SelectableBrowser> Viewer_;
		std::shared_ptr<QToolBar> Toolbar_;
		std::shared_ptr<QAction> Action_;
	public:
		SearchHandler (const Description&, IEntityManager*);

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = QModelIndex ()) const override;

		void Start (const Request&);
	private:
		void HandleJobFinished (Result, const QString&);
	private slots:
		void subscribe ();
	signals:
		void error (const QString&);
		void warning (const QString&);
	};

	typedef std::shared_ptr<SearchHandler> SearchHandler_ptr;
}
}
