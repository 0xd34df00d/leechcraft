/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ljbloggingplatform.h"
#include <QIcon>
#include <QInputDialog>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QMainWindow>
#include <QDomElement>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/xpc/passutils.h>
#include <util/sll/prelude.h>
#include <util/sll/functional.h>
#include "ljaccount.h"
#include "ljaccountconfigurationwidget.h"
#include "postoptionswidget.h"
#include "localstorage.h"
#include "xmlsettingsmanager.h"
#include "polldialog.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	LJBloggingPlatform::LJBloggingPlatform (LocalStorage& storage, const ICoreProxy_ptr& proxy, QObject *parent)
	: Storage_ (storage)
	, ParentBlogginPlatfromPlugin_ (parent)
	, Proxy_ (proxy)
	, LJUser_ (new QAction (proxy->GetIconThemeManager ()->GetIcon ("user-properties"), tr ("Add LJ user"), this))
	, LJPoll_ (new QAction (proxy->GetIconThemeManager ()->GetIcon ("office-chart-pie"), tr ("Create poll"), this))
	, LJCut_ (new QAction (proxy->GetIconThemeManager ()->GetIcon ("user-properties"), tr ("Insert LJ cut"), this))
	, FirstSeparator_ (new QAction (this))
	, MessageCheckingTimer_ (new QTimer (this))
	{
		FirstSeparator_->setSeparator (true);

		connect (LJUser_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddLJUser ()));
		connect (LJPoll_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddLJPoll ()));

		connect (MessageCheckingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkForMessages ()));

		XmlSettingsManager::Instance ().RegisterObject ("CheckingInboxEnabled",
				this, "handleMessageChecking");
		handleMessageChecking ();
	}

	QObject* LJBloggingPlatform::GetQObject ()
	{
		return this;
	}

	IBloggingPlatform::BloggingPlatfromFeatures LJBloggingPlatform::GetFeatures () const
	{
		return BPFSupportsProfiles | BPFSelectablePostDestination | BPFSupportsBackup | BPFSupportComments;
	}

	QObjectList LJBloggingPlatform::GetRegisteredAccounts ()
	{
		return Util::Map (LJAccounts_, Util::Upcast<QObject*>);
	}

	QObject* LJBloggingPlatform::GetParentBloggingPlatformPlugin () const
	{
		return ParentBlogginPlatfromPlugin_;
	}

	QString LJBloggingPlatform::GetBloggingPlatformName () const
	{
		return "LiveJournal";
	}

	QIcon LJBloggingPlatform::GetBloggingPlatformIcon () const
	{
		return QIcon ("lcicons:/plugins/blogique/plugins/metida/resources/images/livejournalicon.svg");
	}

	QByteArray LJBloggingPlatform::GetBloggingPlatformID () const
	{
		return "Blogique.Metida.LiveJournal";
	}

	QList<QWidget*> LJBloggingPlatform::GetAccountRegistrationWidgets (IBloggingPlatform::AccountAddOptions, const QString&)
	{
		QList<QWidget*> result;
		result << new LJAccountConfigurationWidget ();
		return result;
	}

	void LJBloggingPlatform::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		auto w = qobject_cast<LJAccountConfigurationWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		LJAccount *account = new LJAccount (name, Proxy_, this);
		account->FillSettings (w);

		const QString& pass = w->GetPassword ();
		if (!pass.isEmpty ())
			Util::SavePassword (pass,
					"org.LeechCraft.Blogique.PassForAccount/" + account->GetAccountID (),
					Proxy_);

		LJAccounts_ << account;
		saveAccounts ();
		emit accountAdded (account);
		account->Init ();
		Storage_.AddAccount (account->GetAccountID ());
	}

	void LJBloggingPlatform::RemoveAccount (QObject *account)
	{
		LJAccount *acc = qobject_cast<LJAccount*> (account);
		if (LJAccounts_.removeAll (acc))
		{
			emit accountRemoved (account);
			Storage_.RemoveAccount (acc->GetAccountID ());
			account->deleteLater ();
			saveAccounts ();
		}
	}

	QList<QAction*> LJBloggingPlatform::GetEditorActions () const
	{
		return { FirstSeparator_, LJUser_, LJPoll_ };
	}

	QList<InlineTagInserter> LJBloggingPlatform::GetInlineTagInserters () const
	{
		auto cutIcon = Proxy_->GetIconThemeManager ()->GetIcon ("distribute-vertical-equal");
		return
		{
			{
				"lj-cut",
				{},
				[cutIcon] (QAction *action)
				{
					action->setText ("Insert cut");
					action->setIcon (cutIcon);
				}
			}
		};
	}

	QList<QWidget*> LJBloggingPlatform::GetBlogiqueSideWidgets () const
	{
		return { new PostOptionsWidget { Proxy_ } };
	}

	void LJBloggingPlatform::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void LJBloggingPlatform::Prepare ()
	{
		RestoreAccounts ();
	}

	void LJBloggingPlatform::Release ()
	{
		saveAccounts ();
	}

	IAdvancedHTMLEditor::CustomTags_t LJBloggingPlatform::GetCustomTags () const
	{
		IAdvancedHTMLEditor::CustomTags_t tags;

		using enum IAdvancedHTMLEditor::CustomTag::TagType;

		IAdvancedHTMLEditor::CustomTag ljUserTag;
		ljUserTag.TagName_ = "lj";
		ljUserTag.TagType_ = Inline;
		ljUserTag.ToKnown_ = [] (QDomElement& elem)
		{
			const auto& user = elem.attribute ("user");
			elem.setTagName ("span");
			elem.setAttribute ("contenteditable", "false");

			QDomElement linkElem = elem.ownerDocument ().createElement ("a");
			linkElem.setAttribute ("href", QString ("http://%1.livejournal.com/profile").arg (user));
			linkElem.setAttribute ("target", "_blank");

			QDomElement imgElem = elem.ownerDocument ().createElement ("img");
			imgElem.setAttribute ("src", "http://l-stat.livejournal.com/img/userinfo.gif?v=17080");
			linkElem.appendChild (imgElem);

			QDomElement nameElem = elem.ownerDocument ().createElement ("a");
			nameElem.setAttribute ("href", QString ("http://%1.livejournal.com/profile").arg (user));
			nameElem.setAttribute ("target", "_blank");
			nameElem.setAttribute ("id", "nameLink");
			nameElem.setAttribute ("contenteditable", "true");
			nameElem.appendChild (elem.ownerDocument ().createTextNode (user));

			elem.appendChild (linkElem);
			elem.appendChild (nameElem);

			elem.removeAttribute ("user");
		};
		ljUserTag.FromKnown_ = [] (QDomElement& elem)
		{
			auto aElem = elem.firstChildElement ("a");
			while (!aElem.isNull ())
			{
				if (aElem.attribute ("id") == "nameLink")
					break;

				aElem = aElem.nextSiblingElement ("a");
			}
			if (aElem.isNull ())
				return false;
			const auto& username = aElem.text ();

			const auto& children = elem.childNodes ();
			while (!children.isEmpty ())
				elem.removeChild (children.at (0));

			elem.setTagName ("lj");
			elem.setAttribute ("user", username);

			return true;
		};
		tags << ljUserTag;

		IAdvancedHTMLEditor::CustomTag ljCutTag;
		ljCutTag.TagName_ = "lj-cut";
		ljCutTag.TagType_ = Empty;
		ljCutTag.ToKnown_ = [] (QDomElement& elem) -> void
		{
			elem.setTagName ("div");
			const auto& text = elem.attribute ("text");
			elem.removeAttribute ("text");
			elem.setAttribute ("id", "cutTag");
			elem.setAttribute ("style", "overflow:auto;border-width:3px;border-style:dotted;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("text", text);
		};
		ljCutTag.FromKnown_ = [] (QDomElement& elem) -> bool
		{
			if (!elem.hasAttribute ("id") ||
					elem.attribute ("id") != "cutTag")
				return false;

			elem.removeAttribute ("id");
			elem.removeAttribute ("style");
			const auto& text = elem.attribute ("text");
			elem.removeAttribute ("text");
			elem.setTagName ("lj-cut");
			if (!text.isEmpty ())
				elem.setAttribute ("text", text);

			return true;
		};

		tags << ljCutTag;

		IAdvancedHTMLEditor::CustomTag ljPollTag;
		ljPollTag.TagName_ = "lj-poll";
		ljPollTag.TagType_ = Block;
		ljPollTag.ToKnown_ = [] (QDomElement& elem)
		{
			const auto& whoView = elem.attribute ("whoview");
			const auto& whoVote = elem.attribute ("whovote");
			const auto& name = elem.attribute ("name");

			auto children = elem.childNodes ();

			elem.setTagName ("div");
			elem.setAttribute ("style", "overflow:auto;border-width:2px;border-style:solid;border-radius:5px;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("id", "pollDiv");
			elem.setAttribute ("ljPollWhoview", whoView);
			elem.setAttribute ("ljPollWhovote", whoVote);
			elem.setAttribute ("ljPollName", name);
			QString questions;
			for (int i = 0, size = children.size (); i < size; ++i)
			{
				const auto& child = children.at (i);
				QString question;
				QTextStream str (&question);
				child.save (str, 0);

				questions.append (question);
			}
			elem.setAttribute ("ljPollQuestions", QString (questions.toUtf8 ().toBase64 ()));
			while (!children.isEmpty ())
				elem.removeChild (children.at (0));
			auto textElem = elem.ownerDocument ().createTextNode (tr ("Poll: %1").arg (name));
			elem.appendChild (textElem);

		};
		ljPollTag.FromKnown_ = [] (QDomElement& elem)
		{
			if (!elem.hasAttribute ("id") ||
					elem.attribute ("id") != "pollDiv")
				return false;

			auto whoView = elem.attribute ("ljPollWhoview");
			auto whoVote = elem.attribute ("ljPollWhovote");
			auto name = elem.attribute ("ljPollName");
			auto questions = QByteArray::fromBase64 (elem.attribute ("ljPollQuestions").toUtf8 ());

			elem.removeAttribute ("style");
			elem.removeAttribute ("ljPollWhoview");
			elem.removeAttribute ("ljPollWhovot");
			elem.removeAttribute ("ljPollName");
			elem.removeAttribute ("ljPollQuestions");
			elem.removeAttribute ("id");
			elem.removeChild (elem.firstChild ());

			elem.setTagName ("lj-poll");
			elem.setAttribute ("whoview", whoView);
			elem.setAttribute ("whovote", whoVote);
			elem.setAttribute ("name", name);
			QDomDocument doc;
			if (!doc.setContent (questions))
				qWarning () << Q_FUNC_INFO
						<< "unable to parse poll questions from"
						<< questions;
			else
				elem.appendChild (doc.documentElement ());

			return true;
		};

		tags << ljPollTag;

		IAdvancedHTMLEditor::CustomTag ljEmbedTag;
		ljEmbedTag.TagName_ = "lj-embed";
		ljEmbedTag.TagType_ = Block;
		ljEmbedTag.ToKnown_ = [] (QDomElement& elem)
		{
			const auto& id = elem.attribute ("id");
			elem.removeAttribute ("id");

			elem.setTagName ("div");
			elem.setAttribute ("style", "overflow:auto;border-width:2px;border-style:solid;border-radius:5px;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("id", "embedTag");
			elem.setAttribute ("name", id);
			auto textElem = elem.ownerDocument ().createTextNode (tr ("Embedded: %1")
					.arg (id));
			elem.appendChild (textElem);
		};
		ljEmbedTag.FromKnown_ = [] (QDomElement& elem)
		{
			if (!elem.hasAttribute ("id") ||
					elem.attribute ("id") != "embedTag")
				return false;

			elem.removeAttribute ("style");
			elem.removeChild (elem.firstChild ());
			const auto& id = elem.attribute ("name");
			elem.removeAttribute ("id");
			elem.setTagName ("lj-embed");
			elem.setAttribute ("id", id);
			return true;
		};

		tags << ljEmbedTag;

		IAdvancedHTMLEditor::CustomTag ljLikeTag;
		ljLikeTag.TagName_ = "lj-like";
		ljEmbedTag.TagType_ = Block;
		ljLikeTag.ToKnown_ = [] (QDomElement& elem)
		{
			const auto& buttons = elem.attribute ("buttons");
			elem.removeAttribute ("buttons");

			elem.setTagName ("div");
			elem.setAttribute ("style", "overflow:auto;border-width:2px;border-style:solid;border-radius:5px;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("likes", buttons);
			auto textElem = elem.ownerDocument ().createTextNode (tr ("Likes: %1")
					.arg (!buttons.isEmpty () ?
						buttons :
						"repost,facebook,twitter,google,vkontakte,surfingbird,tumblr,livejournal"));
			elem.appendChild (textElem);
		};
		ljLikeTag.FromKnown_ = [] (QDomElement& elem)
		{
			const auto& likes = elem.attribute ("likes");
			if (likes.isEmpty ())
				return false;

			elem.removeAttribute ("likes");
			elem.removeAttribute ("style");
			elem.setTagName ("lj-like");
			elem.setAttribute ("buttons", likes);
			elem.removeChild (elem.firstChild ());
			return true;
		};

		tags << ljLikeTag;

		return tags;
	}

	void LJBloggingPlatform::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
						"_Blogique_Metida_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings.value ("SerializedData").toByteArray ();
			LJAccount *acc = LJAccount::Deserialize (data, Proxy_, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}
			LJAccounts_ << acc;
			emit accountAdded (acc);

			acc->Init ();
			Storage_.AddAccount (acc->GetAccountID ());
		}
		settings.endArray ();
	}

	void LJBloggingPlatform::saveAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
						"_Blogique_Metida_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0, size = LJAccounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					LJAccounts_.at (i)->Serialize ());
		}
		settings.endArray ();
		settings.sync ();
	}

	void LJBloggingPlatform::handleAddLJUser ()
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		QString name = QInputDialog::getText (rootWM->GetPreferredWindow (),
				tr ("Add LJ User"),
				tr ("Enter LJ user name:"));
		if (name.isEmpty ())
			return;

		emit insertTag (QString ("<lj user=\"%1\" />").arg (name));
	}

	void LJBloggingPlatform::handleAddLJPoll ()
	{
		PollDialog pollDlg;
		if (pollDlg.exec () == QDialog::Rejected)
			return;

		QStringList pqParts;
		QString pqPart = QString ("<lj-pq type=\"%1\" %2>%3%4</lj-pq>");
		bool isPqParam = false;
		for (const auto& pollType : pollDlg.GetPollTypes ())
		{
			const auto& map = pollDlg.GetPollFields (pollType);
			QStringList pqParams;
			if (pollType == "check" ||
					pollType == "radio" ||
					pollType == "drop")
			{
				isPqParam = false;
				for (const auto& value : map.values ())
					pqParams << QString ("<lj-pi>%1</lj-pi>")
							.arg (value.toString ());
			}
			else
			{
				isPqParam = true;
				for (const auto& key : map.keys ())
					pqParams << QString ("%1=\"%2\"")
							.arg (key)
							.arg (map [key].toString ());
			}
			pqParts << pqPart
					.arg (pollType)
					.arg (isPqParam ? pqParams.join (" ") : QString ())
					.arg (pollDlg.GetPollQuestion (pollType))
					.arg (!isPqParam ? pqParams.join (" ") : QString ());
		}

		QString pollPart = QString ("<lj-poll name=\"%1\" whovote=\"%2\" whoview=\"%3\">%4</lj-poll>")
				.arg (pollDlg.GetPollName ())
				.arg (pollDlg.GetWhoCanVote ())
				.arg (pollDlg.GetWhoCanView ())
				.arg (pqParts.join (""));
		emit insertTag (pollPart);
	}

	void LJBloggingPlatform::handleAccountValidated (bool validated)
	{
		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an IAccount";;
			return;
		}

		emit accountValidated (acc->GetQObject (), validated);
		if (validated &&
				XmlSettingsManager::Instance ().Property ("CheckingInboxEnabled", true).toBool ())
			checkForMessages ();;
	}

	void LJBloggingPlatform::handleMessageChecking ()
	{
		if (!XmlSettingsManager::Instance ().Property ("CheckingInboxEnabled", true).toBool () &&
				MessageCheckingTimer_->isActive ())
			MessageCheckingTimer_->stop ();
	}

	void LJBloggingPlatform::handleMessageUpdateIntervalChanged ()
	{
		if (XmlSettingsManager::Instance ().Property ("CheckingInboxEnabled", true).toBool ())
			MessageCheckingTimer_->start (XmlSettingsManager::Instance ()
					.property ("UpdateInboxInterval").toInt () * 60 * 1000);
		else if (MessageCheckingTimer_->isActive ())
			MessageCheckingTimer_->stop ();
	}

	void LJBloggingPlatform::checkForMessages ()
	{
		for (auto account : LJAccounts_)
			account->RequestInbox ();
	}
}
}
}
