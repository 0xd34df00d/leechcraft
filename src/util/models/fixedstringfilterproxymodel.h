/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>
#include "modelsconfig.h"

namespace LC::Util
{
	class UTIL_MODELS_API FixedStringFilterProxyModel : public QSortFilterProxyModel
	{
		QString FilterFixedString_;
		QList<int> FilterColumns_ {};

		Qt::ItemDataRole FilterRole_ = Qt::DisplayRole;
		Qt::CaseSensitivity CaseSensitivity_ = Qt::CaseInsensitive;

		bool IsRecursiveFiltering_ = true;
	public:
		explicit FixedStringFilterProxyModel (QObject* = nullptr);
		explicit FixedStringFilterProxyModel (Qt::CaseSensitivity, QObject* = nullptr);

		void SetFilterString (const QString&);
		QString GetFilterString () const;

		bool IsFilterSet () const;

		void SetCaseSensitivity (Qt::CaseSensitivity);
		Qt::CaseSensitivity GetCaseSensitivity () const;

		void SetFilterRole (Qt::ItemDataRole);
		Qt::ItemDataRole GetFilterRole () const;

		void SetFilterColumns (const QList<int>&);
		QList<int> GetFilterColumns () const;

		void SetRecursiveFiltering (bool);
		bool GetRecursiveFiltering () const;
	protected:
		bool IsMatch (const QString&) const;

		bool filterAcceptsRow (int row, const QModelIndex& parent) const override;
	private:
		using QSortFilterProxyModel::setFilterFixedString;
	};
}
