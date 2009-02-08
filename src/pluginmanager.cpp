#include <stdexcept>
#include <QApplication>
#include <QPluginLoader>
#include <QDir>
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QMainWindow>
#include <QCryptographicHash>
#include <plugininterface/proxy.h>
#include <plugininterface/historymodel.h>
#include <interfaces/iinfo.h>
#include <interfaces/iwantnetworkaccessmanager.h>
#include "core.h"
#include "pluginmanager.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;
using LeechCraft::Util::HistoryModel;
using LeechCraft::Util::MergeModel;
using LeechCraft::Util::Proxy;

LeechCraft::PluginManager::PluginManager (QObject *parent)
: QAbstractItemModel (parent)
{
    FindPlugins ();
}

LeechCraft::PluginManager::~PluginManager ()
{
}

int LeechCraft::PluginManager::columnCount (const QModelIndex&) const
{
	return 1;
}

QVariant LeechCraft::PluginManager::data (const QModelIndex& index, int role) const
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
				case 45:
					return Plugins_.at (index.row ())->instance ();
				default:
					return QVariant ();
			}
		default:
			return QVariant ();
	}
}

Qt::ItemFlags LeechCraft::PluginManager::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant LeechCraft::PluginManager::headerData (int, Qt::Orientation, int) const
{
	return QVariant ();
}

QModelIndex LeechCraft::PluginManager::index (int row, int column, const QModelIndex&) const
{
	return createIndex (row, column);
}

QModelIndex LeechCraft::PluginManager::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int LeechCraft::PluginManager::rowCount (const QModelIndex& index) const
{
	if (!index.isValid ())
		return Plugins_.size ();
	else
		return 0;
}

LeechCraft::PluginManager::Size_t LeechCraft::PluginManager::GetSize () const
{
    return Plugins_.size ();
}

void LeechCraft::PluginManager::Release ()
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
            QMessageBox::warning (0,
					tr ("No exit here"),
					tr ("Release of one or more plugins failed."));
        }
        Plugins_.removeAt (Plugins_.size () - 1);
    }
}

void LeechCraft::PluginManager::Release (LeechCraft::PluginManager::Size_t position)
{
    if (position >= GetSize ())
        throw std::runtime_error ("LeechCraft::PluginManager::Release(): position is out of bounds.");

    if (Plugins_ [position] && Plugins_ [position]->isLoaded ())
	{
        qobject_cast<IInfo*> (Plugins_ [position]->instance ())->Release ();
		delete Plugins_ [position]->instance ();
		delete Plugins_ [position];
	}
}

QString LeechCraft::PluginManager::Name (const LeechCraft::PluginManager::Size_t& pos) const
{
    return (qobject_cast<IInfo*> (Plugins_ [pos]->instance ()))->GetName ();
}

QString LeechCraft::PluginManager::Info (const LeechCraft::PluginManager::Size_t& pos) const
{
    return qobject_cast<IInfo*> (Plugins_ [pos]->instance ())->GetInfo ();
}

QObjectList LeechCraft::PluginManager::GetAllPlugins () const
{
    QObjectList result;
    for (PluginsContainer_t::const_iterator i = Plugins_.begin ();
			i != Plugins_.end (); ++i)
        result << (*i)->instance ();
    return result;
}

void LeechCraft::PluginManager::CheckPlugins ()
{
    for (int i = 0; i < Plugins_.size (); ++i)
    {
        QPluginLoader *loader = Plugins_.at (i);

		if (!QFileInfo (loader->fileName ()).isFile ())
		{
			qWarning () << "A plugin isn't really a file, aborting load:"
				<< loader->fileName ();
            Plugins_.removeAt (i--);
			continue;
		}

        loader->load ();
        if (!loader->isLoaded ())
        {
            qWarning () << "Could not load library: "
				<< loader->fileName ()
				<< "; "
				<< loader->errorString ();
            Plugins_.removeAt (i--);
            continue;
        }

        QObject *pluginEntity = loader->instance ();
        IInfo *info = qobject_cast<IInfo*> (pluginEntity);
        if (!info)
        {
            qWarning () << "Library successfully loaded, but it could "
				"not be initialized (casting to IInfo failed): "
					<< loader->fileName ();
            Plugins_.removeAt (i--);
            continue;
        }
    }
}

