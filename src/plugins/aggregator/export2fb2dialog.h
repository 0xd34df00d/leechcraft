/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_export2fb2dialog.h"

namespace LC
{
namespace Util
{
	class CategorySelector;
};

namespace Aggregator
{
	class ChannelsModel;
	struct WriteInfo;

	class Export2FB2Dialog : public QDialog
	{
		Q_OBJECT

		ChannelsModel * const ChannelsModel_;

		Ui::Export2FB2Dialog Ui_;
		Util::CategorySelector *Selector_;
		QStringList CurrentCategories_;

		bool HasBeenTextModified_ = false;
	public:
		Export2FB2Dialog (ChannelsModel*, QWidget* = nullptr);
	private:
		void WriteFB2 (const WriteInfo&);
		void WritePDF (const WriteInfo&);
	private slots:
		void on_Browse__released ();
		void on_File__textChanged (const QString&);
		void on_Name__textEdited ();
		void handleChannelsSelectionChanged ();
		void handleAccepted ();
	};
}
}
