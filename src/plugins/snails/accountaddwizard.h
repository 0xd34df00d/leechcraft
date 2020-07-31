/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizard>

namespace LC
{
namespace Snails
{
	struct AccountConfig;

	class AccountAddWizard final : public QWizard
	{
	public:
		AccountAddWizard (QWidget* = nullptr);

		AccountConfig GetConfig () const;
	};
}
}
