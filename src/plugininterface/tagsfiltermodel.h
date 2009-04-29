#ifndef PLUGININTERFACE_TAGSFILTERMODEL_H
#define PLUGININTERFACE_TAGSFILTERMODEL_H
#include <QSortFilterProxyModel>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class PLUGININTERFACE_API TagsFilterModel : public QSortFilterProxyModel
		{
			Q_OBJECT

			bool NormalMode_;
		public:
			TagsFilterModel (QObject *parent = 0);
		public slots:
			void setTagsMode (bool);
		protected:
			virtual bool filterAcceptsRow (int, const QModelIndex&) const;
			virtual QStringList GetTagsForIndex (int) const = 0;
		};
	};
};

#endif

