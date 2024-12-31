/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <stdexcept>
#include <algorithm>
#include <functional>
#include <numeric>
#include <optional>

#if defined __GNUC__
#include <cstdlib>
#include <cxxabi.h>
#endif

#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QtDebug>
#include <QElapsedTimer>
#include <QtConcurrentMap>
#include <QMessageBox>
#include <QMainWindow>
#include <util/sll/containerconversions.h>
#include <util/sll/prelude.h>
#include <util/sll/scopeguards.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ipluginadaptor.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ishutdownlistener.h>
#include "core.h"
#include "pluginmanager.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"
#include "coreproxy.h"
#include "plugintreebuilder.h"
#include "lcconfig.h"
#include "coreinstanceobject.h"
#include "shortcutmanager.h"
#include "application.h"
#include "loadprogressreporter.h"
#include "settingstab.h"
#include "loaders/sopluginloader.h"
#include "loadprocessbase.h"
#include "splashscreen.h"
#include "clargs.h"

#ifdef WITH_DBUS_LOADERS
#include "loaders/dbuspluginloader.h"
#endif

namespace LC
{
	PluginManager::PluginManager (const QStringList& pluginPaths, QObject *parent)
	: QAbstractItemModel (parent)
	, DBusMode_ (static_cast<Application*> (qApp)->GetParsedArguments ().Multiprocess_)
	, PluginTreeBuilder_ (new PluginTreeBuilder)
	{
		Headers_ << tr ("Name")
			<< tr ("Description");

		if (pluginPaths.isEmpty ())
			FindPlugins ();
		else
		{
			const auto& allPluginsPaths = FindPluginsPaths ();

			qDebug () << "explicit paths given, entering forced loading mode";
			for (const auto& path : pluginPaths)
			{
				const QFileInfo fi { path };
				const auto& toLoad = fi.isFile () ?
						QStringList { fi.absoluteFilePath () } :
						allPluginsPaths.filter (path, Qt::CaseInsensitive);
				for (const auto& single : toLoad)
				{
					qDebug () << "adding" << single;
					const auto loader = MakeLoader (single);
					PluginContainers_.push_back (loader);
				}
			}
		}
	}

	int PluginManager::columnCount (const QModelIndex&) const
	{
		return Headers_.size () + 1;
	}

