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

#ifndef PLUGINS_AZOTH_CHATTAB_H
#define PLUGINS_AZOTH_CHATTAB_H
#include <QWidget>
#include <QPointer>
#include <QPersistentModelIndex>
#include <QDateTime>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
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

	class MsgFormatterWidget;

	class ChatTab : public QWidget
				  , public ITabWidget
				  , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		static QObject *S_ParentMultiTabs_;

		Ui::ChatTab Ui_;
		QToolBar *TabToolbar_;
		QAction *ToggleRichText_;
		QAction *Call_;
#ifdef ENABLE_CRYPT
		QAction *EnableEncryption_;
#endif

		QString EntryID_;

		QColor BgColor_;

		QList<QString> MsgHistory_;
		int CurrentHistoryPosition_;
		QStringList AvailableNickList_;
		int CurrentNickIndex_;
		int LastSpacePosition_;
		QString NickFirstPart_;

		int NumUnreadMsgs_;

		QList<IMessage*> HistoryMessages_;

		QIcon TabIcon_;
		bool IsMUC_;
		int PreviousTextHeight_;

		MsgFormatterWidget *MsgFormatter_;

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

		QByteArray GetTabRecoverData () const;
		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;

		void HandleMUCParticipantsChanged ();

		QObject* GetCLEntry () const;
		QString GetSelectedVariant () const;
	public slots:
		void prepareMessageText (const QString&);
		void appendMessageText (const QString&);
		void selectVariant (const QString&);
		QTextEdit* getMsgEdit ();
	private slots:
		void clearAvailableNick ();
		void handleEditScroll (int);
		void messageSend ();
		void nickComplete ();
		void on_MsgEdit__textChanged ();
		void on_SubjectButton__toggled (bool);
		void on_SubjChange__released ();
		void on_View__loadFinished (bool);
		void handleClearChat ();
		void handleRichTextToggled ();
		void handleQuoteSelection ();
#ifdef ENABLE_MEDIACALLS
		void handleCallRequested ();
		void handleCall (QObject*);
#endif
#ifdef ENABLE_CRYPT
		void handleEnableEncryption ();
		void handleEncryptionStateChanged (QObject*, bool);
#endif
		void handleFileOffered (QObject*);
		void handleFileNoLongerOffered (QObject*);
		void handleOfferActionTriggered ();
		void handleEntryMessage (QObject*);
		void handleVariantsChanged (QStringList);
		void handleAvatarChanged (const QImage&);
		void handleStatusChanged (const EntryStatus&, const QString&);
		void handleChatPartStateChanged (const ChatPartState&, const QString&);
		void handleViewLinkClicked (const QUrl&);
		void handleHistoryUp ();
		void handleHistoryDown ();
		void typeTimeout ();

		void handleGotLastMessages (QObject*, const QList<QObject*>&);

		void handleSendButtonVisible ();
		void handleRichFormatterPosition ();
		void handleFontSettingsChanged ();
		void handleFontSizeChanged ();
	private:
		template<typename T>
		T* GetEntry () const;
		void BuildBasicActions ();
		void InitEntry ();
		void CheckMUC ();
		void HandleMUC ();
		void InitExtraActions ();
		void InitMsgEdit ();
		void RegisterSettings ();

		void RequestLogs ();

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

		void tabRecoverDataChanged ();

		// Hooks
		void hookChatTabCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebView *webView);
		void hookGonnaAppendMsg (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookMadeCurrent (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab);
		void hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
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
