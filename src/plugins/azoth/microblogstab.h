/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "tabbase.h"
#include "ui_microblogstab.h"

namespace LC
{
namespace Azoth
{
	class IAccount;

	class MicroblogsTab : public TabBase
	{
		Ui::MicroblogsTab Ui_;

		IAccount *Account_;
	public:
		static void SetTabData (QObject*, const TabClassInfo&);

		explicit MicroblogsTab (IAccount*);

		void Remove () override;
		QToolBar* GetToolBar () const override;
	};
}
}
