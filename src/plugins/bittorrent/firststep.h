/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_newtorrentfirststep.h"

namespace LC
{
namespace BitTorrent
{
	class FirstStep : public QWizardPage, private Ui::NewTorrentFirstStep
	{
		Q_OBJECT
	public:
		FirstStep (QWidget *parent = 0);

		bool isComplete () const;
	private:
		QString PrepareDirectory () const;
	private slots:
		void on_BrowseOutput__released ();
		void on_BrowseFile__released ();
		void on_BrowseDirectory__released ();
	};
}
}
