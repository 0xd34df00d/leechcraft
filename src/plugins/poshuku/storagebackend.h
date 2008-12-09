#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include <QObject>

/** @brief Abstract base class for storage backends.
 *
 * Specifies interface for all storage backends. Includes functions for
 * storing the history, favorites, saved passwords etc.
 */
class StorageBackend : public QObject
{
	Q_OBJECT
public:
	StorageBackend (QObject* = 0);
	virtual ~StorageBackend ();
};

#endif

