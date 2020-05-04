/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
