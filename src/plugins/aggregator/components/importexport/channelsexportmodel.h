/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/models/checkableproxymodel.h>
#include "common.h"

namespace LC::Aggregator
{
	class ChannelsExportModel : public Util::CheckableProxyModel<IDType_t>
	{
	public:
		using CheckableProxyModel::CheckableProxyModel;

		QVariant data (const QModelIndex& index, int role) const override;
	};
}
