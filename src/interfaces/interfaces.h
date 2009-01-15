#ifndef INTERFACES_H
#define INTERFACES_H
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtPlugin>
#include <QFlags>
#include <QIcon>
#include "structures.h"

class QAbstractItemModel;
class QModelIndex;
class QNetworkAccessManager;

namespace LeechCraft
{
	namespace Util
	{
		class HistoryModel;
		class XmlSettingsDialog;
	};
};

/** @brief Required interface for every plugin.
 *
 * This interface is a base for all plugins loadable by LeechCraft. If a
 * plugin doesn't provide this interface (qobject_cast<IInfo*> to it
 * fails), it would be considered as a malformed one and would be
 * unloaded.
 */
class IInfo
{
public:
	/** @brief Initializes the plugin.
	 *
	 * Init is called by the LeechCraft when it has finished
	 * initializing its core and is ready to initialize plugins.
	 * Generally, all initialization code should be placed into this
	 * method instead of plugin's instance object's constructor.
	 *
	 * @sa Release
	 */
    virtual void Init () = 0;
	/** @brief Returns the name of the plugin.
	 *
	 * This name is used only for the UI, all internals communicate with
	 * each other through pointers to QObjects representing plugin
	 * instance objects.
	 *
	 * @sa GetInfo
	 * @sa GetIcon
	 */
    virtual QString GetName () const = 0;
	/** @brief Returns the information string about the plugin.
	 *
	 * Information string is only used to describe the plugin to the
	 * user.
	 *
	 * @sa GetName
	 * @sa GetInfo
	 */
    virtual QString GetInfo () const = 0;
	/** @brief Returns the list of provided features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * @return List of provided features.
	 *
	 * @sa Needs
	 * @sa Uses
	 * @sa SetProvider
	 */
    virtual QStringList Provides () const = 0;
	/** @brief Returns the list of needed features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * If not all requirements are satisfied, LeechCraft would mark the
	 * plugin as unusable and would not make it active or use its
	 * features returned by Provides() in dependency calculations. So,
	 * the returned list should mention those features that plugin can't
	 * live without and would not work at all.
	 *
	 * There are also some special values:
	 * - *
	 *   Passes all plugins to the plugin.
	 * - services::historyModel
	 *   Pushes the pointer to merged but not filtered history model
	 *   into the plugin via pushHistoryModel(MergeModel*) slot.
	 * - services::downloadersModel
	 *   Pushes the pointer to merged but not filtered downloaders model
	 *   into the plugin via pushDownloadersModel(MergeModel*) slot.
	 *
	 * @return List of needed features.
	 *
	 * @sa Provides
	 * @sa Uses
	 * @sa SetProvider
	 */
    virtual QStringList Needs () const = 0;
	/** @brief Returns the list of used features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * If not all requirements are satisfied, LeechCraft would still
	 * consider the plugin as usable and functional, make it available
	 * to user and use the features returned by Provides() in dependency
	 * calculations. So, the return list should mention those features
	 * that plugin can use but which are not required to start it and do
	 * some basic work.
	 *
	 * @return List of used features.
	 *
	 * @sa Provides
	 * @sa Uses
	 * @sa SetProvider
	 */
    virtual QStringList Uses () const = 0;
	/** @brief Sets the provider plugin for a given feature.
	 *
	 * This function is called by LeechCraft after dependency
	 * calculations to inform plugin about other plugins which provide
	 * the required features.
	 *
	 * @param[in] object Pointer to plugin instance of feature provider.
	 * @param[in] feature The feature which this object provides.
	 *
	 * @sa Provides
	 * @sa Needs
	 * @sa Uses
	 */
    virtual void SetProvider (QObject* object,
			const QString& feature) = 0;
	/** @brief Destroys the plugin.
	 *
	 * This function is called to notify that the plugin would be
	 * unloaded soon - either the application is closing down or the
	 * plugin is unloaded for some reason. Plugin should free its
	 * resources and especially all the GUI stuff in this function
	 * instead of plugin instance's destructor.
	 *
	 * @sa Init
	 */
    virtual void Release () = 0;
	/** @brief Returns the plugin icon.
	 *
	 * The icon is used only in GUI stuff.
	 *
	 * @return Icon object.
	 *
	 * @sa GetName
	 * @sa GetInfo
	 */
    virtual QIcon GetIcon () const = 0;

	/** @brief Virtual destructor.
	 */
    virtual ~IInfo () {}
};

/** @brief Interface for plugins with their own main windows.
 *
 * If a plugin creates a main window and wants to show it upon some user
 * actions (like double-clicking plugin's name in plugins list), it
 * should implement this interface.
 */
