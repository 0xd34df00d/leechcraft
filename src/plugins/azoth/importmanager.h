/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_IMPORTMANAGER_H
#define PLUGINS_AZOTH_IMPORTMANAGER_H
#include <QObject>
#include <interfaces/structures.h>

namespace LC
{
namespace Azoth
{
	class IAccount;

	class ImportManager : public QObject
	{
		Q_OBJECT

		QMap<QString, IAccount*> AccID2OurID_;
		QMap<QString, QList<Entity>> EntityQueues_;
	public:
		ImportManager (QObject* = 0);

		void HandleAccountImport (Entity);
		void HandleHistoryImport (Entity);
	private:
		IAccount* GetAccountID (Entity);
	};
}
}

#endif
