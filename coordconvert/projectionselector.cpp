#include "projectionselector.h"
#include "ui_projectionselector.h"
#include <QFileInfo>
#include <QDebug>

#include <QTextCodec>

#pragma execution_character_set("utf-8")

#include <QSqlQuery>

const int USER_CRS_START_ID = 100000;

ProjectionSelector::ProjectionSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectionSelector),
    db(QSqlDatabase::addDatabase("QSQLITE")),
    mSrsDatabaseFileName(QApplication::applicationDirPath() + "/srs.db"),
    mselectCrsId(0)
{

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));


    ui->setupUi(this);

    db.setDatabaseName(mSrsDatabaseFileName);
    if (!db.open())
    {
        qWarning() << "open the database containing the spatial reference data failed ";
    }

    this->setWindowTitle(QString::fromLocal8Bit("坐标转换"));

    initCRSTree();

    loadCrsList();

    connect(ui->lstCoordinateSystems, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(on_lstCoordinateSystems_currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    connect(ui->cbxHideDeprecated, SIGNAL(clicked()), this,
            SLOT(on_cbxHideDeprecated_stateChanged()));

    connect(ui->strFilter, SIGNAL(textChanged(QString)), this,
            SLOT(on_leSearch_textChanged(QString)));
}

ProjectionSelector::~ProjectionSelector()
{
    // close the database
    db.close();

    delete ui;
}

QString ProjectionSelector::pro4String() const
{
    return ui->teProjection->toPlainText();
}

QString ProjectionSelector::projectName() const
{
    return ui->teSelected->text();
}

void ProjectionSelector::initCRSTree()
{
    ui->lstCoordinateSystems->setAlternatingRowColors(true);
    ui->lstCoordinateSystems->setUniformRowHeights(true);
    ui->lstCoordinateSystems->setColumnCount(3);
	QStringList headerLabels;
    headerLabels << QString::fromLocal8Bit("坐标参照系") << QString::fromLocal8Bit("管理机构标识符") << "" ;

    ui->lstCoordinateSystems->setHeaderLabels(headerLabels);

    ui->lstCoordinateSystems->header()->setSectionResizeMode( NAME_COLUMN, QHeaderView::Stretch );
    ui->lstCoordinateSystems->header()->resizeSection( QGIS_CRS_ID_COLUMN, 0 );
    ui->lstCoordinateSystems->header()->setSectionResizeMode( QGIS_CRS_ID_COLUMN, QHeaderView::Fixed );

    // Hide (internal) ID column
    ui->lstCoordinateSystems->setColumnHidden( QGIS_CRS_ID_COLUMN, true );
}

QString ProjectionSelector::ogcWmsCrsFilterAsSqlExpression( QSet<QString> * crsFilter )
{
    QString sqlExpression = "1";             // it's "SQL" for "true"
    QMap<QString, QStringList> authParts;

    if ( !crsFilter )
        return sqlExpression;

    /*
     Ref: WMS 1.3.0, section 6.7.3 "Layer CRS":

     Every Layer CRS has an identifier that is a character string. Two types of
     Layer CRS identifiers are permitted: "label" and "URL" identifiers:

     Label: The identifier includes a namespace prefix, a colon, a numeric or
        string code, and in some instances a comma followed by additional
        parameters. This International Standard defines three namespaces:
        CRS, EpsgCrsId and AUTO2 [...]

     URL: The identifier is a fully-qualified Uniform Resource Locator that
        references a publicly-accessible file containing a definition of the CRS
        that is compliant with ISO 19111.
  */

    // iterate through all incoming CRSs

    foreach ( QString auth_id, crsFilter->values() )
    {
        QStringList parts = auth_id.split( ":" );

        if ( parts.size() < 2 )
            continue;

        authParts[ parts.at( 0 ).toUpper()].append( parts.at( 1 ).toUpper() );
    }

    if ( authParts.isEmpty() )
        return sqlExpression;

    if ( authParts.size() > 0 )
    {
        QString prefix = " AND (";
        foreach ( QString auth_name, authParts.keys() )
        {
            sqlExpression += QString( "%1(upper(auth_name)='%2' AND upper(auth_id) IN ('%3'))" )
                    .arg( prefix )
                    .arg( auth_name )
                    .arg( authParts[auth_name].join( "','" ) );
            prefix = " OR ";
        }
        sqlExpression += ")";
    }

    qDebug() << ( "exiting with '" + sqlExpression + "'." );

    return sqlExpression;
}

void ProjectionSelector::loadCrsList(QSet<QString> *crsFilter)
{

    // convert our Coordinate Reference System filter into the SQL expression
    QString sqlFilter = ogcWmsCrsFilterAsSqlExpression( crsFilter );

    // Create the top-level nodes for the list view of projections
    // Make in an italic font to distinguish them from real projections
    //
    // Geographic coordinate system node
    QStringList roottext;
    roottext << QString::fromLocal8Bit( "地理坐标系统" );
    mGeoList = new QTreeWidgetItem( ui->lstCoordinateSystems, roottext );

    QFont fontTemp = mGeoList->font( 0 );
    fontTemp.setItalic( true );
    fontTemp.setBold( true );
    mGeoList->setFont( 0, fontTemp );
    mGeoList->setIcon( 0, QIcon( ":/image/images/geographic.png" ) );

    // Projected coordinate system node
    QStringList rootPrjtext;
    rootPrjtext << QString::fromLocal8Bit( "投影坐标系统" );
    mProjList = new QTreeWidgetItem( ui->lstCoordinateSystems, rootPrjtext );

    fontTemp = mProjList->font( 0 );
    fontTemp.setItalic( true );
    fontTemp.setBold( true );
    mProjList->setFont( 0, fontTemp );
    mProjList->setIcon( 0, QIcon( ":/image/images/transformed.png" ));

    //bail out in case the projections db does not exist
    //this is necessary in case the pc is running linux with a
    //read only filesystem because otherwise sqlite will try
    //to create the db file on the fly

    if ( !QFileInfo( mSrsDatabaseFileName ).exists() )
    {
        return;
    }

    // prepare the sql statement
    // get total count of records in the projection table
    QString sql = "select count(*) from tbl_srs";

    QSqlQuery sqlQurey(sql, db);
    if (!sqlQurey.next())
    {
        qDebug() <<"database is not data";
        return ;
    }

    // Set up the query to retrieve the projection information needed to populate the list
    //note I am giving the full field names for clarity here and in case someone
    //changes the underlying view TS
    sql = QString( "select description, srs_id, upper(auth_name||':'||auth_id), is_geo, name, parameters, deprecated from vw_srs where %1 order by name,description" )
            .arg( sqlFilter );

    // XXX Need to free memory from the error msg if one is set
    if (sqlQurey.exec(sql))
    {
        QTreeWidgetItem *newItem;
        // Cache some stuff to speed up creating of the list of projected
        // spatial reference systems
        QString previousSrsType( "" );
        QTreeWidgetItem* previousSrsTypeNode = 0;

        while ( sqlQurey.next() )
        {
            // check to see if the srs is geographic
            int isGeo = sqlQurey.value(3 ).toInt();
            if ( isGeo )
            {
                // this is a geographic coordinate system
                // Add it to the tree (field 0)
                newItem = new QTreeWidgetItem( mGeoList, QStringList( sqlQurey.value(0 ).toString() ) );

                // display the authority name (field 2) in the second column of the list view
                newItem->setText( AUTHID_COLUMN, sqlQurey.value(2).toString()  );

                // display the qgis srs_id (field 1) in the third column of the list view
                newItem->setText( QGIS_CRS_ID_COLUMN, sqlQurey.value(1 ).toString() );
            }
            else
            {
                // This is a projected srs
                QTreeWidgetItem *node;
                QString srsType = sqlQurey.value(4 ).toString() ;
                // Find the node for this type and add the projection to it
                // If the node doesn't exist, create it
                if ( srsType == previousSrsType )
                {
                    node = previousSrsTypeNode;
                }
                else
                { // Different from last one, need to search
                    QList<QTreeWidgetItem*> nodes = ui->lstCoordinateSystems->findItems( srsType, Qt::MatchExactly | Qt::MatchRecursive, NAME_COLUMN );
                    if ( nodes.count() == 0 )
                    {
                        // the node doesn't exist -- create it
                        // Make in an italic font to distinguish them from real projections
                        node = new QTreeWidgetItem( mProjList, QStringList( srsType ) );
                        QFont fontTemp = node->font( 0 );
                        fontTemp.setItalic( true );
                        node->setFont( 0, fontTemp );
                    }
                    else
                    {
                        node = nodes.first();
                    }
                    // Update the cache.
                    previousSrsType = srsType;
                    previousSrsTypeNode = node;
                }
                // add the item, setting the projection name in the first column of the list view
                newItem = new QTreeWidgetItem( node, QStringList( sqlQurey.value(0 ).toString() )  );
                // display the authority id (field 2) in the second column of the list view
                newItem->setText( AUTHID_COLUMN, sqlQurey.value( 2 ).toString()  );
                // display the qgis srs_id (field 1) in the third column of the list view
                newItem->setText( QGIS_CRS_ID_COLUMN, sqlQurey.value( 1 ).toString()  );
                // expand also parent node
                newItem->parent()->setExpanded( true );
            }

            // display the qgis deprecated in the user data of the item
            newItem->setData( 0, Qt::UserRole, sqlQurey.value(6 ).toString()  );
            newItem->setHidden( ui->cbxHideDeprecated->isChecked() );
        }
        mProjList->setExpanded( true );
    }
}

long ProjectionSelector::selectedCrsId()
{
    QTreeWidgetItem* item = ui->lstCoordinateSystems->currentItem();

    if ( item && !item->text( QGIS_CRS_ID_COLUMN ).isEmpty() )
        return ui->lstCoordinateSystems->currentItem()->text( QGIS_CRS_ID_COLUMN ).toLong();
    else
        return 0;
}

//note this line just returns the projection name!
QString ProjectionSelector::selectedName()
{
    // return the selected wkt name from the list view
    QTreeWidgetItem *lvi = ui->lstCoordinateSystems->currentItem();
    return lvi ? lvi->text( NAME_COLUMN ) : QString::null;
}

// Returns the whole proj4 string for the selected projection node
QString ProjectionSelector::selectedProj4String()
{
    // Only return the projection if there is a node in the tree
    // selected that has an srid. This prevents error if the user
    // selects a top-level node rather than an actual coordinate
    // system
    //
    // Get the selected node
    QTreeWidgetItem *item = ui->lstCoordinateSystems->currentItem();
    if ( !item || item->text( QGIS_CRS_ID_COLUMN ).isEmpty() )
        return QString("");

    QString srsId = item->text( QGIS_CRS_ID_COLUMN );

    //
    // Determine if this is a user projection or a system on
    // user projection defs all have srs_id >= 100000
    //
    QString databaseFileName;
    if ( srsId.toLong() >= USER_CRS_START_ID )
    {
        return QString("");
    }
    else //must be a system projection then
    {
        databaseFileName = mSrsDatabaseFileName;
    }

    // prepare the sql statement
    QString sql = QString("select parameters from tbl_srs where srs_id=%1").arg(srsId);
    QSqlQuery sqlQurey(sql, db);
    QString projString;
    if ( sqlQurey.next() )
    {
        projString = sqlQurey.value( 0 ).toString() ;
    }
    else
    {
        qDebug()<<"Selection error. sql="<<sql;
    }

    Q_ASSERT( !projString.isEmpty() );
    return projString;
}


// New coordinate system selected from the list
void ProjectionSelector::on_lstCoordinateSystems_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
    if ( !current )
    {
        qDebug() << ( "no current item" );
        return;
    }

    ui->lstCoordinateSystems->scrollToItem( current );

    // If the item has children, it's not an end node in the tree, and
    // hence is just a grouping thingy, not an actual CRS.
    if ( current->childCount() == 0 )
    {
        // Found a real CRS
        mselectCrsId =  selectedCrsId();

        ui->teProjection->setText( selectedProj4String() );
        ui->teSelected->setText( selectedName() );
    }
    else
    {
        // Not an CRS - remove the highlight so the user doesn't get too confused
        current->setSelected( false );
        ui->teProjection->setText( "" );
        ui->teSelected->setText( "" );
    }
}

