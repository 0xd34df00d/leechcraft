#include "networkaccessmanager.h"
#include <QNetworkRequest>
#include "core.h"

using namespace LeechCraft;

NetworkAccessManager::NetworkAccessManager (QObject *parent)
: QNetworkAccessManager (parent)
{
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

