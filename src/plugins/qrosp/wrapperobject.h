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

#ifndef PLUGINS_QROSP_WRAPPEROBJECT_H
#define PLUGINS_QROSP_WRAPPEROBJECT_H
#include <QObject>
#include <qross/core/action.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			class WrapperObject : public QObject
								, public IInfo
								, public IPlugin2
			{
				QString Type_;
				QString Path_;
				mutable Qross::Action *ScriptAction_;

				QList<QString> Interfaces_;

				QMetaObject *ThisMetaObject_;
				QMap<int, QMetaMethod> Index2MetaMethod_;
				QMap<int, QString> Index2ExportedSignatures_;
			public:
				WrapperObject (const QString&, const QString&);
				virtual ~WrapperObject ();

				const QString& GetType () const;
				const QString& GetPath () const;

				// MOC hacks
				virtual void* qt_metacast (const char*);
				virtual const QMetaObject* metaObject () const;
				virtual int qt_metacall (QMetaObject::Call, int, void**);

				// IInfo
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				// IPlugin2
				QSet<QByteArray> GetPluginClasses () const;
			private:
				template<typename T>
				T Call (const QString& name,
						const QVariantList& args = QVariantList ()) const
				{
					if (!ScriptAction_->functionNames ().contains (name))
						return T ();
					return ScriptAction_->callFunction (name, args).value<T> ();
				}

				void InitScript ();
				void BuildMetaObject ();
			};

			template<>
			void WrapperObject::Call<void> (const QString& name,
					const QVariantList& args) const;
		};
	};
};

#endif
