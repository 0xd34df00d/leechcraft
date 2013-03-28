/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "tocwidget.h"
#include <QStandardItemModel>
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
	TOCWidget::TOCWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.TOCTree_->setModel (Model_);
	}

	void TOCWidget::SetTOC (const TOCEntryLevel_t& topLevel)
	{
		setEnabled (!topLevel.isEmpty ());

		Item2Link_.clear ();
		Model_->clear ();

		AddWorker (Model_, topLevel);

		Ui_.TOCTree_->expandToDepth (0);
	}

	template<typename T>
	void TOCWidget::AddWorker (T addable, const TOCEntryLevel_t& level)
	{
		Q_FOREACH (const auto& entry, level)
		{
			auto item = new QStandardItem (entry.Name_);
			item->setToolTip (entry.Name_);
			item->setEditable (false);
			Item2Link_ [item] = entry.Link_;

			AddWorker (item, entry.ChildLevel_);

			addable->appendRow (item);
		}
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
