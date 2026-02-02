/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_fileofferedpane.h"

namespace LC::Azoth
{
	struct IncomingOffer;
	class TransferJobManager;

	class FileOfferedPane : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::FileOfferedPane)

		Ui::FileOfferedPane Ui_ {};
	public:
		explicit FileOfferedPane (const IncomingOffer&, TransferJobManager&, QWidget* = nullptr);
	};
}
