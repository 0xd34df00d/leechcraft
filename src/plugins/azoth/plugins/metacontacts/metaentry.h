/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QPair>
#include <QStringList>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iadvancedclentry.h>

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	class MetaAccount;

	class MetaEntry : public QObject
					, public ICLEntry
					, public IAdvancedCLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry
				LC::Azoth::IAdvancedCLEntry)

		MetaAccount *Account_;
		QString ID_;
		QString Name_;
		QStringList Groups_;

		QStringList UnavailableRealEntries_;
		QList<QObject*> AvailableRealEntries_;
		QMap<QString, QPair<QObject*, QString>> Variant2RealVariant_;

		QList<IMessage*> Messages_;

		QAction *ActionMCSep_;
		QAction *ActionManageContacts_;
	public:
		MetaEntry (const QString&, MetaAccount*);

		QObjectList GetAvailEntryObjs () const;
		QStringList GetRealEntries () const;
		void SetRealEntries (const QStringList&);
		void AddRealObject (ICLEntry*);

		QString GetMetaVariant (QObject*, const QString&) const;

		// ICLEntry
		QObject* GetQObject ();
		IAccount* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString& name);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type, const QString&, const QString&);
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();

		// IAdvancedCLEntry
		AdvancedFeatures GetAdvancedFeatures () const;
		void DrawAttention (const QString&, const QString&);
	private:
		template<typename T, typename U>
		T ActWithVariant (std::function<T (U, const QString&)>, const QString&) const;

		void ConnectStandardSignals (QObject*);
		void ConnectAdvancedSiganls (QObject*);
	private:
		void PerformRemoval (QObject*);
		void SetNewEntryList (const QList<QObject*>&, bool readdRemoved);
	private slots:
		void handleRealGotMessage (QObject*);
		void handleRealStatusChanged (const EntryStatus&, const QString&);
		void handleRealVariantsChanged (QStringList, QObject* = 0);
		void handleRealNameChanged (const QString&);
		void handleRealCPSChanged (const ChatPartState&, const QString&);

		void handleRealAttentionDrawn (const QString&, const QString&);

		void checkRemovedCLItems (const QList<QObject*>&);

		void handleManageContacts ();
	signals:
		// ICLEntry
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		// IAdvancedCLEntry
		void attentionDrawn (const QString&, const QString&);
		void locationChanged (const QString&);

		// Own
		void shouldRemoveThis ();
	};
}
}
}
