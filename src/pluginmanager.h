#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include <QMap>
#include <QMultiMap>
#include <QPluginLoader>

namespace LeechCraft
{
	class MainWindow;
	class PluginManager : public QAbstractItemModel
	{
		Q_OBJECT

		struct DepTreeItem;
		typedef boost::shared_ptr<DepTreeItem> DepTreeItem_ptr;
		struct DepTreeItem
		{
			QObject *Plugin_;
			bool Initialized_;
			// This plugin depends upon.
			QMultiMap<QString, DepTreeItem_ptr> Needed_;
			QMultiMap<QString, DepTreeItem_ptr> Used_;
			// What depends on this plugin.
			QList<DepTreeItem_ptr> Belongs_;

			DepTreeItem ();
			void Print (int = 0);
		};

		struct Finder
		{
			QObject *Object_;

			Finder (QObject*);
			bool operator() (DepTreeItem_ptr) const;
		};

		// No plugins are dependent on these.
		QList<DepTreeItem_ptr> Roots_;
		typedef boost::shared_ptr<QPluginLoader> QPluginLoader_ptr;
		typedef QList<QPluginLoader_ptr> PluginsContainer_t;
		mutable PluginsContainer_t Plugins_;
		QMap<QString, PluginsContainer_t::const_iterator> FeatureProviders_;

		typedef QList<PluginsContainer_t::iterator> UnloadQueue_t;
		UnloadQueue_t UnloadQueue_;
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
		void Init ();
		void Release ();
		QString Name (const Size_t& pos) const;
		QString Info (const Size_t& pos) const;
		QObjectList GetAllPlugins () const;

		template<typename T> QList<T> GetAllCastableTo () const
		{
			QList<T> result;
			for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
			{
				QObject *instance = (*i)->instance ();
				T casted = qobject_cast<T> (instance);
				if (casted)
					result << casted;
			}
			return result;
		}

		template<typename T> QObjectList GetAllCastableRoots () const
		{
			QObjectList result;
			for (PluginsContainer_t::const_iterator i = Plugins_.begin (); i != Plugins_.end (); ++i)
			{
				QObject *instance = (*i)->instance ();
				if (qobject_cast<T> (instance))
					result << instance;
			}
			return result;
		}

		QObject* GetProvider (const QString&) const;
		void Unload (QObject*);
	private:
		void FindPlugins ();
		/** Tries to load all the plugins and filters out those who fail
		 * various sanity checks.
		 */
		void CheckPlugins ();

		QList<PluginsContainer_t::iterator> FindProviders (const QString&);
		QList<PluginsContainer_t::iterator> FindProviders (const QByteArray&);

		/** Returns dependency item that matches the given object.
		 */
		DepTreeItem_ptr GetDependency (QObject *object);

		/** Calculates the deps.
		 */
		void CalculateDependencies ();
		DepTreeItem_ptr CalculateSingle (PluginsContainer_t::iterator i);

		/** Preinitializes the plugins, pushes second-level plugins to
		 * first-level ones and calls IInfo::Init() on each one.
		 */
		void InitializePlugins ();
		bool InitializeSingle (DepTreeItem_ptr);
		void Release (DepTreeItem_ptr);
		void DumpTree ();
		PluginsContainer_t::iterator Find (DepTreeItem_ptr);
		PluginsContainer_t::iterator Find (QObject*);
		void Unload (PluginsContainer_t::iterator);
	private slots:
		void processUnloadQueue ();
	signals:
		void downloadFinished (QString);
		void loadProgress (const QString&);
	};
};

#endif