class IWindow
{
public:
	/** @brief Sets the parent widget of the window.
	 *
	 * This function is called by the LeechCraft to inform the plugin
	 * about its parent widget.
	 *
	 * @param[in] parent Pointer to parent widget.
	 */
    virtual void SetParent (QWidget *parent) = 0;
	/** @brief Shows the plugin's main window.
	 *
	 * This function is called by LeechCraft when the user has done an
	 * action which means that the plugin should show or hide it's
	 * window (depending of the current state).
	 */
    virtual void ShowWindow () = 0;

	/** @brief Virtual destructor.
	 */
    virtual ~IWindow () {}
};

/** @brief Common interface for all the downloaders.
 *
 * Plugins which provide downloading capabilities and want to be visible
 * by LeechCraft and other plugins as download providers should
 * implement this interface.
 *
 * @sa IJobHolder
 */
class IDownload
{
public:
	enum Error
	{
		EUnknown
		, ENoError
		, ENotFound
		, EAccessDenied
		, ELocalError
	};
    typedef unsigned long int JobID_t;

	/** @brief Returns download speed.
	 *
	 * Returns summed up download speed of the plugin. The value is
	 * primarily used in the interface as there are no ways of
	 * controlling of bandwidth's usage of a particular plugin.
	 *
	 * @return Download speed in bytes.
	 *
	 * @sa GetUploadSpeed
	 */
    virtual qint64 GetDownloadSpeed () const = 0;
	/** @brief Returns upload speed.
	 *
	 * Returns summed up upload speed of the plugin. The value is
	 * primarily used in the interface as there are no ways of
	 * controlling of bandwidth's usage of a particular plugin.
	 *
	 * @return Upload speed in bytes.
	 *
	 * @sa GetDownloadSpeed
	 */
    virtual qint64 GetUploadSpeed () const = 0;

	/** @brief Starts all tasks.
	 *
	 * This is called by LeechCraft when it wants all plugins to start
	 * all of its tasks.
	 */
    virtual void StartAll () = 0;
	/** @brief Stops all tasks.
	 *
	 * This is called by LeechCraft when it wants all plugins to stop
	 * all of its tasks.
	 */
    virtual void StopAll () = 0;

	/** @brief Returns whether plugin can handle given string.
	 *
	 * This function is used to query every loaded plugin providing the
	 * IDownload interface whether it could handle the entity entered by
	 * user or generated automatically with given task parameters.
	 * Entity could be anything from file name to URL to all kinds of
	 * hashes like Magnet links.
	 *
	 * @param[in] entity Entity.
	 * @param[in] parameters Task parameters.
	 *
	 * @sa AddJob
	 * @sa LeechCraft::TaskParameters
	 */
    virtual bool CouldDownload (const QByteArray& entity,
			LeechCraft::TaskParameters parameters) const = 0;

	/** @brief Adds the job with given parameters.
	 *
	 * Adds the job to the downloader and returns the ID of the newly
	 * added job back to identify it.
	 *
	 * @param[in] params Download parameters.
	 * @param[in] taskParams Task parameters.
	 * @return ID of the job for the other plugins to use.
	 *
	 * @sa DownloadParams
	 * @sa LeechCraft::TaskParameters
	 */
    virtual int AddJob (const LeechCraft::DownloadParams& params,
			LeechCraft::TaskParameters taskParams) = 0;

	/** @brief Virtual destructor.
	 */
    virtual ~IDownload () {}
};

/** @brief Interface for plugins providing data for the Downloaders tab.
 *
 * If a plugin wants to show any data in the Downloaders tab, it should
 * implement this interface.
 *
 * Item model is returned by GetRepresentation(), GetControls() and
 * GetAdditionalInfo() are used to retrieve controls and information pane
 * of the plugin, and ItemSelected() is called to notify the plugin about
 * selection of an item. Returned model should have four columns, each
 * for name, state, progress and speed. Controls and additional
 * information pane are only visible when a job handled by the plugin is
 * selected.
 *
 * @sa IDownloader
 */
