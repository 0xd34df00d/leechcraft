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

class QLocalServer;

namespace LC
{
	struct Entity;

	class EntityManager;

	class LocalSocketHandler : public QObject
	{
		Q_OBJECT

		const std::unique_ptr<QLocalServer> Server_;
		const std::unique_ptr<EntityManager> EM_;
	public:
		explicit LocalSocketHandler ();
		~LocalSocketHandler ();
	private slots:
		void handleNewLocalServerConnection ();
	private:
		void StartServer ();
	};
};
