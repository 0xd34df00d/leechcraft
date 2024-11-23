/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/ilink.h>
#include "types.h"

class QTextDocument;

namespace LC::Monocle
{
	struct AreaInfo
	{
		int Page_;
		PageRelativeRectBase Area_;
	};

	class PageLink : public ILink
	{
	public:
		struct LinkInfo
		{
			const QTextDocument& TextDoc_;

			Span Target_;
			std::optional<Span> Source_ {};

			QString ToolTip_ {};
		};
	private:
		LinkInfo Info_;

		mutable std::optional<AreaInfo> CachedTarget_;
		mutable std::optional<AreaInfo> CachedSource_;
	public:
		PageLink (LinkInfo);

		LinkType GetLinkType () const override;
		PageRelativeRectBase GetArea () const override;
		LinkAction GetLinkAction () const override;
		QString GetToolTip () const override;

		static NavigationAction GetNavigationAction (const LinkInfo&);

		int GetSourcePage () const;
	private:
		const AreaInfo& ComputeArea (Span, std::optional<AreaInfo>&) const;
		NavigationAction ComputeNavAction () const;
	};
}
