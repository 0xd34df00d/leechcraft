/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_itemsexportdialog.h"
#include "common.h"

namespace LC::Util
{
	template<typename T>
	class CheckableProxyModel;
}

namespace LC::Aggregator
{
	class ChannelsModel;
	struct ItemsExportFormat;

	class ItemsExportDialog : public QDialog
	{
		Q_OBJECT

		Ui::ItemsExportDialog Ui_;

		std::unique_ptr<Util::CheckableProxyModel<IDType_t>> ChannelsModel_;

		QStringList CurrentCategories_;

		bool TitleBeenModified_ = false;
	public:
		explicit ItemsExportDialog (ChannelsModel&, QWidget* = nullptr);
		~ItemsExportDialog () override;

		QString GetFilename () const;
		ItemsExportFormat GetFormat () const;

		struct ItemsExportInfo
		{
			QSet<IDType_t> Channels_;
			QStringList Categories_;
			bool UnreadOnly_ = false;
		};

		ItemsExportInfo GetItemsExportInfo () const;
	private:
		bool Browse ();
		void CheckDialogAcceptable ();
	private slots:
		void handleChannelsSelectionChanged ();
	};
}
