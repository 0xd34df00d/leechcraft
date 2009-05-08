#ifndef INTERFACES_IENTITYHANDLER_H
#define INTERFACES_IENTITYHANDLER_H
#include <QByteArray>
#include <QtPlugin>
#include "structures.h"

/** @brief Interface for plugins able to handle entities.
 *
 * This is similar to IDownload, but it doesn't require to implement all
 * functions of IDownload, and IDownloaders and IEntityHandlers are
 * shown in different groups when presented to user, for example, in
 * entity handle dialog.
 *
 * @sa IDownload
 */
class IEntityHandler
{
public:
	/** @brief Returns whether plugin can handle given entity.
	 *
	 * This function is used to query every loaded plugin providing the
	 * IDownload interface whether it could handle the entity entered by
	 * user or generated automatically with given task parameters.
	 * Entity could be anything from file name to URL to all kinds of
	 * hashes like Magnet links.
	 *
	 * @param[in] entity A DownloadEntity structure.
	 *
	 * @sa Handle
	 * @sa LeechCraft::DownloadEntity
	 */
	virtual bool CouldHandle (const LeechCraft::DownloadEntity& entity) const = 0;

	/** @brief Gives plugin the ability to handle the entity.
	 *
	 * Notifies IEntityHandler that this entity should be handled.
	 *
	 * @param[in] entity A DownloadEntity structure.
	 *
	 * @sa LeechCraft::DownloadEntity
	 */
	virtual void Handle (LeechCraft::DownloadEntity entity) = 0;

	virtual ~IEntityHandler () {}
};

Q_DECLARE_INTERFACE (IEntityHandler, "org.Deviant.LeechCraft.IEntityHandler/1.0");

#endif

