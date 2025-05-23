/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_feedsettings.h"
#include "common.h"

namespace LC::Aggregator
{
	struct ChannelShort;

	class FeedSettings : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::FeedSettings)

		Ui::FeedSettings Ui_;
		const IDType_t FeedId_;
		const IDType_t ChannelId_;
	public:
		explicit FeedSettings (const ChannelShort&, QWidget* = nullptr);

		void accept () override;
	};
}
