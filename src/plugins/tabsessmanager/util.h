/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QObject;
class QByteArray;
class QVariant;

template<typename P1, typename P2>
struct QPair;

template<typename T>
class QList;

struct TabClassInfo;

namespace LC::TabSessManager
{
	QList<QPair<QByteArray, QVariant>> GetSessionProps (QObject*);

	bool IsGoodSingleTC (const TabClassInfo&);
}
