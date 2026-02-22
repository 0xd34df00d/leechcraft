/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QModelIndex>
#include <QObject>
#include <interfaces/ijobholder.h>

namespace LC::Azoth
{
	class TransferModelManager;

	class RepresentationHandler
		: public QObject
		, public IJobHolderRepresentationHandler
	{
		TransferModelManager& ModelMgr_;
		std::unique_ptr<QToolBar> ToolBar_;

		QList<QModelIndex> Selected_;
	public:
		explicit RepresentationHandler (TransferModelManager&, QObject* = nullptr);
		~RepresentationHandler () override;

		void HandleSelectedRowsChanged (const QList<QModelIndex>&) override;

		QAbstractItemModel& GetRepresentation () override;
		QToolBar* GetControls () override;
	};
}
