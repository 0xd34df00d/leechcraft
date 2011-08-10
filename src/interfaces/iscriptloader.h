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
#include <boost/shared_ptr.hpp>
#include <QVariant>

class IScript
{
public:
	virtual ~IScript () {}

	virtual QVariant InvokeMethod (const QString& name,
			const QVariantList& args) const = 0;
};

typedef boost::shared_ptr<IScript> IScript_ptr;

class IScriptLoaderInstance
{
public:
	virtual ~IScriptLoaderInstance () {}
	
	virtual QObject* GetObject () = 0;
	
	virtual void AddGlobalPrefix () = 0;

	virtual void AddLocalPrefix (QString prefix = QString ()) = 0;
	
	virtual QStringList EnumerateScripts () const = 0;
	
	virtual QVariantMap GetScriptInfo (const QString&) = 0;
	
	/** @brief Loads the given script.
	 * 
	 * This method loads the script identified by the script, and
	 * returns an object used to communicate with the script, or a null
	 * pointer if the given script could not be loaded.
	 * 
	 * Please refer to this class' documentation for more information
	 * regarding script paths.
	 * 
	 * @note Implementations may choose to return a valid pointer even
	 * if the script is loaded correctly. In this case, all operations
	 * on it will fail, though.
	 * 
	 * @param[in] script The script base name.
	 * @return The script wrapper object.
	 */
	virtual IScript_ptr LoadScript (const QString& script) = 0;
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
