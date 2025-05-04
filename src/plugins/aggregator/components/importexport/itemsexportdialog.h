/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_itemsexportdialog.h"

namespace LC::Util
{
	class CategorySelector;
}

namespace LC::Aggregator
{
	class ChannelsModel;
	struct ExportConfig;
	struct PdfConfig;

	class ItemsExportDialog : public QDialog
	{
		Q_OBJECT

		ChannelsModel& ChannelsModel_;

		Ui::ItemsExportDialog Ui_;
		Util::CategorySelector *Selector_;
		QStringList CurrentCategories_;

		bool HasBeenTextModified_ = false;
	public:
		ItemsExportDialog (ChannelsModel&, QWidget* = nullptr);
	private:
		PdfConfig GetPdfConfig (const ExportConfig&) const;
	private slots:
		void on_Browse__released ();
		void on_File__textChanged (const QString&);
		void on_Name__textEdited ();
		void handleChannelsSelectionChanged ();
		void handleAccepted ();
	};
}
