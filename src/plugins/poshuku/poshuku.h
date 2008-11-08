#ifndef POSHUKU_H
#define POSHUKU_H
#include <memory>
#include <QAction>
#include <QTranslator>
#include <QWidget>
#include <interfaces/interfaces.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/tagscompleter.h>
#include "filtermodel.h"
#include "ui_poshuku.h"

class QWebView;

class Poshuku : public QWidget
			  , public IInfo
			  , public IEmbedTab
			  , public IMultiTabs
{
    Q_OBJECT
    Q_INTERFACES (IInfo IEmbedTab IMultiTabs)

	Ui::Poshuku Ui_;

	std::auto_ptr<QTranslator> Translator_;
	std::auto_ptr<TagsCompleter> FavoritesFilterLineCompleter_;
	std::auto_ptr<XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<FilterModel> FavoritesFilterModel_;
public:
	void Init ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	QIcon GetIcon () const;
	QWidget* GetTabContents ();
private:
	void RegisterSettings ();
public slots:
	void openURL (const QString&);
	QWebView* createWindow ();
private slots:
	void on_AddressLine__returnPressed ();
	void on_ActionSettings__triggered ();
	void viewerSettingsChanged ();
	void updateFavoritesFilter ();
signals:
	void bringToFront ();
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
	void changeTabIcon (QWidget*, const QIcon&);
};

#endif

