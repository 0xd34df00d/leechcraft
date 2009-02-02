#ifndef TAGSCOMPLETIONMODEL_H
#define TAGSCOMPLETIONMODEL_H
#include <QStringListModel>
#include <QStringList>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class LEECHCRAFT_API TagsCompletionModel : public QStringListModel
		{
			Q_OBJECT
		public:
			TagsCompletionModel (QObject *parent = 0);

			void UpdateTags (const QStringList&);
		signals:
			void tagsUpdated (const QStringList&);
		};
	};
};

#endif

