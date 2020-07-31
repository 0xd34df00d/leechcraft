/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <util/models/rolenamesmixin.h>

namespace LC
{
namespace Blogique
{
	class CommentsModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
		Q_OBJECT

	public:

		enum CommentRoles
		{
			AccountID = Qt::UserRole + 1,
			EntrySubject,
			EntryUrl,
			EntryID,
			CommentSubject,
			CommentBody,
			CommentAuthor,
			CommentDate,
			CommentUrl,
			CommentID
		};

		explicit CommentsModel (QObject *parent = 0);
	};
}
}