void ProjectionSelector::hideDeprecated( QTreeWidgetItem *item )
{
    if ( item->data( 0, Qt::UserRole ).toBool() )
    {
        item->setHidden( ui->cbxHideDeprecated->isChecked() );
        if ( item->isSelected() && item->isHidden() )
        {
            item->setSelected( false );
            ui->teProjection->setText( "" );
            ui->teSelected->setText( "" );
        }
    }

    for ( int i = 0; i < item->childCount(); i++ )
        hideDeprecated( item->child( i ) );
}

void ProjectionSelector::on_cbxHideDeprecated_stateChanged()
{
    for ( int i = 0; i < ui->lstCoordinateSystems->topLevelItemCount(); i++ )
        hideDeprecated( ui->lstCoordinateSystems->topLevelItem( i ) );
}

void ProjectionSelector::on_leSearch_textChanged( const QString & theFilterTxt )
{
    QString filterTxt = theFilterTxt;
    filterTxt.replace( QRegExp( "\\s+" ), ".*" );
    QRegExp re( filterTxt, Qt::CaseInsensitive );


    // filter crs's
    QTreeWidgetItemIterator it( ui->lstCoordinateSystems );
    while ( *it )
    {
        if (( *it )->childCount() == 0 ) // it's an end node aka a projection
        {
            if (( *it )->text( NAME_COLUMN ).contains( re )
                    || ( *it )->text( AUTHID_COLUMN ).contains( re )
                    )
            {
                ( *it )->setHidden( false );
                QTreeWidgetItem * parent = ( *it )->parent();
                while ( parent )
                {
                    parent->setExpanded( true );
                    parent->setHidden( false );
                    parent = parent->parent();
                }
            }
            else
            {
                ( *it )->setHidden( true );
            }
        }
        else
        {
            ( *it )->setHidden( true );
        }
        ++it;
    }
}
