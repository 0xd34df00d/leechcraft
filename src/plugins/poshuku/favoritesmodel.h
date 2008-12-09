#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H
#include <vector>
#include <QAbstractItemModel>
#include <QStringList>

class FavoritesModel : public QAbstractItemModel
{
	Q_OBJECT
	
	QStringList ItemHeaders_;
public:
	struct FavoritesItem
	{
		QString Title_;
		QString URL_;
		QStringList Tags_;

		bool operator== (const FavoritesItem&) const;
	};
private:
	typedef std::vector<FavoritesItem> items_t;
	items_t Items_;
public:
	enum Columns
	{
		ColumnTitle
		, ColumnURL
		, ColumnTags
	};
	enum { TagsRole = 42 };

	FavoritesModel (QObject* = 0);
	virtual ~FavoritesModel ();

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation,
			int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int,
			const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
	virtual bool setData (const QModelIndex&, const QVariant&,
			int = Qt::EditRole);

	void AddItem (const QString&, const QString&, const QStringList&);
public slots:
	void removeItem (const QModelIndex&);
	void handleItemAdded (const FavoritesModel::FavoritesItem&);
	void handleItemUpdated (const FavoritesModel::FavoritesItem&);
	void handleItemRemoved (const FavoritesModel::FavoritesItem&);
private slots:
	void loadData ();
};

#endif

