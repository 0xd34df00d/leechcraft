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

template<typename>
class QPointer;

class QXmppMessageReceiptManager;

namespace LC::Azoth::Xoox
{
	class GlooxMessage;

	class DeliveryReceiptsIntegrator : public QObject
	{
		QHash<QString, QPointer<GlooxMessage>> UndeliveredMessages_;
	public:
		explicit DeliveryReceiptsIntegrator (QXmppMessageReceiptManager&);

		void ProcessMessage (GlooxMessage&);
	};
}
