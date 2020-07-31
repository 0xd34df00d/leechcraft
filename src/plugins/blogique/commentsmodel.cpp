/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "commentsmodel.h"

namespace LC
{
namespace Blogique
{
	CommentsModel::CommentsModel (QObject *parent)
	: RoleNamesMixin<QStandardItemModel> (parent)
	{
		QHash<int, QByteArray> roleNames;
		roleNames [AccountID] = "accountID";
		roleNames [EntrySubject] = "entrySubject";
		roleNames [EntryUrl] = "entryUrl";
		roleNames [EntryID] = "entryID";
		roleNames [CommentSubject] = "commentSubject";
		roleNames [CommentBody] = "commentBody";
		roleNames [CommentAuthor] = "commentAuthor";
		roleNames [CommentDate] = "commentDate";
		roleNames [CommentUrl] = "commentUrl";
		roleNames [CommentID] = "commentID";
		setRoleNames (roleNames);
	}
}
}