	QVariant PluginManager::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid () ||
				index.row () >= GetSize ())
			return QVariant ();

		if (role == Roles::PluginObject)
		{
			auto loader = AvailablePlugins_ [index.row ()];
			if (!loader || !loader->IsLoaded ())
				return QVariant ();

			return QVariant::fromValue<QObject*> (loader->Instance ());
		}
		else if (role == Roles::PluginID)
		{
			auto loader = AvailablePlugins_ [index.row ()];
			if (!loader || !loader->IsLoaded ())
				return QVariant ();

			return qobject_cast<IInfo*> (loader->Instance ())->GetUniqueID ();
		}
		else if (role == Roles::PluginFilename)
		{
			auto loader = AvailablePlugins_ [index.row ()];
			if (!loader)
				return QVariant ();

			return loader->GetFileName ();
		}

		switch (index.column ())
		{
		case 0:
			switch (role)
			{
			case Qt::DisplayRole:
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "-pg");
				settings.beginGroup ("Plugins");
				settings.beginGroup (AvailablePlugins_.at (index.row ())->GetFileName ());
				QVariant result = settings.value ("Name");
				settings.endGroup ();
				settings.endGroup ();
				return result;
			}
			case Qt::DecorationRole:
			{
				const auto& loader = AvailablePlugins_.at (index.row ());

				const auto& manifestIcon = CoreProxy { loader }.GetIconThemeManager ()->GetPluginIcon ();
				if (!manifestIcon.isNull ())
					return manifestIcon;

				if (!loader->IsLoaded ())
					return DefaultPluginIcon_;
				const auto& res = qobject_cast<IInfo*> (loader->Instance ())->GetIcon ();
				return res.isNull () ? DefaultPluginIcon_ : res;
			}
			case Qt::CheckStateRole:
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "-pg");
				settings.beginGroup ("Plugins");
				settings.beginGroup (AvailablePlugins_.at (index.row ())->GetFileName ());
				bool result = settings.value ("AllowLoad", true).toBool ();
				settings.endGroup ();
				settings.endGroup ();
				return result ? Qt::Checked : Qt::Unchecked;
			}
			case Qt::ForegroundRole:
				return QApplication::palette ()
					.brush (AvailablePlugins_.at (index.row ())->IsLoaded () ?
							QPalette::Normal :
							QPalette::Disabled,
						QPalette::WindowText);
			default:
				return QVariant ();
			}
		case 1:
			if (role == Qt::DisplayRole)
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "-pg");
				settings.beginGroup ("Plugins");
				settings.beginGroup (AvailablePlugins_.at (index.row ())->GetFileName ());
				QVariant result = settings.value ("Info");
				settings.endGroup ();
				settings.endGroup ();
				return result;
			}
			else if (role == Qt::ForegroundRole)
				return QApplication::palette ()
					.brush (AvailablePlugins_.at (index.row ())->IsLoaded () ?
							QPalette::Normal :
							QPalette::Disabled,
						QPalette::WindowText);
			else
				return QVariant ();
		case 2:
			if (role == Qt::SizeHintRole)
				return QSize (32, 32);
			else
				return QVariant ();
		default:
			return QVariant ();
		}
	}

	Qt::ItemFlags PluginManager::flags (const QModelIndex& index) const
	{
		Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		if (index.column () == 0)
			result |= Qt::ItemIsUserCheckable;
		else if (index.column () == 2)
		{
			const int row = index.row ();
			if (AvailablePlugins_ [row]->IsLoaded () &&
					qobject_cast<IHaveSettings*> (AvailablePlugins_ [row]->Instance ()))
				result |= Qt::ItemIsEditable;
		}
		return result;
	}

	QVariant PluginManager::headerData (int column, Qt::Orientation orient, int role) const
	{
		if (role != Qt::DisplayRole ||
				orient != Qt::Horizontal)
			return QVariant ();

		return Headers_.value (column, QString (""));
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
			return AvailablePlugins_.size ();
		else
			return 0;
	}

	bool PluginManager::setData (const QModelIndex& index,
			const QVariant& data, int role)
	{
		if (index.column () != 0 ||
				role != Qt::CheckStateRole)
			return false;

		auto loader = AvailablePlugins_.at (index.row ());

		if (!data.toBool () &&
				PluginContainers_.contains (loader))
		{
			PluginTreeBuilder builder;
			builder.AddObjects (Plugins_);
			builder.Calculate ();

			auto oldSet = Util::AsSet (builder.GetResult ());

			builder.RemoveObject (loader->Instance ());
			builder.Calculate ();

			const auto& newSet = Util::AsSet (builder.GetResult ());
			oldSet.subtract (newSet);

			oldSet.remove (loader->Instance ());

			if (!oldSet.isEmpty ())
			{
				QStringList pluginNames;
				for (auto obj : oldSet)
				{
					const auto ii = qobject_cast<IInfo*> (obj);
					pluginNames << (ii->GetName () + " (" + ii->GetInfo () + ")");
				}

				if (QMessageBox::question (0,
						"LeechCraft",
						tr ("The following plugins would also be disabled as the result:") +
							"<ul><li>" + pluginNames.join ("</li><li>") + "</li></ul>" +
							tr ("Are you sure you want to disable this one?"),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
					return false;

				QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
				settings.beginGroup ("Plugins");

				for (const auto obj : oldSet)
				{
					if (!Obj2Loader_.contains (obj))
						continue;

					auto dl = Obj2Loader_ [obj];

					settings.beginGroup (dl->GetFileName ());
					settings.setValue ("AllowLoad", false);
					settings.endGroup ();

					const int row = AvailablePlugins_.indexOf (dl);
					const QModelIndex& dIdx = createIndex (row, 0);
					emit dataChanged (dIdx, dIdx);
				}

				settings.endGroup ();
			}
		}

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		settings.beginGroup ("Plugins");
		settings.beginGroup (loader->GetFileName ());
		settings.setValue ("AllowLoad", data.toBool ());
		settings.endGroup ();
		settings.endGroup ();

		emit dataChanged (index, index);

		return true;
	}

	PluginManager::Size_t PluginManager::GetSize () const
	{
		return AvailablePlugins_.size ();
	}

	class PluginManager::PluginLoadProcess : public LoadProcessBase
	{
		const QString Title_;
		int Count_;
		int Value_ = 0;
	public:
		PluginLoadProcess (const QString& title, int count)
		: Title_ { title }
		, Count_ { count }
		{
			static_cast<Application*> (qApp)->GetSplashScreen ()->RegisterLoadProcess (this);
		}

		QString GetTitle () const override
		{
			return Title_;
		}

		int GetMin () const override
		{
			return 0;
		}

		int GetMax () const override
		{
			return Count_;
		}

		int GetValue () const override
		{
			return Value_;
		}

		void ReportValue (int value) override
		{
			Value_ = value;
			emit changed ();
		}

		void SetCount (int count)
		{
			Count_ = count;
			emit changed ();
		}
	};

	QObject* PluginManager::TryFirstInit (QObjectList ordered, PluginLoadProcess *proc)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		const auto guard = Util::BeginGroup (settings, "Plugins");

		for (const auto obj : ordered)
		{
			++*proc;

			const auto ii = qobject_cast<IInfo*> (obj);
			try
			{
				qDebug () << "Initializing" << ii->GetName ();
				ii->Init (ii->GetProxy ());

				const auto& path = GetPluginLibraryPath (obj);
				if (path.isEmpty ())
					continue;

				settings.beginGroup (path);
				settings.setValue ("Info", ii->GetInfo ());
				settings.endGroup ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "while initializing"
						<< obj
						<< "got"
						<< e.what ();
				return obj;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
						<< "while initializing"
						<< obj
						<< "caught unknown exception";
				return obj;
			}
		}

		return 0;
	}

	void PluginManager::TryUnload (QObjectList plugins)
	{
		for (const auto object : plugins)
		{
			if (!Obj2Loader_.contains (object))
				continue;

			qDebug () << "trying to unload"
					<< object;
			Obj2Loader_ [object]->Unload ();
			Obj2Loader_.remove (object);

			PluginContainers_.erase (std::remove_if (PluginContainers_.begin (), PluginContainers_.end (),
						[object] (const Loaders::IPluginLoader_ptr& loader)
							{ return loader->Instance () == object; }),
					PluginContainers_.end ());
		}
	}

	void PluginManager::Init (bool safeMode)
	{
		DefaultPluginIcon_ = QIcon ("lcicons:/resources/images/defaultpluginicon.svg");
		CheckPlugins ();
		FillInstances ();

		if (safeMode)
			Plugins_.clear ();

		Plugins_.prepend (Core::Instance ().GetCoreInstanceObject ());

		PluginTreeBuilder_->AddObjects (Plugins_);
		PluginTreeBuilder_->Calculate ();

		auto ordered = PluginTreeBuilder_->GetResult ();

		for (const auto plugin : Plugins_)
			if (!ordered.contains (plugin))
				if (const auto val = Obj2Loader_.value (plugin))
				{
					const auto ii = qobject_cast<IInfo*> (plugin);
					qDebug () << Q_FUNC_INFO
							<< "will try to unload loader for failed instance"
							<< (ii ? ii->GetUniqueID () : "<unknown>");
					qDebug () << val->Unload ();
				}

		const auto fstInitProc = std::make_shared<PluginLoadProcess> (tr ("Plugins initialization: first stage..."),
					ordered.size ());
		const auto sndInitProc = std::make_shared<PluginLoadProcess> (tr ("Plugins initialization: second stage..."),
					ordered.size ());

		const auto& failed = FirstInitAll (fstInitProc.get ());

		SetInitStage (InitStage::BeforeSecond);

		// FirstInitAll() might have resulted in some plugins failing to
		// initialize and the deps tree being recomputed,
		// so we need to update the list of plugins to be initialized.
		ordered = PluginTreeBuilder_->GetResult ();

		auto coreInstanceObj = Core::Instance ().GetCoreInstanceObject ();
		for (auto obj : GetAllCastableRoots<IHaveShortcuts*> ())
			coreInstanceObj->GetShortcutManager ()->AddObject (obj);

		const auto& plugins2 = GetAllCastableRoots<IPlugin2*> ();
		for (const auto provider : GetAllCastableTo<IPluginReady*> ())
		{
			const auto& expected = provider->GetExpectedPluginClasses ();
			for (QObject *ip2 : plugins2)
				if (qobject_cast<IPlugin2*> (ip2)->GetPluginClasses ().intersect (expected).size ())
					provider->AddPlugin (ip2);
		}

		sndInitProc->SetCount (ordered.size ());

		for (const auto obj : ordered)
		{
			++*sndInitProc;
			const auto ii = qobject_cast<IInfo*> (obj);
			try
			{
				qDebug () << "second init" << ii->GetName ();
				ii->SecondInit ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "while initializing"
						<< obj
						<< "got"
						<< e.what ();
				continue;
			}
		}

		SetInitStage (InitStage::PostSecond);

		for (const auto plugin : GetAllPlugins ())
			Core::Instance ().PostSecondInit (plugin);

		SetInitStage (InitStage::Complete);

		TryUnload (failed);
	}

	void PluginManager::Release ()
	{
		auto ordered = PluginTreeBuilder_->GetResult ();
		std::reverse (ordered.begin (), ordered.end ());

		for (const auto obj : ordered)
			if (const auto isl = qobject_cast<IShutdownListener*> (obj))
				isl->HandleShutdownInitiated ();

		for (const auto obj : ordered)
		{
			try
			{
				const auto ii = qobject_cast<IInfo*> (obj);
				if (!ii)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast"
							<< obj
							<< "to IInfo";
					continue;
				}
				qDebug () << "Releasing" << ii->GetName ();
				ii->Release ();

				const auto& loader = Obj2Loader_.value (obj);
				if (!loader)
					continue;

				qDebug () << "Unloading" << loader->GetFileName ();
				loader->Unload ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< obj
						<< e.what ();
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown release error for"
						<< obj;
			}
		}

		qDebug () << Q_FUNC_INFO
				<< "destroying loaders...";
		PluginTreeBuilder_.reset ();
		AvailablePlugins_.clear ();
		Obj2Loader_.clear ();
		Plugins_.clear ();
		PluginContainers_.clear ();
		qDebug () << Q_FUNC_INFO
				<< "done!";
	}

	QString PluginManager::Name (const PluginManager::Size_t& pos) const
	{
		return (qobject_cast<IInfo*> (Plugins_ [pos]))->GetName ();
	}

	QString PluginManager::Info (const PluginManager::Size_t& pos) const
	{
		return qobject_cast<IInfo*> (Plugins_ [pos])->GetInfo ();
	}

	QList<Loaders::IPluginLoader_ptr> PluginManager::GetAllAvailable () const
	{
		return AvailablePlugins_;
	}

	QObjectList PluginManager::GetAllPlugins () const
	{
		if (!CacheValid_)
		{
			CacheValid_ = true;
			SortedCache_ = PluginTreeBuilder_->GetResult ();
			std::sort (SortedCache_.begin (), SortedCache_.end (),
					[] (QObject *p1, QObject *p2)
						{ return qobject_cast<IInfo*> (p1)->GetName () < qobject_cast<IInfo*> (p2)->GetName (); });
		}
		return SortedCache_;
	}

	QString PluginManager::GetPluginLibraryPath (const QObject *object) const
	{
		for (auto loader : PluginContainers_)
			if (loader->Instance () == object)
				return loader->GetFileName ();
		return QString ();
	}

	QObject* PluginManager::GetPluginByID (const QByteArray& id) const
	{
		if (!PluginID2PluginCache_.contains (id))
			for (const auto plugin : GetAllPlugins ())
				if (qobject_cast<IInfo*> (plugin)->GetUniqueID () == id)
				{
					PluginID2PluginCache_ [id] = plugin;
					break;
				}

		return PluginID2PluginCache_ [id];
	}

	QObjectList PluginManager::GetFirstLevels (const QByteArray& pclass) const
	{
		QObjectList result;
		for (auto pluginObj : GetAllCastableRoots<IPluginReady*> ())
			if (qobject_cast<IPluginReady*> (pluginObj)->GetExpectedPluginClasses ().contains (pclass))
				result << pluginObj;
		return result;
	}

	QObjectList PluginManager::GetFirstLevels (const QSet<QByteArray>& pclasses) const
	{
		return std::accumulate (pclasses.begin (), pclasses.end (), QObjectList (),
				[this] (QObjectList list, const QByteArray& pc) -> QObjectList
				{
					for (const auto plugin : GetFirstLevels (pc))
						if (!list.contains (plugin))
							list << plugin;
					return list;
				});
	}

	void PluginManager::InjectPlugin (QObject *object)
	{
		try
		{
			qobject_cast<IInfo*> (object)->Init (CoreProxy::UnsafeWithoutDeps ());
			qobject_cast<IInfo*> (object)->SecondInit ();
			Core::Instance ().PostSecondInit (object);

			IPlugin2 *ip2 = qobject_cast<IPlugin2*> (object);
			if (ip2)
			{
				QSet<QByteArray> classes = ip2->GetPluginClasses ();
				for (const auto ipr : GetAllCastableTo<IPluginReady*> ())
					if (!ipr->GetExpectedPluginClasses ()
							.intersect (classes).isEmpty ())
						ipr->AddPlugin (object);
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to init the object with"
				<< e.what ()
				<< "for"
				<< object;
			throw;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to init"
				<< object;
			throw;
		}

		emit pluginInjected (object);
	}

	void PluginManager::ReleasePlugin (QObject *object)
	{
		try
		{
			qDebug () << "Releasing"
					<< qobject_cast<IInfo*> (object)->GetName ();
			qobject_cast<IInfo*> (object)->Release ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to release the unloading object with"
				<< e.what ()
				<< "for"
				<< object;
			throw;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to release the unloading object"
				<< object;
		}
	}

	void PluginManager::SetAllPlugins (Qt::CheckState state)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		settings.beginGroup ("Plugins");

		for (int i = 0; i < AvailablePlugins_.size (); ++i)
		{
			auto dl = AvailablePlugins_.at (i);
			settings.beginGroup (dl->GetFileName ());
			settings.setValue ("AllowLoad", state == Qt::Checked);
			settings.endGroup ();
			const QModelIndex& dIdx = createIndex (i, 0);
			emit dataChanged (dIdx, dIdx);
		}

		settings.endGroup ();
	}

	QObject* PluginManager::GetQObject ()
	{
		return this;
	}

	void PluginManager::OpenSettings (QObject *plugin)
	{
		auto instObj = Core::Instance ().GetCoreInstanceObject ();

		auto tab = instObj->GetSettingsTab ();
		tab->showSettingsFor (plugin);

		const auto& id = tab->GetTabClassInfo ().TabClass_;
		instObj->TabOpenRequested (id);
	}

	ILoadProgressReporter_ptr PluginManager::CreateLoadProgressReporter (QObject*)
	{
		return std::make_shared<LoadProgressReporter> ();
	}

	QIcon PluginManager::GetPluginIcon (QObject *obj)
	{
		return qobject_cast<IInfo*> (obj)->GetIcon ();
	}

	const QStringList& PluginManager::GetPluginLoadErrors () const
	{
		return PluginLoadErrors_;
	}

	PluginManager::InitStage PluginManager::GetInitStage () const
	{
		return InitStage_;
	}

	void PluginManager::SetInitStage (PluginManager::InitStage stage)
	{
		if (InitStage_ == stage)
			return;

		InitStage_ = stage;
		emit initStageChanged (stage);
	}

	QStringList PluginManager::FindPluginsPaths () const
	{
		QStringList result;

		auto scan = [&result] (const QString& path)
		{
#ifdef Q_OS_WIN32
			const QStringList nameFilters { "*leechcraft_*.dll" };
#else
			const QStringList nameFilters { "*leechcraft_*" };
#endif
			result += Util::Map (QDir { path }.entryInfoList (nameFilters, QDir::Files),
					&QFileInfo::canonicalFilePath);
		};

#ifdef Q_OS_WIN32
		scan (QApplication::applicationDirPath () + "/plugins/bin");
#elif defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
		scan (QApplication::applicationDirPath () + "/../PlugIns");
#else
		QString libdir (PLUGINS_LIBDIR);
	#if defined (INSTALL_PREFIX)
		scan (QString (INSTALL_PREFIX "/%1/leechcraft/plugins" LC_LIBSUFFIX)
				.arg (libdir));
	#else
		scan (QString { "/usr/local/%1/leechcraft/plugins" LC_LIBSUFFIX }
				.arg (libdir));
		scan (QString { "/usr/%1/leechcraft/plugins" LC_LIBSUFFIX }
				.arg (libdir));
	#endif
#endif

		return result;
	}

	void PluginManager::FindPlugins ()
	{
		ScanPlugins (FindPluginsPaths ());
	}

	void PluginManager::ScanPlugins (const QStringList& paths)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		settings.beginGroup ("Plugins");

		for (const auto& path : paths)
		{
			const QFileInfo fileinfo { path };
			auto name = fileinfo.canonicalFilePath ();
			settings.beginGroup (name);

			const auto loader = MakeLoader (name);
			if (settings.value ("AllowLoad", true).toBool ())
				PluginContainers_.push_back (loader);
			AvailablePlugins_.push_back (loader);

			settings.endGroup ();
		}

		settings.endGroup ();
	}

	namespace Checks
	{
		struct Fail
		{
			QString Error_;
			bool Unload_;

			Fail (const QString& e, bool unload = false)
			: Error_ (e)
			, Unload_ (unload)
			{
			}
		};

		void IsFile (Loaders::IPluginLoader_ptr loader)
		{
			if (!QFileInfo (loader->GetFileName ()).isFile ())
			{
				qWarning () << "A plugin isn't really a file, aborting load:"
						<< loader->GetFileName ();
				throw Fail (PluginManager::tr ("Refusing to load plugin from %1 because it's not a file.")
							.arg (QFileInfo (loader->GetFileName ()).fileName ()));
			}
		}

		void APILevel (Loaders::IPluginLoader_ptr loader)
		{
			const auto apiLevel = loader->GetAPILevel ();
			if (apiLevel != CURRENT_API_LEVEL)
			{
				qWarning () << Q_FUNC_INFO
						<< "API level mismatch for"
						<< loader->GetFileName ();

				throw Fail (PluginManager::tr ("Could not load plugin from %1: API level mismatch.")
							.arg (loader->GetFileName ()));
			}
		}

		QString TryDemangle (const QString& errorStr)
		{
#if defined __GNUC__
			static const QString marker { "undefined symbol: " };
			const auto pos = errorStr.indexOf (marker);
			if (pos == -1)
				return {};

			auto mangled = errorStr.mid (pos + marker.size ());
			const auto endPos = mangled.indexOf (')');
			if (endPos >= 0)
				mangled = mangled.left (endPos);

			int status = 0;
			QString result;
			if (auto rawStr = abi::__cxa_demangle (mangled.toLatin1 ().constData (), 0, 0, &status))
			{
				result = QString::fromLatin1 (rawStr);
				std::free (rawStr);
			}
			return result;
#else
			return {};
#endif
		}

		void TryLoad (Loaders::IPluginLoader_ptr loader)
		{
			loader->Load ();
			if (!loader->IsLoaded ())
			{
				qWarning () << "Could not load library:"
					<< loader->GetFileName ()
					<< ";"
					<< loader->GetErrorString ();

				const auto& demangled = TryDemangle (loader->GetErrorString ());
				if (!demangled.isEmpty ())
					qWarning () << "demangled name:"
							<< demangled;

				throw Fail (PluginManager::tr ("Could not load plugin from %1: %2.")
							.arg (loader->GetFileName ())
							.arg (loader->GetErrorString ()));
			}
		}

		void TryInstance (Loaders::IPluginLoader_ptr loader)
		{
			QObject *inst = 0;
			try
			{
				inst = loader->Instance ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to construct the instance with"
					<< e.what ()
					<< "for"
					<< loader->GetFileName ();
				throw Fail (PluginManager::tr ("Could not load plugin from %1: "
								"failed to construct plugin instance with exception %2.")
							.arg (loader->GetFileName ())
							.arg (e.what ()),
						true);
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to construct the instance for"
						<< loader->GetFileName ();
				throw Fail (PluginManager::tr ("Could not load plugin from %1: "
								"failed to construct plugin instance.")
							.arg (loader->GetFileName ()),
						true);
			}

			if (!qobject_cast<IInfo*> (inst))
			{
				qWarning () << "Casting to IInfo failed:"
						<< loader->GetFileName ();
				throw Fail (PluginManager::tr ("Could not load plugin from %1: "
								"unable to cast plugin instance to IInfo*.")
							.arg (loader->GetFileName ()),
						true);
			}
		}
	}

	void PluginManager::CheckPlugins ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		settings.beginGroup ("Plugins");

		QHash<QByteArray, QString> id2source;

		QList<std::function<void (Loaders::IPluginLoader_ptr)>> checks
		{
			Checks::IsFile,
			Checks::TryLoad,
			Checks::APILevel
		};

		const bool shouldDump = qgetenv ("LC_DUMP_SOCHECKS") == "1";

		auto thrCheck = [shouldDump, checks] (Loaders::IPluginLoader_ptr loader) -> std::optional<Checks::Fail>
		{
			QElapsedTimer timer;
			if (shouldDump)
			{
				timer.start ();
				qDebug () << loader->GetFileName () << ": beginning checks";
			}

			for (const auto& check : checks)
				try
				{
					check (loader);
				}
				catch (const Checks::Fail& f)
				{
					return f;
				}
			if (shouldDump)
			{
				qDebug () << loader->GetFileName ()
						<< ": done checks in"
						<< timer.elapsed ()
						<< "ms";
			}

			return {};
		};

		QList<std::optional<Checks::Fail>> fails;
		if (!DBusMode_)
		{
			const auto mid = std::partition (PluginContainers_.begin (), PluginContainers_.end (),
					[] (const Loaders::IPluginLoader_ptr& loader)
					{
						return loader->GetManifest () ["RequireGUIThreadLibraryLoading"].toBool ();
					});
			auto future = QtConcurrent::mapped (mid, PluginContainers_.end (),
					std::function<std::optional<Checks::Fail> (Loaders::IPluginLoader_ptr)> (thrCheck));

			for (auto it = PluginContainers_.begin (); it != mid; ++it)
			{
				qDebug () << "running checks for"
						<< (*it)->GetFileName ()
						<< "in main thread";
				fails << thrCheck (*it);
			}

			fails += future.results ();
		}
		else
			for (const auto& loader : PluginContainers_)
				fails << thrCheck (loader);

		for (int i = fails.size () - 1; i >= 0; --i)
			if (fails [i])
			{
				PluginContainers_.removeAt (i);
				PluginLoadErrors_ << fails [i]->Error_;
			}

		checks.clear ();
		checks << Checks::TryInstance;

		for (int i = 0; i < PluginContainers_.size (); ++i)
		{
			auto loader = PluginContainers_.at (i);

			bool success = true;
			for (auto check : checks)
				try
				{
					check (loader);
				}
				catch (const Checks::Fail& f)
				{
					PluginLoadErrors_ << f.Error_;
					success = false;
					break;
				}

			if (!success)
			{
				PluginContainers_.removeAt (i--);
				continue;
			}

			IInfo *info = qobject_cast<IInfo*> (loader->Instance ());
			try
			{
				const QByteArray& id = info->GetUniqueID ();
				if (id2source.contains (id))
				{
					PluginLoadErrors_ << tr ("Plugin with ID %1 is "
							"already loaded from %2; aborting load "
							"from %3.")
						.arg (QString::fromUtf8 (id.constData ()))
						.arg (id2source [id])
						.arg (loader->GetFileName ());
					PluginContainers_.removeAt (i--);
				}
				else
					id2source [id] = loader->GetFileName ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to obtain plugin ID for plugin from"
						<< loader->GetFileName ()
						<< "with error:"
						<< e.what ();
				PluginContainers_.removeAt (i--);
				continue;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to obtain plugin ID for plugin from"
						<< loader->GetFileName ()
						<< "with unknown error";
				PluginContainers_.removeAt (i--);
				continue;
			}

			QString name = info->GetName ();
			QString pinfo = info->GetInfo ();

			settings.beginGroup (loader->GetFileName ());
			settings.setValue ("Name", name);
			settings.setValue ("Info", pinfo);
			settings.endGroup ();
		}

		settings.endGroup ();
	}

	void PluginManager::FillInstances ()
	{
		for (auto loader : PluginContainers_)
		{
			auto inst = loader->Instance ();
			Plugins_ << inst;
			Obj2Loader_ [inst] = loader;

			const auto ii = qobject_cast<IInfo*> (inst);
			auto proxy = std::make_shared<CoreProxy> (loader);
			ii->SetProxy (proxy);
			ii->SetPluginInstance (inst);

			IPluginAdaptor *ipa = qobject_cast<IPluginAdaptor*> (inst);
			if (ipa)
				Plugins_ << ipa->GetPlugins ();
		}
	}

	QObjectList PluginManager::FirstInitAll (PluginLoadProcess *proc)
	{
		QObjectList ordered = PluginTreeBuilder_->GetResult ();
		QObjectList initialized;
		QObjectList failedList;

		QObject *failed = 0;
		while ((failed = TryFirstInit (ordered, proc)))
		{
			CacheValid_ = false;

			failedList << failed;
			for (const auto obj : ordered)
			{
				if (failed == obj)
					break;
				initialized << obj;
			}

			PluginTreeBuilder_->RemoveObject (failed);

			qDebug () << failed
					<< "failed to initialize, recalculating dep tree...";
			PluginTreeBuilder_->Calculate ();

			ordered = PluginTreeBuilder_->GetResult ();
			for (const auto obj : initialized)
				ordered.removeAll (obj);

			proc->SetCount (ordered.size () + initialized.size ());
			proc->ReportValue (initialized.size ());
		}

		return failedList;
	}

	Loaders::IPluginLoader_ptr PluginManager::MakeLoader (const QString& filename)
	{

#ifdef WITH_DBUS_LOADERS
		if (DBusMode_)
			return std::make_shared<Loaders::DBusPluginLoader> (filename);
#endif
		return std::make_shared<Loaders::SOPluginLoader> (filename);
	}

	QList<PluginManager::Plugins_t::iterator>
	PluginManager::FindProviders (const QString& feature)
	{
		QList<Plugins_t::iterator> result;
		for (Plugins_t::iterator i = Plugins_.begin ();
				i != Plugins_.end (); ++i)
		{
			try
			{
				if (qobject_cast<IInfo*> (*i)->
						Provides ().contains (feature))
					result << i;
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get providers with"
					<< e.what ()
					<< "for"
					<< *i;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get providers"
					<< *i;
			}
		}
		return result;
	}

	QList<PluginManager::Plugins_t::iterator>
	PluginManager::FindProviders (const QSet<QByteArray>& expecteds)
	{
		QList<Plugins_t::iterator> result;
		for (Plugins_t::iterator i = Plugins_.begin ();
				i != Plugins_.end (); ++i)
		{
			IPlugin2 *ip2 = qobject_cast<IPlugin2*> (*i);
			try
			{
				if (ip2)
				{
					auto pcs = ip2->GetPluginClasses ();
					qDebug () << qobject_cast<IInfo*> (*i)->GetName () << pcs << expecteds;
					if (!pcs.intersect (expecteds).isEmpty ())
						result << i;
				}
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get class with"
					<< e.what ()
					<< "for"
					<< *i;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get class"
					<< *i;
			}
		}
		return result;
	}
}
