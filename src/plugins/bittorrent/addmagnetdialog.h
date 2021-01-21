/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addmagnetdialog.h"

namespace LC::BitTorrent
{
	class AddMagnetDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::AddMagnetDialog)

		Ui::AddMagnetDialog Ui_;
	public:
		explicit AddMagnetDialog (QWidget* = nullptr);

		QString GetLink () const;
		QString GetPath () const;
		QStringList GetTags () const;
	};
}
