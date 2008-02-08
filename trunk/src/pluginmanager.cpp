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
#include "mainwindow.h"

using namespace Main;

PluginManager::PluginManager (QObject *parent)
: QObject (parent)
{
    FindPlugins ();
}

PluginManager::~PluginManager ()
{
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
            Release (Plugins_.size () - 1);
            Plugins_.removeAt (Plugins_.size () - 1);
        }
        catch (...)
        {
            QMessageBox::warning (0, tr ("No exit here"), tr ("Release of one or more plugins failed. OS would cleanup after us, but it isn't good anyway, as this failed plugin could fail to save it's state."));
        }
    }
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
    QDir pluginsDir = QDir ("/usr/local/share/leechcraft/plugins");
    foreach (QString filename, pluginsDir.entryList (QStringList ("*leechcraft_*"), QDir::Files))
        Plugins_.push_back (new QPluginLoader (pluginsDir.absoluteFilePath (filename), this));

    pluginsDir = QDir ("/usr/share/leechcraft/plugins");
    foreach (QString filename, pluginsDir.entryList (QStringList ("*leechcraft_*"), QDir::Files))
        Plugins_.push_back (new QPluginLoader (pluginsDir.absoluteFilePath (filename), this));

    pluginsDir = QDir (QApplication::applicationDirPath ());
    if (pluginsDir.cd ("plugins/bin"))
        foreach (QString filename, pluginsDir.entryList (QStringList ("*leechcraft_*"), QDir::Files))
            Plugins_.push_back (new QPluginLoader (pluginsDir.absoluteFilePath (filename), this));
}

