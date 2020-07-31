/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <util/sys/resourceloader.h>
#include <interfaces/iscriptloader.h>

namespace LC
{
namespace Qrosp
{
	class ScriptLoaderInstance : public QObject
							   , public IScriptLoaderInstance
	{
		Q_OBJECT
		Q_INTERFACES (IScriptLoaderInstance)

		mutable QHash<QString, QString> ID2Interpereter_;

		Util::ResourceLoader Loader_;
	public:
		ScriptLoaderInstance (const QString&, QObject* = nullptr);

		QObject* GetQObject ();
		void AddGlobalPrefix ();
		void AddLocalPrefix (QString prefix);
		QStringList EnumerateScripts () const;
		QVariantMap GetScriptInfo (const QString&);
		IScript_ptr LoadScript (const QString&);
	};
}
}
