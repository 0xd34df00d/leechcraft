/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>

class QString;

namespace LC
{
namespace Azoth
{
class IProxyObject;
class ICLEntry;
struct TextMorphResult;

namespace MuCommands
{
	bool HandleNames (IProxyObject*, ICLEntry*, const QString&);

	bool ShowVCard (IProxyObject*, ICLEntry*, const QString&);

	bool ShowVersion (IProxyObject*, ICLEntry*, const QString&);

	bool ShowTime (IProxyObject*, ICLEntry*, const QString&);

	bool Disco (IProxyObject*, ICLEntry*, const QString&);

	bool JoinMuc (IProxyObject*, ICLEntry*, const QString&);

	bool RejoinMuc (IProxyObject*, ICLEntry*, const QString&);

	bool LeaveMuc (IProxyObject*, ICLEntry*, const QString&);

	bool ChangeSubject (IProxyObject*, ICLEntry*, const QString&);

	bool ChangeNick (IProxyObject*, ICLEntry*, const QString&);

	bool Kick (IProxyObject*, ICLEntry*, const QString&);

	bool Ban (IProxyObject*, ICLEntry*, const QString&);

	bool ListPerms (IProxyObject*, ICLEntry*, const QString&);

	bool SetPerm (IProxyObject*, ICLEntry*, const QString&);

	bool Whois (IProxyObject*, ICLEntry*, const QString&);

	bool Invite (IProxyObject*, ICLEntry*, const QString&);

	bool Pm (IProxyObject*, ICLEntry*, const QString&);

	bool Last (IProxyObject*, ICLEntry*, const QString&);

	bool Ping (IProxyObject*, ICLEntry*, const QString&);

	TextMorphResult Subst (IProxyObject*, ICLEntry*, const QString&);
}
}
}
