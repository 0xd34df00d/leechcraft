/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Xoox
{
	class GlooxProtocol;

	class RosterSaver : public QObject
	{
		Q_OBJECT

		GlooxProtocol * const Proto_;
		IProxyObject * const Proxy_;

		bool SaveRosterScheduled_ = false;
	public:
		RosterSaver (GlooxProtocol*, IProxyObject*, QObject* = nullptr);
	private:
		void LoadRoster ();

	private slots:
		void scheduleSaveRoster (int = 2000);

		void saveRoster ();

		void handleAccount (QObject*);
		void checkItemsInvalidation (const QList<QObject*>&);
	};
}
}
}
