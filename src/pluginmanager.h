#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include <QVector>
#include <QAbstractItemModel>
#include <QMap>
#include <QPluginLoader>
#include "plugininfo.h"
#include "interfaces/interfaces.h"

namespace Main
{
    class MainWindow;
    class PluginManager : public QAbstractItemModel
    {
        Q_OBJECT

        typedef QList<QPluginLoader*> PluginsContainer_t;
        PluginsContainer_t Plugins_;
        QMap<PluginsContainer_t::const_iterator, bool> DependenciesMet_;
        QMap<PluginsContainer_t::const_iterator, QStringList> FailedDependencies_;
    public:
        typedef PluginsContainer_t::size_type Size_t;
        PluginManager (QObject *parent = 0);
        virtual ~PluginManager ();

		virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
		virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		virtual Qt::ItemFlags flags (const QModelIndex&) const;
		virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
		virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
		virtual QModelIndex parent (const QModelIndex&) const;
		virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

        Size_t GetSize () const;
        void Release ();
        void Release (Size_t);
        QString Name (const Size_t& pos) const;
        QString Info (const Size_t& pos) const;
        QObject* FindByID (IInfo::ID_t) const;
        QObjectList GetAllPlugins () const;
        template<typename T> QObjectList GetAllCastableTo () const
        {
            QObjectList result;
            for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
            {
                QObject *instance = (*i)->instance ();
                if (dynamic_cast<T> (instance))
                    result << instance;
            }
            return result;
        }
        void InitializePlugins (const MainWindow*);
        void CalculateDependencies ();
    private:
        void FindPlugins ();
    signals:
        void gotPlugin (const PluginInfo*);
        void downloadFinished (QString);
    };
};

#endif

