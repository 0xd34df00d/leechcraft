#ifndef OPMLITEM_H
#define OPMLITEM_H
#include <QString>
#include <QStringList>

struct OPMLItem
{
	QString URL_;
	QString HTMLUrl_;
	QString Title_;
	QString Description_;
	QStringList Categories_;
	int MaxArticleAge_;
	int FetchInterval_;
	int MaxArticleNumber_;
	bool CustomFetchInterval_;
	//    <outline htmlUrl="" title="Оформление KDE"
	//    useCustomFetchInterval="false" maxArticleAge="0"
	//    fetchInterval="0" maxArticleNumber="0"
	//    archiveMode="globalDefault" version="RSS" type="rss"
	//    xmlUrl="http://www.kde.org/kde-look-content.rdf"
	//    id="2097705275" text="Оформление KDE" description="" />
};

#endif

