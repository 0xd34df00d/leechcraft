#ifndef FINISHEDJOB_H
#define FINISHEDJOB_H
#include <QObject>
#include <QVariantList>
#include "impbase.h"
class JobRepresentation;

class FinishedJob : public QObject
{
    Q_OBJECT

    QString URL_;
    QString Local_;
    ImpBase::length_t Size_;
    QString Speed_;
    QString TimeToComplete_;
public:
    explicit FinishedJob ();
    FinishedJob (const JobRepresentation& jr, QObject *parent = 0);
    virtual ~FinishedJob ();
    const QString& GetURL () const;
    const QString& GetLocal () const;
    ImpBase::length_t GetSize () const;
    const QString& GetSpeed () const;
    const QString& GetTimeToComplete () const;
    void FeedVariantList (const QVariantList&);
    static FinishedJob* FromVariantList (const QVariantList&);
};

#endif

