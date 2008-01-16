#include <QtGui/QtGui>
#include "pluginlisttablewidgeticon.h"

PluginListTableWidgetIcon::PluginListTableWidgetIcon ()
: QTableWidgetItem (UserType)
, Text_ (0)
, Icon_ (0)
, WhatsThisMessage_ (0)
, TooltipMessage_ (0)
, StatusbarMessage_ (0)
{
}

PluginListTableWidgetIcon::~PluginListTableWidgetIcon ()
{
	delete Text_;
	delete Icon_;
	delete WhatsThisMessage_;
	delete TooltipMessage_;
	delete StatusbarMessage_;
}

QVariant PluginListTableWidgetIcon::data (int role) const
{
	switch (role)
	{
		case Qt::DisplayRole:
			return Text_ ? *Text_ : QVariant ();
		case Qt::ToolTipRole:
			return TooltipMessage_ ? *TooltipMessage_ : QVariant ();
		case Qt::DecorationRole:
			return Icon_ ? *Icon_ : QVariant ();
		case Qt::StatusTipRole:
			return StatusbarMessage_ ? *StatusbarMessage_ : QVariant ();
		case Qt::WhatsThisRole:
			return WhatsThisMessage_ ? *WhatsThisMessage_ : QVariant ();
		default:
			return QVariant ();
	}
}

void PluginListTableWidgetIcon::setData (int role, const QVariant& value)
{
	switch (role)
	{
		case Qt::DisplayRole:
			delete Text_;
			Text_ = new QString (value.toString ());
			break;
		case Qt::DecorationRole:
			delete Icon_;
			Icon_ = new QIcon (value.value<QIcon> ());
			break;
		case Qt::ToolTipRole:
			delete TooltipMessage_;
			TooltipMessage_ = new QString (value.toString ());
			break;
		case Qt::StatusTipRole:
			delete StatusbarMessage_;
			StatusbarMessage_ = new QString (value.toString ());
			break;
		case Qt::WhatsThisRole:
			delete WhatsThisMessage_;
			WhatsThisMessage_ = new QString (value.toString ());
			break;
		default:
			return;
	}
}

