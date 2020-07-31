/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/blogique/ibloggingplatform.h>

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	class LocalBlogAccount;

	class LocalBloggingPlatform : public QObject
								, public IBloggingPlatform
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blogique::IBloggingPlatform)

		QObject *ParentBlogginPlatfromPlugin_;
		QObject *PluginProxy_ = nullptr;
		QList<LocalBlogAccount*> Accounts_;

		enum BloqiqueSidePosition
		{
			First,
			Second
		};
	public:
		explicit LocalBloggingPlatform (QObject *parent = nullptr);

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

		void HandleAccountObject (LocalBlogAccount*);

	private slots:
		void saveAccounts ();

	signals:
		void accountAdded (QObject *account) override;
		void accountRemoved (QObject *account) override;
		void accountValidated (QObject *account, bool validated) override;
		void insertTag (const QString& tag) override;
	};
}
}
}
