/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Slava Barinov <rayslava@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "pasteservicebase.h"

namespace LC::Azoth::Autopaste
{
	class PasteOrgRuService final : public PasteServiceBase
	{
	public:
		using PasteServiceBase::PasteServiceBase;

		void Paste (const PasteParams&) override;
	protected:
		void HandleMetadata (QNetworkReply*) override;
	};
}

