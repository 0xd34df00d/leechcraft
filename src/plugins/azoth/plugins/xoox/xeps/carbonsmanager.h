/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>

class QXmppMessage;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class CarbonsManager : public QXmppClientExtension
	{
		Q_OBJECT

		QString LastReqId_;
		bool LastReqState_;

		bool LastConfirmedState_ = false;
	public:
		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement& stanza);

		void SetEnabled (bool);
		bool IsEnabled () const;

		bool CheckMessage (const QXmppMessage&);

		void ExcludeMessage (QXmppMessage&);
	private:
		void HandleMessage (const QXmppElement&);
	signals:
		void stateChanged (bool);
		void stateChangeError (const QXmppIq&);

		void gotMessage (const QXmppMessage&);
	};
}
}
}
