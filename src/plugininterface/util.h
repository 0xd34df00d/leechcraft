#ifndef LEECHCRAFT_UTIL_UTIL_H
#define LEECHCRAFT_UTIL_UTIL_H
#include "config.h"

class QString;
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
		 * @exception std::runtime_error Throws if the path could not be
		 * created.
		 */
		LEECHCRAFT_API void CreateIfNotExists (const QString& path);
	};
};

#endif

