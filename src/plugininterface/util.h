/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGININTERFACE_UTIL_H
#define PLUGININTERFACE_UTIL_H
#include "piconfig.h"
#include <QString>
#include <QDir>
#include <QModelIndex>
#include <QtXml/QDomElement>
#include <QtDebug>
#include <interfaces/structures.h>

class QTranslator;

namespace LeechCraft
{
	namespace Util
	{
		/** @brief An utility function that creates a QString from
		 * UTF8-encoded std::string.
		 *
		 * @param[in] str The UTF-8 encoded std::string.
		 * @return The QString containing the same string.
		 */
		inline QString FromStdString (const std::string& str)
		{
			return QString::fromUtf8 (str.c_str ());
		}

		template<typename In, typename Out, typename Pred>
		Out copy_if (In first, In last, Out res, Pred p)
		{
			while (first != last)
			{
				if (p (*first))
					*res++ = *first;
				++first;
			}
			return res;
		}

		template<typename T>
		struct ValueFinder
		{
			typedef typename T::data_type data_type;
			data_type Object_;

			ValueFinder (data_type data)
			: Object_ (data)
			{
			}

			bool operator() (typename T::value_type i)
			{
				return i.second == Object_;
			}
		};

		/** @brief Return the user-readable representation of the entity.
		 *
		 * @param[in] entity The Entity from which to make the
		 * user-readable representation.
		 * @return The user-readable string describing the entity.
		 */
		PLUGININTERFACE_API QString GetUserText (const Entity& entity);

		/** @brief Makes a formatted size from number.
		 *
		 * Converts, for example, 1048576 to 1.0 MB.
		 *
		 * @param[in] sourcesize Size in bytes.
		 * @return Formatted string.
		 *
		 * @sa SetStrings
		 * @sa MakeTimeFromLong
		 */
		PLUGININTERFACE_API QString MakePrettySize (qint64);

		/** @brief Makes a formatted time from number.
		 *
		 * Converts, for example 256 to 00:04:16.
		 *
		 * @param[in] time Time interval in seconds.
		 * @return DateTime object.
		 *
		 * @sa MakePrettySize
		 */
		PLUGININTERFACE_API QString MakeTimeFromLong (ulong);

		/** @brief Loads and installs a translator.
		 *
		 * Attempts to load and install a translator for the current
		 * locale. The name is formed like this:
		 * 'prefix_' + base + '_' + locale
		 * If base is an empty string, the second _ isn't appended.
		 *
		 * First resources are searched (:/), then APPDIR/translations
		 * on Windows and /usr/[local/]share/appname/translations on
		 * Unix.
		 *
		 * @param[in] base Base name of the translation file.
		 * @param[in] appname Base name of the application.
		 * @return The translator object if loading is successful, NULL
		 * otherwise.
		 */
		PLUGININTERFACE_API QTranslator* InstallTranslator (const QString& base,
				const QString& prefix = "leechcraft",
				const QString& appname = "leechcraft");

		/** @brief Returns the current locale name, like en_US.
		 *
		 * First, this function checks the locale value stored in
		 * "Language" key of settings object with organizationName() and
		 * applicationName(). If it's equal to "system", this function
		 * queries the LANG environment variable, and if it is empty or
		 * in invalid format (not like en_US), it takes the value of
		 * QLocale::system().name().
		 *
		 * Then, if the resulting name the name of the language only,
		 * GetLocaleName() tries to find any countries for that
		 * language. If any countries are found, the code of the first
		 * found country is appended, else "_00" is appended.
		 *
		 * @return Current locale name.
		 *
		 * @sa GetLanguage()
		 */
		PLUGININTERFACE_API QString GetLocaleName ();

		/** @brief Returns the current language name.
		 *
		 * This function works as GetLocaleName() except it doesn't
		 * return (and doesn't query for) country name.
		 *
		 * @return Current language name.
		 *
		 * @sa GetLocaleName()
		 */
		PLUGININTERFACE_API QString GetLanguage ();

		/** @brief Creates a path if it doesn't exist.
		 *
		 * Creates a relative path ~/.leechcraft/path and throws an
		 * exception if this could not be done or if such path already
		 * exists and it is not readable.
		 *
		 * @param[in] path The path to create.
		 * @return The newly created dir.
		 * @exception std::runtime_error Throws if the path could not be
		 * created.
		 */
		PLUGININTERFACE_API QDir CreateIfNotExists (const QString& path);

		/** @brief Returns the path relative to user directory.
		 *
		 * Returns a QDir object that represents the ~/.leechcraft/path
		 * or throws an exceptions if this path doesn't exist.
		 *
		 * @param[in] path The path to return relative to ~/.leechcraft
		 * @return The QDir object for that path.
		 * @exception std::runtime_error Throws if the path doesn't
		 * exist.
		 */
		PLUGININTERFACE_API QDir GetUserDir (const QString& path);

