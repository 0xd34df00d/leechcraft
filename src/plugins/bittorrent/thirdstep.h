/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_newtorrentthirdstep.h"

namespace LC
{
namespace BitTorrent
{
	class ThirdStep : public QWizardPage, private Ui::NewTorrentThirdStep
	{
		Q_OBJECT

		quint64 TotalSize_;
	public:
		ThirdStep (QWidget *parent = 0);
		virtual void initializePage ();
	private slots:
		void on_PieceSize__currentIndexChanged ();
	};
}
}
