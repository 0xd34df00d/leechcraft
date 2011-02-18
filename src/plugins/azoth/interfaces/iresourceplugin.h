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

#ifndef PLUGINS_AZOTH_INTERFACES_IRESOURCEPLUGIN_H
#define PLUGINS_AZOTH_INTERFACES_IRESOURCEPLUGIN_H
#include <QtGlobal>
#include <QStringList>
#include <QMap>
#include <QImage>

class QAbstractItemModel;
class QWebFrame;

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Base interface for specific resource sources.
	 * 
	 * All resource source loaders, like smile packs resource loader
	 * or chat window style resource loader derive from this interface.
	 * 
	 * In your plugin you should never derive from this interface
	 * directly, instead, a relevant interface should be used.
	 * 
	 * @sa ISmileResourceSource
	 */
	class IResourceSource
	{
	public:
		virtual ~IResourceSource () {}
		
		/** @brief Returns the model with the options for resource.
		 * 
		 * The model would be set as the datasource model for the
		 * corresponding combobox item in the settings dialog.
		 * 
		 * The model should have at least one column, and the text of
		 * the items in that column would be used in queries for the
		 * associated resources (see derived interfaces' documentation
		 * for more information).
		 * 
		 * @sa XmlSettingsDialog::SetDataSource(), ISmileResourceSource
		 */
		virtual QAbstractItemModel* GetOptionsModel () const = 0;
	};
	
	/** @brief Interface for smile resource loaders.
	 * 
	 * This interface should be used for resource sources that deal with
	 * loading different emoticon packs. For example, plugins that
	 * enable loading of Psi-style or QIP-style emoticon packs would
	 * return objects implementing this interface from the corresponding
	 * IResourcePlugin::GetResourceSources() method.
	 */
	class IEmoticonResourceSource : virtual public IResourceSource
	{
	public:
		virtual ~IEmoticonResourceSource () {}

		/** @brief Returns the strings that are replaceable with
		 * emoticons in the given pack.
		 * 
		 * This function is used to obtain the list of strings that
		 * could be replaced by an emoticonin the given emoticon pack.
		 * This function should return all possible variants for each
		 * emoticon, for example, ":)", ":-)" and "(:" for the same
		 * icon.
		 * 
		 * @param[in] pack The emoticon pack to query, which is one of
		 * items returned from GetOptionsModel().
		 * @return List of emoticon strings supported by this pack.
		 * 
		 * @sa IResourceSource::GetOptionsModel()
		 */
		virtual QStringList GetEmoticonStrings (const QString& pack) const = 0;

		/** @brief Returns emoticons and their string representations from
		 * the given emoticon pack.
		 * 
		 * The returned map should contain all the smiles present in the
		 * pack and their string representations — that is, the string
		 * that the smile should be replaced with. Obviously, string
		 * representation should be present in GetSmileStrings(), and
		 * GetImage() should return the corresponding smile for it.
		 * 
		 * @param[in] pack The smile pack to query, which is one of
		 * items returned from GetOptionsModel().
		 * @return All smiles in the pack and corresponding strings.
		 * 
		 * @sa IResourceSource::GetOptionsModel()
		 */
		virtual QMap<QImage, QString> GetReprImages (const QString& pack) const = 0;

		/** @brief Returns the data corresponding to the given smile.
		 * 
		 * Please note that this data most likely shouldn't be a
		 * serialized (or saved) QImage or QPixmap. Instead, unmodified
		 * contents of the corresponding file should be returned, since
		 * the image may contain animation, and conversion to QImage or
		 * QPixmap wouldn't preserve it.
		 * 
		 * The returned byte array most likely should be just a result
		 * of QIODevice::readAll(), and it will be inserted into
		 * corresponding place via the data URI scheme.
		 * 
		 * @param[in] pack The smile pack to use, which is one of items
		 * returned from GetOptionsModel().
		 * @param[in] string The string corresponding to the smile,
		 * which is one of strings in GetSmileStrings() for the same
		 * pack.
		 * 
		 * @return The unmodified contents of the file with the image.
		 * 
		 * @sa GetSmileStrings(), IResourceSource::GetOptionsModel()
		 */
		virtual QByteArray GetImage (const QString& pack, const QString& string) const = 0;
	};
	
	class IChatStyleResourceSource : public IResourceSource
	{
	public:
		virtual ~IChatStyleResourceSource () {}
		
		virtual QString GetHTMLTemplate (const QString& style, QObject *entry) const = 0;
		
		virtual bool AppendMessage (QWebFrame *frame, QObject *message,
				const QString& color, bool isHightlightMsg, bool isActiveChat) = 0;
				
		virtual void FrameFocused (QWebFrame *frame) = 0;
	};

	/** @brief Interface for plugins having resource sources, like smile
	 * support or chat window styles.
	 * 
	 * Each plugin that wishes to provide the Azoth core with various
	 * resource sources, like, support for loading of QIP- or Psi-style
	 * plugins or some special styles (like Adium ones, for example),
	 * should implement this interface. Since it's plugin for another
	 * plugin, it should also implement IPlugin2, and it should return
	 * "org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin"
	 * string from the IPlugin2::GetPluginClasses(), among others.
	 */
	class IResourcePlugin
	{
	public:
		virtual ~IResourcePlugin () {}
		
		/** @brief Returns the resource sources that this plugin provides.
		 * 
		 * Each object in this list should implement at least one
		 * interface derived from IResourceSource — that is,
		 * ISmileResourceSource.
		 * 
		 * @return List of resource sources.
		 * 
		 * @sa ISmileResourceSource
		 */
		virtual QList<QObject*> GetResourceSources () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IEmoticonResourceSource,
		"org.Deviant.LeechCraft.Azoth.IEmoticonResourceSource/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IChatStyleResourceSource,
		"org.Deviant.LeechCraft.Azoth.IChatStyleResourceSource/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IResourcePlugin,
		"org.Deviant.LeechCraft.Azoth.IResourcePlugin/1.0");

#endif
