#include <stdexcept>
#include <algorithm>
#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QMainWindow>
#include <QCryptographicHash>
#include <plugininterface/proxy.h>
#include <plugininterface/historymodel.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ipluginready.h>
#include <interfaces/iwantnetworkaccessmanager.h>
#include "core.h"
#include "pluginmanager.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;
using LeechCraft::Util::HistoryModel;
using LeechCraft::Util::MergeModel;
using LeechCraft::Util::Proxy;

LeechCraft::PluginManager::DepTreeItem::DepTreeItem ()
: Plugin_ (0)
, Initialized_ (false)
{
}

void LeechCraft::PluginManager::DepTreeItem::Print (int margin)
{
	QString pre (margin, ' ');
	qDebug () << pre << Plugin_ << Initialized_;
	qDebug () << pre << "Needed:";
	QList<DepTreeItem_ptr> items = Needed_.values ();
	Q_FOREACH (DepTreeItem_ptr item, items)
		item->Print (margin + 2);
	qDebug () << pre << "Used:";
	items = Used_.values ();
	Q_FOREACH (DepTreeItem_ptr item, items)
		item->Print (margin + 2);
}

LeechCraft::PluginManager::Finder::Finder (QObject *o)
: Object_ (o)
{
}

bool LeechCraft::PluginManager::Finder::operator() (LeechCraft::PluginManager::DepTreeItem_ptr item) const
{
	return Object_ == item->Plugin_;
}

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
					return QVariant::fromValue<QObject*> (Plugins_.at (index.row ())->instance ());
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

void LeechCraft::PluginManager::Init ()
{
	CheckPlugins ();
	CalculateDependencies ();
	InitializePlugins ();
}

