/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_CAPSMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_CAPSMANAGER_H
#include <QObject>
#include <QXmppDiscoveryIq.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class CapsDatabase;

	class CapsManager : public QObject
	{
		Q_OBJECT

		ClientConnection *Connection_;
		CapsDatabase *DB_;
		QHash<QString, QString> Caps2String_;
	public:
		CapsManager (ClientConnection*);

		void FetchCaps (const QString&, const QByteArray&);
		QStringList GetRawCaps (const QByteArray&) const;
		QStringList GetCaps (const QByteArray&) const;
		QStringList GetCaps (const QStringList&) const;

		void SetIdentities (const QByteArray&, const QList<QXmppDiscoveryIq::Identity>&);
		QList<QXmppDiscoveryIq::Identity> GetIdentities (const QByteArray&) const;
	public slots:
		void handleInfoReceived (const QXmppDiscoveryIq&);
		void handleItemsReceived (const QXmppDiscoveryIq&);
	};
}
}
}

#endif
