/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_COOKIESFILTER_H
#define PLUGINS_POSHUKU_COOKIESFILTER_H
#include <QSortFilterProxyModel>

namespace LC
{
namespace Poshuku
{
	class CookiesFilter : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		CookiesFilter (QObject* = 0);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const;
	};
}
}

#endif
