#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H
#include <vector>
#include <QAbstractItemModel>
#include <QStringList>

class FavoritesModel : public QAbstractItemModel
{
	Q_OBJECT
	
	QStringList ItemHeaders_;

	struct FavoritesItem
	{
		QString Title_;
		QString URL_;
		QStringList Tags_;
	};

	std::vector<FavoritesItem> Items_;
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
	void RemoveItem (const QModelIndex&);
private:
	void SaveData ();
	void LoadData ();
};

#endif

