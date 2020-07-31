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
#include "effects.h"

class QAction;

class IPluginsManager;

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	class EffectProcessor;
	class ScriptHandler;

	class ViewsManager : public QObject
	{
		Q_OBJECT

		QHash<QObject*, EffectProcessor*> View2Effect_;
		QHash<QObject*, QAction*> View2EnableAction_;

		ScriptHandler * const ScriptHandler_;
	public:
		ViewsManager (IPluginsManager*, QObject* = nullptr);

		void AddView (QWidget*);

		QAction* GetEnableAction (QWidget*) const;
	private:
		QList<Effect_t> GetCurrentEffects ();
	private slots:
		void handleViewDestroyed (QObject*);
		void handleEffectsChanged ();
	};
}
}
}
