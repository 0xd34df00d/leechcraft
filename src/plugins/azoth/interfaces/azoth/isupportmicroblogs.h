/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QList>
#include <QMap>
#include <QDateTime>
#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	struct PostAuthor
	{
		QString Name_;
		QString URI_;
	};

	struct Post
	{
		QString ID_;
		QMap<QString, QString> Contents_;

		QDateTime Published_;
		QDateTime Updated_;

		PostAuthor Author_;
	};

	class ISupportMicroblogs
	{
	public:
		virtual ~ISupportMicroblogs () {}

		virtual void RequestLastPosts (int) = 0;
	protected:
		virtual void gotNewPost (const Post&) = 0;

		virtual void gotRecentPosts (const QList<Post>&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportMicroblogs,
		"org.Deviant.LeechCraft.Azoth.ISupportMicroblogs/1.0")
