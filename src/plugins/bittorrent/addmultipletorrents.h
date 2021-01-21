/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_addmultipletorrents.h"

namespace LC::BitTorrent
{
	class AddMultipleTorrents : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::AddMultipleTorrents)

		Ui::AddMultipleTorrents Ui_;
	public:
		explicit AddMultipleTorrents (QWidget *parent = nullptr);

		QString GetOpenDirectory () const;
		QString GetSaveDirectory () const;
		QStringList GetTags () const;
		bool ShouldAddAsStarted () const;
		bool OnlyIfExists () const;
	};
}
