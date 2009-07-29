#ifndef PLUGINS_LCFTP_STRUCTURES_H
#define PLUGINS_LCFTP_STRUCTURES_H
#include <QUrl>
#include <QString>
#include <QDateTime>
#include <QMetaType>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct TaskData
			{
				enum Direction
				{
					DDownload,
					DUpload
				};
				Direction Direction_;

				int ID_;
				QUrl URL_;
				/** If the filename is empty, than only the listing is
				 * fetched.
				 */
				QString Filename_;
				/** If a task is internal, it wouldn't be announced to
				 * the outer world, and the fetched entries wouldn't be
				 * downloaded recursively by the Core.
				 */
				bool Internal_;
			};

			bool operator== (const TaskData&, const TaskData&);

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

