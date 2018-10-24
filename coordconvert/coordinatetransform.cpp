#include "coordinatetransform.h"
#include <QDebug>
CoordinateTransform::CoordinateTransform(const QString srcCRS, const QString &desCRS)
    : mShortCircuit(false),mSourceCRS(srcCRS), mDestCRS(desCRS)
{
    if (mSourceCRS == desCRS)
        mShortCircuit = true;

    mSourceProjection = pj_init_plus( mSourceCRS.toUtf8() );
    mDestinationProjection = pj_init_plus( mDestCRS.toUtf8() );

    mInitialisedFlag = true;
    if ( !mDestinationProjection )
    {
      mInitialisedFlag = false;
    }
    if ( !mSourceProjection )
    {
      mInitialisedFlag = false;
    }
}

QPointF CoordinateTransform::transform(const QPointF p, CoordinateTransform::TransformDirection direction) const
{
    if ( mShortCircuit || !mInitialisedFlag )
      return p;
    // transform x
    double x = p.x();
    double y = p.y();
    double z = 0.0;
    try
    {
      transformCoords( 1, &x, &y, &z, direction );
    }
    catch ( ... )
    {
      // rethrow the exception
      qDebug() << ( "rethrowing exception" );
      throw;
    }

    return QPointF( x, y );
}

QPointF CoordinateTransform::transform(const double x, const double y, CoordinateTransform::TransformDirection direction) const
{
    try
      {
        return transform( QPointF( x, y ), direction );
      }
      catch (...)
      {
        // rethrow the exception
        qDebug() << ( "rethrowing exception" );
        throw;
      }
}

void CoordinateTransform::transformInPlace(QVector<double> &x, QVector<double> &y, QVector<double> &z, CoordinateTransform::TransformDirection direction) const
{
    if ( mShortCircuit || !mInitialisedFlag )
       return;

     Q_ASSERT( x.size() == y.size() );

     // Apparently, if one has a std::vector, it is valid to use the
     // address of the first element in the vector as a pointer to an
     // array of the vectors data, and hence easily interface with code
     // that wants C-style arrays.

     try
     {
       transformCoords( x.size(), &x[0], &y[0], &z[0], direction );
     }
    catch ( ... )
    {
        // rethrow the exception
        qDebug()<< ( "rethrowing exception" );
        throw;
    }
}

void CoordinateTransform::transformPolygon(QPolygonF &poly, CoordinateTransform::TransformDirection direction) const
{
    if ( mShortCircuit || !mInitialisedFlag )
     {
       return;
     }

     //create x, y arrays
     int nVertices = poly.size();

     QVector<double> x( nVertices );
     QVector<double> y( nVertices );
     QVector<double> z( nVertices );

     for ( int i = 0; i < nVertices; ++i )
     {
       const QPointF& pt = poly.at( i );
       x[i] = pt.x();
       y[i] = pt.y();
       z[i] = 0;
     }

     try
     {
       transformCoords( nVertices, x.data(), y.data(), z.data(), direction );
     }
     catch ( ... )
     {
       // rethrow the exception
       qDebug()<< ( "rethrowing exception" );
       throw;
     }

     for ( int i = 0; i < nVertices; ++i )
     {
       QPointF& pt = poly[i];
       pt.rx() = x[i];
       pt.ry() = y[i];
     }
}

void CoordinateTransform::transformCoords(const int &numPoints, double *x, double *y, double *z, CoordinateTransform::TransformDirection direction) const
{
    double xorg = *x;
    double yorg = *y;
    qDebug() << ( QString( "[[[[[[ Number of points to transform: %1 ]]]]]]" ).arg( numPoints ) );


    // use proj4 to do the transform
    QString dir;
    // if the source/destination projection is lat/long, convert the points to radians
    // prior to transforming
    if (( pj_is_latlong( mDestinationProjection ) && ( direction == ReverseTransform ) )
        || ( pj_is_latlong( mSourceProjection ) && ( direction == ForwardTransform ) ) )
    {
      for ( int i = 0; i < numPoints; ++i )
      {
        x[i] *= DEG_TO_RAD;
        y[i] *= DEG_TO_RAD;
        z[i] *= DEG_TO_RAD;
      }

    }
    int projResult;
    if ( direction == ReverseTransform )
    {
      projResult = pj_transform( mDestinationProjection, mSourceProjection, numPoints, 0, x, y, z );
    }
    else
    {
      Q_ASSERT( mSourceProjection != 0 );
      Q_ASSERT( mDestinationProjection != 0 );
      projResult = pj_transform( mSourceProjection, mDestinationProjection, numPoints, 0, x, y, z );
    }

    if ( projResult != 0 )
    {
      //something bad happened....
      QString points;

      for ( int i = 0; i < numPoints; ++i )
      {
        if ( direction == ForwardTransform )
        {
          points += QString( "(%1, %2)\n" ).arg( x[i], 0, 'f' ).arg( y[i], 0, 'f' );
        }
        else
        {
          points += QString( "(%1, %2)\n" ).arg( x[i] * RAD_TO_DEG, 0, 'f' ).arg( y[i] * RAD_TO_DEG, 0, 'f' );
        }
      }

      dir = ( direction == ForwardTransform ) ? QObject::tr( "forward transform" ) : QObject::tr( "inverse transform" );

      QString msg = QObject::tr( "%1 of\n"
                        "%2"
                        "Error: %3" )
                    .arg( dir )
                    .arg( points )
                    .arg( QString::fromUtf8( pj_strerrno( projResult ) ) );

      qDebug()<< ( "Projection failed emitting invalid transform signal: " + msg );

    }

    // if the result is lat/long, convert the results from radians back
    // to degrees
    if (( pj_is_latlong( mDestinationProjection ) && ( direction == ForwardTransform ) )
        || ( pj_is_latlong( mSourceProjection ) && ( direction == ReverseTransform ) ) )
    {
      for ( int i = 0; i < numPoints; ++i )
      {
        x[i] *= RAD_TO_DEG;
        y[i] *= RAD_TO_DEG;
        z[i] *= RAD_TO_DEG;
      }
    }

    qDebug() << ( QString( "[[[[[[ Projected %1, %2 to %3, %4 ]]]]]]" )
                 .arg( xorg, 0, 'g', 15 ).arg( yorg, 0, 'g', 15 )
                 .arg( *x, 0, 'g', 15 ).arg( *y, 0, 'g', 15 ) );
}
