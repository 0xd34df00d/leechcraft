#include <QApplication>
#include <QPluginLoader>
#include <QDir>
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QMainWindow>
#include <exceptions/outofbounds.h>
#include <exceptions/logic.h>
#include <plugininterface/proxy.h>
#include "core.h"
#include "pluginmanager.h"
#include "plugininfo.h"

PluginManager::Iterator::Iterator (int pos, QObject *parent)
: QObject (parent)
, Position_ (pos)
, PointeeCache_ (0)
, CacheValid_ (false)
{
}

PluginManager::Iterator::Iterator (const Iterator& obj)
: QObject (obj.parent ())
, Position_ (obj.Position_)
, PointeeCache_ (obj.PointeeCache_)
, CacheValid_ (obj.CacheValid_)
{
}

QObject* PluginManager::Iterator::operator* ()
{
	if (!CacheValid_)
	{
		PluginManager *p = qobject_cast<PluginManager*> (parent ());

		if (Position_ >= p->Plugins_.size ())
			throw Exceptions::OutOfBounds ("PluginManager::Iterator::operator*(): wrong position.");

		PointeeCache_ = p->Plugins_.at (Position_)->instance ();
		CacheValid_ = true;
	}

	return PointeeCache_;
}

PluginManager::Iterator& PluginManager::Iterator::operator++ ()
{
	++Position_;
	CacheValid_ = false;
	return *this;
}

PluginManager::Iterator& PluginManager::Iterator::operator-- ()
{
	--Position_;
	CacheValid_ = false;
	return *this;
}

bool PluginManager::Iterator::operator== (const Iterator& obj)
{
	return Position_ == obj.Position_;
}

bool PluginManager::Iterator::operator!= (const Iterator& obj)
{
	return !(*this == obj);
}

PluginManager::PluginManager (QObject *parent)
: QObject (parent)
{
	FindPlugins ();
}

PluginManager::~PluginManager ()
{
	while (Plugins_.size ())
	{
		try
		{
			Release (Plugins_.size () - 1);
			Plugins_.remove (Plugins_.size () - 1);
		}
		catch (...)
		{
			QMessageBox::warning (0, tr ("No exit here"), tr ("Release of one or more plugins failed. OS would cleanup after us, but it isn't good anyway, as this failed plugin could fail to save it's state."));
		}
	}
}

PluginManager::Size_t PluginManager::GetSize () const
{
	return Plugins_.size ();
}

void PluginManager::Release (PluginManager::Size_t position)
{
	if (position >= GetSize ())
		throw Exceptions::OutOfBounds ("PluginManager::Release(): position is out of bounds.");

	if (Plugins_ [position] && Plugins_ [position]->isLoaded ())
		qobject_cast<IInfo*> (Plugins_ [position]->instance ())->Release ();
}

QString PluginManager::Name (const PluginManager::Size_t& pos) const
{
	return (qobject_cast<IInfo*> (Plugins_ [pos]->instance ()))->GetName ();
}

QString PluginManager::Info (const PluginManager::Size_t& pos) const
{
	return qobject_cast<IInfo*> (Plugins_ [pos]->instance ())->GetInfo ();
}

PluginManager::Iterator PluginManager::FindByID (IInfo::ID_t id)
{
	for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
		if (qobject_cast<IInfo*> ((*i)->instance ())->GetID () == id)
			return Iterator (i - Plugins_.begin (), this);
	return Iterator (Plugins_.size (), this);
}

PluginManager::Iterator PluginManager::Begin ()
{
	return Iterator (0, this);
}

PluginManager::Iterator PluginManager::End ()
{
	return Iterator (Plugins_.size (), this);
}

void PluginManager::InitializePlugins ()
{
	for (PluginsContainer_t::iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
	{
		QPluginLoader *loader = *i;

		loader->load ();
		if (!loader->isLoaded ())
		{
			qWarning () << "Could not load library: " << loader->fileName () << "; " << loader->errorString ();
			Plugins_.erase (i--);
			continue;
		}

		QObject *pluginEntity = loader->instance ();
		IInfo *info = qobject_cast<IInfo*> (pluginEntity);
		if (!info)
		{
			qWarning () << "Library successfully loaded, but it could not be initialized (casting to IInfo failed): " << loader->fileName ();
			Plugins_.erase (i--);
			continue;
		}
		info->Init ();
		info->SetID (i - Plugins_.begin ());
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
				for (int i = 0; i < needs.size (); ++i)
					if (qprovides.contains (needs [i]))
					{
						info->SetProvider (qpEntity, needs [i]);
						needs.removeAt (i--);
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
			for (PluginsContainer_t::const_iterator j = Plugins_.begin (); j != Plugins_.end (); ++j)
			{
				if (j == i)
					continue;

				QObject *qpEntity = (*j)->instance ();
				IInfo *qinfo = qobject_cast<IInfo*> (qpEntity);
				QStringList qprovides = qinfo->Provides ();
				for (int i = 0; i < uses.size (); ++i)
					if (qprovides.contains (uses [i]))
					{
						info->SetProvider (qpEntity, uses [i]);
						uses.removeAt (i--);
					}
			}
		}
	}
}

void PluginManager::ThrowPlugins ()
{
	for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
	{
		QObject *pEntity = (*i)->instance ();
		IInfo *info = qobject_cast<IInfo*> (pEntity);

		IWindow *window = qobject_cast<IWindow*> (pEntity);
		QIcon pIcon;
		if (window)
			pIcon = window->GetIcon ();

		PluginInfo *pInfo = new PluginInfo (info->GetName (),
											info->GetInfo (),
											pIcon,
											info->GetStatusbarMessage (),
											info->Provides (),
											info->Needs (),
											info->Uses (),
											DependenciesMet_ [i],
											FailedDependencies_ [i]);
		emit gotPlugin (pInfo);
	}
}

void PluginManager::FindPlugins ()
{
	QDir pluginsDir = QDir (qApp->applicationDirPath ());
	pluginsDir.cd ("plugins");
	pluginsDir.cd ("bin");

	QStringList pluginNames;
	foreach (QString filename, pluginsDir.entryList (QDir::Files))
		Plugins_.push_back (new QPluginLoader (pluginsDir.absoluteFilePath (filename), this));
}

