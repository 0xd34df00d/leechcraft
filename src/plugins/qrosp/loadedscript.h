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

namespace Qross
{
	class Action;
}

namespace LC
{
namespace Qrosp
{
	class LoadedScript : public QObject
					   , public IScript
	{
		Q_OBJECT
		Q_INTERFACES (IScript)

		Qross::Action * const ScriptAction_;
		bool Imported_ = false;
	public:
		LoadedScript (const QString&, const QString&, QObject* = nullptr);

		QObject* GetQObject ();
		QVariant InvokeMethod (const QString&, const QVariantList&);
		void AddQObject (QObject* object, const QString& name);
		void Execute ();
	private:
		void CheckImports ();
	};
}
}
