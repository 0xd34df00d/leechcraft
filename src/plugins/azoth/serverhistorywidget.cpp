/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverhistorywidget.h"
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QtDebug>
#include <util/gui/clearlineeditaddon.h>
#include <interfaces/azoth/ihaveserverhistory.h>
#include "proxyobject.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "util.h"

namespace LC
{
namespace Azoth
{
	namespace
	{
		const auto MaxMsgCount = 25;
	}

	QObject* ServerHistoryWidget::S_ParentMultiTabs_ = 0;
	TabClassInfo ServerHistoryWidget::S_TC_ = TabClassInfo ();

	ServerHistoryWidget::ServerHistoryWidget (QObject *account, QWidget *parent)
	: QWidget { parent }
	, Toolbar_ { new QToolBar { this } }
	, AccObj_ { account }
	, IHSH_ { qobject_cast<IHaveServerHistory*> (account) }
	, ContactsFilter_ { new QSortFilterProxyModel { this } }
	{
		Ui_.setupUi (this);

		if (!IHSH_)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IHaveServerHistory"
					<< account;
			return;
		}

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.ContactsFilter_);

		ContactsFilter_->setFilterCaseSensitivity (Qt::CaseInsensitive);

		const auto& sortParams = IHSH_->GetSortParams ();
		ContactsFilter_->setSortRole (sortParams.Role_);
		ContactsFilter_->setSortCaseSensitivity (Qt::CaseInsensitive);

		ContactsFilter_->setDynamicSortFilter (true);
		ContactsFilter_->setSourceModel (IHSH_->GetServerContactsModel ());

		Ui_.ContactsView_->setModel (ContactsFilter_);
		Ui_.ContactsView_->sortByColumn (sortParams.Column_, sortParams.Order_);

		connect (AccObj_,
				SIGNAL (serverHistoryFetched (QModelIndex, QByteArray, SrvHistMessages_t)),
				this,
				SLOT (handleFetched (QModelIndex, QByteArray, SrvHistMessages_t)));

		connect (Ui_.ContactsFilter_,
				SIGNAL (textChanged (QString)),
				ContactsFilter_,
				SLOT (setFilterFixedString (QString)));

		auto prevAct = Toolbar_->addAction (tr ("Previous page"),
				this, SLOT (navigatePrevious ()));
		prevAct->setProperty ("ActionIcon", "go-previous");

		auto nextAct = Toolbar_->addAction (tr ("Next page"),
				this, SLOT (navigateNext ()));
		nextAct->setProperty ("ActionIcon", "go-next");
	}

	void ServerHistoryWidget::SetTabData (QObject *plugin, const TabClassInfo& tc)
	{
		S_ParentMultiTabs_ = plugin;
		S_TC_ = tc;
	}

	TabClassInfo ServerHistoryWidget::GetTabClassInfo () const
	{
		return S_TC_;
	}

	QObject* ServerHistoryWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	void ServerHistoryWidget::Remove ()
	{
		emit removeTab ();
	}

	QToolBar* ServerHistoryWidget::GetToolBar () const
	{
		return Toolbar_;
	}

	void ServerHistoryWidget::SelectEntry (ICLEntry *entry)
	{
		const auto entryObj = entry->GetQObject ();

		const auto model = IHSH_->GetServerContactsModel ();
		for (int i = 0; i < model->rowCount (); ++i)
		{
			const auto& idx = model->index (i, 0);
			if (idx.data (ServerHistoryRole::CLEntry).value<QObject*> () == entryObj)
			{
				const auto& mapped = ContactsFilter_->mapFromSource (idx);
				if (!mapped.isValid ())
					return;

				Ui_.ContactsView_->setCurrentIndex (mapped);
				on_ContactsView__clicked (mapped);
			}
		}
	}

	int ServerHistoryWidget::GetReqMsgCount () const
	{
		return std::max (20, FirstMsgCount_);
	}

	void ServerHistoryWidget::handleFetched (const QModelIndex& index,
			const QByteArray& startId, const SrvHistMessages_t& messages)
	{
		if (index.row () != ContactsFilter_->mapToSource (Ui_.ContactsView_->currentIndex ()).row ())
			return;

		if (FirstMsgCount_ == -1)
			FirstMsgCount_ = messages.size ();

		CurrentID_ = startId;

		Ui_.MessagesView_->clear ();

		const auto& bgColor = palette ().color (QPalette::Base);
		const auto& colors = GenerateColors ("hash", bgColor);

		QString preNick = XmlSettingsManager::Instance ().property ("PreNickText").toString ();
		QString postNick = XmlSettingsManager::Instance ().property ("PostNickText").toString ();
		preNick.replace ('<', "&lt;");
		postNick.replace ('<', "&lt;");

		for (const auto& message : messages)
		{
			const auto& color = GetNickColor (message.Nick_, colors);

			auto msgText = message.RichBody_;
			if (msgText.isEmpty ())
			{
				msgText = message.Body_.toHtmlEscaped ();
				FormatterProxyObject {}.FormatLinks (msgText);
				msgText.replace ('\n', "<br/>");
			}

			QString html = "[" + message.TS_.toString () + "] " + preNick;
			html += "<font color='" + color + "'>" + message.Nick_ + "</font> ";
			html += postNick + ' ' + msgText;

			html.prepend (QString ("<font color='#") +
					(message.Dir_ == IMessage::Direction::In ? "0000dd" : "dd0000") +
					"'>");
			html += "</font>";

			Ui_.MessagesView_->append (html);
		}

		MaxID_ = messages.value (0).ID_;
	}

	void ServerHistoryWidget::on_ContactsView__clicked (const QModelIndex& index)
	{
		CurrentID_ = "-1";
		MaxID_ = "-1";
		FirstMsgCount_ = -1;
		IHSH_->FetchServerHistory (ContactsFilter_->mapToSource (index), CurrentID_, MaxMsgCount);
	}

	void ServerHistoryWidget::on_MessagesView__anchorClicked (const QUrl& url)
	{
		const auto& current = Ui_.ContactsView_->currentIndex ();
		const auto entryObj = current.data (ServerHistoryRole::CLEntry).value<QObject*> ();
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		Core::Instance ().HandleURLGeneric (url, true, entry);
	}

	void ServerHistoryWidget::navigatePrevious ()
	{
		const auto& index = ContactsFilter_->mapToSource (Ui_.ContactsView_->currentIndex ());
		IHSH_->FetchServerHistory (index, MaxID_, GetReqMsgCount ());
	}

	void ServerHistoryWidget::navigateNext ()
	{
		const auto& index = ContactsFilter_->mapToSource (Ui_.ContactsView_->currentIndex ());
		IHSH_->FetchServerHistory (index, CurrentID_, -GetReqMsgCount ());
	}
}
}
