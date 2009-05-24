#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H
#include <deque>
#include <vector>
#include <QAbstractItemModel>
#include <QStringList>
#include <QDateTime>

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem;
	};
};

class QTimer;
class QAction;

struct HistoryItem
{
	QString Title_;
	QDateTime DateTime_;
	QString URL_;
};

Q_DECLARE_METATYPE (HistoryItem);

typedef std::vector<HistoryItem> history_items_t;

class HistoryModel : public QAbstractItemModel
{
	Q_OBJECT

	QTimer *GarbageTimer_;
	LeechCraft::Util::TreeItem *RootItem_;
	QAction *FolderIconProxy_;
	QAction *UnknownURLProxy_;
public:
	enum Columns
	{
		ColumnTitle
		, ColumnURL
		, ColumnDate
	};

	HistoryModel (QObject* = 0);
	virtual ~HistoryModel ();

	int columnCount (const QModelIndex& = QModelIndex ()) const;
	QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	Qt::ItemFlags flags (const QModelIndex&) const;
	QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
	QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
	QModelIndex parent (const QModelIndex&) const;
	int rowCount (const QModelIndex& = QModelIndex ()) const;

	void AddItem (const QString&, const QString&, const QDateTime&);
private:
	void Add (const HistoryItem&);
private slots:
	void loadData ();
	void handleItemAdded (const HistoryItem&);
};

#endif

