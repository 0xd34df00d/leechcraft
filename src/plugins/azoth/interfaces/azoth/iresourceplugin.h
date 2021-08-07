/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IRESOURCEPLUGIN_H
#define PLUGINS_AZOTH_INTERFACES_IRESOURCEPLUGIN_H
#include <QtGlobal>
#include <QStringList>
#include <QMap>
#include <QImage>

class QAbstractItemModel;

inline uint qHash (const QImage& image)
{
	return image.cacheKey ();
}

namespace LC
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
	 * @sa IEmoticonResourceSource
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
		 * @sa XmlSettingsDialog::SetDataSource()
		 * @sa IEmoticonResourceSource
		 * @sa IChatStyleResourceSource
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
		virtual QSet<QString> GetEmoticonStrings (const QString& pack) const = 0;

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
		virtual QHash<QImage, QString> GetReprImages (const QString& pack) const = 0;

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
	 * 
	 * @sa IEmoticonResourceSource, IChatStyleResourceSource
	 */
	class IResourcePlugin
	{
	public:
		virtual ~IResourcePlugin () {}
		
		/** @brief Returns the resource sources that this plugin provides.
		 * 
		 * Each object in this list should implement at least one
		 * interface derived from IResourceSource — that is,
		 * IEmoticonResourceSource or IChatStyleResourceSource.
		 * 
		 * @return List of resource sources.
		 * 
		 * @sa IEmoticonResourceSource, IChatStyleResourceSource
		 */
		virtual QList<QObject*> GetResourceSources () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IEmoticonResourceSource,
		"org.Deviant.LeechCraft.Azoth.IEmoticonResourceSource/1.0")
Q_DECLARE_INTERFACE (LC::Azoth::IResourcePlugin,
		"org.Deviant.LeechCraft.Azoth.IResourcePlugin/1.0")

#endif
