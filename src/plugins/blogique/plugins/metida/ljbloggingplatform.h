/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QObject>
#include <interfaces/blogique/ibloggingplatform.h>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJAccount;

	class LJBloggingPlatform: public QObject
							, public IBloggingPlatform
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IBloggingPlatform)

		QObject *ParentBlogginPlatfromPlugin_;
		QObject *PluginProxy_;
		QList<LJAccount*> LJAccounts_;

		QAction *LJUser_;
		QAction *LJCut_;
		QAction *FirstSeparator_;

		QTimer *MessageCheckingTimer_;
		QTimer *CommentsCheckingTimer_;

		enum BloqiqueSidePosition
		{
			First,
			Second
		};
	public:
		LJBloggingPlatform (QObject *parent = 0);

		QObject* GetQObject ();
		BloggingPlatfromFeatures GetFeatures () const;
		QObjectList GetRegisteredAccounts ();
		QObject* GetParentBloggingPlatformPlugin () const;
		QString GetBloggingPlatformName () const;
		QIcon GetBloggingPlatformIcon () const;
		QByteArray GetBloggingPlatformID () const;

		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions);
		void RegisterAccount (const QString& name, const QList<QWidget*>& widgets);
		void RemoveAccount (QObject *account);
		QList<QAction*> GetEditorActions () const;
		QList<QWidget*> GetBlogiqueSideWidgets () const;

		void SetPluginProxy (QObject *proxy);
		void Prepare ();
		void Release ();

	private:
		void RestoreAccounts ();

	private slots:
		void saveAccounts ();
		void handleAddLJUser ();
		void handleAddLJCut ();
	public slots:
		void handleAccountValidated (bool validated);
		void handleMessageChecking ();
		void handleCommentsChecking ();
		void checkForMessages ();
		void checkForComments ();

	signals:
		void accountAdded (QObject *account);
		void accountRemoved (QObject *account);
		void accountValidated (QObject *account, bool validated);
	};
}
}
}
