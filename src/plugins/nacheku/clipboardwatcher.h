/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <interfaces/structures.h>

class IEntityManager;

namespace LC
{
namespace Nacheku
{
	/** @brief Watches clipboard for downloadable contents.
	 *
	 * ClipboardWatcher polls the clipboard periodically for new content
	 * and emits a new entity if it's found.
	 *
	 * The entities emitted from ClipboardWatcher do have
	 * LC::FromUserInitiated flag set, Entity_ is an utf8-ed
	 * text from the clipboard.
	 */
	class ClipboardWatcher : public QObject
	{
		QString PreviousClipboardContents_;
	public:
		explicit ClipboardWatcher (IEntityManager*, QObject *parent = nullptr);
	};
}
}

