/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#pragma once

#include <QXmppClientExtension.h>
#include <interfaces/azoth/ihaveserverhistory.h>

class QXmppMessage;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class Xep0313PrefIq;

	class Xep0313Manager : public QXmppClientExtension
	{
		Q_OBJECT

		QMap<QString, SrvHistMessages_t> Messages_;
		QMap<QString, QString> QueryId2Jid_;

		int NextQueryNumber_ = 0;
	public:
		static bool Supports0313 (const QStringList& features);
		static QString GetNsUri ();

		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);

		void RequestPrefs ();
		void SetPrefs (const Xep0313PrefIq&);

		void RequestHistory (const QString& jid, QString baseId, int count);
	private:
		void HandleMessage (const QXmppElement&);
		void HandleHistoryQueryFinished (const QDomElement&);

		void HandlePrefs (const QDomElement&);
	signals:
		void gotPrefs (const Xep0313PrefIq&);

		void serverHistoryFetched (const QString&,
				const QString&, const SrvHistMessages_t&);
	};
}
}
}
