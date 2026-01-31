/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QByteArray;
class QString;

namespace LC::Azoth
{
	bool CheckWithDefaultValue (const QString& entryId, const QString& group, const QByteArray& propName);
	void UpdateWithDefaultValue (bool value, const QString& entryId, const QString& group, const QByteArray& propName);
}
