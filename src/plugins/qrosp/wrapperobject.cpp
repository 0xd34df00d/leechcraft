/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "wrapperobject.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QMenu>
#include <QtGlobal>
#ifndef QROSP_NO_QTSCRIPT
#include <QScriptEngine>
#endif
#include <QStandardItemModel>
#include <private/qmetaobjectbuilder_p.h>
#include <qross/core/script.h>
#include <qross/core/manager.h>
#include <qross/core/wrapperinterface.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/ihookproxy.h>
#include <util/sll/parsejson.h>
#include <util/sll/typelist.h>
#include <util/util.h>
#include "utilproxy.h"
#include "wrappers/coreproxywrapper.h"
#include "wrappers/hookproxywrapper.h"
#include "wrappers/entitywrapper.h"
#include "wrappers/pluginsmanagerwrapper.h"
#include "wrappers/shortcutproxywrapper.h"
#include "wrappers/tagsmanagerwrapper.h"

Q_DECLARE_METATYPE (QList<QAction*>);
Q_DECLARE_METATYPE (QList<QMenu*>);
Q_DECLARE_METATYPE (QUrl*);
Q_DECLARE_METATYPE (QString*);

#define SCALL(x) (Call<x> (ScriptAction_))

namespace LC
{
namespace Qrosp
{
	WrapperObject::WrapperObject (const QString& type,
			const QString& path)
	: Type_ (type)
	, Path_ (path)
	, ScriptAction_ (new Qross::Action (0, QUrl::fromLocalFile (path)))
	, ThisMetaObject_ (0)
	{
		qRegisterMetaType<QUrl*> ("QUrl*");
		qRegisterMetaType<QString*> ("QString*");
		qRegisterMetaType<QNetworkAccessManager*> ("QNetworkAccessManager*");
		qRegisterMetaType<QStandardItemModel*> ("QStandardItemModel*");

		BuildMetaObject ();
		ScriptAction_->addObject (this, "Signals");

		ScriptAction_->setInterpreter (type);
		ScriptAction_->setFile (path);
		ScriptAction_->trigger ();

		Interfaces_ = SCALL (QStringList) ("SupportedInterfaces");
		if (!Interfaces_.contains ("IInfo"))
			Interfaces_ << "IInfo";
		if (!Interfaces_.contains ("org.Deviant.LeechCraft.IInfo/1.0"))
			Interfaces_ << "org.Deviant.LeechCraft.IInfo/1.0";

		LoadScriptTranslations ();
		InitScript ();
	}

	void WrapperObject::LoadScriptTranslations ()
	{
		QFileInfo fileInfo = QFileInfo (Path_);
		QString path = fileInfo.absolutePath ();

		QString localeName = Util::GetLocaleName ();
		QString filename = fileInfo.completeBaseName () + "_" + localeName;

		Translator_.reset (new QTranslator);
		if (!Translator_->load (filename, path))
		{
			Translator_.reset ();
			return;
		}

		qApp->installTranslator (Translator_.get ());
	}

	void WrapperObject::InitScript ()
	{
		SCALL (void) ("SetScriptPath", { QFileInfo (Path_).absolutePath () });

		if (Type_ == "qtscript")
			InitQTS ();
	}

	void WrapperObject::InitQTS ()
	{
#ifndef QROSP_NO_QTSCRIPT
		QStringList reqs = SCALL (QStringList) ("Requires");
		QObject *scriptEngineObject = 0;
		QMetaObject::invokeMethod (ScriptAction_->script (),
				"engine", Q_RETURN_ARG (QObject*, scriptEngineObject));
		QScriptEngine *engine = qobject_cast<QScriptEngine*> (scriptEngineObject);
		if (!engine)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to obtain script engine from the"
					<< "script action though we are Qt Script";
			return;
		}

		for (const auto& req : reqs)
			engine->importExtension (req);
#endif
	}

	namespace
	{
		QVariantMap ParseManifest (const QString& path)
		{
			QFile file (path);
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open file"
						<< path;
				return QVariantMap ();
			}

