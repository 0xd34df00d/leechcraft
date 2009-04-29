#ifndef PLUGININTERFACE_UTIL_H
#define PLUGININTERFACE_UTIL_H
#include "config.h"
#include <QString>
#include <QDir>
#include <QtXml/QDomElement>

class QTranslator;

namespace LeechCraft
{
	namespace Util
	{
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

		/** @brief Loads and installs a translator.
		 *
		 * Attempts to load from resources (:/) and install a translator
		 * for the current locale. The name is formed like this:
		 * ':/leechcraft_' + base + '_' + locale
		 *
		 * @param[in] base Base name of the translation file.
		 */
		PLUGININTERFACE_API QTranslator* InstallTranslator (const QString& base);

		/** @brief Creates a path if it isn't existing.
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

		/** @brief Returns a temporary filename.
		 *
		 * This function returns a name of a temporary file that could
		 * be created, not createing the file itself.
		 *
		 * @param[in] pattern Pattern of the filename.
		 * @return The filename.
		 */
		PLUGININTERFACE_API QString GetTemporaryName (const QString& pattern = QString ("lc_temp.XXXXXX"));

		template<typename TagGetter, typename TagSetter>
		QDomElement GetElementForTags (const QStringList& tags,
				QDomNode& node,
				QDomDocument& document,
				const QString& elementName,
				TagGetter tagGetter,
				TagSetter tagSetter)
		{
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

