/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>

namespace LC
{
namespace Poshuku
{
	class ILinkOpenModifier
	{
	public:
		virtual void InstallOn (QWidget *view) = 0;

		struct OpenBehaviourSuggestion
		{
			bool NewTab_ = false;
			bool Invert_ = false;
		};

		virtual OpenBehaviourSuggestion GetOpenBehaviourSuggestion () const = 0;

		virtual void ResetSuggestionState () = 0;
	};

	using ILinkOpenModifier_ptr = std::shared_ptr<ILinkOpenModifier>;
}
}
