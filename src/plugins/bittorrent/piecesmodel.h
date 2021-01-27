/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QStringList>

namespace LC::BitTorrent
{
	class PiecesModel : public QAbstractItemModel
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::PiecesModel)

		QStringList Headers_;
		struct Info
		{
			int Index_;
			int FinishedBlocks_;
			int TotalBlocks_;

			bool operator== (const Info&) const;
		};
		QList<Info> Pieces_;

		const int Index_;
	public:
		explicit PiecesModel (int, QObject *parent = nullptr);

		int columnCount (const QModelIndex&) const override;
		QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		bool hasChildren (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& parent = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& parent = {}) const override;

		void Update ();
	};
}
