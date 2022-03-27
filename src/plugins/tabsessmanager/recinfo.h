/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_TABSESSMANAGER_RECINFO_H
#define PLUGINS_TABSESSMANAGER_RECINFO_H
#include <QByteArray>
#include <QString>
#include <QIcon>
#include <QVariant>
#include <QMetaType>

namespace LC
{
namespace TabSessManager
{
	struct RecInfo
	{
		int Order_;
		QByteArray Data_;
		QList<QPair<QByteArray, QVariant>> Props_;
		QString Name_;
		QIcon Icon_;
		int WindowID_;
	};
}
}

Q_DECLARE_METATYPE (LC::TabSessManager::RecInfo)

#endif
