#include "networkaccessmanager.h"
#include <QNetworkRequest>
#include <QDir>
#include <QFile>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QSettings>
#include <plugininterface/util.h>
#include <plugininterface/proxy.h>
#include <plugininterface/customcookiejar.h>
#include "core.h"
#include "networkdiskcache.h"
#include "authenticationdialog.h"
#include "sslerrorsdialog.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

NetworkAccessManager::NetworkAccessManager (QObject *parent)
: QNetworkAccessManager (parent)
, CookieSaveTimer_ (new QTimer ())
{
	connect (this,
			SIGNAL (authenticationRequired (QNetworkReply*,
					QAuthenticator*)),
			this,
			SLOT (handleAuthentication (QNetworkReply*,
					QAuthenticator*)));
	connect (this,
			SIGNAL (proxyAuthenticationRequired (const QNetworkProxy&,
					QAuthenticator*)),
			this,
			SLOT (handleProxyAuthentication (const QNetworkProxy&,
					QAuthenticator*)));
	connect (this,
			SIGNAL (sslErrors (QNetworkReply*,
					const QList<QSslError>&)),
			this,
			SLOT (handleSslErrors (QNetworkReply*,
					const QList<QSslError>&)));

	CustomCookieJar *jar = new CustomCookieJar (this);
	setCookieJar (jar);
	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (file.open (QIODevice::ReadOnly))
		jar->Load (file.readAll ());
	else
		qWarning () << Q_FUNC_INFO
			<< "could not open file"
			<< file.fileName ()
			<< file.errorString ();

	try
	{
		CreateIfNotExists ("core/cache");
		NetworkDiskCache *cache = new NetworkDiskCache (this);
		cache->setCacheDirectory (QDir::homePath () + "/.leechcraft/core/cache");
		setCache (cache);
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< "so continuing without cache";
	}

	connect (CookieSaveTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (saveCookies ()));
	CookieSaveTimer_->start (10000);
}

NetworkAccessManager::~NetworkAccessManager ()
{
	saveCookies ();
}

QNetworkReply* NetworkAccessManager::createRequest (QNetworkAccessManager::Operation op,
		const QNetworkRequest& req, QIODevice *out)
{
	QNetworkRequest r = req;
	HookProxy_ptr proxy (new HookProxy);
	Q_FOREACH (HookSignature<HIDNetworkAccessManagerCreateRequest>::Signature_t f,
			Core::Instance ().GetHooks<HIDNetworkAccessManagerCreateRequest> ())
	{
		QNetworkReply *rep = f (proxy.get (), &op, &r, &out);
		if (proxy->IsCancelled ())
			return rep;
	}

	QNetworkReply *result = QNetworkAccessManager::createRequest (op, r, out);
	emit requestCreated (op, req, result);
	return result;
}

void LeechCraft::NetworkAccessManager::DoCommonAuth (const QString& msg, QAuthenticator *authen)
{
	QString realm = authen->realm ();

	QString suggestedUser = authen->user ();
	QString suggestedPassword = authen->password ();

	StorageBackend *backend = Core::Instance ().GetStorageBackend ();

	if (suggestedUser.isEmpty ())
		backend->GetAuth (realm, suggestedUser, suggestedPassword);

	std::auto_ptr<AuthenticationDialog> dia (
			new AuthenticationDialog (msg,
				suggestedUser,
				suggestedPassword,
				qApp->activeWindow ())
			);
	if (dia->exec () == QDialog::Rejected)
		return;

	QString login = dia->GetLogin ();
	QString password = dia->GetPassword ();
	authen->setUser (login);
	authen->setPassword (password);

	if (dia->ShouldSave ())
		backend->SetAuth (realm, login, password);
}

void LeechCraft::NetworkAccessManager::handleAuthentication (QNetworkReply *reply,
		QAuthenticator *authen)
{
	QString msg = tr ("The URL<br /><code>%1</code><br />with "
			"realm<br /><em>%2</em><br />requires authentication.")
		.arg (reply->url ().toString ())
		.arg (authen->realm ());
	msg = msg.left (200);

	DoCommonAuth (msg, authen);
}

void LeechCraft::NetworkAccessManager::handleProxyAuthentication (const QNetworkProxy& proxy,
		QAuthenticator *authen)
{
	QString msg = tr ("The proxy <br /><code>%1</code><br />with "
			"realm<br /><em>%2</em><br />requires authentication.")
		.arg (proxy.hostName () + ":" + QString::number (proxy.port ()))
		.arg (authen->realm ());
	msg = msg.left (200);

	DoCommonAuth (msg, authen);
}

void LeechCraft::NetworkAccessManager::handleSslErrors (QNetworkReply *reply,
		const QList<QSslError>& errors)
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("SSL exceptions");
	QStringList keys = settings.allKeys ();
	if (keys.contains (reply->url ().toString ())) 
	{
		if (settings.value (reply->url ().toString ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else if (keys.contains (reply->url ().host ()))
	{
		if (settings.value (reply->url ().host ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else
	{
		QString msg = tr ("<code>%1</code><br />has SSL errors."
				" What do you want to do?")
			.arg (reply->url ().toString ());
		std::auto_ptr<SslErrorsDialog> dia (
				new SslErrorsDialog (msg,
					errors,
					qApp->activeWindow ())
				);

		bool ignore = (dia->exec () == QDialog::Accepted);
		if (ignore)
			reply->ignoreSslErrors ();

		SslErrorsDialog::RememberChoice choice = dia->GetRememberChoice ();

		if (choice != SslErrorsDialog::RCNot)
		{
			if (choice == SslErrorsDialog::RCFile)
				settings.setValue (reply->url ().toString (),
						ignore);
			else
				settings.setValue (reply->url ().host (),
						ignore);
		}
	}
	settings.endGroup ();
}

void LeechCraft::NetworkAccessManager::saveCookies () const
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	if (!dir.exists ("core") &&
			!dir.mkdir ("core"))
	{
		emit error (tr ("Could not create Core directory."));
		return;
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
	{
		emit error (tr ("Could not save cookies, error opening cookie file."));
		qWarning () << Q_FUNC_INFO
			<< file.errorString ();
	}
	else
	{
		CustomCookieJar *jar = static_cast<CustomCookieJar*> (cookieJar ());
		if (!jar)
		{
			qWarning () << Q_FUNC_INFO
				<< "jar is NULL";
			return;
		}
		file.write (jar->Save ());
	}
}

