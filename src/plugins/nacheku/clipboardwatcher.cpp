/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clipboardwatcher.h"
#include <QApplication>
#include <QTimer>
#include <QClipboard>
#include <util/xpc/util.h>
#include <util/sll/slotclosure.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Nacheku
{
	ClipboardWatcher::ClipboardWatcher (IEntityManager *iem, QObject *parent)
	: QObject { parent }
	{
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this, iem]
			{
				if (!XmlSettingsManager::Instance ()
						.property ("WatchClipboard").toBool ())
					return;

				const QString& text = QApplication::clipboard ()->text ();
				if (text.isEmpty () || text == PreviousClipboardContents_)
					return;

				PreviousClipboardContents_ = text;

				iem->HandleEntity (Util::MakeEntity (text.toUtf8 (),
						{},
						FromUserInitiated));
			},
			QApplication::clipboard (),
			SIGNAL (changed (QClipboard::Mode)),
			this
		};
	}
}
}

