/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTimer>
#include <interfaces/ijobholder.h>
#include "listactions.h"
#include "tabwidget.h"
#include "speedselectoraction.h"

namespace LC::BitTorrent
{
	class RepresentationHandler
		: public QObject
		, public IJobHolderRepresentationHandler
	{
		Q_OBJECT

		TabWidget TabWidget_;
		ListActions Actions_;

		SpeedSelectorAction DownSelectorAction_;
		SpeedSelectorAction UpSelectorAction_;

		std::unique_ptr<QMenu> Menu_ { Actions_.MakeContextMenu () };

		QTimer UpdateTimer_;
	public:
		explicit RepresentationHandler ();
		~RepresentationHandler () override;

		QAbstractItemModel& GetRepresentation () override;

		void HandleCurrentRowChanged (const QModelIndex& index) override;
		void HandleSelectedRowsChanged (const QModelIndexList& indexes) override;

		QWidget* GetInfoWidget () override;
		QToolBar* GetControls () override;
		QMenu* GetContextMenu () override;

		void UpdateSpeedControllerOptions ();
	signals:
		void torrentTabFocusRequested (const QModelIndex&);
	};
}
