#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QDateTime>
#include <QStringList>
#include <plugininterface/historymodel.h>

class HistoryModel : public LeechCraft::Util::HistoryModel
{
	Q_OBJECT
public:
	struct Item
	{
		QString Filename_;
		QString URL_;
		qint64 Size_;
		QDateTime DateTime_;
	};
private:
	std::vector<Item> Items_;
	bool SaveScheduled_;
public:
	HistoryModel (QObject* = 0);
	virtual ~HistoryModel ();

	void Add (const Item&);
	void RemoveItem (const QModelIndex&);

	virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
private:
	void ReadSettings ();
	void ScheduleSave ();
private slots:
	void writeSettings ();
};

#endif

