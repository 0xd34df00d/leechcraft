/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_WRAPPEROBJECT_H
#define PLUGINS_QROSP_WRAPPERS_WRAPPEROBJECT_H
#include <memory>
#include <QObject>
#include <qross/core/action.h>
#include <interfaces/iinfo.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavetabs.h>

class QTranslator;

namespace LC
{
namespace Qrosp
{
	class WrapperObject : public QObject
						, public IInfo
						, public IEntityHandler
						, public IJobHolder
						, public IPlugin2
						, public IActionsExporter
						, public IHaveTabs
	{
		QString Type_;
		QString Path_;
		mutable Qross::Action *ScriptAction_;

		QList<QString> Interfaces_;

		QMetaObject *ThisMetaObject_;
		QMap<int, QMetaMethod> Index2MetaMethod_;
		QMap<int, QByteArray> Index2ExportedSignatures_;

		std::shared_ptr<QTranslator> Translator_;

		void SetProxy (ICoreProxy_ptr) {}
		void SetPluginInstance (QObject*) {}
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

		// IEntityHandler
		EntityTestHandleResult CouldHandle (const LC::Entity&) const;
		void Handle (LC::Entity);

		// IJobHolder
		QAbstractItemModel* GetRepresentation () const;

		// IToolBarEmbedder
		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		// IPlugin2
		QSet<QByteArray> GetPluginClasses () const;

		// IHaveTabs
		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		// Signals hacks
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
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
		void InitQTS ();
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
}
}

#endif
