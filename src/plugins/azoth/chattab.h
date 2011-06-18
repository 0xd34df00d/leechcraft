/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_CHATTAB_H
#define PLUGINS_AZOTH_CHATTAB_H
#include <QWidget>
#include <QPointer>
#include <QPersistentModelIndex>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include "interfaces/azothcommon.h"
#include "ui_chattab.h"

namespace LeechCraft
{
namespace Azoth
{
	struct EntryStatus;
	class ICLEntry;
	class IMUCEntry;
	class IMessage;
	class ITransferManager;

	class ChatTab : public QWidget
				  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject *S_ParentMultiTabs_;

		Ui::ChatTab Ui_;
		QToolBar *TabToolbar_;
		QAction *SendFile_;

		QString EntryID_;

		QColor BgColor_;

		QList<QString> MsgHistory_;
		int CurrentHistoryPosition_;
		QStringList AvailableNickList_;
		int CurrentNickIndex_;
		int LastSpacePosition_;
		QString NickFirstPart_;

		int NumUnreadMsgs_;

		QIcon TabIcon_;
		bool IsMUC_;
		int PreviousTextHeight_;

		ITransferManager *XferManager_;
		
		QTimer *TypeTimer_;
		
		ChatPartState PreviousState_;
	public:
		static void SetParentMultiTabs (QObject*);

		ChatTab (const QString&, QWidget* = 0);
		~ChatTab ();
		
		/** Prepare (or update after it has been changed) the theme.
		 */
		void PrepareTheme ();

		/** Is called after the tab has been added to the tabwidget so
		 * that it could set its icon and various other stuff.
		 */
		void HasBeenAdded ();

		TabClassInfo GetTabClassInfo () const;
		QList<QAction*> GetTabBarContextMenuActions () const;
		QObject* ParentMultiTabs ();
		QToolBar* GetToolBar () const;
		void Remove ();
		void TabMadeCurrent ();
		void TabLostCurrent ();
		
		QObject* GetCLEntry () const;
	private slots:
		void clearAvailableNick ();
		void handleEditScroll (int);
		void messageSend ();
		void nickComplete ();
		void on_MsgEdit__textChanged ();
		void on_SubjectButton__toggled (bool);
		void on_SubjChange__released ();
		void handleSendFile ();
		void handleClearChat ();
		void handleFileOffered (QObject*);
		void handleFileNoLongerOffered (QObject*);
		void handleOfferActionTriggered ();
		void handleEntryMessage (QObject*);
		void handleVariantsChanged (QStringList);
		void handleStatusChanged (const EntryStatus&, const QString&);
		void handleChatPartStateChanged (const ChatPartState&, const QString&);
		void handleViewLinkClicked (const QUrl&);
		void handleHistoryUp ();
		void handleHistoryDown ();
		void handleAddToBookmarks ();
		void handleConfigureMUC ();
		void typeTimeout ();
		
		void handleFontSizeChanged ();
	private:
		template<typename T>
		T* GetEntry () const;
		void CheckMUC ();
		void HandleMUC ();
		QStringList GetMUCParticipants () const;

		void ReformatTitle ();
		void UpdateTextHeight ();
		void SetChatPartState (ChatPartState);

		/** Appends the message to the message view area.
		 */
		void AppendMessage (IMessage*);

		/** Processes the outgoing messages, replacing /nick with calls
		 * to the entity to change nick, for example, etc.
		 *
		 * If this function returns true, processing (and sending) the
		 * message should be aborted.
		 *
		 * @return true if the processing should be aborted, false
		 * otherwise.
		 */
		bool ProcessOutgoingMsg (ICLEntry*, QString&);

		/** Updates the tab icon and other usages of state icon from the
		 * TabIcon_.
		 */
		void UpdateStateIcon ();

		/** Insert nickname into message edit field.
		 * @param nickname a nick to insert, in html format.
		 */
		void InsertNick (const QString& nicknameHtml);

	signals:
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void needToClose (ChatTab*);
		void entryMadeCurrent (QObject*);

		// Hooks
		void hookChatTabCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebView *webView);
		void hookMadeCurrent (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab);
		void hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant,
				QString text);
		void hookMessageCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
		void hookThemeReloaded (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QWebView *view,
				QObject *entry);
	};

	typedef QPointer<ChatTab> ChatTab_ptr;
}
}

#endif
