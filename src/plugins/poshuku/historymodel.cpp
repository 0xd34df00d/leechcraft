#include "historymodel.h"
#include <algorithm>
#include <QTimer>
#include <QVariant>
#include <QAction>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include <plugininterface/treeitem.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "poshuku.h"

using LeechCraft::Util::TreeItem;

namespace
{
	/** Returns the number of the section for the given date.
	 *
	 * - Today
	 * - Yesterday
	 * - Two days ago
	 * - Last week
	 * - Last month
	 * - Last 2 months
	 * - Last 3 months
	 * - Last 4 months
	 * - ...
	 * - Last N months
	 */
	int SectionNumber (const QDateTime& date)
	{
		QDateTime current = QDateTime::currentDateTime ();
		QDate orig = current.date ();
		if (date.daysTo (current) == 0)
			return 0;
		else if (date.daysTo (current) == 1)
			return 1;
		else if (date.daysTo (current) == 2)
			return 2;
		else if (date.daysTo (current) <= 7)
			return 3;

		int i = 0;
		while (true)
		{
			current.setDate (orig.addMonths (--i));

			if (date.daysTo (current) <= 0)
				return -i + 3;
		}
	}

	QString SectionName (int number)
	{
		switch (number)
		{
			case 0:
				return QObject::tr ("Today");
			case 1:
				return QObject::tr ("Yesterday");
			case 2:
				return QObject::tr ("Two days ago");
			case 3:
				return QObject::tr ("Last week");
			case 4:
				return QObject::tr ("Last month");
			default:
				return QObject::tr ("Last %1 months")
					.arg (number - 3);
		}
	}
};

HistoryModel::HistoryModel (QObject *parent)
: QAbstractItemModel (parent)
{
	QList<QVariant> headers;
	headers << tr ("Title")
		<< tr ("Date")
		<< tr ("URL");
	QTimer::singleShot (0,
			this,
			SLOT (loadData ()));
	RootItem_ = new TreeItem (headers);

	GarbageTimer_ = new QTimer (this);
	GarbageTimer_->start (60 * 60 * 1000);
	connect (GarbageTimer_,
			SIGNAL (timeout ()),
			this,
			SLOT (loadData ()));

	FolderIconProxy_ = new QAction (this);
	FolderIconProxy_->setProperty ("ActionIcon", "poshuku_foldericon");
	UnknownURLProxy_ = new QAction (this);
	UnknownURLProxy_->setProperty ("ActionIcon", "poshuku_unknownurlicon");
}

HistoryModel::~HistoryModel ()
{
	delete RootItem_;
}

int HistoryModel::columnCount (const QModelIndex& index) const
{
	if (index.isValid ())
		return static_cast<TreeItem*> (index.internalPointer ())->ColumnCount ();
	else
		return RootItem_->ColumnCount ();
}

QVariant HistoryModel::data (const QModelIndex& index, int role) const
{
	if (!index.isValid ())
		return QVariant ();

	return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column (), role);
}

Qt::ItemFlags HistoryModel::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant HistoryModel::headerData (int h, Qt::Orientation orient,
		int role) const
{
	if (orient == Qt::Horizontal && role == Qt::DisplayRole)
		return RootItem_->Data (h);

	return QVariant ();
}

QModelIndex HistoryModel::index (int row, int col,
		const QModelIndex& parent) const
{
	if (!hasIndex (row, col, parent))
		return QModelIndex ();

	TreeItem *parentItem;

	if (!parent.isValid ())
		parentItem = RootItem_;
	else
		parentItem = static_cast<TreeItem*> (parent.internalPointer ());

	TreeItem *childItem = parentItem->Child (row);
	if (childItem)
		return createIndex (row, col, childItem);
	else
		return QModelIndex ();
}

QModelIndex HistoryModel::parent (const QModelIndex& index) const
{
	if (!index.isValid ())
		return QModelIndex ();

	TreeItem *childItem = static_cast<TreeItem*> (index.internalPointer ()),
			 *parentItem = childItem->Parent ();

	if (parentItem == RootItem_)
		return QModelIndex ();

	return createIndex (parentItem->Row (), 0, parentItem);
}

int HistoryModel::rowCount (const QModelIndex& parent) const
{
	TreeItem *parentItem;
	if (parent.column () > 0)
		return 0;

	if (!parent.isValid ())
		parentItem = RootItem_;
	else
		parentItem = static_cast<TreeItem*> (parent.internalPointer ());

	return parentItem->ChildCount ();
}

void HistoryModel::AddItem (const QString& title, const QString& url,
		const QDateTime& date)
{
	HistoryItem item =
	{
		title,
		date,
		url
	};
	Core::Instance ().GetStorageBackend ()->AddToHistory (item);
}

void HistoryModel::Add (const HistoryItem& item)
{
	int section = SectionNumber (item.DateTime_);

	while (section >= RootItem_->ChildCount ())
	{
		QList<QVariant> data;
		data << SectionName (RootItem_->ChildCount ())
			<< QString ("")
			<< QString ("");
		TreeItem *folder = new TreeItem (data, RootItem_);
		folder->ModifyData (0,
				FolderIconProxy_->icon (),
				Qt::DecorationRole);
		RootItem_->AppendChild (folder);
	}

	QList<QVariant> data;
	data << item.Title_
		<< item.URL_
		<< item.DateTime_;

	TreeItem *folder = RootItem_->Child (section);

	TreeItem *thisItem = new TreeItem (data, RootItem_->Child (section));
	folder->PrependChild (thisItem);

	QUrl url (item.URL_);
	url.setFragment (QString ());
	url.setPath (QString ());
	url.setQueryItems (QList<QPair<QString, QString> > ());
	QIcon icon = QWebSettings::iconForUrl (url);
	if (icon.isNull ())
		icon = UnknownURLProxy_->icon ();
	thisItem->ModifyData (0,
			icon, Qt::DecorationRole);
}

void HistoryModel::loadData ()
{ 
	int age = XmlSettingsManager::Instance ()->
		property ("HistoryClearOlderThan").toInt ();
	int maxItems = XmlSettingsManager::Instance ()->
		property ("HistoryKeepLessThan").toInt ();
	Core::Instance ().GetStorageBackend ()->ClearOldHistory (age, maxItems);

	std::vector<HistoryItem> items;
	Core::Instance ().GetStorageBackend ()->LoadHistory (items);

	if (!items.size ())
		return;

	for (std::vector<HistoryItem>::const_reverse_iterator i = items.rbegin (),
			end = items.rend (); i != end; ++i)
		Add (*i);

	reset ();
}

void HistoryModel::handleItemAdded (const HistoryItem& item)
{
	beginInsertRows (index (SectionNumber (item.DateTime_), 0),
			0, 0);
	Add (item);
	endInsertRows ();
}

