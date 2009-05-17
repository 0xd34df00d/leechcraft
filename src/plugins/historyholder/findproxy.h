#ifndef PLUGINS_HISTORYHOLDER_FINDPROXY_H
#define PLUGINS_HISTORYHOLDER_FINDPROXY_H
#include <QObject>
#include <plugininterface/tagsfiltermodel.h>
#include <interfaces/ifinder.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace HistoryHolder
		{
			class FindProxy : public LeechCraft::Util::TagsFilterModel
							, public IFindProxy
			{
				Q_OBJECT
				Q_INTERFACES (IFindProxy);
			public:
				FindProxy (const LeechCraft::Request&);
				QAbstractItemModel* GetModel ();
			protected:
				QStringList GetTagsForIndex (int) const;
			};
		};
	};
};

#endif

