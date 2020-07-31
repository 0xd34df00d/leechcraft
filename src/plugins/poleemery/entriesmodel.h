/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include "structures.h"
#include "operationsmanager.h"

namespace LC
{
namespace Poleemery
{
	class EntriesModel : public QAbstractItemModel
	{
		Q_OBJECT

		const QStringList HeaderData_;

		QList<EntryBase_ptr> Entries_;
		QList<BalanceInfo> Sums_;

		bool RatePriceEditable_ = true;
		bool ModifiesStorage_ = true;
	public:
		enum Columns
		{
			Date,
			Name,

			Price,
			EntryCurrency,
			EntryRate,
			NativePrice,

			Count,
			Shop,
			Categories,
			Account,
			AccBalance,
			SumBalance,

			MaxCount
		};

		EntriesModel (QObject* = 0);

		QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const;
		QModelIndex parent (const QModelIndex& child) const;
		int rowCount (const QModelIndex& parent = QModelIndex()) const;
		int columnCount (const QModelIndex& parent = QModelIndex()) const;
		Qt::ItemFlags flags (const QModelIndex& index) const;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
		bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
		QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

		void SetRatePriceEditable (bool);
		void SetModifiesStorage (bool);

		void AddEntry (EntryBase_ptr);
		void AddEntries (QList<EntryBase_ptr>);
		void RemoveEntry (const QModelIndex&);

		EntryBase_ptr GetEntry (const QModelIndex&) const;
		QList<EntryBase_ptr> GetEntries () const;
		QList<BalanceInfo> GetSumInfos () const;
	public slots:
		void recalcSums ();
	};
}
}
