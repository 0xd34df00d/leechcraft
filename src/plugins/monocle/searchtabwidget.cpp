/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "searchtabwidget.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "textsearchhandler.h"

namespace LC
{
namespace Monocle
{
	namespace
	{
		enum class SearchModelRole
		{
			PageFirstIdx = Qt::UserRole + 1,
			OverallIdx
		};
	}

	SearchTabWidget::SearchTabWidget (TextSearchHandler *handler, QWidget *parent)
	: QWidget { parent }
	, Model_ { new QStandardItemModel { this } }
	, SearchHandler_ { handler }
	{
		Ui_.setupUi (this);
		Ui_.ResultsTree_->setModel (Model_);
		connect (handler,
				SIGNAL (gotSearchResults (TextSearchHandlerResults)),
				this,
				SLOT (handleSearchResults (TextSearchHandlerResults)));
	}

	void SearchTabWidget::HandleDoc (const IDocument_ptr&)
	{
		Model_->clear ();
		Root2Results_.clear ();
	}

	void SearchTabWidget::handleSearchResults (const TextSearchHandlerResults& results)
	{
		if (std::all_of (results.Positions_.begin (), results.Positions_.end (),
				[] (const QList<QRectF>& list) { return list.isEmpty (); }))
			return;

		QList<QStandardItem*> pageItems;
		int globalPosIdx = 0;
		for (const auto& pair : Util::Stlize (results.Positions_))
		{
			const auto& posList = pair.second;
			if (posList.isEmpty ())
				continue;

			const auto pageItem = new QStandardItem { tr ("Page %1").arg (pair.first + 1) };
			pageItem->setData (globalPosIdx, static_cast<int> (SearchModelRole::PageFirstIdx));
			pageItem->setEditable (false);
			for (int i = 0; i < posList.size (); ++i, ++globalPosIdx)
			{
				const auto posItem = new QStandardItem { tr ("Occurrence %1").arg (i + 1) };
				posItem->setData (globalPosIdx, static_cast<int> (SearchModelRole::OverallIdx));
				posItem->setEditable (false);
				pageItem->appendRow (posItem);
			}

			pageItems << pageItem;
		}

		if (pageItems.isEmpty ())
			return;

		const auto searchItem = new QStandardItem { results.Text_ };
		searchItem->appendRows (pageItems);
		searchItem->setEditable (false);

		Root2Results_ [searchItem] = results;

		Model_->insertRow (0, searchItem);
		Ui_.ResultsTree_->expand (searchItem->index ());
	}

	namespace
	{
		int GetPosIdx (const QStandardItem *item)
		{
			const auto overallIdxVar = item->data (static_cast<int> (SearchModelRole::OverallIdx));
			if (!overallIdxVar.isNull ())
				return overallIdxVar.toInt ();

			const auto pageIdxVar = item->data (static_cast<int> (SearchModelRole::PageFirstIdx));
			if (!pageIdxVar.isNull ())
				return pageIdxVar.toInt ();

			return 0;
		}
	}

	void SearchTabWidget::on_ResultsTree__activated (const QModelIndex& index)
	{
		const auto item = Model_->itemFromIndex (index);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown index"
					<< index;
			return;
		}

		auto root = item;
		while (const auto parent = root->parent ())
			root = parent;

		if (!Root2Results_.contains (root))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown root index for"
					<< index;
			return;
		}

		const auto results = Root2Results_.value (root);
		SearchHandler_->SetPreparedResults (results, GetPosIdx (item));
	}
}
}

