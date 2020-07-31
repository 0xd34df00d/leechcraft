/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QMetaType>

class QDataStream;

namespace LC
{
namespace Snails
{
	enum class FolderType
	{
		Inbox,
		Drafts,
		Sent,
		Important,
		Junk,
		Trash,
		Other
	};

	struct Folder
	{
		QStringList Path_;
		FolderType Type_;
	};

	bool operator== (const Folder&, const Folder&);
}
}

Q_DECLARE_METATYPE (LC::Snails::Folder)
Q_DECLARE_METATYPE (QList<LC::Snails::Folder>)

QDataStream& operator<< (QDataStream&, const LC::Snails::Folder&);
QDataStream& operator>> (QDataStream&, LC::Snails::Folder&);
