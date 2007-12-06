#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include <QVector>
#include <QObject>
#include "interfaces/interfaces.h"

class QPluginLoader;
class PluginInfo;

class PluginManager : public QObject
{
	Q_OBJECT

	typedef QVector<QPluginLoader*> PluginsContainer_t;
	QVector<QPluginLoader*> Plugins_;
public:
	typedef PluginsContainer_t::size_type Size_t;
	class Iterator : public QObject
	{
		friend class PluginManager;
		int Position_;
		QObject *PointeeCache_;
		bool CacheValid_;

		Iterator (int pos = 0, QObject *parent = 0);
	public:
		Iterator (const Iterator&);
		QObject* operator* ();
		const QObject* operator* () const;
		Iterator& operator++ ();
		Iterator& operator-- ();

		bool operator== (const Iterator&);
		bool operator!= (const Iterator&);
	};
	friend class Iterator;

	PluginManager (QObject *parent = 0);
	virtual ~PluginManager ();
	Size_t GetSize () const;
	void Release (Size_t);
	QString Name (const Size_t& pos) const;
	QString Info (const Size_t& pos) const;
	Iterator FindByID (IInfo::ID_t);
	Iterator Begin ();
	Iterator End ();
	void InitializePlugins ();
	void CalculateDependencies ();
	void ThrowPlugins ();
private:
	void FindPlugins ();
signals:
	void gotPlugin (const PluginInfo*);
};

#endif

