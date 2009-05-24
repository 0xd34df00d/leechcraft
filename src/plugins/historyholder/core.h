#ifndef PLUGINS_HISTORYHOLDER_CORE_H
#define PLUGINS_HISTORYHOLDER_CORE_H
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <interfaces/ihaveshortcuts.h>

class QToolBar;

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
				boost::shared_ptr<QToolBar> ToolBar_;
				ICoreProxy_ptr CoreProxy_;
				QAction *Remove_;

				enum Shortcuts
				{
					SRemove
				};
			public:
				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetCoreProxy () const;
				void Handle (const LeechCraft::DownloadEntity&);

				void SetShortcut (int, const QKeySequence&);
				QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;

				int columnCount (const QModelIndex&) const;
				QVariant data (const QModelIndex&, int) const;
				QVariant headerData (int, Qt::Orientation, int) const;
				QModelIndex index (int, int, const QModelIndex&) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex&) const;
			private:
				void WriteSettings ();
			private slots:
				void remove ();
				void handleActivated (const QModelIndex&);
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::HistoryHolder::Core::HistoryEntry);

#endif