class IJobHolder
{
public:
	/** @brief Returns the item representation model.
	 *
	 * The returned model should have four columns, each for name,
	 * state, progress and speed. Inside of LeechCraft it would be
	 * merged with other models from other plugins.
	 *
	 * @return Representation model.
	 *
	 * @sa GetHistory
	 * @sa GetControls
	 * @sa GetAdditionalInfo
	 */
    virtual QAbstractItemModel* GetRepresentation () const = 0;
	/** @brief Returns the history model.
	 *
	 * If the returned value is 0, it is ignored. Otherwise it would be
	 * merged with other history models in LeechCraft's History tab.
	 *
	 * @return History model.
	 *
	 * @sa GetRepresentation
	 * @sa GetControls
	 * @sa GetAdditionalInfo
	 */
	virtual LeechCraft::Util::HistoryModel* GetHistory () const = 0;
	/** @brief Returns the widget with controls.
	 *
	 * Returned widget would be placed above the view with the jobs, so
	 * usually it has controls of the job, but in fact it can have
	 * anything you want. It is only visible when a job from your plugin
	 * is selected. If a job from other plugin is selected, then other
	 * plugin's controls would be placed, and if no jobs are selected at
	 * all, then no controls would be placed.
	 *
	 * @return Widget with controls.
	 *
	 * @sa GetRepresentation
	 * @sa GetHistory
	 * @sa GetAdditionalInfo
	 */
	virtual QWidget* GetControls () const = 0;
	/** @brief Returns the widget with information.
	 *
	 * Returned widget would be placed below the view with the jobs, so
	 * usually it has additional information about the job, but in fact
	 * it can have anything you want. It is only visible when a job from
	 * your plugin is selected. If a job from other plugin is selected,
	 * then other plugin's controls would be placed, and if no jobs are
	 * selected at all, then no controls would be placed.
	 *
	 * @return Widget with additional info.
	 *
	 * @sa GetRepresentation
	 * @sa GetHistory
	 * @sa GetControls
	 */
	virtual QWidget* GetAdditionalInfo () const = 0;
	/** @brief Notifies plugin about item selection.
	 * 
	 * @param item Selected item.
	 */
	virtual void ItemSelected (const QModelIndex& item) = 0;

	/** @brief Virtual destructor.
	 */
    virtual ~IJobHolder () {}
};

/** @brief Interface for plugins which can import/export data to
 * persistent storage.
 *
 * If a plugin can save/load its data and settings to the disk or some
 * other persistent storage and wants LeechCraft to know about it, it
 * should implement this interface.
 */
class IImportExport
{
public:
	/** @brief Loads settings.
	 *
	 * Loads the settings from byte array previously retrieved from this
	 * plugin with ExportSettings.
	 *
	 * @param[in] data Byte array with settings.
	 *
	 * @sa ImportData
	 * @sa ExportSettings
	 * @sa ExportData
	 */
	virtual void ImportSettings (const QByteArray& data) = 0;
	/** @brief Loads data.
	 *
	 * Loads the data from byte array previously retrieved from this
	 * plugin with ExportData.
	 *
	 * @param[in] data Byte array with data.
	 *
	 * @sa ImportSettings
	 * @sa ExportSettings
	 * @sa ExportData
	 */
	virtual void ImportData (const QByteArray& data) = 0;
	/** @brief Saves settings.
	 *
	 * Saves the settings into byte array which should be loadable with
	 * ImportSettings().
	 *
	 * @return Byte array with settings.
	 *
	 * @sa ImportSettings
	 * @sa ImportData
	 * @sa ExportData
	 */
	virtual QByteArray ExportSettings () const = 0;
	/** @brief Loads settings.
	 *
	 * Saves the data into byte array which should be loadable with
	 * ImportData().
	 *
	 * @return Byte array with data.
	 *
	 * @sa ImportSettings
	 * @sa ImportData
	 * @sa ExportSettings
	 */
	virtual QByteArray ExportData () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IImportExport () {}
};

/** @brief Interface for plugins having taggable download jobs.
 *
 * If a plugin has jobs which could be tagged and wants LeechCraft to
 * know about it then it should implement this interface, but it would
 * not be used if the plugin doesn't implement IDownload and IJobHolder.
 * Tags could be used later by LeechCraft to do some searches, grouping
 * and filtering, for example.
 *
 * @sa IDownload
 * @sa IJobHolder
 */
class ITaggableJobs
{
public:
	/** @brief Sets the list with tags for a job.
	 *
	 * This function should replace the list with tags for a job which
	 * is in jobRow row in the model returned by
	 * IJobHolder::GetRepresentation().
	 *
	 * @param[in] jobRow row with the job.
	 * @param[in] tagsList List with tags.
	 */
	virtual void SetTags (int jobRow, const QStringList& tagsList) = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~ITaggableJobs () {}
};


/** @brief Interface for plugins embedding a tab into main LeechCraft's
 * window.
 *
 * Implementing this interface means that plugin wants to embed a tab
 * into LeechCraft's main window. IInfo::GetName() would be used as a
 * name for the tab. If your plugin could open/close multiple tabs, have
 * a look at IMultiTabs.
 *
 * Plugin is expected to implement following signals:
 * - addNewTab(const QString&,QWidget*) which adds a new tab with the
 *   given name and widget contents.
 * - removeTab(QWidget*) which removes tab with given contents.
 * - changeTabName(QWidget*,const QString&) which changes tab name of
 *   the tab with the given widget.
 * - changeTabIcon(QWidget*,const QIcon&) which changes the icon of the
 *   tab with the given widget.
 * - statusBarChanged(QWidget*,const QString&) notifies that the status
 *   bar message of the given widget is changed. Note that the message
 *   would be updated only if the given widget is visible.
 *
 * @sa IMultiTabs
 * @sa IWindow
 */
