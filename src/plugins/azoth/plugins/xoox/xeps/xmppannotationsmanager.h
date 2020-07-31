/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>
#include "xmppannotationsiq.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPAnnotationsManager : public QXmppClientExtension
	{
		Q_OBJECT
	public:
		void SetNotes (const QList<XMPPAnnotationsIq::NoteItem>&);
		void RequestNotes ();

		bool handleStanza (const QDomElement &element);
	signals:
		void notesReceived (const QList<XMPPAnnotationsIq::NoteItem>&);
	};
}
}
}
