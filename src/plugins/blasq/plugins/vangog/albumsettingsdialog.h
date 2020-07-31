/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <interfaces/core/icoreproxy.h>
#include "ui_albumsettingsdialog.h"

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	class AlbumSettingsDialog : public QDialog
	{
		Q_OBJECT

		Ui::AlbumSettingsDialog Ui_;
	public:
		AlbumSettingsDialog (const QString&, ICoreProxy_ptr, QWidget* = 0);

		QString GetName () const;
		QString GetDesc () const;
		int GetPrivacyLevel () const;
	private slots:
		void validate ();
	};
}
}
}
