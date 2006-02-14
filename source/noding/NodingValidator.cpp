/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding.h>

namespace geos {

void
NodingValidator::checkValid()
{
	checkNoInteriorPointsSame();
	checkProperIntersections();
}


void
NodingValidator::checkProperIntersections()
{
	for (int i=0; i<(int)segStrings->size();i++) {
		SegmentString *ss0=(*segStrings)[i];
		for (int j=0; j<(int)segStrings->size();j++) {
			SegmentString *ss1=(*segStrings)[j];
			checkProperIntersections(ss0, ss1);
		}
	}
}

void
NodingValidator::checkProperIntersections(const SegmentString *ss0, const SegmentString *ss1)
{
	const CoordinateSequence *pts0=ss0->getCoordinates();
	const CoordinateSequence *pts1=ss1->getCoordinates();
	unsigned int npts0=pts0->getSize();
	unsigned int npts1=pts1->getSize();

	for (unsigned int i0=0; i0<npts0-1; i0++) {
		for (unsigned int i1=0; i1<npts1-1; i1++) {
			checkProperIntersections(ss0, i0, ss1, i1);
		}
	}
}

void
NodingValidator::checkProperIntersections(const SegmentString *e0, int segIndex0, const SegmentString *e1, int segIndex1)
{
	if (e0 == e1 && segIndex0 == segIndex1) return;

	//numTests++;
	const Coordinate& p00=e0->getCoordinates()->getAt(segIndex0);
	const Coordinate& p01=e0->getCoordinates()->getAt(segIndex0+1);
	const Coordinate& p10=e1->getCoordinates()->getAt(segIndex1);
	const Coordinate& p11=e1->getCoordinates()->getAt(segIndex1+1);
	li.computeIntersection(p00, p01, p10, p11);
	if (li.hasIntersection()) {
		if (   li.isProper()
			|| hasInteriorIntersection(li, p00, p01)
			|| hasInteriorIntersection(li, p00, p01)) {
				throw  GEOSException("found non-noded intersection at "+ p00.toString() + "-" + p01.toString()+ " and "+ p10.toString() + "-" + p11.toString());
		}
	}
}

/**
 * @return true if there is an intersection point which is not an
 * endpoint of the segment p0-p1
 */
bool
NodingValidator::hasInteriorIntersection(const LineIntersector& aLi,
		const Coordinate& p0, const Coordinate& p1)
{
	for (int i=0, n=aLi.getIntersectionNum(); i<n; i++)
	{
		const Coordinate &intPt=aLi.getIntersection(i);
		if (!(intPt==p0 || intPt==p1))
			return true;
	}
	return false;
}

void
NodingValidator::checkNoInteriorPointsSame()
{
	for (unsigned int i=0; i<segStrings->size(); ++i) {
		const SegmentString *ss0=(*segStrings)[i];
		const CoordinateSequence *pts=ss0->getCoordinates();
		checkNoInteriorPointsSame(pts->getAt(0), segStrings);
		checkNoInteriorPointsSame(pts->getAt(pts->getSize()-1),
			segStrings);
	}
}

void
NodingValidator::checkNoInteriorPointsSame(const Coordinate& testPt,
		const SegmentString::NonConstVect* aSegStrings)
{
	unsigned int nSegStrings=segStrings->size();
	for (unsigned int i=0; i<nSegStrings; ++i) {
			const SegmentString *ss0=(*segStrings)[i];
			const CoordinateSequence *pts=ss0->getCoordinates();
			unsigned int npts=pts->getSize();
			for (unsigned int j=1; j<npts-1; ++j)
			{
				if (pts->getAt(j)==testPt)
					throw  GEOSException("found bad noding at pt " + testPt.toString());
			}
	}
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.8  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.7  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.6  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.5  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.4  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

