#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <QString>
#include <QPair>
#include "pluginmanager.h"
#include "interfaces/interfaces.h"

class QTimer;
class QLocalServer;
class QAbstractProxyModel;
class MergeModel;
class QAction;
class FilterModel;

namespace Main
{
	class MainWindow;
    class Core : public QObject
    {
        Q_OBJECT

        PluginManager *PluginManager_;
		MainWindow *ReallyMainWindow_;
        QTimer *ClipboardWatchdog_;
        QString PreviousClipboardContents_;
		std::auto_ptr<QLocalServer> Server_;
		std::auto_ptr<MergeModel> MergeModel_;
		std::auto_ptr<MergeModel> HistoryMergeModel_;
		std::auto_ptr<FilterModel> FilterModel_;
		std::auto_ptr<FilterModel> HistoryFilterModel_;
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

		void SetReallyMainWindow (Main::MainWindow*);

		QAbstractItemModel* GetPluginsModel () const;
		QAbstractProxyModel* GetTasksModel () const;
		QAbstractProxyModel* GetHistoryModel () const;
		MergeModel* GetUnfilteredTasksModel () const;
		MergeModel* GetUnfilteredHistoryModel () const;
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

		virtual bool eventFilter (QObject*, QEvent*);
	public slots:
		void handleProxySettings () const;
		void handlePluginAction ();
    private slots:
        void handleFileDownload (const QString&, bool = false);
        void handleClipboardTimer ();
		void embeddedTabWantsToFront ();
		void handleNewTab (const QString&, QWidget*);
		void handleRemoveTab (QWidget*);
		void handleRemoveTab (int);
		void handleChangeTabName (QWidget*, const QString&);
		void handleChangeTabIcon (QWidget*, const QIcon&);
	private:
		int FindTabForWidget (QWidget*) const;
		QModelIndex MapToSource (const QModelIndex&) const;
    signals:
        void error (QString);
        void downloadFinished (const QString&);
    };
};

#endif

