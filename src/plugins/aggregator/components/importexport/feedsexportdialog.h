/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSet>
#include "common.h"
#include "ui_feedsexportdialog.h"

namespace LC::Util
{
	template<typename T>
	class CheckableProxyModel;
}

namespace LC::Aggregator
{
	class FeedsExportDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::FeedsExportDialog)

		Ui::FeedsExportDialog Ui_;
		std::unique_ptr<Util::CheckableProxyModel<IDType_t>> ChannelsModel_;
	public:
		explicit FeedsExportDialog (QAbstractItemModel&, QWidget* = nullptr);
		~FeedsExportDialog () override;

		QString GetDestination () const;
		QString GetTitle () const;
		QString GetOwner () const;
		QString GetOwnerEmail () const;
		QSet<IDType_t> GetSelectedFeeds () const;
	private:
		void Browse ();
	};
}
