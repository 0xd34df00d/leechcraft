/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_panelsettingsdialog.h"

class QStandardItemModel;
class QModelIndex;

namespace LC
{
namespace Util
{
	class XmlSettingsDialog;
}

namespace SB2
{
	struct SettingsItem
	{
		QString Name_;
		QIcon Icon_;
		Util::XmlSettingsDialog *XSD_;
	};
	using SettingsList_t = QList<SettingsItem>;

	class PanelSettingsDialog : public QDialog
	{
		Ui::PanelSettingsDialog Ui_;
		QStandardItemModel * const ItemsModel_;

		const QList<SettingsItem> Items_;
	public:
		PanelSettingsDialog (const SettingsList_t&, QWidget* = nullptr);
		~PanelSettingsDialog ();
	private:
		void HandleDialogButtonClicked (QAbstractButton*);
	};
}
}
