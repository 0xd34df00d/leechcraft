#include "linkhistory.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			LinkHistory::LinkHistory (QObject *parent)
			: QWebHistoryInterface (parent)
			{
			}
			
			void LinkHistory::addHistoryEntry (const QString& url)
			{
				if (!History_.contains (url))
					History_ << url;
			}
			
			bool LinkHistory::historyContains (const QString& url) const
			{
				if (History_.contains (url))
					return true;
				return false;
			}
		};
	};
};

