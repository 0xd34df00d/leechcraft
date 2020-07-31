/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "entitygeneratingpage.h"
#include "ui_jsonbookmarksimportpage.h"

namespace LC
{
struct Entity;

namespace NewLife
{
namespace Importers
{
	class JsonBookmarksImportPage : public EntityGeneratingPage
	{
		Ui::JsonBookmarksImportPage Ui_;
	public:
		JsonBookmarksImportPage (const ICoreProxy_ptr&, QWidget* = nullptr);

		int nextId () const override;
		void initializePage () override;
		bool isComplete() const override;
	private:
		void HandleAccepted ();
		void BrowseFile ();
	};
}
}
}
