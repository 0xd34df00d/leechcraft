/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_newtorrentsecondstep.h"

namespace LC
{
namespace BitTorrent
{
	class SecondStep : public QWizardPage, private Ui::NewTorrentSecondStep
	{
		Q_OBJECT
	public:
		SecondStep (QWidget *parent = 0);
		QStringList GetPaths () const;
	private slots:
		void on_AddPath__released ();
		void on_RemoveSelected__released ();
		void on_Clear__released ();
	};
}
}
