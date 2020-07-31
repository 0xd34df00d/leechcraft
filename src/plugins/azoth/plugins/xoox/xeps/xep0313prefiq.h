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
	class Xep0313PrefIq : public QXmppIq
	{
		QStringList Allowed_;
		QStringList Forbidden_;
	public:
		enum class DefaultPolicy
		{
			Always,
			Never,
			Roster
		};
	private:
		DefaultPolicy Policy_ = DefaultPolicy::Roster;
	public:
		Xep0313PrefIq (Type type = QXmppIq::Get);

		QStringList GetAllowed () const;
		void SetAllowed (const QStringList&);

		QStringList GetForbidden () const;
		void SetForbidden (const QStringList&);

		DefaultPolicy GetDefaultPolicy () const;
		void SetDefaultPolicy (DefaultPolicy);
	protected:
		void parseElementFromChild (const QDomElement&);
		void toXmlElementFromChild (QXmlStreamWriter*) const;
	};
}
}
}
