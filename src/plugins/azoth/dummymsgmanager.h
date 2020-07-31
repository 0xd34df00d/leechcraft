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

namespace LC
{
namespace Azoth
{
	class CoreMessage;
	class IMessage;

	class DummyMsgManager : public QObject
	{
		Q_OBJECT

		QHash<QObject*, QList<CoreMessage*>> Messages_;

		DummyMsgManager ();
	public:
		DummyMsgManager (const DummyMsgManager&) = delete;
		DummyMsgManager (DummyMsgManager&&) = delete;

		static DummyMsgManager& Instance ();

		void AddMessage (CoreMessage*);
		void ClearMessages (QObject*);

		QList<IMessage*> GetIMessages (QObject*) const;
	private slots:
		void entryDestroyed ();
	};
}
}
