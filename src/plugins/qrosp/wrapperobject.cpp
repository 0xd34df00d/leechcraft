/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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
#include <plugininterface/util.h>
#include "utilproxy.h"
#include "wrappers/coreproxywrapper.h"
#include "wrappers/hookproxywrapper.h"
#include "wrappers/entitywrapper.h"
#include "wrappers/pluginsmanagerwrapper.h"
#include "wrappers/shortcutproxywrapper.h"
#include "wrappers/tagsmanagerwrapper.h"
#include "third-party/qmetaobjectbuilder.h"

class QWebView;
class QWebPage;

Q_DECLARE_METATYPE (QList<QAction*>);
Q_DECLARE_METATYPE (QList<QMenu*>);
Q_DECLARE_METATYPE (QUrl*);
Q_DECLARE_METATYPE (QString*);
Q_DECLARE_METATYPE (QWebView*);
Q_DECLARE_METATYPE (QWebPage*);

#define SCALL(x) (Call<x > (ScriptAction_))

namespace LeechCraft
{
	namespace Plugins
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
				QVariantList pathArgs;
				pathArgs << QFileInfo (Path_).absolutePath ();
				SCALL (void) ("SetScriptPath", pathArgs);

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
				builder.setClassName (QString ("LeechCraft::Plugins::Qross::%1::%2")
							.arg (Type_)
							.arg (SCALL (QString) ("GetName").remove (' ')).toLatin1 ());

				int currentMetaMethod = 0;

				if (scriptDir.exists ("ExportedSlots"))
				{
					QFile slotsFile (scriptDir.filePath ("ExportedSlots"));
					slotsFile.open (QIODevice::ReadOnly);
					QList<QByteArray> sigSlots = slotsFile.readAll ().split ('\n');
					Q_FOREACH (QByteArray signature, sigSlots)
					{
						signature = signature.trimmed ();
						if (signature.isEmpty ())
							continue;
						Index2ExportedSignatures_ [currentMetaMethod++] = signature;
						builder.addSlot (signature);
					}
				}

				if (scriptDir.exists ("ExportedSignals"))
				{
					QFile sigsFile (scriptDir.filePath ("ExportedSignals"));
					sigsFile.open (QIODevice::ReadOnly);
					QList<QByteArray> sigSignals = sigsFile.readAll ().split ('\n');
					Q_FOREACH (QByteArray signature, sigSignals)
					{
						signature = signature.trimmed ();
						if (signature.isEmpty ())
							continue;
						Index2ExportedSignatures_ [currentMetaMethod++] = signature;
						builder.addSignal (signature);
					}
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
				if (!strcmp (interfaceName, "IEmbedTab") ||
						!strcmp (interfaceName, "org.Deviant.LeechCraft.IEmbedTab/1.0"))
					return static_cast<IEmbedTab*> (this);

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
				return SCALL (QStringList) ("Needs");
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

			QWidget* WrapperObject::GetTabContents ()
			{
				return SCALL (QWidget*) ("GetTabContents");
			}

			QToolBar* WrapperObject::GetToolBar () const
			{
				return SCALL (QToolBar*) ("GetToolBar");
			}

			bool WrapperObject::CouldHandle (const Entity& e) const
			{
				QVariantList args;
				args << QVariant::fromValue<QObject*> (new EntityWrapper (e));
				return SCALL (bool) ("CouldHandle", args);
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
				case AEPCommonContextMenu:
					args << "AEPCommonContextMenu";
					break;
				case AEPQuickLaunch:
					args << "AEPQuickLaunch";
					break;
				case AEPToolsMenu:
					args << "AEPToolsMenu";
					break;
				case AEPTrayMenu:
					args << "AEPTrayMenu";
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
		};
	};
};
