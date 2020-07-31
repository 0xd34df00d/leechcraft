/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iscriptloader.h>
#include "effects.h"

class QTimer;
class QFileSystemWatcher;
class IPluginsManager;

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	class ScriptHandler : public QObject
	{
		Q_OBJECT

		IPluginsManager * const IPM_;

		QString Path_;
		QList<Effect_t> Effects_;

		IScript_ptr CurrentScript_;

		QTimer * const DelayTimer_;

		QFileSystemWatcher * const FileWatcher_;
	public:
		ScriptHandler (IPluginsManager*, QObject* = nullptr);

		void SetScriptPath (const QString&);
		QList<Effect_t> GetEffects () const;
	private slots:
		void reload ();
		void reevaluate ();
	signals:
		void effectsListChanged ();
	};
}
}
}
