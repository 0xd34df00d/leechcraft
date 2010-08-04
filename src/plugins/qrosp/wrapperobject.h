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

#ifndef PLUGINS_QROSP_WRAPPERS_WRAPPEROBJECT_H
#define PLUGINS_QROSP_WRAPPERS_WRAPPEROBJECT_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <qross/core/action.h>
#include <interfaces/iinfo.h>
#include <interfaces/ijobholder.h>
#include <interfaces/imenuembedder.h>
#include <interfaces/iplugin2.h>
#include <interfaces/itoolbarembedder.h>
#include <interfaces/itraymenu.h>
#include <interfaces/iembedtab.h>
#include <interfaces/ientityhandler.h>

class QTranslator;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			class WrapperObject : public QObject
								, public IInfo
								, public IEmbedTab
								, public IEntityHandler
								, public IJobHolder
								, public IMenuEmbedder
								, public IPlugin2
								, public IToolBarEmbedder
								, public ITrayMenu
			{
				QString Type_;
				QString Path_;
				mutable Qross::Action *ScriptAction_;

				QList<QString> Interfaces_;

				QMetaObject *ThisMetaObject_;
				QMap<int, QMetaMethod> Index2MetaMethod_;
				QMap<int, QByteArray> Index2ExportedSignatures_;

				boost::shared_ptr<QTranslator> Translator_;
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
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				// IEmbedTab
				QWidget* GetTabContents ();
				QToolBar* GetToolBar () const;

				// IEntityHandler
				bool CouldHandle (const LeechCraft::Entity&) const;
				void Handle (LeechCraft::Entity);

				// IJobHolder
				QAbstractItemModel* GetRepresentation () const;

				// IMenuEmbedder
				QList<QMenu*> GetToolMenus () const;
				QList<QAction*> GetToolActions () const;

				// IToolBarEmbedder
				QList<QAction*> GetActions () const;

				// ITrayMenu
				QList<QAction*> GetTrayActions () const;
				QList<QMenu*> GetTrayMenus () const;

				// IPlugin2
				QSet<QByteArray> GetPluginClasses () const;
			private:
				template<typename T>
				struct Call
				{
					Qross::Action *ScriptAction_;

					Call (Qross::Action *sa) : ScriptAction_ (sa) {}

					T operator() (const QString& name,
							const QVariantList& args = QVariantList ()) const
					{
						if (!ScriptAction_->functionNames ().contains (name))
							return T ();

						QVariant result = ScriptAction_->callFunction (name, args);
						if (result.canConvert<T> ())
							return result.value<T> ();

						qWarning () << Q_FUNC_INFO
								<< "unable to unwrap result"
								<< result;
						return T ();
					}
				};

				void LoadScriptTranslations ();
				void InitScript ();
				void BuildMetaObject ();
			};

			template<typename T>
			struct WrapperObject::Call<T*>
			{
				Qross::Action *ScriptAction_;

				Call (Qross::Action *sa) : ScriptAction_ (sa) {}

				T* operator() (const QString& name,
						const QVariantList& args = QVariantList ()) const
				{
					if (!ScriptAction_->functionNames ().contains (name))
						return 0;

					QVariant result = ScriptAction_->callFunction (name, args);
					if (result.canConvert<T*> ())
						return result.value<T*> ();
					else if (result.canConvert<QObject*> ())
					{
						QObject *object = result.value<QObject*> ();
						return qobject_cast<T*> (object);
					}

					qDebug () << Q_FUNC_INFO
							<< "unable to unwrap result"
							<< result;
					return 0;
				}
			};

			template<>
			struct WrapperObject::Call<void>
			{
				Qross::Action *ScriptAction_;

				Call (Qross::Action *sa) : ScriptAction_ (sa) {}

				void operator() (const QString&,
						const QVariantList& = QVariantList ()) const;
			};

			template<>
			struct WrapperObject::Call<QVariant>
			{
				Qross::Action *ScriptAction_;

				Call (Qross::Action *sa) : ScriptAction_ (sa) {}

				QVariant operator() (const QString& name,
						const QVariantList& args) const;
			};
		};
	};
};

#endif
