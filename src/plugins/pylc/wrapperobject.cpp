/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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
#include <QIcon>
#include "coreproxywrapper.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			void* WrapperObject::qt_metacast (const char *_clname)
			{
				if (!_clname) return 0;
				if (!strcmp(_clname, "IInfo"))
					return static_cast< IInfo*>(const_cast< WrapperObject*>(this));
				if (!strcmp(_clname, "org.Deviant.LeechCraft.IInfo/1.0"))
					return static_cast< IInfo*>(const_cast< WrapperObject*>(this));
				return QObject::qt_metacast(_clname);
			}

			WrapperObject::WrapperObject (const QString& filename,
					QObject *parent)
			: QObject (parent)
			, Filename_ (filename)
			{
				Module_ = PythonQt::self ()->createModuleFromFile (filename, filename);
			}

			void WrapperObject::Init (ICoreProxy_ptr proxy)
			{
				PythonQt::self ()->registerClass (&CoreProxyWrapper::staticMetaObject);
				QVariantList args;
				args << QVariant::fromValue<QObject*> (new CoreProxyWrapper (proxy));
				try
				{
					Call ("Init", args);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
				}
			}

			void WrapperObject::SecondInit ()
			{
				try
				{
					Call ("SecondInit");
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
				}
			}

			void WrapperObject::Release ()
			{
				try
				{
					Call ("Release");
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
				}
			}

			QString WrapperObject::GetName () const
			{
				try
				{
					return Call ("GetName").toString ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return QString ();
				}
			}

			QString WrapperObject::GetInfo () const
			{
				try
				{
					return Call ("GetInfo").toString ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return QString ();
				}
			}

			QIcon WrapperObject::GetIcon () const
			{
				return QIcon ();
			}

			QStringList WrapperObject::Provides () const
			{
				try
				{
					return Call ("Provides").toStringList ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return QStringList ();
				}
			}

			QStringList WrapperObject::Needs () const
			{
				try
				{
					return Call ("Needs").toStringList ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return QStringList ();
				}
			}

			QStringList WrapperObject::Uses () const
			{
				try
				{
					return Call ("Uses").toStringList ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return QStringList ();
				}
			}

			void WrapperObject::SetProvider (QObject*, const QString&)
			{
			}

			QVariant WrapperObject::Call (const QString& name, const QVariantList& args) const
			{
				if (Module_.isNull ())
				{
					qWarning () << Q_FUNC_INFO
						<< "Module_ is null"
						<< name
						<< args;
					throw std::runtime_error ("module is null");
				}

				PythonQtObjectPtr instance =
					PythonQt::self ()->lookupObject (Module_, "instance");
				QVariant result = PythonQt::self ()->call (instance, name, args);
				if (PythonQt::self ()->handleError ())
				{
					qWarning () << Q_FUNC_INFO
						<< "Py error occured"
						<< name
						<< args;
					throw std::runtime_error ("Py error occured");
				}
				else
					return result;
			}

			bool WrapperObject::Implements (const char *cstr)
			{
				QStringList implemented = Call ("Implements", QVariantList ()).toStringList (); 
				return implemented.count (QString (cstr));
			}
		};
	};
};

