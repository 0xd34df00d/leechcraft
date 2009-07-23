#ifndef PLUGINS_LCFTP_STRUCTURES_H
#define PLUGINS_LCFTP_STRUCTURES_H
#include <QUrl>
#include <QString>
#include <QDateTime>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct TaskData
			{
				int ID_;
				QUrl URL_;
				QString Filename_;
			};

			struct FetchedEntry
			{
				QUrl URL_;
				quint64 Size_;
				QDateTime DateTime_;
				bool IsDir_;
				QString Name_;
				TaskData PreviousTask_;
			};
		}
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::LCFTP::TaskData);
Q_DECLARE_METATYPE (LeechCraft::Plugins::LCFTP::FetchedEntry);

#endif

