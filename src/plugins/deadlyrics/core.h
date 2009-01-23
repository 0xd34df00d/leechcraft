#ifndef CORE_H
#define CORE_H
#include <vector>
#include <QObject>
#include "searcher.h"

class QNetworkAccessManager;

class Core : public QObject
{
	Q_OBJECT

	typedef std::vector<Searcher*> searchers_t;
	searchers_t Searchers_;
	QNetworkAccessManager *Manager_;

	Core ();
public:
	static Core& Instance ();
	void Release ();

	void SetNetworkAccessManager (QNetworkAccessManager*);
	QNetworkAccessManager* GetNetworkAccessManager () const;

	void Start (const QString&);
	void Abort ();
private slots:
	void handleTextFetched (const QString&);
signals:
	void entityUpdated (LeechCraft::FoundEntity);
};

#endif

