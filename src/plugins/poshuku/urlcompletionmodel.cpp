/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urlcompletionmodel.h"
#include <stdexcept>
#include <QUrl>
#include <QTimer>
#include <QApplication>
#include <QtDebug>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/core/icoreproxy.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
	URLCompletionModel::URLCompletionModel (QObject *parent)
	: QAbstractItemModel { parent }
	, ValidateTimer_ { new QTimer { this } }
	{
		ValidateTimer_->setSingleShot (true);
		connect (ValidateTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (validate ()));
		ValidateTimer_->setInterval (QApplication::keyboardInputInterval () / 2);
	}

	int URLCompletionModel::columnCount (const QModelIndex&) const
	{
		return 1;
	}

	QVariant URLCompletionModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		if (role == Qt::DisplayRole)
			return Items_ [index.row ()].Title_ + " [" + Items_ [index.row ()].URL_ + "]";
		else if (role == Qt::DecorationRole)
			return Core::Instance ().GetIcon (QUrl (Items_ [index.row ()].URL_));
		else if (role == Qt::EditRole)
			return Base_;
		else if (role == RoleURL)
			return Items_ [index.row ()].URL_;
		else
			return {};
	}

	Qt::ItemFlags URLCompletionModel::flags (const QModelIndex&) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant URLCompletionModel::headerData (int, Qt::Orientation, int) const
	{
		return {};
	}

	QModelIndex URLCompletionModel::index (int row, int column,
			const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		return createIndex (row, column);
	}

	QModelIndex URLCompletionModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int URLCompletionModel::rowCount (const QModelIndex& index) const
	{
		if (index.isValid ())
			return 0;

		return Items_.size ();
	}

	void URLCompletionModel::AddItem (const QString& title, const QString& url, size_t pos)
	{
		const HistoryItem item
		{
			title,
			QDateTime (),
			url
		};

		pos = std::min (static_cast<size_t> (Items_.size ()), pos);

		beginInsertRows ({}, pos, pos);
		Items_.insert (Items_.begin () + pos, item);
		endInsertRows ();
	}

	void URLCompletionModel::setBase (const QString& str)
	{
		Valid_ = false;
		Base_ = str;

		ValidateTimer_->stop ();
		ValidateTimer_->start ();
	}

	void URLCompletionModel::validate ()
	{
		PopulateNonHook ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		int size = Items_.size ();
		emit hookURLCompletionNewStringRequested (proxy, this, Base_, size);
		if (!proxy->IsCancelled ())
			return;

		if (size)
		{
			beginRemoveRows ({}, 0, size - 1);
			Items_.erase (Items_.begin (), Items_.begin () + size);
			endRemoveRows ();
		}
	}

	void URLCompletionModel::handleItemAdded (const HistoryItem&)
	{
		Valid_ = false;
	}

	void URLCompletionModel::PopulateNonHook ()
	{
		if (Valid_)
			return;

		Valid_ = true;

		if (!Items_.isEmpty ())
		{
			int finalIdx = Items_.size () - 1;
			beginRemoveRows ({}, 0, finalIdx);
			Items_.clear ();
			endRemoveRows ();
		}

		history_items_t newItems;
		if (Base_.startsWith ('!'))
		{
			auto cats = Core::Instance ().GetProxy ()->GetSearchCategories ();
			cats.sort ();
			for (const auto& cat : cats)
				newItems.push_back ({ cat, {}, "!" + cat });
		}
		else
		{
			try
			{
				newItems = Core::Instance ().GetStorageBackend ()->LoadResemblingHistory (Base_);
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO << e.what ();
				Valid_ = false;
			}
		}

		if (newItems.isEmpty ())
			return;

		beginInsertRows ({}, 0, newItems.size () - 1);
		Items_ = std::move (newItems);
		endInsertRows ();
	}
}
}
