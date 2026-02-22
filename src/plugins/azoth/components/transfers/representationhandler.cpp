/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "representationhandler.h"
#include <QToolBar>
#include "transfermodelmanager.h"

namespace LC::Azoth
{
	RepresentationHandler::RepresentationHandler (TransferModelManager& modelMgr, QObject *parent)
	: QObject { parent }
	, ModelMgr_ { modelMgr }
	, ToolBar_ { std::make_unique<QToolBar> () }
	{
		const auto abort = ToolBar_->addAction (tr ("Abort"));
		abort->setProperty ("ActionIcon", "process-stop");
		connect (abort,
				&QAction::triggered,
				this,
				[this]
				{
					for (const auto& idx : Selected_)
						if (const auto job = idx.data (Util::ProgressManager::CustomDataRole).value<ITransferJob*> ())
							job->Abort ();
						else
							qWarning () << "no job for" << idx;
				});
	}

	RepresentationHandler::~RepresentationHandler () = default;

	void RepresentationHandler::HandleSelectedRowsChanged (const QList<QModelIndex>& selected)
	{
		Selected_ = selected;
	}

	QAbstractItemModel& RepresentationHandler::GetRepresentation ()
	{
		return ModelMgr_.GetTransfersModel ();
	}

	QToolBar* RepresentationHandler::GetControls ()
	{
		return &*ToolBar_;
	}
}
