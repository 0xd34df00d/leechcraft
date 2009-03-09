#ifndef LEECHCRAFT_UTIL_UTIL_H
#define LEECHCRAFT_UTIL_UTIL_H
#include "config.h"
#include <QString>
#include <QDir>

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
		LEECHCRAFT_API QTranslator* InstallTranslator (const QString& base);

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
		LEECHCRAFT_API QDir CreateIfNotExists (const QString& path);

		/** @brief Returns a temporary filename.
		 *
		 * This function returns a name of a temporary file that could
		 * be created, not createing the file itself.
		 *
		 * @param[in] pattern Pattern of the filename.
		 * @return The filename.
		 */
		LEECHCRAFT_API QString GetTemporaryName (const QString& pattern = QString ("lc_temp.XXXXXX"));
	};
};

#endif

