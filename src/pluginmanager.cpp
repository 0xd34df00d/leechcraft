#include <stdexcept>
#include <QApplication>
#include <QPluginLoader>
#include <QDir>
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QMainWindow>
#include <plugininterface/proxy.h>
#include "core.h"
#include "pluginmanager.h"
#include "mainwindow.h"

using namespace Main;

PluginManager::PluginManager (QObject *parent)
: QAbstractItemModel (parent)
{
    FindPlugins ();
}

PluginManager::~PluginManager ()
{
}

int PluginManager::columnCount (const QModelIndex& parent) const
{
	return 1;
}

QVariant PluginManager::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () || index.row () >= GetSize ())
		return QVariant ();

	switch (index.column ())
	{
		case 0:
			switch (role)
			{
				case Qt::DisplayRole:
					return qobject_cast<IInfo*> (Plugins_.at (index.row ())->
							instance ())->GetName ();
				case Qt::DecorationRole:
					return qobject_cast<IInfo*> (Plugins_.at (index.row ())->
							instance ())->GetIcon ();
				default:
					return QVariant ();
			}
		default:
			return QVariant ();
	}
}

Qt::ItemFlags PluginManager::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PluginManager::headerData (int, Qt::Orientation, int) const
{
	return QVariant ();
}

QModelIndex PluginManager::index (int row, int column, const QModelIndex&) const
{
	return createIndex (row, column);
}

QModelIndex PluginManager::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int PluginManager::rowCount (const QModelIndex& index) const
{
	if (!index.isValid ())
		return Plugins_.size ();
	else
		return 0;
}

PluginManager::Size_t PluginManager::GetSize () const
{
    return Plugins_.size ();
}

void PluginManager::Release ()
{
    while (Plugins_.size ())
    {
        try
        {
            qDebug () << Q_FUNC_INFO << Name (Plugins_.size () - 1);
            Release (Plugins_.size () - 1);
        }
        catch (const std::exception& e)
        {
            qWarning () << Q_FUNC_INFO << e.what ();
        }
        catch (...)
        {
            QMessageBox::warning (0, tr ("No exit here"), tr ("Release of one or more plugins failed. OS would cleanup after us, but it isn't good anyway, as this failed plugin could fail to save it's state."));
        }
        Plugins_.removeAt (Plugins_.size () - 1);
    }
}

void PluginManager::Release (PluginManager::Size_t position)
{
    if (position >= GetSize ())
        throw std::runtime_error ("PluginManager::Release(): position is out of bounds.");

    if (Plugins_ [position] && Plugins_ [position]->isLoaded ())
	{
        qobject_cast<IInfo*> (Plugins_ [position]->instance ())->Release ();
		delete Plugins_ [position]->instance ();
		delete Plugins_ [position];
	}
}

QString PluginManager::Name (const PluginManager::Size_t& pos) const
{
    return (qobject_cast<IInfo*> (Plugins_ [pos]->instance ()))->GetName ();
}

QString PluginManager::Info (const PluginManager::Size_t& pos) const
{
    return qobject_cast<IInfo*> (Plugins_ [pos]->instance ())->GetInfo ();
}

QObject* PluginManager::FindByID (IInfo::ID_t id) const
{
    for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
        if (qobject_cast<IInfo*> ((*i)->instance ())->GetID () == id)
            return (*i)->instance ();
    return 0;
}

QObjectList PluginManager::GetAllPlugins () const
{
    QObjectList result;
    for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
        result << (*i)->instance ();
    return result;
}

void PluginManager::InitializePlugins (const MainWindow* win)
{
    for (int i = 0; i < Plugins_.size (); ++i)
    {
        QPluginLoader *loader = Plugins_.at (i);

        loader->load ();
        if (!loader->isLoaded ())
        {
            qWarning () << "Could not load library: " << loader->fileName () << "; " << loader->errorString ();
            Plugins_.removeAt (i--);
            continue;
        }

        QObject *pluginEntity = loader->instance ();
        IInfo *info = qobject_cast<IInfo*> (pluginEntity);
        if (!info)
        {
            qWarning () << "Library successfully loaded, but it could not be initialized (casting to IInfo failed): " << loader->fileName ();
            Plugins_.removeAt (i--);
            continue;
        }
        MainWindowExternals ex = { win->GetRootPluginsMenu () };
        info->SetID (i);
        info->PushMainWindowExternals (ex);
        info->Init ();
    }
}

void PluginManager::CalculateDependencies ()
{
    for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
    {
        QObject *pEntity = (*i)->instance ();
        IInfo *info = qobject_cast<IInfo*> (pEntity);
        QStringList needs = info->Needs (),
                    uses = info->Uses ();
        DependenciesMet_ [i] = true;

        if (!needs.isEmpty ())
        {
            for (PluginsContainer_t::const_iterator j = Plugins_.begin (); j != Plugins_.end (); ++j)
            {
                if (j == i)
                    continue;

                QObject *qpEntity = (*j)->instance ();
                IInfo *qinfo = qobject_cast<IInfo*> (qpEntity);
                QStringList qprovides = qinfo->Provides ();
                for (int k = 0; k < needs.size (); ++k)
                    if (qprovides.contains (needs [k]))
                    {
                        info->SetProvider (qpEntity, needs [k]);
                        needs.removeAt (k--);
                    }
            }
            if (!needs.isEmpty ())
            {
                DependenciesMet_ [i] = false;
                FailedDependencies_ [i] = needs;
                qWarning () << Q_FUNC_INFO << "not all plugins providing needs of" << info->GetName () << "are found. The remaining ones are:" << needs;
            }
        }
        if (!uses.isEmpty ())
        {
            QMap<QString, bool> usesMet;
            for (int j = 0; j < uses.size (); ++j)
                usesMet [uses.at (j)] = false;

            for (PluginsContainer_t::const_iterator j = Plugins_.begin (); j != Plugins_.end (); ++j)
            {
                if (j == i)
                    continue;

                QObject *qpEntity = (*j)->instance ();
                IInfo *qinfo = qobject_cast<IInfo*> (qpEntity);
                QStringList qprovides = qinfo->Provides ();
                for (int k = 0; k < uses.size (); ++k)
                    if (qprovides.contains (uses [k]))
                    {
                        info->SetProvider (qpEntity, uses [k]);
                        usesMet [uses [k]] = true;
                    }
            }
        }
    }
}

void PluginManager::FindPlugins ()
{
    QDir pluginsDir = QDir ("/usr/local/lib/leechcraft/plugins");
    foreach (QString filename, pluginsDir.entryList (QStringList ("*leechcraft_*"), QDir::Files))
        Plugins_.push_back (new QPluginLoader (pluginsDir.absoluteFilePath (filename), this));

    pluginsDir = QDir ("/usr/lib/leechcraft/plugins");
    foreach (QString filename, pluginsDir.entryList (QStringList ("*leechcraft_*"), QDir::Files))
        Plugins_.push_back (new QPluginLoader (pluginsDir.absoluteFilePath (filename), this));

    pluginsDir = QDir (QApplication::applicationDirPath ());
    if (pluginsDir.cd ("plugins/bin"))
        foreach (QString filename, pluginsDir.entryList (QStringList ("*leechcraft_*"), QDir::Files))
            Plugins_.push_back (new QPluginLoader (pluginsDir.absoluteFilePath (filename), this));
}

