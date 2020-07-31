/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>

namespace LC
{
namespace Azoth
{
struct RIEXItem;

namespace Xoox
{
	class EntryBase;

	class RIEXManager : public QXmppClientExtension
	{
		Q_OBJECT
	public:
		QStringList discoveryFeatures () const override;
		bool handleStanza (const QDomElement&) override;

		void SuggestItems (EntryBase *to, QList<RIEXItem> items, QString message = {});
	signals:
		void gotItems (QString from, QList<LC::Azoth::RIEXItem> items, const QString& body);
	};
}
}
}
