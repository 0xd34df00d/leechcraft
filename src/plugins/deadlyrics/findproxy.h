#ifndef FINDPROXY_H
#define FINDPROXY_H
#include <vector>
#include <QAbstractItemModel>
#include <interfaces/ifinder.h>
#include "searcher.h"

class FindProxy : public QAbstractItemModel
				, public IFindProxy
{
	Q_OBJECT
	Q_INTERFACES (IFindProxy);

	LeechCraft::Request Request_;
	std::vector<QByteArray> Hashes_;
	lyrics_t Lyrics_;

	FindProxy (const FindProxy&);
	FindProxy& operator= (const FindProxy&);
public:
	FindProxy (const LeechCraft::Request&, QObject* = 0);
	virtual ~FindProxy ();

	QAbstractItemModel* GetModel ();

	int columnCount (const QModelIndex&) const;
	QVariant data (const QModelIndex&, int) const;
	QModelIndex index (int, int, const QModelIndex&) const;
	QModelIndex parent (const QModelIndex&) const;
	int rowCount (const QModelIndex&) const;
private slots:
	void handleTextFetched (const Lyrics&, const QByteArray&);
};

#endif

