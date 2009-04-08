#ifndef CORE_H
#define CORE_H
#include <memory>
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QString>
#include <QPair>
#include <QTimer>
#include <QNetworkAccessManager>
#include <interfaces/structures.h>
#include "pluginmanager.h"
#include "tabcontainer.h"
#include "plugininterface/mergemodel.h"
#include "storagebackend.h"
#include "requestnormalizer.h"

class QLocalServer;
class QAbstractProxyModel;
class QAction;
class IDownload;
class IShortcutProxy;

namespace LeechCraft
{
	class FilterModel;
	class MainWindow;

	/** Contains all the plugins' models, maps from end-user's tree view
	 * to plugins' models and much more.
	 */
    class Core : public QObject
    {
        Q_OBJECT

        PluginManager *PluginManager_;
		MainWindow *ReallyMainWindow_;
        QTimer *ClipboardWatchdog_;
        QString PreviousClipboardContents_;
		std::auto_ptr<QLocalServer> Server_;
		boost::shared_ptr<Util::MergeModel> MergeModel_;
		boost::shared_ptr<Util::MergeModel> HistoryMergeModel_;
		std::auto_ptr<RequestNormalizer> RequestNormalizer_;
		std::auto_ptr<TabContainer> TabContainer_;
		std::auto_ptr<QNetworkAccessManager> NetworkAccessManager_;
		std::auto_ptr<QTimer> CookieSaveTimer_;
		std::auto_ptr<StorageBackend> StorageBackend_;
		typedef std::map<const QAbstractItemModel*, QObject*> repres2object_t;
		// Contains unfolded representations
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
		const IShortcutProxy* GetShortcutProxy () const;

		/** Returns all plugins that implement IHaveSettings as
		 * QObjectList.
		 *
		 * @return List of objects.
		 */
		QObjectList GetSettables () const;
		QObjectList GetShortcuts () const;
		QAbstractItemModel* GetPluginsModel () const;
		QAbstractItemModel* GetTasksModel () const;
		PluginManager* GetPluginManager () const;
		
		/** Returns controls for the model with a given index. The
		 * return value can't be NULL.
		 *
		 * The passed index shouldn't be mapped to source from filter
		 * model, Core will do it itself.
		 *
		 * @param[in] index Unmapped index for which the widget should
		 * be returned.
		 * @return Widget with controls, usually a toolbar.
		 *
		 * @sa GetAdditionalInfo
		 */
		QWidget* GetControls (const QModelIndex& index) const;

		/** Returns additional info for the model with a given index, or
		 * NULL if the model doesn't provide it.
		 *
		 * The passed index shouldn't be mapped to source from filter
		 * model, Core will do it itself.
		 *
		 * @param[in] index Unmapped index for which the widget should
		 * be returned.
		 * @return Widget with additional info/controls.
		 *
		 * @sa GetControls
		 */
		QWidget* GetAdditionalInfo (const QModelIndex& index) const;

		/** Returns list of tags for a given row using given model. It's
		 * assumed that the passed model is actually a MergeModel.
		 *
		 * @param[in] row The row in the merge model for which the tags
		 * should be retrieved.
		 * @param[in] model The MergeModel which contains the row.
		 * @return Tags for the row.
		 */
		QStringList GetTagsForIndex (int row, QAbstractItemModel *model) const;

        void DelayedInit ();
        bool ShowPlugin (int);
        void TryToAddJob (const QString&, const QString&);

		void SetNewRow (const QModelIndex&);

		/** Returns true if both indexes belong to the same model. If
		 * both indexes are invalid, true is returned.
		 *
		 * The passed indexes shouldn't be mapped to source from filter
		 * model or merge model, Core will do it itself.
		 *
		 * @param[in] i1 The first index.
		 * @param[in] i2 The second index.
		 * @return Whether the indexes belong to the same model.
		 */
		bool SameModel (const QModelIndex& i1, const QModelIndex& i2) const;
		void UpdateFiltering (const QString&);
		void Activated (const QModelIndex&);
        
        QPair<qint64, qint64> GetSpeeds () const;

		/** Counts how much tabs couldn't be removed. A tab is
		 * considered unremovable if it's from IEmbedTab or LeechCraft's
		 * internals.
		 *
		 * @return Number of unremovable tabs.
		 */
		int CountUnremoveableTabs () const;

		QNetworkAccessManager* GetNetworkAccessManager () const;

		virtual bool eventFilter (QObject*, QEvent*);
	public slots:
		void handleProxySettings () const;
		void handlePluginAction ();
		void toggleMultiwindow ();
		void deleteSelectedHistory (const QModelIndex&);
    private slots:
		/** Handles the entity which could be anything - path to a file,
		 * link, contents of a .torrent file etc. If the entity is a
		 * string, this parameter is considered to be an UTF-8
		 * representation of it.
		 *
		 * If id is not null and the job is handled by a downloader,
		 * the return value of IDownloader::AddJob() is assigned to *id.
		 * The same is with the provider.
		 *
		 * @param[in] entity DownloadEntity.
		 * @param[out] id The ID of the job if applicable.
		 * @param[out] provider The provider that downloads this job.
		 * @return True if the entity was actually handled.
		 */
		bool handleGotEntity (LeechCraft::DownloadEntity entity,
				int *id = 0, QObject **provider = 0);
		void handleCouldHandle (const LeechCraft::DownloadEntity&, bool*);
        void handleClipboardTimer ();
		void embeddedTabWantsToFront ();
		void handleStatusBarChanged (QWidget*, const QString&);
		void handleLog (const QString&);
		void handleAuthentication (QNetworkReply*, QAuthenticator*);
		void handleProxyAuthentication (const QNetworkProxy&, QAuthenticator*);
		void handleSslErrors (QNetworkReply*, const QList<QSslError>&);
		void pullCommandLine ();
		void handleNewLocalServerConnection ();
		void saveCookies () const;
	private:
		bool CouldHandle (const LeechCraft::DownloadEntity&);
		void DoCommonAuth (const QString&, QAuthenticator*);
		/** Maps totally unmapped index to the plugin's source model
		 * through merge model and filter model.
		 *
		 * @param[in] index The original unmapped index.
		 * @return Mapped index from the plugin's model.
		 *
		 * @exception std::runtime_error Throws if the required model
		 * could not be found.
		 */
		QModelIndex MapToSource (const QModelIndex&) const;
		void InitDynamicSignals (QObject*);
		void InitJobHolder (QObject*);
		void InitEmbedTab (QObject*);
		void InitMultiTab (QObject*);
		QModelIndex MapToSourceRecursively (QModelIndex) const;
    signals:
        void error (QString) const;
		void log (const QString&);
        void downloadFinished (const QString&);
		void loadProgress (const QString&);
    };
};

#endif

