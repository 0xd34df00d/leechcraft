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
#include <util/sll/qtutil.h>

namespace LC::Monocle
{
	TOCWidget::TOCWidget (QWidget *parent)
	: QWidget { parent }
	, Model_ { new QStandardItemModel { this } }
	{
		Ui_.setupUi (this);
		Ui_.TOCTree_->setModel (Model_);

		connect (Ui_.TOCTree_,
				&QTreeView::activated,
				this,
				[this] (const QModelIndex& index)
				{
					auto item = Model_->itemFromIndex (index);
					auto linkPos = Item2Link_.find (item);
					if (linkPos == Item2Link_.end ())
					{
						qWarning () << "no link for item" << item << index;
						return;
					}

					emit navigationRequested (*linkPos);
				});
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

	void TOCWidget::SetCurrentPage (int index)
	{
		const auto linkPos = Link2Item_.upperBound ({ .PageNumber_ = index });
		if (linkPos == Link2Item_.begin ())
			return;

		const auto item = *std::prev (linkPos);
		Ui_.TOCTree_->setCurrentIndex (item->index ());
	}

	namespace
	{
		QString NormalizeName (QString name)
		{
			return name
					.replace ("\r\n"_qs, "\n"_qs)
					.replace ('\r', '\n');
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
}
