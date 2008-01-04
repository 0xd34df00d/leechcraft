#include <QMainWindow>
#include <limits>
#include <iostream>
#include "pluginmanager.h"
#include "core.h"
#include "exceptions/logic.h"
#include "exceptions/outofbounds.h"

Core::Core (QObject *parent)
: QAbstractTableModel (parent)
{
	PreparePools ();
	PluginManager_ = new PluginManager (this);
	connect (this, SIGNAL (throwError (QString, Errors::Severity)), parent, SLOT (catchError (QString, Errors::Severity)));
	connect (PluginManager_, SIGNAL (gotPlugin (const PluginInfo*)), this, SIGNAL (gotPlugin (const PluginInfo*)));
}

Core::~Core ()
{
}

void Core::SetReallyMainWindow (QMainWindow *win)
{
	ReallyMainWindow_ = win;
}

QMainWindow* Core::GetReallyMainWindow ()
{
	return ReallyMainWindow_;
}

void Core::DoDelayedInit ()
{
	PluginManager_->InitializePlugins ();
	PluginManager_->CalculateDependencies ();
	PluginManager_->ThrowPlugins ();
	QObjectList plugins = PluginManager_->GetAllPlugins ();
	foreach (QObject *plugin, plugins)
	{
		connect (this, SIGNAL (hidePlugins ()), plugin, SLOT (handleHidePlugins ()));
	}
}

bool Core::ShowPlugin (IInfo::ID_t id)
{
	PluginManager::Iterator i = PluginManager_->FindByID (id);
	QObject *plugin = *i;
	IWindow *w = qobject_cast<IWindow*> (plugin);
	if (w)
	{
		w->ShowWindow ();
		return true;
	}
	else
		return false;
}

void Core::HideAll ()
{
	emit hidePlugins ();
}

QPair<qint64, qint64> Core::GetSpeeds () const
{
	qint64 download = 0;
	qint64 upload = 0;
	for (PluginManager::Iterator i = PluginManager_->Begin (); i != PluginManager_->End (); ++i)
	{
		QObject *plugin = *i;
		IDownload *di = qobject_cast<IDownload*> (plugin);
		if (di)
		{
			download += di->GetDownloadSpeed ();
			upload += di->GetUploadSpeed ();
		}
	}

	return QPair<qint64, qint64> (download, upload);
}

int Core::rowCount (const QModelIndex&) const
{
    return PluginManager_->GetSize ();
}

int Core::columnCount (const QModelIndex&) const
{
    return 2;
}

QVariant Core::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ()) return QVariant ();

    int r = index.row (), c = index.column ();

    if (r >= rowCount ())
		return QVariant ();

    switch (role)
    {
		case (Qt::DisplayRole):
			return GetTaskData (r, c);
		default:
			return QVariant ();
    }
}

QVariant Core::headerData (int section, Qt::Orientation orient, int role) const
{
    if (role == Qt::DisplayRole)
		if (orient == Qt::Horizontal)
			return GetTaskData (-1, section);
		else
			return QAbstractTableModel::headerData (section, orient, role);
    else
		return QAbstractTableModel::headerData (section, orient, role);
}

Qt::ItemFlags Core::flags (const QModelIndex& index) const
{
    if (!index.isValid ())
		return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags (index);
}

bool Core::setData (const QModelIndex& index, const QVariant& value, int role)
{
	Q_UNUSED (index);
	Q_UNUSED (value);
	Q_UNUSED (role);
}

bool Core::removeRows (int pos, int rows, const QModelIndex& parent)
{
    Q_UNUSED (parent)
    beginRemoveRows (QModelIndex (), pos, pos + rows - 1);

    for (int row = 0; row < rows; ++row)
	{
		try
		{
			PluginManager_->Release (pos + row);
		}
		catch (const Exceptions::OutOfBounds& e)
		{
			throwError (QString::fromStdString ("Caught an error " + e.GetName () + "; more info: " + e.GetReason ()), Errors::Error);
		}
		catch (const Exceptions::Logic& e)
		{
			throwError (QString::fromStdString ("Caught an error " + e.GetName () + "; more info: " + e.GetReason ()), Errors::Warning);
		}
		catch (const Exceptions::Generic& e)
		{
			throwError (QString::fromStdString ("Unknown error " + e.GetName () + "; more info: " + e.GetReason ()), Errors::Notice);
		}
	}

    endRemoveRows ();
    return true;
}

QModelIndex Core::parent (const QModelIndex& index)
{
    Q_UNUSED (index)
    return QModelIndex ();
}

void Core::invalidate (unsigned int id)
{
	QModelIndex first = index (id, 2);
	QModelIndex last = index (id, columnCount ());

	emit dataChanged (first, last);
}

void Core::PreparePools ()
{
	for (int i = 0; i < 255; ++i)
		TasksIDPool_.push_back (i);
}

QVariant Core::GetTaskData (int row, int column) const
{
	if (row == -1)
	{
		switch (column)
		{
			case 0:
				return tr ("Name");
			case 1:
				return tr ("Name");
		}
	}
	switch (column)
	{
		case 0:
			return PluginManager_->Name (row);
		case 1:
			return PluginManager_->Info (row);
		default:
			return QVariant ();
	}
}

