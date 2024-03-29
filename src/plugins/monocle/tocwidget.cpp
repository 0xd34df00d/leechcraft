/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tocwidget.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "documenttab.h"

namespace LC
{
namespace Monocle
{
	TOCWidget::TOCWidget (DocumentTab& docTab, QWidget *parent)
	: QWidget { parent }
	, DocTab_ { docTab }
	, Model_ { new QStandardItemModel { this } }
	{
		Ui_.setupUi (this);
		Ui_.TOCTree_->setModel (Model_);
	}

	void TOCWidget::SetTOC (const TOCEntryLevel_t& topLevel)
	{
		setEnabled (!topLevel.isEmpty ());

		Item2Link_.clear ();
		Link2Item_.clear ();
		Model_->clear ();

		AddWorker (Model_, topLevel);

		Ui_.TOCTree_->expandToDepth (0);
	}

	namespace
	{
		QString NormalizeName (QString name)
		{
			return name
					.replace ("\r\n", "\n")
					.replace ("\r", "\n");
		}
	}

	template<typename T>
	void TOCWidget::AddWorker (T addable, const TOCEntryLevel_t& level)
	{
		for (const auto& entry : level)
		{
			const auto& name = NormalizeName (entry.Name_);

			auto item = new QStandardItem (QString { name }.replace ('\n', ' '));
			item->setToolTip (name);
			item->setEditable (false);
			Item2Link_ [item] = entry.Navigation_;
			Link2Item_ [entry.Navigation_] = item;

			AddWorker (item, entry.ChildLevel_);

			addable->appendRow (item);
		}
	}

	void TOCWidget::updateCurrentPage (int index)
	{
		const auto linkPos = Link2Item_.upperBound ({ .PageNumber_ = index });
		if (linkPos == Link2Item_.begin ())
			return;

		const auto item = *(linkPos - 1);
		Ui_.TOCTree_->setCurrentIndex (item->index ());
	}

	void TOCWidget::on_TOCTree__activated (const QModelIndex& index)
	{
		auto item = Model_->itemFromIndex (index);
		auto linkPos = Item2Link_.find (item);
		if (linkPos == Item2Link_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no link for item"
					<< item
					<< index;
			return;
		}

		DocTab_.Navigate (*linkPos);
	}
}
}
