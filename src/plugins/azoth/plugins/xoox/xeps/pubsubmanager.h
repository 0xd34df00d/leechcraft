/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QSet>
#include <QXmppClientExtension.h>
#include "pepeventbase.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PEPEventBase;

	class PubSubManager : public QXmppClientExtension
	{
		Q_OBJECT
	public:
		using Creator_t = std::function<PEPEventBase* ()>;
	private:
		QMap<QString, Creator_t> Node2Creator_;

		QSet<QString> AutosubscribeNodes_;
	public:
		template<typename T>
		void RegisterCreator ()
		{
			RegisterCreator (T::GetNodeString (), StandardCreator<T>);
		}
		void RegisterCreator (const QString&, const Creator_t&);

		template<typename T>
		void SetAutosubscribe (bool enabled)
		{
			SetAutosubscribe (T::GetNodeString (), enabled);
		}
		void SetAutosubscribe (QString, bool);

		void PublishEvent (PEPEventBase*);

		void RequestItem (const QString& jid,
				const QString& node, const QString& id);

		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement& elem);
	private:
		bool HandleIq (const QDomElement&);
		bool HandleMessage (const QDomElement&);
		void ParseItems (QDomElement, const QString&);
	signals:
		void gotEvent (const QString&, PEPEventBase*);
	};
}
}
}
