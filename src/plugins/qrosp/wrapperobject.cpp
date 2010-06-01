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
#include "coreproxywrapper.h"

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
			, ScriptAction_ (new Qross::Action (this, QUrl::fromLocalFile (path)))
			{
				ScriptAction_->setInterpreter (type);
				ScriptAction_->setFile (path);
				ScriptAction_->trigger ();

				Interfaces_ = Call<QStringList> ("SupportedInterfaces");
				qDebug () << Q_FUNC_INFO
						<< path
						<< Interfaces_;
				if (!Interfaces_.contains ("IInfo"))
					Interfaces_ << "IInfo";
				if (!Interfaces_.contains ("org.Deviant.LeechCraft.IInfo/1.0"))
					Interfaces_ << "org.Deviant.LeechCraft.IInfo/1.0";

				QVariantList pathArgs;
				pathArgs << QFileInfo (path).absolutePath ();
				Call<void> ("SetScriptPath", pathArgs);

#ifndef QROSP_NO_QTSCRIPT
				if (type == "qtscript")
				{
					QStringList requires = Call<QStringList> ("Requires");
					qDebug () << "JS plugin asked for:"
							<< requires;
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

			WrapperObject::~WrapperObject ()
			{
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