			return Util::ParseJson (&file, Q_FUNC_INFO).toMap ();
		}
	}

	void WrapperObject::BuildMetaObject ()
	{
		QMetaObjectBuilder builder;
		builder.setSuperClass (QObject::metaObject ());
		builder.setClassName (QString ("LC::Qross::%1::%2")
					.arg (Type_)
					.arg (SCALL (QString) ("GetUniqueID").remove ('.')).toLatin1 ());

		int currentMetaMethod = 0;

		const auto& manifest = ParseManifest (Path_ + ".manifest.json");

		auto prepareSig = [] (auto&& sig) { return std::forward<decltype (sig)> (sig).trimmed ().toLatin1 (); };

		for (auto signature : manifest ["ExportedSlots"].toStringList ())
			if (auto sigArray = prepareSig (signature); !sigArray.isEmpty ())
			{
				Index2ExportedSignatures_ [currentMetaMethod++] = sigArray;
				builder.addSlot (sigArray);
			}

		for (auto signature : manifest ["ExportedSignals"].toStringList ())
			if (auto sigArray = prepareSig (signature); !sigArray.isEmpty ())
			{
				Index2ExportedSignatures_ [currentMetaMethod++] = sigArray;
				builder.addSignal (sigArray);
			}

		ThisMetaObject_ = builder.toMetaObject ();

		for (int i = ThisMetaObject_->methodOffset (),
				offset = ThisMetaObject_->methodOffset (),
				count = ThisMetaObject_->methodCount (); i < count; ++i)
			Index2MetaMethod_ [i - offset] = ThisMetaObject_->method (i);
	}

	WrapperObject::~WrapperObject ()
	{
		delete ScriptAction_;
		qFreeAligned (ThisMetaObject_);
	}

	const QString& WrapperObject::GetType () const
	{
		return Type_;
	}

	const QString& WrapperObject::GetPath () const
	{
		return Path_;
	}

	void* WrapperObject::qt_metacast (const char *interfaceName)
	{
		if (!Interfaces_.contains (interfaceName))
			return QObject::qt_metacast (interfaceName);

		using tl = Util::Typelist<IInfo*, IEntityHandler*, IPlugin2*, IJobHolder*, IActionsExporter*, IHaveTabs*>;

		auto matcher = [interfaceName] (auto iface) { return !strcmp (interfaceName, qobject_interface_iid<decltype (iface)> ()); };
		return Util::FirstMatching (matcher,
				[this] (auto iface) -> void* { return static_cast<decltype (iface)> (this); },
				[interfaceName, this] { return QObject::qt_metacast (interfaceName); },
				tl {});
	}

	const QMetaObject* WrapperObject::metaObject () const
	{
		return ThisMetaObject_ ?
				ThisMetaObject_ :
				QObject::d_ptr->dynamicMetaObject ();
	}

	namespace
	{
		QVariant WrapParameter (QByteArray type, void *elem)
		{
			type = QMetaObject::normalizedType (type);
			if (type == "LC::IHookProxy_ptr")
				return QVariant::fromValue<QObject*> (new HookProxyWrapper (*static_cast<IHookProxy_ptr*> (elem)));
			else if (type == "LC::Entity")
				return QVariant::fromValue<QObject*> (new EntityWrapper (*static_cast<Entity*> (elem)));
			else
			{
				type.replace ("const ", "");
				int id = QMetaType::type (type);
				if (!id)
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown type"
							<< type;
					return QVariant ();
				}
				else
					return QVariant (QMetaType::type (type),
							elem);
			}
		}
	}

	int WrapperObject::qt_metacall (QMetaObject::Call call,
			int id, void **argsArray)
	{
		id = QObject::qt_metacall (call, id, argsArray);
		if (id < 0)
			return id;
		if (call == QMetaObject::InvokeMetaMethod)
		{
			QMetaMethod method = Index2MetaMethod_ [id];
			if (method.methodType () == QMetaMethod::Signal)
				QMetaObject::activate (this, metaObject (), id, argsArray);
			else
			{
				QVariantList args;
				for (int i = 0, size = method.parameterTypes ().size ();
						i < size; ++i)
					args << WrapParameter (method.parameterTypes ().at (i),
							argsArray [i + 1]);
				QString name (method.methodSignature ());
				name = name.left (name.indexOf ('('));
				SCALL (void) (name, args);
			}
		}
		return id;
	}

	void WrapperObject::Init (ICoreProxy_ptr proxy)
	{
		QVariantList args;
		args << QVariant::fromValue<QObject*> (new CoreProxyWrapper (proxy));
		SCALL (void) ("Init", args);
	}

	void WrapperObject::SecondInit ()
	{
		SCALL (void) ("SecondInit");
	}

	void WrapperObject::Release ()
	{
		SCALL (void) ("Release");
		ScriptAction_->finalize ();
	}

	QByteArray WrapperObject::GetUniqueID () const
	{
		return SCALL (QByteArray) ("GetUniqueID");
	}

	QString WrapperObject::GetName () const
	{
		return SCALL (QString) ("GetName");
	}

	QString WrapperObject::GetInfo () const
	{
		return SCALL (QString) ("GetInfo");
	}

	QIcon WrapperObject::GetIcon () const
	{
		return SCALL (QIcon) ("GetIcon");
	}

	QStringList WrapperObject::Provides () const
	{
		return SCALL (QStringList) ("Provides");
	}

	QStringList WrapperObject::Needs () const
	{
		return SCALL (QStringList) ("Needs") << "qrosp";
	}

	QStringList WrapperObject::Uses () const
	{
		return SCALL (QStringList) ("Uses");
	}

	void WrapperObject::SetProvider (QObject *provider, const QString& feature)
	{
		QVariantList args;
		args << QVariant::fromValue<QObject*> (provider);
		args << feature;
		SCALL (void) ("SetProvider", args);
	}

	EntityTestHandleResult WrapperObject::CouldHandle (const Entity& e) const
	{
		QVariantList args;
		args << QVariant::fromValue<QObject*> (new EntityWrapper (e));
		const QVariantMap& map = SCALL (QVariantMap) ("CouldHandle", args);
		EntityTestHandleResult r;
		r.HandlePriority_ = map ["Priority"].toInt ();
		return r;
	}

	void WrapperObject::Handle (Entity e)
	{
		QVariantList args;
		args << QVariant::fromValue<QObject*> (new EntityWrapper (e));
		SCALL (void) ("Handle", args);
	}

	QAbstractItemModel* WrapperObject::GetRepresentation () const
	{
		return SCALL (QAbstractItemModel*) ("GetRepresentation");
	}

	QList<QAction*> WrapperObject::GetActions (ActionsEmbedPlace place) const
	{
		QVariantList args;
		switch (place)
		{
		case ActionsEmbedPlace::CommonContextMenu:
			args << "AEPCommonContextMenu";
			break;
		case ActionsEmbedPlace::QuickLaunch:
			args << "AEPQuickLaunch";
			break;
		case ActionsEmbedPlace::ToolsMenu:
			args << "AEPToolsMenu";
			break;
		case ActionsEmbedPlace::TrayMenu:
			args << "AEPTrayMenu";
			break;
		case ActionsEmbedPlace::LCTray:
			args << "AEPLCTray";
			break;
		}
		return SCALL (QList<QAction*>) ("GetActions", args);
	}

	QSet<QByteArray> WrapperObject::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		for (const auto& pclass : SCALL (QStringList) ("GetPluginClasses"))
			result << pclass.toUtf8 ();
		return result;
	}

	TabClasses_t WrapperObject::GetTabClasses () const
	{
		TabClasses_t result;
		for (const auto& mapVar : SCALL (QVariantList) ("GetPluginClasses"))
		{
			const auto& map = mapVar.toMap ();
			TabClassInfo info
			{
				map ["TabClass_"].toByteArray (),
				map ["VisibleName_"].toString (),
				map ["Description_"].toString (),
				map ["Icon_"].value<QIcon> (),
				map ["Priority_"].value<quint16> (),
				TabFeature::TFEmpty
			};
			const auto& tabFeatures = map ["Features_"].toStringList ();
			auto checkFeature = [&tabFeatures, &info] (const QString& str, TabFeature feature)
			{
				if (tabFeatures.contains (str))
					info.Features_ |= feature;
			};
			checkFeature ("OpenableByRequest", TabFeature::TFOpenableByRequest);
			checkFeature ("Singe", TabFeature::TFSingle);
			checkFeature ("ByDefault", TabFeature::TFByDefault);
			checkFeature ("SuggestOpening", TabFeature::TFSuggestOpening);
		}
		return result;
	}

	void WrapperObject::TabOpenRequested (const QByteArray& tabClass)
	{
	}

	void WrapperObject::gotActions (QList<QAction*>, ActionsEmbedPlace)
	{
		qWarning () << Q_FUNC_INFO
				<< "is called, but this should never happen";
	}

	void WrapperObject::Call<void>::operator() (const QString& name,
			const QVariantList& args) const
	{
		if (ScriptAction_->functionNames ().contains (name))
			ScriptAction_->callFunction (name, args);
	}

	QVariant WrapperObject::Call<QVariant>::operator() (const QString& name,
			const QVariantList& args) const
	{
		if (!ScriptAction_->functionNames ().contains (name))
			return QVariant ();
		return ScriptAction_->callFunction (name, args);
	}
}
}
