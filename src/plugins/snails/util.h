/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QString;
class QStringList;
class QByteArray;

class IEntityManager;

namespace LC::Snails
{
	class Account;

	QString PlainBody2HTML (const QString&);

	void RunAttachmentSaveDialog (Account*, IEntityManager*,
			const QByteArray& id, const QStringList& folder, const QString& name);
}
