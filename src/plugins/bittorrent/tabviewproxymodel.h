/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC
{
namespace BitTorrent
{
	class TabViewProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT

		enum class StateFilterMode
		{
			All,
			Downloading,
			Seeding
		} StateFilter_;
	public:
		TabViewProxyModel (QObject* = 0);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const;
	public slots:
		void setStateFilterMode (int);
	};
}
}
