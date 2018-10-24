/***************************************************************************
               ProjectionSelector.h  - CoordinateSystems Select
                             -------------------
    begin                : Dec 2014/8
    copyright            : (C) 2014 ctb
    email                : fchentianbao@126.com
 ***************************************************************************/
#ifndef PROJECTIONSELECTOR_H
#define PROJECTIONSELECTOR_H

#include <QSqlDatabase>

#include <QtWidgets/QDialog>

class QTreeWidgetItem;

namespace Ui {
class ProjectionSelector;
}

class ProjectionSelector : public QDialog
{
    Q_OBJECT

public:

    explicit ProjectionSelector(QWidget *parent = 0);
    ~ProjectionSelector();

    enum columns { NAME_COLUMN, AUTHID_COLUMN, QGIS_CRS_ID_COLUMN, NONE };

    QString pro4String() const;

    QString projectName() const;

private slots:
    void on_cbxHideDeprecated_stateChanged();
    void on_lstCoordinateSystems_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *);
    void on_leSearch_textChanged( const QString & );

private:
    void initCRSTree();

    QString ogcWmsCrsFilterAsSqlExpression(QSet<QString> *crsFilter);
    void loadUserCrsList( QSet<QString> *crsFilter = 0 );

    void loadCrsList( QSet<QString> *crsFilter = 0 );

    long selectedCrsId();
    QString selectedName();
    QString selectedProj4String();

    void hideDeprecated(QTreeWidgetItem *item);

private:
    Ui::ProjectionSelector *ui;

    QSqlDatabase db;
    QString mSrsDatabaseFileName;
    long mselectCrsId;

    //! User defined projections node
    QTreeWidgetItem *mUserProjList;
    //! GEOGCS node
    QTreeWidgetItem *mGeoList;
    //! PROJCS node
    QTreeWidgetItem *mProjList;
};

#endif // PROJECTIONSELECTOR_H
