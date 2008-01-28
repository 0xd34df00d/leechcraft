#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include <QVector>
#include <QObject>
#include <QMap>
#include <QPluginLoader>
#include "plugininfo.h"
#include "interfaces/interfaces.h"

namespace Main
{
	class MainWindow;
	class PluginManager : public QObject
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
		Size_t GetSize () const;
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
		void ThrowPlugins ();
	private:
		void FindPlugins ();
	signals:
		void gotPlugin (const PluginInfo*);
		void downloadFinished (QString);
	};
};

#endif

