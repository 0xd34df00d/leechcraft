/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QString>
#include <QHash>
#include "interfaces/azoth/azothcommon.h"

class QMenu;
class QAction;

namespace LC::Azoth::StatusChange
{
	QMenu* CreateMenu (QObject *context, const std::function<void (State, QString)>& handler, QWidget *parent = nullptr);

	QString GetStatusText (State, const QString& = {});

	void ChangeAllAccountsStatus (State, const QString&);
}
