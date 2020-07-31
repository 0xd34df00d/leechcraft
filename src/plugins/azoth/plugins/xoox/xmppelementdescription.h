/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QHash>
#include <QList>
#include <util/sll/requiresinit.h>

class QXmppElement;

namespace LC::Azoth::Xoox
{
	struct XmppElementDescription
	{
		Util::RequiresInit<QString> TagName_;
		QString Value_ = {};

		QHash<QString, QString> Attributes_ = {};

		QList<XmppElementDescription> Children_ = {};
	};

	QXmppElement ToElement (const XmppElementDescription&);
}
