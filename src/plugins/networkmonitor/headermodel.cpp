#include "headermodel.h"

using namespace LeechCraft::Plugins::NetworkMonitor;

HeaderModel::HeaderModel (QObject *parent)
: QStandardItemModel (parent)
{
	setHorizontalHeaderLabels (QStringList (tr ("Name"))
			<< tr ("Value"));
}

void HeaderModel::AddHeader (const QString& name, const QString& value)
{
	QList<QStandardItem*> items;
	items.push_back (new QStandardItem (name));
	items.push_back (new QStandardItem (value));
	appendRow (items);
}

