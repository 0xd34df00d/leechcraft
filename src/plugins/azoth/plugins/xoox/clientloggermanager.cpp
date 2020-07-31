/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clientloggermanager.h"
#include <QDir>
#include <QXmppClient.h>
#include <QXmppLogger.h>
#include <util/sys/paths.h>
#include "accountsettingsholder.h"
#include "clientconnection.h"

namespace LC::Azoth::Xoox
{
	ClientLoggerManager::ClientLoggerManager (QXmppClient& client, AccountSettingsHolder& settings, QObject *parent)
	: QObject { parent }
	, FileLogSink_ { new QXmppLogger { this } }
	{
		QFile::remove (Util::CreateIfNotExists ("azoth").filePath ("qxmpp.log"));

		auto [jid, bare] = ClientConnection::Split (settings.GetFullJID ());
		QString logName = jid + ".qxmpp.log";
		logName.replace ('@', '_');
		const QString& path = Util::CreateIfNotExists ("azoth/xoox/logs").filePath (logName);
		QFileInfo info (path);
		if (info.size () > 1024 * 1024 * 10)
			QFile::remove (path);

		const auto logger = new QXmppLogger { this };
		logger->setLoggingType (QXmppLogger::SignalLogging);
		logger->setMessageTypes (QXmppLogger::AnyMessage);
		connect (logger,
				&QXmppLogger::message,
				this,
				[this] (QXmppLogger::MessageType type, const QString& msg)
				{
					const auto& curPath = FileLogSink_->logFilePath ();
					if (!QFile::exists (curPath))
						FileLogSink_->reopen ();
					FileLogSink_->log (type, msg);

					if (Signaled_)
						EmitConsoleLog (type, msg);
				});
		client.setLogger (logger);

		FileLogSink_->setLogFilePath (path);
		FileLogSink_->setMessageTypes (QXmppLogger::AnyMessage);

		auto setFileLogging = [this] (bool fileLog)
		{
			FileLogSink_->setLoggingType (fileLog ? QXmppLogger::FileLogging : QXmppLogger::NoLogging);
		};
		connect (&settings,
				&AccountSettingsHolder::fileLogChanged,
				this,
				setFileLogging);
		setFileLogging (settings.GetFileLogEnabled ());
	}

	void ClientLoggerManager::SetSignaledLog (bool signaled)
	{
		Signaled_ = signaled;
	}

	void ClientLoggerManager::EmitConsoleLog (QXmppLogger::MessageType type, const QString& msg)
	{
		QString entryId;
		QDomDocument doc;
		if (doc.setContent (msg))
		{
			const auto& elem = doc.documentElement ();
			if (type == QXmppLogger::ReceivedMessage)
				entryId = elem.attribute ("from");
			else if (type == QXmppLogger::SentMessage)
				entryId = elem.attribute ("to");
		}

		switch (type)
		{
		case QXmppLogger::SentMessage:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PacketDirection::Out, entryId);
			break;
		case QXmppLogger::ReceivedMessage:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PacketDirection::In, entryId);
			break;
		default:
			break;
		}
	}

}
