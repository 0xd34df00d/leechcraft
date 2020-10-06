/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QPointer>
#include <QHash>

typedef struct Tox Tox;

namespace LC::Azoth::Sarin
{
	class ToxAccount;
	class ToxThread;
	class ChatMessage;

	class MessagesManager : public QObject
	{
		Q_OBJECT

		std::weak_ptr<ToxThread> Thread_;

		struct MessageSendResult
		{
			uint32_t Result_;

			QByteArray Privkey_;
			QPointer<ChatMessage> Msg_;
		};

		QHash<uint32_t, QPointer<ChatMessage>> MsgId2Msg_;
	public:
		explicit MessagesManager (ToxAccount*);

		void SendMessage (const QByteArray& privkey, ChatMessage*);
	private:
		void HandleReadReceipt (quint32);
		void HandleInMessage (qint32, const QString&);

		void SetThread (const std::shared_ptr<ToxThread>&);
	signals:
		void invokeHandleInMessage (qint32, const QString&);
		void invokeHandleReadReceipt (quint32);

		void gotMessage (const QByteArray&, const QString&);
	};
}
