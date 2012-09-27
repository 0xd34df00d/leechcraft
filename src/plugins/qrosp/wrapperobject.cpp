/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "wrapperobject.h"

#include <QCoreApplication>
#include <QTranslator>
#ifndef QROSP_NO_QTSCRIPT
#include <QScriptEngine>
#endif
#include <QStandardItemModel>

#include <qross/core/script.h>
#include <qross/core/manager.h>
#include <qross/core/wrapperinterface.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/ihookproxy.h>
#include <util/util.h>
#include "utilproxy.h"
#include "wrappers/coreproxywrapper.h"
#include "wrappers/hookproxywrapper.h"
#include "wrappers/entitywrapper.h"
#include "wrappers/pluginsmanagerwrapper.h"
#include "wrappers/shortcutproxywrapper.h"
#include "wrappers/tagsmanagerwrapper.h"

#if QT_VERSION < 0x040800
#include "third-party/qmetaobjectbuilder.h"
#else
#include "third-party/qmetaobjectbuilder_48.h"
#endif

class QWebView;
class QWebPage;

Q_DECLARE_METATYPE (QList<QAction*>);
Q_DECLARE_METATYPE (QList<QMenu*>);
Q_DECLARE_METATYPE (QUrl*);
Q_DECLARE_METATYPE (QString*);
Q_DECLARE_METATYPE (QWebView*);
Q_DECLARE_METATYPE (QWebPage*);

#define SCALL(x) (Call<x> (ScriptAction_))

namespace LeechCraft
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
		qRegisterMetaType<QWebView*> ("QWebView*");
		qRegisterMetaType<QWebPage*> ("QWebPage*");
		qRegisterMetaType<QNetworkAccessManager*> ("QNetworkAccessManager*");
		qRegisterMetaType<QStandardItemModel*> ("QStandardItemModel*");
		ScriptAction_->addObject (this, "Signals");
		BuildMetaObject ();

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

#ifndef QROSP_NO_QTSCRIPT
		if (Type_ == "qtscript")
		{
			QStringList requires = SCALL (QStringList) ("Requires");
			QObject *scriptEngineObject = 0;
			QMetaObject::invokeMethod (ScriptAction_->script (),
					"engine", Q_RETURN_ARG (QObject*, scriptEngineObject));
			QScriptEngine *engine = qobject_cast<QScriptEngine*> (scriptEngineObject);
			if (!engine)
				qWarning () << Q_FUNC_INFO
						<< "unable to obtain script engine from the"
						<< "script action though we are Qt Script";
			else
				Q_FOREACH (QString req, requires)
					engine->importExtension (req);
		}
