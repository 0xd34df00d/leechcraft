/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include "filter.h"

namespace LC
{
namespace Poshuku
{
class IWebView;

namespace CleanWeb
{
	class RuleOptionDialog;

	class UserFiltersModel : public QAbstractItemModel
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		const QStringList Headers_;

		Filter Filter_;
	public:
		UserFiltersModel (const ICoreProxy_ptr&, QObject* = 0);

		int columnCount (const QModelIndex& = QModelIndex ()) const;
		QVariant data (const QModelIndex&, int) const;
		QVariant headerData (int, Qt::Orientation, int) const;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& = QModelIndex ()) const;

		const Filter& GetFilter () const;
		bool InitiateAdd (const QString& = QString ());
		void Modify (int);
		void Remove (int);

		void AddMultiFilters (QStringList);

		void ReadSettings ();
		void WriteSettings ();

		void BlockImage (const QUrl&, IWebView*);
	private:
		bool Add (const RuleOptionDialog&);
		void SplitRow (int*, bool*) const;
	signals:
		void filtersChanged ();
	};
}
}
}
