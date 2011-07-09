/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef INTERFACES_ISCRIPTLOADER_H
#define INTERFACES_ISCRIPTLOADER_H
#include <QVariant>

class IScript
{
public:
	virtual ~IScript () {}

	virtual QVariant InvokeMethod (const QString& name,
			const QVariantList& args) const = 0;
};

class IScriptLoaderInstance
{
public:
	virtual ~IScriptLoaderInstance () {}
	
	virtual QObject* GetObject () = 0;
	
	virtual void AddGlobalPrefix () = 0;

	virtual void AddLocalPrefix (QString prefix = QString ()) = 0;
	
	virtual QStringList EnumerateScripts () const = 0;
	
	virtual QVariantMap GetScriptInfo (const QString&) = 0;
	
	virtual IScript* LoadScript (const QString&) = 0;
};

class IScriptLoader
{
public:
	virtual ~IScriptLoader () {}
	
	virtual IScriptLoaderInstance* CreateScriptLoaderInstance (const QString& relPath) = 0;
};

Q_DECLARE_INTERFACE (IScript, "org.Deviant.LeechCraft.IScript/1.0");
Q_DECLARE_INTERFACE (IScriptLoaderInstance, "org.Deviant.LeechCraft.IScriptLoaderInstance/1.0");
Q_DECLARE_INTERFACE (IScriptLoader, "org.Deviant.LeechCraft.IScriptLoader/1.0");

#endif
