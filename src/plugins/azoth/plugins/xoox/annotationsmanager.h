/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include "xeps/xmppannotationsiq.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class XMPPAnnotationsManager;

	class AnnotationsManager : public QObject
	{
		XMPPAnnotationsManager& XMPPAnnManager_;
		QHash<QString, XMPPAnnotationsIq::NoteItem> JID2Note_;
	public:
		explicit AnnotationsManager (ClientConnection&, QObject* = nullptr);
		
		XMPPAnnotationsIq::NoteItem GetNote (const QString&) const;
		void SetNote (const QString&, const XMPPAnnotationsIq::NoteItem&);
	};
}
}
}
