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
		QStringMatcher Filter_;
		QList<int> Columns_ {};
		QList<int> Roles_ { Qt::DisplayRole };
	public:
		explicit FixedStringFilterProxyModel (QObject* = nullptr);
		explicit FixedStringFilterProxyModel (Qt::CaseSensitivity, QObject* = nullptr);

		void SetFilterString (const QString&);
		QString GetFilterString () const;

		bool IsFilterSet () const;

		void SetCaseSensitivity (Qt::CaseSensitivity);
		Qt::CaseSensitivity GetCaseSensitivity () const;

		void SetFilterRoles (const QList<int>&);
		QList<int> GetFilterRoles () const;

		void SetFilterColumns (const QList<int>&);
		QList<int> GetFilterColumns () const;
	protected:
		bool IsMatch (const QString&) const;

		bool filterAcceptsRow (int row, const QModelIndex& parent) const override;
	private:
		using QSortFilterProxyModel::setFilterFixedString;
	};
}
