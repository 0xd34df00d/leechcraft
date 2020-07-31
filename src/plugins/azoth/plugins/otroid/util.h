/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

extern "C"
{
#include <libotr/proto.h>
}

#ifndef OTRL_PRIVKEY_FPRINT_HUMAN_LEN
#define OTRL_PRIVKEY_FPRINT_HUMAN_LEN 45
#endif

class QIcon;
class QString;

namespace LC
{
namespace Azoth
{
class IAccount;

namespace OTRoid
{
	QIcon GetAccountIcon (IAccount*);

	void WriteKeys (OtrlUserState, const QString&);
}
}
}
