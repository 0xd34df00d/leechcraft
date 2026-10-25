/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <util/db/oral/utilitytypes.h>
#include <interfaces/azoth/azothcommon.h>

namespace LC::Util::oral
{
	template<Azoth::IdKind K>
	struct ConvertT<Azoth::EntryId<K>>
		: ConvertVia<
			Azoth::EntryId<K>,
			QString,
			&Azoth::EntryId<K>::ToString,
			&Azoth::EntryId<K>::FromString
		> {};
}
