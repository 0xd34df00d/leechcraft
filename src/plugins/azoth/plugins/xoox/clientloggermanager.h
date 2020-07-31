/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QXmppLogger.h>
#include <interfaces/azoth/ihaveconsole.h>

class QXmppClient;
class QXmppLogger;

namespace LC::Azoth::Xoox
{
	class AccountSettingsHolder;

	class ClientLoggerManager : public QObject
	{
		Q_OBJECT

		bool Signaled_ = false;
		QXmppLogger *FileLogSink_;
	public:
		ClientLoggerManager (QXmppClient&, AccountSettingsHolder&, QObject* = nullptr);

		void SetSignaledLog (bool);
	private:
		void EmitConsoleLog (QXmppLogger::MessageType, const QString&);
	signals:
		void gotConsoleLog (const QByteArray&, IHaveConsole::PacketDirection, const QString&);
	};
}
