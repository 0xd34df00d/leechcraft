/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppIq.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class Xep0313ReqIq : public QXmppIq
	{
		QString JID_;
		QString ItemId_;
		int Count_;

		QString QueryID_;
	public:
		enum class Direction
		{
			Unspecified,
			Before,
			After
		};
	private:
		Direction Dir_ = Direction::Unspecified;
	public:
		Xep0313ReqIq (const QString&, const QString&, int, Direction, const QString& queryId);
	protected:
		void parseElementFromChild (const QDomElement&);
		void toXmlElementFromChild (QXmlStreamWriter*) const;
	};
}
}
}
