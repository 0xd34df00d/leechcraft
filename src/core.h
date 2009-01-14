#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <QString>
#include <QPair>
#include "pluginmanager.h"
#include "tabcontainer.h"
#include "plugininterface/mergemodel.h"
#include "interfaces/interfaces.h"

class QTimer;
class QLocalServer;
class QAbstractProxyModel;
class QAction;

namespace LeechCraft
{
	class FilterModel;
	class MainWindow;
    class Core : public QObject
    {
        Q_OBJECT

        PluginManager *PluginManager_;
		MainWindow *ReallyMainWindow_;
        QTimer *ClipboardWatchdog_;
        QString PreviousClipboardContents_;
		std::auto_ptr<QLocalServer> Server_;
		std::auto_ptr<Util::MergeModel> MergeModel_;
		std::auto_ptr<Util::MergeModel> HistoryMergeModel_;
		std::auto_ptr<FilterModel> FilterModel_;
		std::auto_ptr<FilterModel> HistoryFilterModel_;
		std::auto_ptr<TabContainer> TabContainer_;
		typedef std::map<const QAbstractItemModel*, QObject*> repres2object_t;
		mutable repres2object_t Representation2Object_;
		mutable repres2object_t History2Object_;
		typedef std::map<const QAction*, QAbstractItemModel*> action2model_t;
		mutable action2model_t Action2Model_;

        Core ();
    public:
		enum FilterType
		{
			FTFixedString
			, FTWildcard
			, FTRegexp
			, FTTags
		};

        virtual ~Core ();
		static Core& Instance ();
        void Release ();

		void SetReallyMainWindow (MainWindow*);

		QObjectList GetSettables () const;
		QAbstractItemModel* GetPluginsModel () const;
		QAbstractProxyModel* GetTasksModel () const;
		QAbstractProxyModel* GetHistoryModel () const;
		Util::MergeModel* GetUnfilteredTasksModel () const;
		Util::MergeModel* GetUnfilteredHistoryModel () const;
		QWidget* GetControls (const QModelIndex&) const;
		QWidget* GetAdditionalInfo (const QModelIndex&) const;

		QStringList GetTagsForIndex (int, QAbstractItemModel*) const;

        void DelayedInit ();
        bool ShowPlugin (int);
        void TryToAddJob (const QString&, const QString&);

		void Activated (const QModelIndex&);
		void SetNewRow (const QModelIndex&);
		bool SameModel (const QModelIndex&, const QModelIndex&) const;
		void UpdateFiltering (const QString&, FilterType, bool, bool = false);
		void HistoryActivated (int);
        
        QPair<qint64, qint64> GetSpeeds () const;
		int CountUnremoveableTabs () const;

		virtual bool eventFilter (QObject*, QEvent*);
	public slots:
		void handleProxySettings () const;
		void handlePluginAction ();
		void toggleMultiwindow ();
		void deleteSelectedHistory (const QModelIndex&);
    private slots:
        void handleGotEntity (const QByteArray&, bool = false);
        void handleClipboardTimer ();
		void embeddedTabWantsToFront ();
		void handleStatusBarChanged (QWidget*, const QString&);
		void handleLog (const QString&);
	private:
		QModelIndex MapToSource (const QModelIndex&) const;
		void InitJobHolder (QObject*);
		void InitEmbedTab (QObject*);
		void InitMultiTab (QObject*);
    signals:
        void error (QString);
		void log (const QString&);
        void downloadFinished (const QString&);
		void loadProgress (const QString&);
    };
};

#endif

