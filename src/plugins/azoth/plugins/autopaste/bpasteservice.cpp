/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bpasteservice.h"
#include "highlight2str.h"

namespace LC::Azoth::Autopaste
{
	void BPasteService::Paste (const PasteParams& params)
	{
		const auto& highlight = HlConverters::SpacePaste (params.High_);
		auto data = "submit=Paste!&lexer=" + highlight + "&expiry=1week&code=";
		data += params.Text_.toUtf8 ().toPercentEncoding ();

		PasteImpl (params, "https://bpaste.net/", data);
	}
}
