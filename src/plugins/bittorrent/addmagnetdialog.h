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

namespace LC
{
namespace BitTorrent
{
	class AddMagnetDialog : public QDialog
	{
		Ui::AddMagnetDialog Ui_;
	public:
		AddMagnetDialog (QWidget* = 0);

		QString GetLink () const;
		QString GetPath () const;
		QStringList GetTags () const;
	};
}
}
