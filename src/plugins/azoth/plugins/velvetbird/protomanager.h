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
#include <blist.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
struct Entity;

namespace Azoth
{
namespace VelvetBird
{
	class Protocol;

	class ProtoManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QList<std::shared_ptr<Protocol>> Protocols_;
	public:
		ProtoManager (ICoreProxy_ptr, QObject*);

		void PluginsAvailable ();

		void Release ();

		QList<QObject*> GetProtoObjs () const;

		void Show (PurpleBuddyList*);
		void Update (PurpleBuddyList*, PurpleBlistNode*);
		void Remove (PurpleBuddyList*, PurpleBlistNode*);
	};
}
}
}
