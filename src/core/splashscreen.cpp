/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "splashscreen.h"
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <util/sll/slotclosure.h>
#include "loadprocessbase.h"

namespace LC
{
	void SplashScreen::RegisterLoadProcess (LoadProcessBase *proc)
	{
		Processes_ << proc;
		repaint ();

		connect (proc,
				SIGNAL (changed ()),
				this,
				SLOT (repaint ()));

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, proc]
			{
				Processes_.removeOne (proc);
				repaint ();
			},
			proc,
			SIGNAL (destroyed (QObject*)),
			this
		};
	}

	void SplashScreen::drawContents (QPainter *painter)
	{
		QSplashScreen::drawContents (painter);

		const auto margin = 10;
		int ypos = margin;
		const auto height = 1.3 * fontMetrics ().height ();

		QStyleOptionProgressBar opt;
		opt.initFrom (this);
		opt.rect.setWidth (width ());
		opt.rect.setHeight (height);
		opt.textVisible = true;

		auto& p = opt.palette;
		p.setColor (QPalette::Base, Qt::transparent);
		p.setColor (QPalette::Window, Qt::transparent);
		p.setColor (QPalette::Highlight, "#FF3B00");
		p.setColor (QPalette::Text, "#FF3B00");
		p.setColor (QPalette::HighlightedText, "#1B181F");

		for (const auto proc : Processes_)
		{
			opt.rect.moveTop (ypos);
			ypos += height;

			opt.minimum = proc->GetMin ();
			opt.maximum = proc->GetMax ();
			opt.progress = proc->GetValue ();
			opt.text = proc->GetTitle () + " " + tr ("(%1 of %2)").arg (opt.progress).arg (opt.maximum);

			style ()->drawControl (QStyle::CE_ProgressBar, &opt, painter, this);
		}
	}
}
