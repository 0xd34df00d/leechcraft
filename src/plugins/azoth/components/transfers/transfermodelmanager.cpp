/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transfermodelmanager.h"
#include <QFileInfo>
#include <QMimeDatabase>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/xpc/progressmanager.h>
#include <util/azoth/emitters/transfermanager.h>
#include "types.h"

namespace LC::Azoth
{
	using namespace Transfers;

	TransferModelManager::TransferModelManager (QObject *parent)
	: QObject { parent }
	, Offers_
	{
		{
			{ Qt::DecorationRole, [] (const IncomingOfferRow& row) { return QVariant { row.Icon_ }; } },
			{ Roles::RowType, [] (const IncomingOfferRow&) { return QVariant::fromValue (RowType::Offer); } },
			{ Roles::EntryId, [] (const IncomingOfferRow& row) { return QVariant { row.Offer_.EntryId_ }; } },
			{ Roles::OrigFilename, [] (const IncomingOfferRow& row) { return QVariant { row.Offer_.Name_ }; } },
			{ Roles::Size, [] (const IncomingOfferRow& row) { return QVariant { row.Offer_.Size_ }; } },
			{ Roles::FullOffer, [] (const IncomingOfferRow& row) { return QVariant::fromValue (row.Offer_); }}
		}
	}
	{
		Jobs_.SetGlobalData (QVariant::fromValue (RowType::Transfer), Roles::RowType);

		FullModel_.addSourceModel (&Jobs_.GetModel ());
		FullModel_.addSourceModel (&Offers_);
	}

	QAbstractItemModel& TransferModelManager::GetFullModel ()
	{
		return FullModel_;
	}

	QAbstractItemModel& TransferModelManager::GetTransfersModel ()
	{
		return Jobs_.GetModel ();
	}

	void TransferModelManager::AddOffer (const IncomingOffer& offer)
	{
		const auto& mimeType = QMimeDatabase {}.mimeTypeForFile (offer.Name_, QMimeDatabase::MatchExtension);
		auto icon = QIcon::fromTheme (mimeType.iconName ());
		if (icon.isNull ())
			icon = QIcon::fromTheme ("application-octet-stream"_qs);

		Offers_.AddItem ({ offer, icon });
	}

	void TransferModelManager::RemoveOffer (const IncomingOffer& offer)
	{
		const auto& items = Offers_.GetItems ();
		if (const auto pos = std::ranges::find (items, offer, &IncomingOfferRow::Offer_);
			pos != items.end ())
			Offers_.RemoveItem (pos);
		else
			qWarning () << "unknown offer";
	}

	namespace
	{
		QString GetRowLabelTemplate (const JobContext& context)
		{
			return Util::Visit (context.Dir_,
					[] (JobContext::In) { return TransferModelManager::tr ("Receiving %1 from %2"); },
					[] (JobContext::Out) { return TransferModelManager::tr ("Sending %1 to %2"); });
		}

		std::pair<ProcessState, QString> GetProcessInfo (const TransferState& state)
		{
			return Util::Visit (state,
					[] (Phase phase) -> std::pair<ProcessState, QString>
					{
						switch (phase)
						{
						case Phase::Starting:
							return { ProcessState::Running, TransferModelManager::tr ("starting") };
						case Phase::Transferring:
							return { ProcessState::Running, TransferModelManager::tr ("transferring") };
						case Phase::Finished:
							return { ProcessState::Finished, {} };
						}

						qWarning () << "unhandled state" << static_cast<int> (phase);
						return { ProcessState::Unknown, {} };
					},
					[] (const Error& error) { return std::pair { ProcessState::Error, error.Message_ }; });
		}
	}

	void TransferModelManager::AddJob (ITransferJob& job, const JobContext& context)
	{
		const auto& filename = GetFilename (context);

		std::shared_ptr row = Jobs_.AddRow (
				{
					.Name_ = GetRowLabelTemplate (context).arg (filename, context.EntryName_),
					.Specific_ = ProcessInfo
					{
						.Parameters_ = FromUserInitiated,
						.Kind_ = Util::Visit (context.Dir_,
								[] (JobContext::In) { return ProcessKind::Download; },
								[] (JobContext::Out) { return ProcessKind::Upload; }),
					},
				},
				{
					.State_ = ProcessState::Paused,
					.Total_ = context.Size_,
					.CustomStateText_ = tr ("offered"),
					.CustomData_ = QVariant::fromValue (&job),
				});

		auto& emitter = job.GetTransferJobEmitter ();
		connect (&emitter,
				&Emitters::TransferJob::stateChanged,
				this,
				[row] (const TransferState& state)
				{
					const auto [proc, descr] = GetProcessInfo (state);
					row->SetState (proc, descr);
				});
		connect (&emitter,
				&Emitters::TransferJob::transferProgress,
				this,
				[row] (qint64 done, qint64 total)
				{
					row->SetDone (done);
					row->SetTotal (total);
				});
	}
}
