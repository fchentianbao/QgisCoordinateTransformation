/***************************************************************************
               CoordinateTransform.h  - Coordinate Transforms
                             -------------------
    begin                : Dec 2014/8
    copyright            : (C) 2014 ctb
    email                : fchentianbao@126.com
 ***************************************************************************/
#ifndef COORDINATETRANSFORM_H
#define COORDINATETRANSFORM_H

#include <QPoint>
#include <QPolygonF>
#include <QString>
#include <proj_api.h>

class CoordinateTransform
{
public:
    CoordinateTransform(const QString srcCRS, const QString &desCRS);


    //! Enum used to indicate the direction (forward or inverse) of the transform
     enum TransformDirection
     {
       ForwardTransform,     /*!< Transform from source to destination CRS. */
       ReverseTransform      /*!< Transform from destination to source CRS. */
     };



     /*! Transform the point from Source Coordinate System to Destination Coordinate System
      * @param p Point to transform
      * @param direction TransformDirection (defaults to ForwardTransform)
      * @return QPointF in Destination Coordinate System
      */
     QPointF transform( const QPointF p, TransformDirection direction = ForwardTransform ) const;

     /*! Transform the point specified by x,y from Source Coordinate System to Destination Coordinate System
      * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
      * otherwise points are transformed from map canvas CS to layerCS.
      * @param x x cordinate of point to transform
      * @param y y coordinate of point to transform
      * @param direction TransformDirection (defaults to ForwardTransform)
      * @return QPointF in Destination Coordinate System
      */
     QPointF transform( const double x, const double y, TransformDirection direction = ForwardTransform ) const;

     //! @note not available in python bindings
     void transformInPlace( QVector<double>& x, QVector<double>& y, QVector<double>& z,
                            TransformDirection direction = ForwardTransform ) const;

     void transformPolygon( QPolygonF& poly, TransformDirection direction = ForwardTransform ) const;

private:

     void transformCoords(const int& numPoints,
                           double *x, double *y, double *z, TransformDirection direction) const;
private:


    /*!
     * Flag to indicate that the source and destination coordinate systems are
     * equal and not transformation needs to be done
     */
    bool mShortCircuit;

    /*!
     * flag to show whether the transform is properly initialised or not
     */
    bool mInitialisedFlag;

    /*!
     * CoordinateReferenceSystem of the source (layer) coordinate system
     */
    QString mSourceCRS;

    /*!
     * CoordinateReferenceSystem of the destination (map canvas) coordinate system
     */
    QString mDestCRS;

    /*!
     * Proj4 data structure of the source projection (layer coordinate system)
     */
    projPJ mSourceProjection;

    /*!
     * Proj4 data structure of the destination projection (map canvas coordinate system)
     */
    projPJ mDestinationProjection;
};

#endif // COORDINATETRANSFORM_H
