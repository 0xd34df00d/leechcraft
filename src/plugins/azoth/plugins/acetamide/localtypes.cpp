/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localtypes.h"
#include <array>

namespace LC::Azoth::Acetamide::Lits
{
#define DEFINE_LIT(a) const QString a = QStringLiteral (#a);

	DEFINE_LIT (HumanReadableName)
	DEFINE_LIT (StoredName)
	DEFINE_LIT (Server)
	DEFINE_LIT (Port)
	DEFINE_LIT (ServerPassword)
	DEFINE_LIT (Encoding)
	DEFINE_LIT (Channel)
	DEFINE_LIT (ChannelPassword)
	DEFINE_LIT (Nickname)
	DEFINE_LIT (SSL)
	DEFINE_LIT (Autojoin)
	DEFINE_LIT (AccountID)

	const QString AzothAcetamide = QStringLiteral ("Azoth Acetamide");
}

namespace LC::Azoth::Acetamide
{
	uint qHash (const ChannelOptions& opts)
	{
		std::array items { opts.ChannelName_, opts.ChannelPassword_, opts.ServerName_ };
		return qHashRange (items.begin (), items.end (), qGlobalQHashSeed ());
	}
}
