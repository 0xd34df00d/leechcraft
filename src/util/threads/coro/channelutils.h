/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "channel.h"
#include "inparallel.h"
#include "task.h"

namespace LC::Util
{
	template<typename T>
	Channel_ptr<T> MergeChannels (QVector<Channel_ptr<T>> channels)
	{
		auto output = std::make_shared<Channel<T>> ();
		[] (Channel_ptr<T> output, QVector<Channel_ptr<T>> channels) -> Task<void>
		{
			co_await InParallel (channels, [] (auto inChan, auto outChan) -> Task<void>
					{
						while (auto value = co_await *inChan)
							outChan->Send (std::move (*value));
					}, output);
			output->Close ();
		} (output, std::move (channels));
		return output;
	}
}
