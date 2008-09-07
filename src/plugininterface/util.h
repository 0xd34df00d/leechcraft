#ifndef UTIL_H
#define UTIL_H
#include "config.h"

class QString;

namespace LeechCraft
{
	namespace Util
	{
		template<typename In, typename Out, typename Pred>
		LEECHCRAFT_API
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

		LEECHCRAFT_API void InstallTranslator (const QString&);
	};
};

#endif

