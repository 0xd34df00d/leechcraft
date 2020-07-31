/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_RELOADINTERVALSELECTOR_H
#define PLUGINS_POSHUKU_RELOADINTERVALSELECTOR_H
#include <QDialog>
#include "ui_reloadintervalselector.h"

namespace LC
{
namespace Poshuku
{
	class ReloadIntervalSelector : public QDialog
	{
		Q_OBJECT

		Ui::ReloadIntervalSelector Ui_;
	public:
		ReloadIntervalSelector (QWidget* = 0);

		QTime GetInterval () const;
	};
}
}

#endif
