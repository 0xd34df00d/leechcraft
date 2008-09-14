#ifndef UTIL_H
#define UTIL_H
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

		LEECHCRAFT_API QTranslator* InstallTranslator (const QString&);
	};
};

#endif