class IEmbedTab
{
public:
	/** @brief Returns the widget with tab contents.
	 *
	 * @return Widget with tab contents.
	 */
	virtual QWidget* GetTabContents () = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IEmbedTab () {}
};

/** @brief Interface for plugins having (and opening/closing) multiple
 * tabs.
 *
 * When a plugin wants to add a new tab into LeechCraft, it emits the
 * addNewTab(const QString&, QWidget*) signal, where the first parameter
 * is the name of the new tab, and the second one is the pointer to the
 * widget with tab contents. Newly added widget would be reparented by
 * LeechCraft.
 * To remove a tab, it emits removeTab(QWidget*), where the parameter is
 * the pointer to a previously added tab's widget.
 * To change tab's name, plugin emits changeTabName(QWidget*, const
 * QString&), where the first parameter is the pointer to previously
 * inserted tab and the second one is the new name.
 * To change tab's icon, plugin emits changeTabIcon(QWidget*, const
 * QIcon&), where the first parameter is the pointer to previously
 * inserted tab and the seocnd one is the new icon.
 * To bring the tab to front, plugin emits raiseTab(QWidget*) signal,
 * where the first parameter is the pointer to previously inserted tab.
 *
 * @sa IEmbedTab
 * @sa IWindow
 */
class IMultiTabs
{
public:
	/** @brief Virtual destructor.
	 */
	virtual ~IMultiTabs () {}
};

/** @brief Interface for plugins providing custom facilities.
 *
 * This interface should be used by plugins which provide custom
 * abilities not related to LeechCraft and not accounted by other
 * interfaces. All communication goes via signal/slot connections.
 */
class ICustomProvider
{
public:
	/** @brief Queries the plugin whether it implements a given feature.
	 *
	 * @param[in] feature Queried feature.
	 * @return Query result.
	 */
	virtual bool ImplementsFeature (const QString& feature) const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~ICustomProvider () {}
};

/** @brief Interface for plugins that have user-configurable settings.
 *
 * Plugins that have user-configurable settings should implement this
 * interface if they want to appear in a common settings configuration
 * dialog.
 */
class IHaveSettings
{
public:
	/** @brief Gets the settings dialog manager object from the plugin.
	 *
	 * The returned XmlSettingsDialog would be integrated into common
	 * settings dialog where user can configure all the plugins that
	 * provide this interface.
	 *
	 * @return The XmlSettingsDialog object that manages the settings
	 * dialog of the plugin.
	 */
	virtual LeechCraft::Util::XmlSettingsDialog* GetSettingsDialog () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IHaveSettings () {}
};

/** @brief Interface for plugins which want to share network access
 * manager.
 *
 * The network access manager shares cache, cookies and other stuff
 * between plugins, so it's often handy to have one network access
 * manager for all plugins.
 */
class IWantNetworkAccessManager
{
public:
	/** @brief Sets the network access manager.
	 *
	 * The passed network manager object should be still owned by the
	 * LeechCraft, plugin shouldn't take the ownership.
	 *
	 * @param nam[in] The QNetworkAccessManager object that's shared
	 * among the plugins.
	 */
	virtual void SetNetworkAccessManager (QNetworkAccessManager *nam) = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IWantNetworkAccessManager () {}
};

Q_DECLARE_INTERFACE (IInfo, "org.Deviant.LeechCraft.IInfo/1.0");
Q_DECLARE_INTERFACE (IWindow, "org.Deviant.LeechCraft.IWindow/1.0");
Q_DECLARE_INTERFACE (IDownload, "org.Deviant.LeechCraft.IDownload/1.0");
Q_DECLARE_INTERFACE (IJobHolder, "org.Deviant.LeechCraft.IJobHolder/1.0");
Q_DECLARE_INTERFACE (IImportExport, "org.Deviant.LeechCraft.IImportExport/1.0");
Q_DECLARE_INTERFACE (ITaggableJobs, "org.Deviant.LeechCraft.ITaggableJobs/1.0");
Q_DECLARE_INTERFACE (IEmbedTab, "org.Deviant.LeechCraft.IEmbedTab/1.0");
Q_DECLARE_INTERFACE (IMultiTabs, "org.Deviant.LeechCraft.IMultiTabs/1.0");
Q_DECLARE_INTERFACE (ICustomProvider, "org.Deviant.LeechCraft.ICustomProvider/1.0");
Q_DECLARE_INTERFACE (IHaveSettings, "org.Deviant.LeechCraft.IHaveSettings/1.0");
Q_DECLARE_INTERFACE (IWantNetworkAccessManager, "org.Deviant.LeechCraft.IWantNetworkAccessManager/1.0");

#endif

