/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QVector>
#include "interfaces/core/ientitymanager.h"

namespace LC
{
	class EntityManager : public QObject
						, public IEntityManager
	{
		Q_OBJECT
		Q_INTERFACES (IEntityManager)

		QObject * const Plugin_;

		struct QueueEntry
		{
			Entity Entity_;
			QObject *Desired_ = nullptr;
		};
		QVector<QueueEntry> DelegateAfterInit_;
		QVector<QueueEntry> HandleAfterInit_;
	public:
		explicit EntityManager (QObject *plugin, QObject *parent);

		DelegationResult DelegateEntity (Entity, QObject* = nullptr) override;
		Q_INVOKABLE bool HandleEntity (LC::Entity, QObject* = nullptr) override;
		Q_INVOKABLE bool CouldHandle (const LC::Entity&) override;
		QList<QObject*> GetPossibleHandlers (const Entity&) override;
	private:
		template<typename F>
		std::optional<bool> EnsureUiThread (F&&);

		bool CheckInitStage (const Entity&, QObject*, QVector<QueueEntry>&);
		void RunQueues ();
	};
}
