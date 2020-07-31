/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addpeerdialog.h"

namespace LC
{
namespace BitTorrent
{
	class AddPeerDialog : public QDialog
	{
		Q_OBJECT

		Ui::AddPeerDialog Ui_;
	public:
		AddPeerDialog (QWidget* = 0);

		QString GetIP () const;
		int GetPort () const;
	};
}
}
