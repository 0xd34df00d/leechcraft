/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/typegetter.h>
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

	template<
			typename T = void,
			typename F,
			typename... Args,
			typename ItemType =
				std::conditional_t<
					!std::is_same_v<T, void>,
					T,
					typename ArgType_t<F, 0>::element_type::ItemType_t
				>
		>
	Channel_ptr<ItemType> WithChannel (F&& f, Args&&... args)
	{
		auto output = std::make_shared<Channel<ItemType>> ();
		[] (auto f, Channel_ptr<ItemType> output, auto... args) -> Task<void>
		{
			co_await std::invoke (f, output, args...);
			output->Close ();
		} (std::forward<F> (f), output, std::forward<Args> (args)...);
		return output;
	}

	template<
			typename F,
			typename Conv = std::identity,
			typename... Args,
			typename ItemType = RetType_t<F>::ResultType_t,
			typename ConvertedType = std::invoke_result_t<Conv, ItemType>
		>
	Channel_ptr<ConvertedType> ChannelFromSingleResult (F&& f, Conv&& conv, Args&&... args)
	{
		auto output = std::make_shared<Channel<ConvertedType>> ();
		[] (auto f, auto conv, Channel_ptr<ConvertedType> output, auto... args) -> Task<void>
		{
			auto res = co_await std::invoke (f, args...);
			output->Send (conv (std::move (res)));
			output->Close ();
		} (std::forward<F> (f), std::forward<Conv> (conv), output, std::forward<Args> (args)...);
		return output;
	}
}
