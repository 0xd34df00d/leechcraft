#ifndef CORE_H
#define CORE_H
#include <vector>
#include <QAbstractItemModel>
#include "searcher.h"

class QNetworkAccessManager;

class Core : public QAbstractItemModel
{
	Q_OBJECT

	typedef std::vector<Searcher*> searchers_t;
	searchers_t Searchers_;
	QNetworkAccessManager *Manager_;
	typedef std::vector<Lyrics> lyrics_t;
	lyrics_t Lyrics_;

	Core ();
public:
	static Core& Instance ();
	void Release ();

	int columnCount (const QModelIndex&) const;
	QVariant data (const QModelIndex&, int) const;
	QModelIndex index (int, int, const QModelIndex&) const;
	QModelIndex parent (const QModelIndex&) const;
	int rowCount (const QModelIndex&) const;

	void SetNetworkAccessManager (QNetworkAccessManager*);
	QNetworkAccessManager* GetNetworkAccessManager () const;

	QByteArray Start (const LeechCraft::Request&);
	void Stop (const QByteArray&);
	void Reset ();

	void LyricsAvailable (const Lyrics&);
private slots:
	void handleTextFetched (const Lyrics&);
signals:
	void entityUpdated (LeechCraft::FoundEntity);
};

#endif

