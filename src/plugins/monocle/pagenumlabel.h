/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSpinBox>

namespace LC
{
namespace Monocle
{
	class PageNumLabel : public QSpinBox
	{
		bool Inverted_ = false;
	public:
		PageNumLabel (QWidget* = nullptr);

		void SetTotalPageCount (int);
		void SetCurrentPage (int);
	private:
		void stepBy (int) override;
	};
}
}
