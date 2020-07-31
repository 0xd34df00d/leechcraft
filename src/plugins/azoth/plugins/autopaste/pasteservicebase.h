/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <QNetworkReply>
#include <interfaces/core/icoreproxy.h>

class QNetworkAccessManager;

namespace LC
{
struct Entity;

namespace Azoth::Autopaste
{
	enum class Highlight
	{
		None,
		C,
		CPP,
		Haskell,
		Java,
		Python,
		Shell,
		XML
	};

	class PasteServiceBase : public QObject
	{
		const ICoreProxy_ptr Proxy_;
		QPointer<QObject> Entry_;
	public:
		struct PasteParams
		{
			QNetworkAccessManager *NAM_;
			QString Text_;
			Highlight High_;
		};

		PasteServiceBase (QObject *entry, const ICoreProxy_ptr&, QObject* = nullptr);

		virtual void Paste (const PasteParams&) = 0;
	protected:
		void InitReply (QNetworkReply*);
		void FeedURL (const QString&);
		void HandleError (QNetworkReply::NetworkError, QNetworkReply*);

		virtual void HandleFinished (QNetworkReply*);
		virtual void HandleMetadata (QNetworkReply*);
	};
}
}
