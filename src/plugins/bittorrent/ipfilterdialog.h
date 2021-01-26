/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ltutils.h"
#include "ui_ipfilterdialog.h"

namespace LC::BitTorrent
{
	using BanRange_t = QPair<QString, QString>;

	class IPFilterDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::IPFilterDialog)

		Ui::IPFilterDialog Ui_;
	public:
		explicit IPFilterDialog (const BanList_t&, QWidget* = nullptr);

		BanList_t GetFilter () const;
	private:
		void Add ();
		void Modify ();
	};
}
