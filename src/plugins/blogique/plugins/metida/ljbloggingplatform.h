/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/blogique/ibloggingplatform.h>

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJAccount;
	class LocalStorage;

	class LJBloggingPlatform: public QObject
							, public IBloggingPlatform
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blogique::IBloggingPlatform)

		LocalStorage& Storage_;
		QObject * const ParentBlogginPlatfromPlugin_;
		const ICoreProxy_ptr Proxy_;

		QObject *PluginProxy_ = nullptr;
		QList<LJAccount*> LJAccounts_;

		QAction *LJUser_;
		QAction *LJPoll_;
		QAction *LJCut_;
		QAction *FirstSeparator_;

		QTimer *MessageCheckingTimer_;
	public:
		LJBloggingPlatform (LocalStorage& storage, const ICoreProxy_ptr&, QObject *parent);

		QObject* GetQObject () override;
		BloggingPlatfromFeatures GetFeatures () const override;
		QObjectList GetRegisteredAccounts () override;
		QObject* GetParentBloggingPlatformPlugin () const override;
		QString GetBloggingPlatformName () const override;
		QIcon GetBloggingPlatformIcon () const override;
		QByteArray GetBloggingPlatformID () const override;

		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions, const QString&) override;
		void RegisterAccount (const QString& name, const QList<QWidget*>& widgets) override;
		void RemoveAccount (QObject *account) override;
		QList<QAction*> GetEditorActions () const override;
		QList<InlineTagInserter> GetInlineTagInserters () const override;
		QList<QWidget*> GetBlogiqueSideWidgets () const override;

		void SetPluginProxy (QObject *proxy);
		void Prepare ();
		void Release ();

		IAdvancedHTMLEditor::CustomTags_t GetCustomTags () const override;
	private:
		void RestoreAccounts ();

	private slots:
		void saveAccounts ();
		void handleAddLJUser ();
		void handleAddLJPoll ();
	public slots:
		void handleAccountValidated (bool validated);
		void handleMessageChecking ();
		void handleMessageUpdateIntervalChanged ();
		void checkForMessages ();
	signals:
		void accountAdded (QObject *account) override;
		void accountRemoved (QObject *account) override;
		void accountValidated (QObject *account, bool validated) override;
		void insertTag (const QString& tag) override;
	};
}
}
}
