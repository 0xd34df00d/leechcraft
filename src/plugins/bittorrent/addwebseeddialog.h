/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addwebseeddialog.h"
#include "types.h"

namespace LC::BitTorrent
{
	class AddWebSeedDialog : public QDialog
	{
		Ui::AddWebSeedDialog Ui_;
	public:
		explicit AddWebSeedDialog (QWidget* = nullptr);

		QString GetURL () const;
		WebSeedType GetType () const;
	};
}
