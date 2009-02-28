#ifndef IJOBHOLDER_H
#define IJOBHOLDER_H
#include <QtPlugin>

class QModelIndex;
class QAbstractItemModel;
class QWidget;

namespace LeechCraft
{
	namespace Util
	{
		class HistoryModel;
	};
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
	 * This model is also used to retrieve controls and additional info
	 * for a given index.
	 *
	 * Returned widget would be placed above the view with the jobs, so
	 * usually it has controls of the job, but in fact it can have
	 * anything you want. It is only visible when a job from your plugin
	 * is selected. If a job from other plugin is selected, then other
	 * plugin's controls would be placed, and if no jobs are selected at
	 * all, then no controls would be placed.
	 *
	 * Additional information a retrieved using the RoleAdditionalInfo.
	 * Returned widget would be placed below the view with the jobs, so
	 * usually it has additional information about the job, but in fact
	 * it can have anything you want. It is only visible when a job from
	 * your plugin is selected. If a job from other plugin is selected,
	 * then other plugin's controls would be placed, and if no jobs are
	 * selected at all, then no controls would be placed.
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
	/** @brief Notifies plugin about item selection.
	 * 
	 * @param item Selected item.
	 */
	virtual void ItemSelected (const QModelIndex& item) = 0;

	/** @brief Virtual destructor.
	 */
    virtual ~IJobHolder () {}
};

Q_DECLARE_INTERFACE (IJobHolder, "org.Deviant.LeechCraft.IJobHolder/1.0");

#endif

