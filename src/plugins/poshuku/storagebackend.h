#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include <vector>
#include <QObject>
#include "historymodel.h"

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

	virtual void Prepare () = 0;
	virtual void LoadHistory (std::vector<HistoryModel::HistoryItem>& items) const = 0;
	virtual void AddToHistory (const HistoryModel::HistoryItem& item) = 0;
signals:
	void added (const HistoryModel::HistoryItem&);
};

#endif

