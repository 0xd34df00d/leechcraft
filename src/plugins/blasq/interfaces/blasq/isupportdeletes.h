/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include "collection.h"

namespace LC
{
namespace Blasq
{
	enum class DeleteFeature
	{
		DeleteImages,
		DeleteCollections
	};

	class ISupportDeletes
	{
	public:
		virtual ~ISupportDeletes () {}

		/** @brief Deletes the given collection or image.
		 */
		virtual void Delete (const ItemRef&) = 0;

		virtual bool SupportsFeature (DeleteFeature) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blasq::ISupportDeletes, "org.LeechCraft.Blasq.ISupportDeletes/1.0")
