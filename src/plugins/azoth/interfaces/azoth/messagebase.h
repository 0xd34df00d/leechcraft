/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace Azoth
{
	class MessageBase
	{
	protected:
		bool IsOTR_ = false;

		bool IsForwarded_ = false;
	public:
		void ToggleOTRMessage (bool otr)
		{
			IsOTR_ = otr;
		}

		bool IsOTRMessage () const
		{
			return IsOTR_;
		}

		void ToggleForwarded (bool forwarded)
		{
			IsForwarded_ = forwarded;
		}

		bool IsForwarded () const
		{
			return IsForwarded_;
		}
	};
}
}
