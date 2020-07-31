/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QXmppElement;
class QDomElement;
class QString;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PEPEventBase
	{
	public:
		virtual ~PEPEventBase ();

		virtual QXmppElement ToXML () const = 0;
		virtual void Parse (const QDomElement&) = 0;
		virtual QString Node () const = 0;

		virtual PEPEventBase* Clone () const = 0;

		virtual QString GetEventID () const;
	};

	template<typename T>
	PEPEventBase* StandardCreator ()
	{
		return new T ();
	}
}
}
}