		/** @brief Returns a temporary filename.
		 *
		 * This function returns a name of a temporary file that could
		 * be created, not createing the file itself.
		 *
		 * @param[in] pattern Pattern of the filename.
		 * @return The filename.
		 */
		PLUGININTERFACE_API QString GetTemporaryName (const QString& pattern = QString ("lc_temp.XXXXXX"));

		/** @brief An utility function to make a Entity.
		 *
		 * Creates a Entity that wraps the given entity from
		 * given location with parameterrs identified by tp and given
		 * mime type (which is null by default).
		 *
		 * This function is provided for convenience and is equivalent
		 * to manually filling the Entity.
		 *
		 * @param[in] entity The Entity_ field of the Entity.
		 * @param[in] location The Location_ field of the Entity.
		 * @param[in] tp The Params_ field of the Entity.
		 * @param[in] mime The Mime_ field of the Entity.
		 * @return The resulting Entity.
		 *
		 * @sa Entity, MakeNotification()
		 */
		PLUGININTERFACE_API Entity MakeEntity (const QVariant& entity,
				const QString& location,
				LeechCraft::TaskParameters tp,
				const QString& mime = QString ());

		/** @brief An utility function to make a Entity with
		 * notification.
		 *
		 * Creates a Entity that holds information about
		 * user-visible notification. These notifications have
		 * "x-leechcraft/notification" MIME.
		 *
		 * You can further customize the returned Entity to suit
		 * your exact needs. See the documentation for Entity
		 * about such entities.
		 *
		 * @param[in] header The header of the notification.
		 * @param[in] text The text of the notification.
		 * @param[in] priority The priority level of the notification.
		 * @return The Entity containing the corresponding
		 * notification.
		 *
		 * @sa Entity, MakeEntity()
		 */
		PLUGININTERFACE_API Entity MakeNotification (const QString& header,
				const QString& text,
				Priority priority);

		PLUGININTERFACE_API QModelIndexList GetSummarySelectedRows (QObject *sender);

		/** @brief Returns the action that is set to act as a separator.
		 *
		 * That is the action with setSeparator(true);
		 *
		 * @param[in] parent The parent of the action.
		 * @return The separator action.
		 */
		PLUGININTERFACE_API QAction* CreateSeparator (QObject *parent);

		PLUGININTERFACE_API QVariantList GetPersistentData (const QList<QVariant>& keys,
				QObject *object);

		/** @brief Returns an element for a given tags list.
		 *
		 * This function tries to implement projection from tags to a
		 * hierarchical structure in form of XML. It traverses the tags
		 * list and creates child nodes from the document, appending
		 * the hierarchical structure's tree root to the node. It
		 * returns the parent element to which the item should be
		 * appended.
		 *
		 * For empty tags list it just returns node converted to the
		 * QDomElement.
		 *
		 * tagSetter is a function or functor that should be able to
		 * take two parameters, a QDomElement and a QString, and set
		 * tags for it.
		 *
		 * tagGetter is a function or functor that should be able to
		 * take one parameter, a QDomElement, and return a QString for
		 * it with tags previously set with tagSetter.
		 *
		 * @param[in] tags List of tags.
		 * @param[in] node The parent-most node to which all other nodes
		 * are appended.
		 * @param[in] document The document containing all these nodes.
		 * @param[in] elementname The name of the XML element that
		 * carries info about the tags.
		 * @param[in] tagSetter Setter function for the tags for the
		 * given element.
		 * @param[in] tagGetter Getter function for the tags for the
		 * given element.
		 * @return Parent element of the item with tags.
		 */
		template<typename TagGetter, typename TagSetter>
		QDomElement GetElementForTags (const QStringList& tags,
				QDomNode& node,
				QDomDocument& document,
				const QString& elementName,
				TagGetter tagGetter,
				TagSetter tagSetter)
		{
			if (!tags.size ())
			{
				qWarning () << Q_FUNC_INFO
					<< "no tags"
					<< elementName;
				return node.toElement ();
			}

			QDomNodeList elements = node.childNodes ();
			for (int i = 0; i < elements.size (); ++i)
			{
				QDomElement elem = elements.at (i).toElement ();
				if (tagGetter (elem) == tags.at (0))
				{
					if (tags.size () > 1)
					{
						QStringList childTags = tags;
						childTags.removeAt (0);
						return GetElementForTags (childTags, elem,
								document, elementName,
								tagGetter, tagSetter);
					}
					else
						return elem;
				}
			}

			QDomElement result = document.createElement (elementName);
			tagSetter (result, tags.at (0));
			node.appendChild (result);
			if (tags.size () > 1)
			{
				QStringList childTags = tags;
				childTags.removeAt (0);
				return GetElementForTags (childTags, result,
						document, elementName,
						tagGetter, tagSetter);
			}
			else
				return result;
		}
	};
};

#endif

