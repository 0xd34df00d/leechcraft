/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QColor>
#include <QList>
#include <interfaces/azoth/azothcommon.h>

class QWidget;

namespace LC::Azoth
{
	QList<QColor> GenerateColors (const QString& coloring, QColor);
	QString GetNickColor (const QString& nick, const QList<QColor>& colors);

	QWidget* GetDialogParent ();
	QString GetActivityIconName (const QString&, const QString&);
	QString StateToString (State);
}