void LeechCraft::PluginManager::InitializePlugins ()
{
    for (int i = 0; i < Plugins_.size (); ++i)
    {
        QPluginLoader *loader = Plugins_.at (i);
        QObject *pluginEntity = loader->instance ();
        IInfo *info = qobject_cast<IInfo*> (pluginEntity);

		emit loadProgress (info->GetName ());

		IWantNetworkAccessManager *iwnam =
			qobject_cast<IWantNetworkAccessManager*> (pluginEntity);
		if (iwnam)
			iwnam->SetNetworkAccessManager (Core::Instance ().GetNetworkAccessManager ());

        info->Init ();
	}
}

void LeechCraft::PluginManager::CalculateDependencies ()
{
    for (PluginsContainer_t::const_iterator i = Plugins_.begin ();
			i != Plugins_.end (); ++i)
    {
        QObject *pEntity = (*i)->instance ();
        IInfo *info = qobject_cast<IInfo*> (pEntity);
        QStringList needs = info->Needs (),
                    uses = info->Uses ();
        DependenciesMet_ [i] = true;

		QStringList provides = info->Provides ();
		
		for (QStringList::const_iterator j = provides.begin (),
				end = provides.end (); j != end; ++j)
			FeatureProviders_ [*j] = i;

        if (!needs.isEmpty ())
        {
			for (int j = 0; j < needs.size (); ++j)
			{
				QList<QString> parsed = needs.at (j).split ("::",
						QString::SkipEmptyParts);
				if (parsed.size () < 2)
					continue;

				if (parsed [0] == "services")
				{
					if (parsed [1] == "historyModel")
					{
						QMetaObject::invokeMethod (pEntity,
								"pushHistoryModel",
								Q_ARG (MergeModel*,
									Core::Instance ().GetUnfilteredHistoryModel ()));
					}
					else if (parsed [1] == "downloadersModel")
					{
						QMetaObject::invokeMethod (pEntity,
								"pushDownloadersModel",
								Q_ARG (MergeModel*,
									Core::Instance ().GetUnfilteredTasksModel ()));
					}
					else if (parsed [1] == "subscriptions" && parsed.size () >= 3)
					{
						if (parsed [2] == "selectedDownloaderChanged")
							SelectedDownloaderWatchers_ << pEntity;
					}
				}

				needs.removeAt (j--);
			}

			if (needs.contains ("*"))
			{
				for (PluginsContainer_t::const_iterator j = Plugins_.begin ();
						j != Plugins_.end (); ++j)
				{
					if (j == i)
						continue;

					QObject *qpEntity = (*j)->instance ();
					info->SetProvider (qpEntity, "*");
				}
				needs.removeAll ("*");
			}
			else
			{
				for (PluginsContainer_t::const_iterator j = Plugins_.begin ();
						j != Plugins_.end (); ++j)
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
					qWarning () << Q_FUNC_INFO
						<< "not all plugins providing needs of"
						<< info->GetName () 
						<< "are found. The remaining ones are:"
						<< needs;
				}
			}
        }
        if (!uses.isEmpty ())
        {
            QMap<QString, bool> usesMet;
            for (int j = 0; j < uses.size (); ++j)
                usesMet [uses.at (j)] = false;

            for (PluginsContainer_t::const_iterator j = Plugins_.begin ();
					j != Plugins_.end (); ++j)
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

QObject* LeechCraft::PluginManager::GetProvider (const QString& feature) const
{
	if (!FeatureProviders_.contains (feature))
		return 0;
	return (*FeatureProviders_ [feature])->instance ();
}

QObjectList LeechCraft::PluginManager::GetSelectedDownloaderWatchers () const
{
	return SelectedDownloaderWatchers_;
}

void LeechCraft::PluginManager::FindPlugins ()
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

