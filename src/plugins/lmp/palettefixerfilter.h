/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPalette>

namespace LC
{
namespace LMP
{
	class PaletteFixerFilter : public QObject
	{
		QWidget * const W_;
		bool IsChanging_;
	public:
		PaletteFixerFilter (QWidget *parent);

		void UpdatePalette (QPalette);

		bool eventFilter (QObject*, QEvent*);
	};
}
}
