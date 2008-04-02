#ifndef TAGSCOMPLETIONMODEL_H
#define TAGSCOMPLETIONMODEL_H
#include <QAbstractItemModel>
#include <QStringList>

class QLineEdit;

class TagsCompletionModel : public QAbstractItemModel
{
    Q_OBJECT

    QStringList Tags_;
    const QLineEdit* LineEdit_;
public:
    TagsCompletionModel (QObject *parent = 0);

    virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual bool hasChildren (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

    void UpdateTags (const QStringList&);
    QStringList GetTags () const;
    void SetLineEdit (const QLineEdit*);
};

#endif

