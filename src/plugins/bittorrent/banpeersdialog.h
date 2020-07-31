/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_banpeersdialog.h"

namespace LC
{
namespace BitTorrent
{
	class BanPeersDialog : public QDialog
	{
		Q_OBJECT

		Ui::BanPeersDialog Ui_;
	public:
		BanPeersDialog (QWidget* = 0);

		void SetIP (const QString&);
		void SetIP (const QString&, const QString&);
		QString GetStart () const;
		QString GetEnd () const;
	};
}
}
