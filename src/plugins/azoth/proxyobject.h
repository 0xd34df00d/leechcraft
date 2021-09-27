/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QIcon>
#include <QColor>
#include <QDateTime>
#include <QRegExp>
#include "interfaces/azoth/iproxyobject.h"

namespace LC
{
namespace Azoth
{
	class FormatterProxyObject : public IFormatterProxyObject
	{
		QRegExp LinkRegexp_;
	public:
		FormatterProxyObject ();

		QList<QColor> GenerateColors (const QString&, QColor) const override;
		QString GetNickColor (const QString&, const QList<QColor>&) const override;
		QString FormatDate (QDateTime, QObject*) const override;
		QString FormatNickname (QString, QObject*, const QString&) const override;
		QString EscapeBody (QString, IMessage::EscapePolicy) const override;
		QString FormatBody (QString, QObject*, const QList<QColor>&) const override;
		void PreprocessMessage (QObject*) override;
		void FormatLinks (QString&) override;
		QStringList FindLinks (const QString&) override;
	};

	class ProxyObject : public QObject
					  , public IProxyObject
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProxyObject)

		QHash<QString, AuthStatus> SerializedStr2AuthStatus_;

		FormatterProxyObject Formatter_;
		IAvatarsManager * const AvatarsManager_;
	public:
		ProxyObject (IAvatarsManager*, QObject* = nullptr);
	public slots:
		QObject* GetSettingsManager () override;
		void SetPassword (const QString&, QObject*) override;
		QString GetAccountPassword (QObject*, bool) override;
		bool IsAutojoinAllowed () override;
		QString StateToString (State) const override;
		QString AuthStatusToString (AuthStatus) const override;
		AuthStatus AuthStatusFromString (const QString&) const override;
		QObject* GetAccount (const QString&) const override;
		QList<QObject*> GetAllAccounts () const override;
		QObject* GetEntry (const QString&, const QString&) const override;
		void OpenChat (const QString&, const QString&, const QString&, const QString&) const override;
		QWidget* FindOpenedChat (const QString&, const QByteArray&) const override;
		Util::ResourceLoader* GetResourceLoader (PublicResourceLoader) const override;
		QIcon GetIconForState (State) const override;

		QObject* CreateCoreMessage (const QString&, const QDateTime&,
				IMessage::Type, IMessage::Direction, QObject*, QObject*) override;

		QString ToPlainBody (QString) override;

		bool IsMessageRead (QObject*) override;
		void MarkMessagesAsRead (QObject*) override;

		QString PrettyPrintDateTime (const QDateTime&) override;

		std::optional<CustomStatus> FindCustomStatus (const QString&) const override;
		QStringList GetCustomStatusNames () const override;

		QImage GetDefaultAvatar (int) const override;

		void RedrawItem (QObject*) const override;

		QObject* GetFirstUnreadMessage (QObject *entryObj) const override;

		IFormatterProxyObject& GetFormatterProxy () override;
		IAvatarsManager* GetAvatarsManager() override;
	};
}
}
