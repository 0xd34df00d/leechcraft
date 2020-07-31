/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tocwidget.h"
#include <algorithm>
#include <iterator>
#include <QStandardItemModel>
#include <QtDebug>

namespace LC
{
namespace Monocle
{
	uint qHash (const LC::Monocle::ILink_ptr& link)
	{
		return ::qHash (link.get ());
	}

	TOCWidget::TOCWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.TOCTree_->setModel (Model_);
	}

	namespace
	{
		auto Tuplize (const IPageLink_ptr& link)
		{
			return std::make_tuple (link->GetPageNumber (),
					link->NewX (),
					link->NewY ());
		}
	}

	void TOCWidget::SetTOC (const TOCEntryLevel_t& topLevel)
	{
		setEnabled (!topLevel.isEmpty ());

		Item2Link_.clear ();
		Link2Item_.clear ();
		IntraDocPageLinks_.clear ();
		Model_->clear ();

		AddWorker (Model_, topLevel);

		std::sort (IntraDocPageLinks_.begin (), IntraDocPageLinks_.end (),
				[] (const auto& left, const auto& right)
				{
					return Tuplize (left) < Tuplize (right);
				});

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
			Item2Link_ [item] = entry.Link_;
			Link2Item_ [entry.Link_] = item;

			AddWorker (item, entry.ChildLevel_);

			addable->appendRow (item);

			if (const auto ipl = std::dynamic_pointer_cast<IPageLink> (entry.Link_))
				if (ipl->GetDocumentFilename ().isEmpty ())
					IntraDocPageLinks_ << ipl;
		}
	}

	void TOCWidget::updateCurrentPage (int index)
	{
		const auto linkPos = std::upper_bound (IntraDocPageLinks_.begin (),
				IntraDocPageLinks_.end (),
				index,
				[] (int index, const auto& link) { return index < link->GetPageNumber (); });
		if (linkPos == IntraDocPageLinks_.begin ())
			return;

		const auto item = Link2Item_.value (std::dynamic_pointer_cast<ILink> (*std::prev (linkPos)));
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no item for page"
					<< index;
			return;
		}

		Ui_.TOCTree_->setCurrentIndex (item->index ());
	}

	void TOCWidget::on_TOCTree__activated (const QModelIndex& index)
	{
		auto item = Model_->itemFromIndex (index);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid item for"
					<< index;
			return;
		}

		auto link = Item2Link_ [item];
		if (!link)
		{
			qWarning () << Q_FUNC_INFO
					<< "no link for item"
					<< item
					<< index;
			return;
		}

		link->Execute ();
	}
}
}
