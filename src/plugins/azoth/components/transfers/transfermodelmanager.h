/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QConcatenateTablesProxyModel>
#include <QCoreApplication>
#include <QIcon>
#include <QObject>
#include <util/models/itemsmodel.h>
#include <util/xpc/progressmanager.h>
#include "interfaces/azoth/itransfermanager.h"
#include "types.h"

namespace LC::Util
{
	class ProgressManager;
}

namespace LC::Azoth
{
	class TransferModelManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::TransferModelManager)

		Util::ProgressManager Jobs_;
	public:
		struct Roles
		{
			enum Common
			{
				RowType = Util::ProgressManager::MaxRole + 1,
				EntryId,
				OrigFilename,
				Size,

				MaxCommonRole
			};

			enum Offers
			{
				FullOffer = MaxCommonRole,
			};
		};

		enum class RowType : std::uint8_t
		{
			Offer,
			Transfer,
		};
	private:
		struct IncomingOfferRow
		{
			IncomingOffer Offer_;
			QIcon Icon_;
		};
		Util::RoledItemsModel<IncomingOfferRow> Offers_;

		QConcatenateTablesProxyModel FullModel_;
	public:
		explicit TransferModelManager (QObject *parent = nullptr);

		QAbstractItemModel& GetFullModel ();
		QAbstractItemModel& GetTransfersModel ();

		void AddOffer (const IncomingOffer&);
		void RemoveOffer (const IncomingOffer&);

		void AddJob (ITransferJob& job, const Transfers::JobContext&);
	};
}
