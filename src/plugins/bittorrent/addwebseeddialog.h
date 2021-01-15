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

namespace LC
{
namespace BitTorrent
{
	class AddWebSeedDialog : public QDialog
	{
		Q_OBJECT

		Ui::AddWebSeedDialog Ui_;
	public:
		AddWebSeedDialog (QWidget* = 0);

		QString GetURL () const;
		WebSeedType GetType () const;
	};
}
}
