/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QString;
class QObject;

namespace LC
{
namespace Azoth
{
	class IPendingVersionQuery
	{
	public:
		virtual ~IPendingVersionQuery () {}
	protected:
		virtual void versionReceived () = 0;
	};

	class IHaveQueriableVersion
	{
	public:
		virtual ~IHaveQueriableVersion () {}

		virtual QObject* QueryVersion (const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IPendingVersionQuery,
		"org.LeechCraft.Azoth.IPendingVersionQuery/1.0")
Q_DECLARE_INTERFACE (LC::Azoth::IHaveQueriableVersion,
		"org.LeechCraft.Azoth.IHaveQueriableVersion/1.0")
