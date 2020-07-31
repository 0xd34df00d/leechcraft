/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "palettefixerfilter.h"
#include <QWidget>
#include <QEvent>
#include <QApplication>

namespace LC
{
namespace LMP
{
	PaletteFixerFilter::PaletteFixerFilter (QWidget *parent)
	: QObject (parent)
	, W_ (parent)
	, IsChanging_ (false)
	{
		UpdatePalette (W_->palette ());
		parent->installEventFilter (this);
	}

	void PaletteFixerFilter::UpdatePalette (QPalette pal)
	{
		pal.setColor (QPalette::Base, pal.color (QPalette::Window));
		pal.setColor (QPalette::AlternateBase, pal.color (QPalette::Window));
		pal.setColor (QPalette::Text, pal.color (QPalette::WindowText));
		IsChanging_ = true;
		W_->setPalette (pal);
		IsChanging_ = false;
	}

	bool PaletteFixerFilter::eventFilter (QObject*, QEvent *e)
	{
		if (IsChanging_)
			return false;

		QPalette palette;
		if (e->type () == QEvent::ApplicationPaletteChange ||
				e->type () == QEvent::PaletteChange)
			palette = qApp->palette ();
		else
			return false;

		UpdatePalette (palette);

		return false;
	}
}
}