#endif
	}

	void WrapperObject::BuildMetaObject ()
	{
		QString path = QFileInfo (Path_).absolutePath ();
		QDir scriptDir (path);

		QMetaObjectBuilder builder;
		builder.setSuperClass (QObject::metaObject ());
		builder.setClassName (QString ("LeechCraft::Qross::%1::%2")
					.arg (Type_)
					.arg (SCALL (QString) ("GetUniqueID").remove ('.')).toLatin1 ());

		ThisMetaObject_ = builder.toMetaObject ();

		int currentMetaMethod = 0;

		Q_FOREACH (auto signature, SCALL (QStringList) ("ExportedSlots"))
		{
			signature = signature.trimmed ();
			if (signature.isEmpty ())
				continue;
			const auto& sigArray = signature.toLatin1 ();
			Index2ExportedSignatures_ [currentMetaMethod++] = sigArray;
			builder.addSlot (sigArray);
		}

		Q_FOREACH (auto signature, SCALL (QStringList) ("ExportedSlots"))
		{
			signature = signature.trimmed ();
			if (signature.isEmpty ())
				continue;
			const auto& sigArray = signature.toLatin1 ();
			Index2ExportedSignatures_ [currentMetaMethod++] = sigArray;
			builder.addSignal (sigArray);
		}

		qFree (ThisMetaObject_);
		ThisMetaObject_ = builder.toMetaObject ();

		for (int i = ThisMetaObject_->methodOffset (),
				offset = ThisMetaObject_->methodOffset (),
				count = ThisMetaObject_->methodCount (); i < count; ++i)
			Index2MetaMethod_ [i - offset] = ThisMetaObject_->method (i);
	}

	WrapperObject::~WrapperObject ()
	{
		delete ScriptAction_;
		qFree (ThisMetaObject_);
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
		if (!strcmp (interfaceName, "IInfo") ||
				!strcmp (interfaceName, "org.Deviant.LeechCraft.IInfo/1.0"))
			return static_cast<IInfo*> (this);
		if (!strcmp (interfaceName, "IEntityHandler") ||
				!strcmp (interfaceName, "org.Deviant.LeechCraft.IEntityHandler/1.0"))
			return static_cast<IEntityHandler*> (this);
		if (!strcmp (interfaceName, "IPlugin2") ||
				!strcmp (interfaceName, "org.Deviant.LeechCraft.IPlugin2/1.0"))
			return static_cast<IPlugin2*> (this);
		if (!strcmp (interfaceName, "IJobHolder") ||
				!strcmp (interfaceName, "org.Deviant.LeechCraft.IJobHolder/1.0"))
			return static_cast<IJobHolder*> (this);
		if (!strcmp (interfaceName, "IActionsExporter") ||
				!strcmp (interfaceName, "org.Deviant.LeechCraft.IActionsExporter/1.0"))
			return static_cast<IActionsExporter*> (this);
		if (!strcmp (interfaceName, "IHaveTabs") ||
				!strcmp (interfaceName, "org.Deviant.LeechCraft.IHaveTabs/1.0"))
			return static_cast<IHaveTabs*> (this);

		return QObject::qt_metacast (interfaceName);
	}

	const QMetaObject* WrapperObject::metaObject () const
	{
		return ThisMetaObject_ ? ThisMetaObject_ : QObject::d_ptr->metaObject;
	}

	namespace
	{
		QVariant WrapParameter (QByteArray type, void *elem)
		{
			type = QMetaObject::normalizedType (type);
			if (type == "LeechCraft::IHookProxy_ptr")
				return QVariant::fromValue<QObject*> (new HookProxyWrapper (*static_cast<IHookProxy_ptr*> (elem)));
			else if (type == "LeechCraft::Entity")
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
				QString name (method.signature ());
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
		default:
			qWarning () << Q_FUNC_INFO
					<< "unhandled place"
					<< static_cast<int> (place);
			break;
		}
		return SCALL (QList<QAction*>) ("GetActions", args);
	}

	QSet<QByteArray> WrapperObject::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		Q_FOREACH (QString pclass,
				SCALL (QStringList) ("GetPluginClasses"))
			result << pclass.toUtf8 ();
		return result;
	}

	TabClasses_t WrapperObject::GetTabClasses () const
	{
		TabClasses_t result;
		Q_FOREACH (const QVariant& mapVar, SCALL (QVariantList) ("GetPluginClasses"))
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

	void WrapperObject::addNewTab (const QString&, QWidget*)
	{
		qWarning () << Q_FUNC_INFO
				<< "is called, but this should never happen";
	}

	void WrapperObject::removeTab (QWidget*)
	{
		qWarning () << Q_FUNC_INFO
				<< "is called, but this should never happen";
	}

	void WrapperObject::changeTabName (QWidget*, const QString&)
	{
		qWarning () << Q_FUNC_INFO
				<< "is called, but this should never happen";
	}

	void WrapperObject::changeTabIcon (QWidget*, const QIcon&)
	{
		qWarning () << Q_FUNC_INFO
				<< "is called, but this should never happen";
	}

	void WrapperObject::statusBarChanged (QWidget*, const QString&)
	{
		qWarning () << Q_FUNC_INFO
				<< "is called, but this should never happen";
	}

	void WrapperObject::raiseTab (QWidget*)
	{
		qWarning () << Q_FUNC_INFO
				<< "is called, but this should never happen";
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
