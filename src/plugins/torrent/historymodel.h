#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H
#include <vector>
#include <QModelIndex>
#include <QDateTime>
#include <plugininterface/historymodel.h>

class HistoryModel : public LeechCraft::HistoryModel
{
	Q_OBJECT

public:
	struct HistoryItem
	{
		QString Name_;
		QString Where_;
		quint64 TorrentSize_;
		QDateTime DateTime_;
		QStringList Tags_;
	};
private:
	typedef std::vector<HistoryItem> items_t;
	std::vector<HistoryItem> Items_;
public:
	enum { TagsRole = 46 };
	HistoryModel (QObject* = 0);
	virtual ~HistoryModel ();

    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	void RemoveItem (const QModelIndex&);

	void AddItem (const HistoryItem&);
};

#endif

