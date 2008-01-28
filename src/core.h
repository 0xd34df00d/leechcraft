#ifndef CORE_H
#define CORE_H
#include <QAbstractListModel>
#include <QList>
#include <QMultiMap>
#include <QFile>
#include <QPair>
#include "common.h"
#include "pluginmanager.h"
#include "plugininfo.h"
#include "interfaces/interfaces.h"

class QString;

namespace Main
{
	class MainWindow;
	class Core : public QAbstractTableModel
	{
		Q_OBJECT

		QList<int> TasksIDPool_;
		PluginManager *PluginManager_;
		MainWindow *ReallyMainWindow_;
	public:
		Core (QObject *parent = 0);
		~Core ();

		void SetReallyMainWindow (MainWindow*);
		MainWindow *GetReallyMainWindow ();

		void DelayedInit ();
		void InitTask (const QString&);
		bool ShowPlugin (IInfo::ID_t);
		void HideAll ();
		void TryToAddJob (const QString&);
		
		QPair<qint64, qint64> GetSpeeds () const;

		virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
		virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
		virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
		virtual Qt::ItemFlags flags (const QModelIndex& index) const;
		virtual bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
		virtual bool removeRows (int pos, int rows, const QModelIndex& parent = QModelIndex ());
		virtual QModelIndex parent (const QModelIndex& index = QModelIndex ());
	private slots:
		void invalidate (unsigned int);
		void handleFileDownload (const QString&);
	private:
		void PreparePools ();
		void FetchPlugins ();

		QVariant GetTaskData (int, int) const;
	signals:
		void error (QString);
		void pushTask (const QString&, int);
		void gotPlugin (const PluginInfo*);
		void hidePlugins ();
		void downloadFinished (const QString&);
	};
};

#endif

