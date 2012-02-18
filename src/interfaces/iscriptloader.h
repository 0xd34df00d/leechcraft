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

#ifndef INTERFACES_ISCRIPTLOADER_H
#define INTERFACES_ISCRIPTLOADER_H
#include <memory>
#include <QVariant>

/** @brief Interface for objects representing scripts.
 *
 * This interface is to be implemented by the objects that represent
 * loaded scripts.
 */
class IScript
{
public:
	virtual ~IScript () {}

	/** @brief Runs the given function and returns its value.
	 *
	 * This method invokes the method identified by name with the given
	 * args, if any, and returns its value.
	 *
	 * If there is no such method or call has failed, the returned
	 * variant is null.
	 *
	 * @param[in] name The name of the method to invoke.
	 * @param[in] args The list of arguments for the method.
	 * @return The return value of the method, if any, or null variant
	 * if the call has failed.
	 */
	virtual QVariant InvokeMethod (const QString& name,
			const QVariantList& args) const = 0;
};

typedef std::shared_ptr<IScript> IScript_ptr;

/** @brief Interface for script loaders.
 *
 * Script loaders are those objects that are directly responsible for
 * loading scripts and exposing them as IScript. Scripts are loaded via
 * the LoadScript() method.
 *
 * The list of all available scripts with the current set of prefixes
 * could also be obtained via the
 *
 * The scripts are identified by their name. Script loader must properly
 * append the corresponding extension while checking for the existence
 * of the corresponding script.
 *
 * The scripts are loaded from the path which is built as concatenation
 * of prefix path (adjusted via AddGlobalPrefix() and AddLocalPrefix()
 * methods), relative path (which is passed to the
 * IScriptLoader::CreateScriptLoaderInstance() method as parameter) and
 * interpreter name. The following interpreter names are defined (and
 * the corresponding extensions):
 * - qtscript for *.qs, *.js, *.es.
 * - python for *.py
 * - ruby for *.rb
 *
 * Several local prefixes could be added by the consecutive calls to
 * AddGlobalPrefix() and AddLocalPrefix() respectively. The script
 * loader must check the prefixes in the reversed order of their
 * addition, so if you want the local scripts to have higher precedence
 * than global ones, you should call AddLocalPrefix() after calling
 * AddGlobalPrefix().
 *
 * @sa IScriptLoader, IScript
 */
class IScriptLoaderInstance
{
public:
	virtual ~IScriptLoaderInstance () {}

	/** @brief Returns this loader instance as a QObject.
	 *
	 * @return This script loader instance as a QObject.
	 */
	virtual QObject* GetObject () = 0;

	/** @brief Adds a global load prefix.
	 *
	 * This is /usr/[local/]share/leechcraft/scripts on Unix-like OSes
	 * and %APP_PATH%\share\scripts on Windows.
	 */
	virtual void AddGlobalPrefix () = 0;

	/** @brief Adds a local load prefix.
	 *
	 * This is ~/.leechcraft/data/scripts/ + prefix.
	 */
	virtual void AddLocalPrefix (QString prefix = QString ()) = 0;

	/** @brief Lists all available scripts.
	 *
	 * This function lists the names of all available scripts, ready to
	 * be passed to LoadScript(), from the locations available with the
	 * current set of prefixes.
	 *
	 * @return The list of available scripts.
	 */
	virtual QStringList EnumerateScripts () const = 0;

	/** @brief Returns the metadata of the given script.
	 */
	virtual QVariantMap GetScriptInfo (const QString& script) = 0;

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

/** @brief Interface for plugins providing scripting.
 *
 * This interface is to be implemented by plugins that support loading
 * scripts if they wish to provide scripting support for other plugins.
 *
 * Plugins that wish to load and execute scripts create instances of
 * script loaders and use them for loading script objects.
 *
 * See the documentation of IScriptLoaderInstance for details about
 * script paths and such.
 *
 * @sa IScriptLoaderInstance
 */
class IScriptLoader
{
public:
	virtual ~IScriptLoader () {}

	/** @brief Creates an instance of the script loader.
	 *
	 * The loader instance is the object that is used to load the
	 * scripts.
	 *
	 * The script loader loads them from the (prefix + relPath) path,
	 * where path is one of paths later added to the instance's list of
	 * search paths via the IScriptLoaderInstance::AddGlobalPrefix() and
	 * IScriptLoaderInstance::AddLocalPrefix() functions.
	 *
	 * Ownership is transferred to the caller.
	 *
	 * @param[in] relPath The relative path of the scripts to be loaded.
	 * @return The script loader for the given relative path.
	 */
	virtual IScriptLoaderInstance* CreateScriptLoaderInstance (const QString& relPath) = 0;
};

Q_DECLARE_INTERFACE (IScript, "org.Deviant.LeechCraft.IScript/1.0");
Q_DECLARE_INTERFACE (IScriptLoaderInstance, "org.Deviant.LeechCraft.IScriptLoaderInstance/1.0");
Q_DECLARE_INTERFACE (IScriptLoader, "org.Deviant.LeechCraft.IScriptLoader/1.0");

#endif