void LeechCraft::PluginManager::Release ()
{
    while (Roots_.size ())
    {
        try
        {
			Release (Roots_.takeAt (0));
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

QObject* LeechCraft::PluginManager::GetProvider (const QString& feature) const
{
	if (!FeatureProviders_.contains (feature))
		return 0;
	return (*FeatureProviders_ [feature])->instance ();
}

void LeechCraft::PluginManager::FindPlugins ()
{
	QDir pluginsDir = QDir ("/usr/local/lib/leechcraft/plugins");
	Q_FOREACH (QString filename,
			pluginsDir.entryList (QStringList ("*leechcraft_*"),
				QDir::Files))
		Plugins_.push_back (QPluginLoader_ptr (new QPluginLoader (pluginsDir.absoluteFilePath (filename))));

	pluginsDir = QDir ("/usr/lib/leechcraft/plugins");
	Q_FOREACH (QString filename,
			pluginsDir.entryList (QStringList ("*leechcraft_*"),
				QDir::Files))
		Plugins_.push_back (QPluginLoader_ptr (new QPluginLoader (pluginsDir.absoluteFilePath (filename))));

	pluginsDir = QDir (QApplication::applicationDirPath ());
	if (pluginsDir.cd ("plugins/bin"))
		Q_FOREACH (QString filename,
				pluginsDir.entryList (QStringList ("*leechcraft_*"),
					QDir::Files))
			Plugins_.push_back (QPluginLoader_ptr (new QPluginLoader (pluginsDir.absoluteFilePath (filename))));
}

void LeechCraft::PluginManager::CheckPlugins ()
{
    for (int i = 0; i < Plugins_.size (); ++i)
    {
        QPluginLoader_ptr loader = Plugins_.at (i);

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
            qWarning () << "Could not load library:"
				<< loader->fileName ()
				<< ";"
				<< loader->errorString ();
            Plugins_.removeAt (i--);
            continue;
        }

        QObject *pluginEntity = loader->instance ();
        IInfo *info = qobject_cast<IInfo*> (pluginEntity);
        if (!info)
        {
            qWarning () << "Casting to IInfo failed:"
					<< loader->fileName ();
            Plugins_.removeAt (i--);
            continue;
        }
    }
}

QList<LeechCraft::PluginManager::PluginsContainer_t::const_iterator>
	LeechCraft::PluginManager::FindProviders (const QString& feature) const
{
	QList<PluginsContainer_t::const_iterator> result;
	for (PluginsContainer_t::const_iterator i = Plugins_.begin (),
			end = Plugins_.end (); i != end; ++i)
		if (qobject_cast<IInfo*> ((*i)->instance ())->
				Provides ().contains (feature))
			result << i;
	return result;
}

QList<LeechCraft::PluginManager::PluginsContainer_t::const_iterator>
	LeechCraft::PluginManager::FindProviders (const QByteArray& expected) const
{
	QList<PluginsContainer_t::const_iterator> result;
	for (PluginsContainer_t::const_iterator i = Plugins_.begin (),
			end = Plugins_.end (); i != end; ++i)
	{
		IPlugin2 *ip2 = qobject_cast<IPlugin2*> ((*i)->instance ());
		if (ip2 && ip2->GetPluginClass () == expected)
			result << i;
	}
	return result;
}

LeechCraft::PluginManager::DepTreeItem_ptr
	LeechCraft::PluginManager::GetDependency
		(QObject *entity)
{
	struct Finder
	{
		QObject *Plugin_;
		DepTreeItem_ptr Result_;

		Finder (QObject *plugin)
		: Plugin_ (plugin)
		{
		}

		bool operator() (DepTreeItem_ptr item)
		{
			QList<DepTreeItem_ptr> deps =
				item->Needed_.values () + item->Used_.values ();
			std::sort (deps.begin (), deps.end ());
			QList<DepTreeItem_ptr>::const_iterator last =
				std::unique (deps.begin (), deps.end ());

			for (QList<DepTreeItem_ptr>::const_iterator i = deps.begin ();
					i != last; ++i)
				if ((*i)->Plugin_ == Plugin_)
				{
					Result_ = *i;
					return true;
				}
			for (QList<DepTreeItem_ptr>::const_iterator i = deps.begin ();
					i != last; ++i)
				if (operator() (*i))
					return true;

			return false;
		}
	};

	Finder finder (entity);
	Q_FOREACH (DepTreeItem_ptr dep, Roots_)
	{
		if (dep->Plugin_ == entity)
		{
			Roots_.removeAll (dep);
			return dep;
		}
		else
		{
			if (finder (dep))
				return finder.Result_;
		}
	}

	return DepTreeItem_ptr ();
}

void LeechCraft::PluginManager::CalculateDependencies ()
{
	// This step prepares the most full graph that could possibly be.
	for (PluginsContainer_t::const_iterator i = Plugins_.begin (),
			end = Plugins_.end (); i != end; ++i)
		if (!GetDependency ((*i)->instance ()))
			Roots_ << CalculateSingle (i);
}

LeechCraft::PluginManager::DepTreeItem_ptr
	LeechCraft::PluginManager::CalculateSingle
		(LeechCraft::PluginManager::PluginsContainer_t::const_iterator i)
{
	QObject *entity = (*i)->instance ();
	IInfo *info = qobject_cast<IInfo*> (entity);
	QStringList needs = info->Needs ();
	QStringList uses = info->Uses ();

	DepTreeItem_ptr newDep (new DepTreeItem ());
	newDep->Plugin_ = entity;

	Q_FOREACH (QString need, needs)
	{
		QList<PluginsContainer_t::const_iterator> providers = FindProviders (need);
		Q_FOREACH (PluginsContainer_t::const_iterator p,
				providers)
		{
			// It's initialized already.
			if (p < i)
			{
				DepTreeItem_ptr depprov = GetDependency ((*p)->instance ());
				if (depprov)
					newDep->Needed_.insert (need, depprov);
			}
			else if (p > i)
				newDep->Needed_.insert (need, CalculateSingle (p));
		}
	}

	Q_FOREACH (QString use, uses)
	{
		QList<PluginsContainer_t::const_iterator> providers = FindProviders (use);
		Q_FOREACH (PluginsContainer_t::const_iterator p,
				providers)
		{
			// It's initialized already.
			if (p < i)
			{
				DepTreeItem_ptr depprov = GetDependency ((*p)->instance ());
				if (depprov)
					newDep->Used_.insert (use, depprov);
			}
			else if (p > i)
				newDep->Used_.insert (use, CalculateSingle (p));
		}
	}

	IPluginReady *ipr = qobject_cast<IPluginReady*> (entity);
	if (ipr)
	{
		QList<PluginsContainer_t::const_iterator> providers =
			FindProviders (ipr->GetExpectedPluginClass ());
		Q_FOREACH (PluginsContainer_t::const_iterator p,
				providers)
		{
			// It's initialized already.
			if (p < i)
			{
				DepTreeItem_ptr depprov = GetDependency ((*p)->instance ());
				if (depprov)
					newDep->Used_.insert ("__lc_plugin2", depprov);
			}
			else if (p > i)
				newDep->Used_.insert ("__lc_plugin2", CalculateSingle (p));
		}
	}

	return newDep;
}

void LeechCraft::PluginManager::InitializePlugins ()
{
	Q_FOREACH (DepTreeItem_ptr item, Roots_)
		InitializeSingle (item);
}

bool LeechCraft::PluginManager::InitializeSingle (LeechCraft::PluginManager::DepTreeItem_ptr item)
{
	QList<QString> keys = item->Needed_.uniqueKeys ();
	Q_FOREACH (QString key, keys)
	{
		QList<DepTreeItem_ptr> providers = item->Needed_.values (key);
		bool wasSuccessful = false;
		for (QList<DepTreeItem_ptr>::const_iterator i = providers.begin (),
				end = providers.end (); i != end; ++i)
		{
			if (!(*i)->Initialized_)
				InitializeSingle (*i);

			if (!(*i)->Initialized_)
				item->Needed_.remove (key, *i);
			else
			{
				wasSuccessful = true;

				qobject_cast<IInfo*> (item->Plugin_)->
					SetProvider ((*i)->Plugin_, key);;
			}
		}

		if (!wasSuccessful)
			return false;
	}

	keys = item->Used_.uniqueKeys ();
	Q_FOREACH (QString key, keys)
	{
		QList<DepTreeItem_ptr> providers = item->Used_.values (key);
		for (QList<DepTreeItem_ptr>::const_iterator i = providers.begin (),
				end = providers.end (); i != end; ++i)
		{
			if (!(*i)->Initialized_)
				InitializeSingle (*i);

			if (!(*i)->Initialized_)
				item->Used_.remove (key, *i);
			else
			{
				if (key == "__lc_plugin2")
					qobject_cast<IPluginReady*> (item->Plugin_)->
						AddPlugin ((*i)->Plugin_);
				else
					qobject_cast<IInfo*> (item->Plugin_)->
						SetProvider ((*i)->Plugin_, key);
			}
		}
	}

	try
	{
		IInfo *ii = qobject_cast<IInfo*> (item->Plugin_);
		IWantNetworkAccessManager *iwnam =
			qobject_cast<IWantNetworkAccessManager*> (item->Plugin_);
		if (iwnam)
			iwnam->SetNetworkAccessManager (Core::Instance ().GetNetworkAccessManager ());

		ii->Init ();
		item->Initialized_ = true;
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
		return false;
	}
	return true;
}

void LeechCraft::PluginManager::Release (DepTreeItem_ptr item)
{
	QList<DepTreeItem_ptr> deps =
		item->Needed_.values () + item->Used_.values ();
	std::sort (deps.begin (), deps.end ());
	QList<DepTreeItem_ptr>::const_iterator last =
		std::unique (deps.begin (), deps.end ());

	for (QList<DepTreeItem_ptr>::const_iterator i = deps.begin ();
			i != last; ++i)
		Release (*i);

	if (item->Initialized_)
	{
		IInfo *ii = qobject_cast<IInfo*> (item->Plugin_);
		qDebug () << Q_FUNC_INFO << ii->GetName ();
		ii->Release ();
		item->Initialized_ = false;
	}
}

void LeechCraft::PluginManager::DumpTree ()
{
	Q_FOREACH (DepTreeItem_ptr item, Roots_)
		item->Print ();
}

