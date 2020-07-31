/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include "collectiontypes.h"

namespace LC
{
namespace LMP
{
	class ILocalCollection
	{
	public:
		virtual ~ILocalCollection () {}

		virtual Collection::Artists_t GetAllArtists () const = 0;

		virtual void RecordPlayedTrack (int trackId, const QDateTime& date) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::ILocalCollection, "org.LeechCraft.LMP.ILocalCollection/1.0")
