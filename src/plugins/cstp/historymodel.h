#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H
#include <deque>
#include <QAbstractItemModel>
#include <QString>
#include <QDateTime>
#include <QStringList>

class HistoryModel : public QAbstractItemModel
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
	enum
	{
		HFilename
		, HURL
		, HSize
		, HDateTime
	};
	std::deque<Item> Items_;
	QStringList Headers_;
	bool SaveScheduled_;
public:
	HistoryModel (QObject* = 0);
	virtual ~HistoryModel ();

	void Add (const Item&);
	void Remove (const QModelIndex&);

	virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
	virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex&) const;
	virtual bool hasChildren (const QModelIndex&) const;
	virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
	virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
private:
	void ReadSettings ();
	void ScheduleSave ();
private slots:
	void writeSettings ();
};

#endif

