#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <QMetaType>

class QMenu;

namespace LeechCraft
{
	/** @brief Describes single task parameter.
	 */
	enum TaskParameter
	{
		/** Use default parameters.
		 */
		NoParameters = 0,
		/** Task should not be started automatically after addition.
		 */
		NoAutostart = 1,
		/** Task should not be saved in history.
		 */
		DoNotSaveInHistory = 2,
		/** Task is created as a result of user's actions.
		 */
		FromUserInitiated = 8,
		/** User should not be notified about task finish.
		 */
		DoNotNotifyUser = 32,
		/** Task is used internally and would not be visible to the user
		 * at all.
		 */
		Internal = 64,
		/** Task should not be saved as it would have no meaning after
		 * next start.
		 */
		NotPersistent = 128,
		/** When the task is finished, it should not be announced via
		 * gotEntity() signal.
		 */
		DoNotAnnounceEntity = 256
	};

	Q_DECLARE_FLAGS (TaskParameters, TaskParameter);

	/** @brief Describes parameters of an entity.
	 *
	 * This struct is used both for addition of new download jobs and
	 * for announcing about finished/available entities, so its members
	 * are context-dependent.
	 *
	 * @sa LeechCraft::TaskParameter
	 */
	struct DownloadEntity
	{
		/** @brief The entity that this object represents.
		 *
		 * In the context of entity delegation it represents the entity
		 * that should be downloaded or handled. For example, contents
		 * of a torrent file, a magnet link, an RSS document.
		 *
		 * Local files should start with file://
		 *
		 * In the context of announcing about a finished entity, it
		 * could contain previously mentioned entities as well.
		 */
		QByteArray Entity_;
		/** @brief Source or destination.
		 *
		 * In the context of entity delegation this parameter represents
		 * where the other plugin wants this job to be downloaded.
		 *
		 * In the context of entity announce this represents from where
		 * this entity came - original URL, for example.
		 */
		QString Location_;
		/** @brief MIME type of the entity.
		 *
		 * An empty mime is considered to be no mime.
		 */
		QString Mime_;
		TaskParameters Parameters_;
	};

	enum CustomDataRoles
	{
		/** The role for the string list with tags. So, QStringList is
		 * expected to be returned.
		 */
		RoleTags = 100,
		/* The role for the additional controls for a given item.
		 * QWidget* is expected to be returned.
		 */
		RoleControls,
		/** The role for the widget appearing on the right part of the
		 * screen when the user selects an item. QWidget* is expected to
		 * be returned.
		 */
		RoleAdditionalInfo,
		/** The role for the hash of the item, used to compare two
		 * different results, possibly from two different models.
		 * QByteArray is expected to be returned.
		 */
		RoleHash,
		/** This should return MIME of an item if it's available,
		 * otherwise an empty string should be returned.
		 */
		RoleMime
	};
};

Q_DECLARE_METATYPE (LeechCraft::DownloadEntity);
Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::TaskParameters);

#endif

