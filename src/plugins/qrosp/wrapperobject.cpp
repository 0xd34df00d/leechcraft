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

#ifndef QROSP_NO_QTSCRIPT
#include <QScriptEngine>
#endif

#include <qross/core/script.h>
#include <qross/core/manager.h>
#include <qross/core/wrapperinterface.h>
#include "wrappers/coreproxywrapper.h"
#include "wrappers/hookproxywrapper.h"
#include "wrappers/entitywrapper.h"
#include "wrappers/pluginsmanagerwrapper.h"
#include "wrappers/shortcutproxywrapper.h"
#include "wrappers/tagsmanagerwrapper.h"
#include "third-party/qmetaobjectbuilder.h"

Q_DECLARE_METATYPE (QList<QAction*>);
Q_DECLARE_METATYPE (QList<QMenu*>);

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
			, ThisMetaObject_ (0)
			, ScriptAction_ (new Qross::Action (0, QUrl::fromLocalFile (path)))
			{
				BuildMetaObject ();

				ScriptAction_->addObject (this, "Signals");

				ScriptAction_->setInterpreter (type);
				ScriptAction_->setFile (path);
				ScriptAction_->trigger ();

				Interfaces_ = Call<QStringList> ("SupportedInterfaces");
				if (!Interfaces_.contains ("IInfo"))
					Interfaces_ << "IInfo";
				if (!Interfaces_.contains ("org.Deviant.LeechCraft.IInfo/1.0"))
					Interfaces_ << "org.Deviant.LeechCraft.IInfo/1.0";

				InitScript ();
			}

			void WrapperObject::InitScript ()
			{
				QVariantList pathArgs;
				pathArgs << QFileInfo (Path_).absolutePath ();
				Call<void> ("SetScriptPath", pathArgs);

#ifndef QROSP_NO_QTSCRIPT
				if (Type_ == "qtscript")
				{
					QStringList requires = Call<QStringList> ("Requires");
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
							.arg (Call<QString> ("GetName").remove (' ')).toLatin1 ());

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
						SignalsIDs_ << currentMetaMethod;
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
				if (!strcmp (interfaceName, "IPlugin2") ||
						!strcmp (interfaceName, "org.Deviant.LeechCraft.IPlugin2/1.0"))
					return static_cast<IPlugin2*> (this);
				if (!strcmp (interfaceName, "IJobHolder") ||
						!strcmp (interfaceName, "org.Deviant.LeechCraft.IJobHolder/1.0"))
					return static_cast<IJobHolder*> (this);
				if (!strcmp (interfaceName, "IMenuEmbedder") ||
						!strcmp (interfaceName, "org.Deviant.LeechCraft.IMenuEmbedder/1.0"))
					return static_cast<IMenuEmbedder*> (this);
			}

			const QMetaObject* WrapperObject::metaObject () const
			{
				return ThisMetaObject_ ? ThisMetaObject_ : QObject::d_ptr->metaObject;
			}

			namespace
			{
				QVariant MakeFromParameter (const QByteArray& type, void *elem)
				{
					qDebug () << type << elem << QMetaType::type (type);
					if (type == "LeechCraft::IHookProxy_ptr")
						return QVariant::fromValue<QObject*> (new HookProxyWrapper (*static_cast<IHookProxy_ptr*> (elem)));
					else if (type == "LeechCraft::Entity")
						return QVariant::fromValue<QObject*> (new EntityWrapper (*static_cast<Entity*> (elem)));
					else
						return QVariant (QMetaType::type (type),
								elem);
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
							args << MakeFromParameter (method.parameterTypes ().at (i),
									argsArray [i + 1]);
						QString name (method.signature ());
						name = name.left (name.indexOf ('('));
						Call<void> (name, args);
					}
				}
				return id;
			}

			void WrapperObject::Init (ICoreProxy_ptr proxy)
			{
				QVariantList args;
				args << QVariant::fromValue<QObject*> (new CoreProxyWrapper (proxy));
				Call<void> ("Init", args);
			}

			void WrapperObject::SecondInit ()
			{
				Call<void> ("SecondInit");
			}

			void WrapperObject::Release ()
			{
				Call<void> ("Release");
			}

			QString WrapperObject::GetName () const
			{
				return Call<QString> ("GetName");
			}

			QString WrapperObject::GetInfo () const
			{
				return Call<QString> ("GetInfo");
			}

			QIcon WrapperObject::GetIcon () const
			{
				return Call<QIcon> ("GetIcon");
			}

			QStringList WrapperObject::Provides () const
			{
				return Call<QStringList> ("Provides");
			}

			QStringList WrapperObject::Needs () const
			{
				return Call<QStringList> ("Needs");
			}

			QStringList WrapperObject::Uses () const
			{
				return Call<QStringList> ("Uses");
			}

			void WrapperObject::SetProvider (QObject *provider, const QString& feature)
			{
				QVariantList args;
				args << QVariant::fromValue<QObject*> (provider);
				args << feature;
				Call<void> ("SetProvider", args);
			}

			QAbstractItemModel* WrapperObject::GetRepresentation () const
			{
				return Call<QAbstractItemModel*> ("GetRepresentation");
			}

			QList<QMenu*> WrapperObject::GetToolMenus () const
			{
				return Call<QList<QMenu*> > ("GetToolMenus");
			}

			QList<QAction*> WrapperObject::GetToolActions () const
			{
				return Call<QList<QAction*> > ("GetToolActions");
			}

			QList<QAction*> WrapperObject::GetActions () const
			{
				return Call<QList<QAction*> > ("GetActions");
			}

			QList<QAction*> WrapperObject::GetTrayActions () const
			{
				return Call<QList<QAction*> > ("GetTrayActions");
			}

			QList<QMenu*> WrapperObject::GetTrayMenus () const
			{
				return Call<QList<QMenu*> > ("GetTrayMenus");
			}

			QSet<QByteArray> WrapperObject::GetPluginClasses () const
			{
				QSet<QByteArray> result;
				Q_FOREACH (QString pclass,
						Call<QStringList> ("GetPluginClasses"))
					result << pclass.toUtf8 ();
				return result;
			}

			template<>
			void WrapperObject::Call<void> (const QString& name,
					const QVariantList& args) const
			{
				if (ScriptAction_->functionNames ().contains (name))
					ScriptAction_->callFunction (name, args);
			}
		};
	};
};
