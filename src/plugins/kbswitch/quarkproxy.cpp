/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkproxy.h"
#include <QCursor>
#include <QMenu>
#include <QIcon>
#include <QtDebug>
#include <util/sys/paths.h>
#include "kbctl.h"

namespace LC
{
namespace KBSwitch
{
	QuarkProxy::QuarkProxy (QObject *parent)
	: QObject (parent)
	{
		connect (&KBCtl::Instance (),
				SIGNAL (groupChanged (int)),
				this,
				SLOT (handleGroupChanged (int)));
		handleGroupChanged (KBCtl::Instance ().GetCurrentGroup ());
	}

	QString QuarkProxy::GetCurrentLangCode () const
	{
		return CurrentLangCode_;
	}

	void QuarkProxy::setNextLanguage ()
	{
		KBCtl::Instance ().EnableNextGroup ();
	}

	void QuarkProxy::contextMenuRequested ()
	{
		QMenu menu;

		auto& kbctl = KBCtl::Instance ();
		const auto& enabled = kbctl.GetEnabledGroups ();
		const auto curGrpIdx = kbctl.GetCurrentGroup ();

		for (int i = 0; i < enabled.size (); ++i)
		{
			auto actionName = QString ("%1 (%2)")
					.arg (kbctl.GetLayoutDesc (i))
					.arg (enabled.at (i));

			const auto& variant = kbctl.GetGroupVariant (i);
			if (!variant.isEmpty ())
				actionName += " (" + variant + ")";

			const auto& iconPath = Util::GetSysPath (Util::SysPath::Share,
					"global_icons/flags", enabled.at (i) + ".png");

			const auto act = menu.addAction (QIcon { iconPath },
					actionName,
					this,
					SLOT (handleGroupSelectAction ()));
			act->setCheckable (true);
			if (curGrpIdx == i)
				act->setChecked (true);
			act->setProperty ("KBSwitch/GrpIdx", i);
		}

		menu.exec (QCursor::pos ());
	}

	void QuarkProxy::handleGroupSelectAction ()
	{
		const auto grpIdx = sender ()->property ("KBSwitch/GrpIdx").toInt ();
		KBCtl::Instance ().EnableGroup (grpIdx);
	}

	void QuarkProxy::handleGroupChanged (int group)
	{
		CurrentLangCode_ = KBCtl::Instance ().GetLayoutName (group);
		emit currentLangCodeChanged ();
	}
}
}
