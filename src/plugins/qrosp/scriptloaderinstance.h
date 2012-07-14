/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_QROSP_SCRIPTLOADERINSTANCE_H
#define PLUGINS_QROSP_SCRIPTLOADERINSTANCE_H
#include <QObject>
#include <QStringList>
#include <interfaces/iscriptloader.h>

namespace LeechCraft
{
namespace Qrosp
{
	class ScriptLoaderInstance : public QObject
							   , public IScriptLoaderInstance
	{
		Q_OBJECT
		Q_INTERFACES (IScriptLoaderInstance)
		
		mutable QHash<QString, QString> ID2Interpereter_;
		
		QString RelativePath_;
		QStringList Prefixes_;
	public:
		ScriptLoaderInstance (const QString&, QObject* = 0);
		
		QObject* GetObject ();
		void AddGlobalPrefix ();
		void AddLocalPrefix (QString prefix);
		QStringList EnumerateScripts () const;
		QVariantMap GetScriptInfo (const QString&);
		IScript_ptr LoadScript (const QString&);
	};
}
}

#endif
