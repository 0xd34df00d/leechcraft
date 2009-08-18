#include "findproxy.h"
#include "core.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;
using namespace LeechCraft::Plugins::Chatter;

FindProxy::FindProxy (const Request& r)
{
	setSourceModel (&Core::Instance ());
	setDynamicSortFilter (true);

	setFilterCaseSensitivity (r.CaseSensitive_ ?
			Qt::CaseSensitive : Qt::CaseInsensitive);

	switch (r.Type_)
	{
		case Request::RTWildcard:
			setFilterWildcard (r.String_);
			break;
		case Request::RTRegexp:
			setFilterRegExp (r.String_);
			break;
		default:
			setFilterFixedString (r.String_);
			if (r.Type_ == Request::RTTag)
				setTagsMode (true);
			break;
	}
}

QAbstractItemModel* FindProxy::GetModel ()
{
	return this;
}

QStringList FindProxy::GetTagsForIndex (int row) const
{
	return sourceModel ()->data (sourceModel ()->
			index (row, 0), RoleTags).toStringList ();
}

