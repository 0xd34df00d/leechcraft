#ifndef PLUGINS_HISTORYHOLDER_CORE_H
#define PLUGINS_HISTORYHOLDER_CORE_H
#include <QAbstractItemModel>
#include <QDateTime>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace HistoryHolder
		{
			class Core : public QAbstractItemModel
			{
				Q_OBJECT

				Core ();

			public:
				struct HistoryEntry
				{
					LeechCraft::DownloadEntity Entity_;
					QDateTime DateTime_;
				};
			private:
				typedef QList<HistoryEntry> History_t;
				History_t History_;
				QStringList Headers_;
			public:
				enum Roles
				{
					RTags = Qt::UserRole + 1
				};

				static Core& Instance ();
				void Release ();
				void Handle (const LeechCraft::DownloadEntity&);
				void Remove (const QModelIndex&);

				int columnCount (const QModelIndex&) const;
				QVariant data (const QModelIndex&, int) const;
				QVariant headerData (int, Qt::Orientation, int) const;
				QModelIndex index (int, int, const QModelIndex&) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex&) const;
			private:
				void WriteSettings ();
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::HistoryHolder::Core::HistoryEntry);

#endif

