/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_FUA_SETTINGS_H
#define PLUGINS_POSHUKU_PLUGINS_FUA_SETTINGS_H
#include <QWidget>
#include "ui_settings.h"

class QStandardItemModel;

namespace LC
{
namespace Poshuku
{
namespace Fua
{
	class FUA;

	class Settings : public QWidget
	{
		Q_OBJECT

		Ui::Settings Ui_;
		FUA *Fua_;
		QStandardItemModel *Model_;
	public:
		Settings (QStandardItemModel*, FUA*);
	private slots:
		void on_Add__released ();
		void on_Modify__released ();
		void on_Remove__released ();
	};
}
}
}

#endif
