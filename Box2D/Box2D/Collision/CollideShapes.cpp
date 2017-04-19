/*
 * Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/Box2D
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <Box2D/Collision/Collision.hpp>
#include <Box2D/Collision/ShapeSeparation.hpp>
#include <Box2D/Collision/ReferenceFace.hpp>
#include <Box2D/Collision/EdgeInfo.hpp>
#include <Box2D/Collision/DistanceProxy.hpp>
#include <Box2D/Collision/CollideShapes.hpp>
#include <Box2D/Collision/Shapes/CircleShape.hpp>
#include <Box2D/Collision/Shapes/PolygonShape.hpp>
#include <Box2D/Collision/Shapes/EdgeShape.hpp>
#include <array>

#define BOX2D_MAGIC(x) (x)

using namespace box2d;

static inline IndexSeparation GetPolygonSeparation(const PolygonShape& polygon, const EdgeInfo& edge)
{
	auto max_s = -MaxFloat * Meter;
	auto index = IndexSeparation::InvalidIndex;
	{
		const auto totalRadius = GetVertexRadius(polygon) + edge.GetVertexRadius();
		const auto perp = GetRevPerpendicular(edge.GetNormal());
		const auto count = polygon.GetVertexCount();
		const auto edgeVertex1 = edge.GetVertex1();
		const auto edgeVertex2 = edge.GetVertex2();
		for (auto i = decltype(count){0}; i < count; ++i)
		{
			const auto polygonNormal = -polygon.GetNormal(i);
			const auto polygonVertex = polygon.GetVertex(i);
			const auto s = Min(Dot(polygonNormal, polygonVertex - edgeVertex1),
							   Dot(polygonNormal, polygonVertex - edgeVertex2));
			
			if (s > totalRadius) // No collision
			{
				return IndexSeparation{StripUnit(s), i};
			}
			
			// Adjacency
			if (Dot(polygonNormal, perp) >= 0)
			{
				if (Dot(polygonNormal - edge.GetUpperLimit(), edge.GetNormal()) < BOX2D_MAGIC(-StripUnit(DefaultAngularSlop)))
				{
					continue;
				}
			}
			else
			{
				if (Dot(polygonNormal - edge.GetLowerLimit(), edge.GetNormal()) < BOX2D_MAGIC(-StripUnit(DefaultAngularSlop)))
				{
					continue;
				}
			}
			
			if (max_s < s)
			{
				max_s = s;
				index = i;
			}
		}
	}
	return IndexSeparation{StripUnit(max_s), index};
}

static inline IndexPairSeparation GetMaxSeparation(const DistanceProxy& shape1, const Transformation& xf1,
												   const DistanceProxy& shape2, const Transformation& xf2,
												   Length stop = MaxFloat * Meter)
{
	return GetMaxSeparation(shape1.GetVertices(), shape1.GetNormals(), xf1, shape2.GetVertices(), xf2, stop);
}

/// Gets the incident edge clip list.
/// @param indexA Index of the incident edge of shape A.
/// @param normalA Normal of the incident edge of shape A.
/// @param xfA Transformation for the incident edge of shape A to world coordinates.
/// @param shapeB Shape B to return two consecutive vertices from.
/// @param xf2 Transformation for converting shape 2 vertices to world coordinates.
/// @return Zero or two face-vertex clip vertices in world coordinates.
static ClipList GetIncidentEdgeClipList(ContactFeature::index_t indexA, UnitVec2 normalA, const Transformation& xfA,
										const PolygonShape& shapeB, const Transformation& xf2)
{
	assert(shapeB.GetVertexCount() > 1);

	// Find the incident edge on shape B.
	const auto separation = [&]() {
		// Get the normal of the reference edge in shape2's frame.
		const auto rel_normal1 = InverseRotate(Rotate(normalA, xfA.q), xf2.q);
		return GetMostAntiParallelSeparation(shapeB.GetNormals(), rel_normal1, UnitVec2::GetZero());
	}();
	
	// Build the clip list for the incident edge.
	const auto i1 = separation.index;
	const auto i2 = GetModuloNext(i1, shapeB.GetVertexCount());
	return ClipList{
		ClipVertex{Transform(shapeB.GetVertex(i1), xf2), GetFaceVertexContactFeature(indexA, i1)},
		ClipVertex{Transform(shapeB.GetVertex(i2), xf2), GetFaceVertexContactFeature(indexA, i2)}
	};
}

static ClipList GetIncidentEdgeClipList(ContactFeature::index_t indexA, UnitVec2 normalA, const Transformation& xfA,
										const PolygonShape& shapeB, const Transformation& xfB,
										ContactFeature::index_t indexB)
{
	const auto edge0 = GetModuloPrev(indexB, shapeB.GetVertexCount());
	const auto edge1 = indexB;
	const auto rel_normalA = InverseRotate(Rotate(normalA, xfA.q), xfB.q);
	const auto normal0 = shapeB.GetNormal(edge0);
	const auto normal1 = shapeB.GetNormal(edge1);
	const auto s0 = Dot(rel_normalA, normal0);
	const auto s1 = Dot(rel_normalA, normal1);
	const auto i1 = (s0 < s1)? edge0: edge1;
	const auto i2 = GetModuloNext(i1, shapeB.GetVertexCount());
	return ClipList{
		ClipVertex{Transform(shapeB.GetVertex(i1), xfB), GetFaceVertexContactFeature(indexA, i1)},
		ClipVertex{Transform(shapeB.GetVertex(i2), xfB), GetFaceVertexContactFeature(indexA, i2)}
	};
}

/// Gets the incident edge clip list.
/// @return Zero one or two face-vertex clip vertices.
static ClipList GetIncidentEdgeClipList(ContactFeature::index_t index1, const UnitVec2 normal1, const PolygonShape& shape2)
{
	const auto separation = GetMostAntiParallelSeparation(shape2.GetNormals(), normal1, UnitVec2::GetZero());
	const auto i1 = separation.index;
	const auto i2 = GetModuloNext(i1, shape2.GetVertexCount());
#if 0
	return ClipList{
		ClipVertex{shape2.GetVertex(i1), GetFaceVertexContactFeature(0, i1)},
		ClipVertex{shape2.GetVertex(i2), GetFaceVertexContactFeature(0, i2)}
	};
#else
	return ClipList{
		ClipVertex{shape2.GetVertex(i1), GetFaceVertexContactFeature(index1, i1)},
		ClipVertex{shape2.GetVertex(i2), GetFaceVertexContactFeature(index1, i2)}
	};
#endif
}

static inline ClipList GetClipPoints(IndexSeparation::index_type iv1, Length sideOffset1, UnitVec2 normal1,
									 IndexSeparation::index_type iv2, Length sideOffset2, UnitVec2 normal2,
									 const ClipList& incidentEdge)
{
	const auto points = ClipSegmentToLine(incidentEdge, normal1, sideOffset1, iv1);
	return ClipSegmentToLine(points, normal2, sideOffset2, iv2);
}

static Manifold GetManifoldFaceA(const EdgeInfo& edgeInfo,
								 const PolygonShape& localShapeB,
								 const Transformation& xf)
{
	const auto ref_face = GetReferenceFace(edgeInfo);

	// const auto incidentEdge = GetIncidentEdgeClipList(0, edgeInfo.GetNormal(), localShapeB);
	const auto incidentEdge = GetIncidentEdgeClipList(ref_face.GetIndex1(), edgeInfo.GetNormal(), localShapeB);
	
	// Clip incident edge against extruded edge1 side edges.
	const auto clipPoints = GetClipPoints(ref_face.GetIndex1(), ref_face.GetOffset1(), ref_face.GetNormal1(),
										  ref_face.GetIndex2(), ref_face.GetOffset2(), ref_face.GetNormal2(),
										  incidentEdge);
	if (clipPoints.size() != 2)
	{
		return Manifold{};
	}

	auto manifold = Manifold::GetForFaceA(ref_face.GetNormal(), 0, ref_face.GetVertex1());
	const auto totalRadius = edgeInfo.GetVertexRadius() + GetVertexRadius(localShapeB);
	for (auto i = decltype(clipPoints.size()){0}; i < clipPoints.size(); ++i)
	{
		const auto separation = Dot(ref_face.GetNormal(), clipPoints[i].v - ref_face.GetVertex1());
		if (separation <= totalRadius)
		{
			manifold.AddPoint(clipPoints[i].cf.typeB, clipPoints[i].cf.indexB, InverseTransform(clipPoints[i].v, xf));
		}
	}
	return manifold;
}

static Manifold GetManifoldFaceB(const EdgeInfo& edgeInfo,
								 const PolygonShape& shapeB,
								 const PolygonShape& localShapeB,
								 PolygonShape::vertex_count_t index)
{
#if 1
	// XXX Is this code really correct??
	const auto incidentEdge = ClipList{
		ClipVertex{edgeInfo.GetVertex1(), GetVertexFaceContactFeature(0, index)}, // 0, index originally
		ClipVertex{edgeInfo.GetVertex2(), GetVertexFaceContactFeature(1, index)} // also 0, index originally
	};
#else
	const auto incidentEdge = ClipList{
		ClipVertex{edgeInfo.GetVertex1(), GetFaceVertexContactFeature(index, 0)},
		ClipVertex{edgeInfo.GetVertex2(), GetFaceVertexContactFeature(index, 1)}
	};
#endif
	
	const auto ref_face = GetReferenceFace(localShapeB, index);

	// Clip incident edge against extruded edge1 side edges.
	const auto clipPoints = GetClipPoints(ref_face.GetIndex1(), ref_face.GetOffset1(), ref_face.GetNormal1(),
										  ref_face.GetIndex2(), ref_face.GetOffset2(), ref_face.GetNormal2(),
										  incidentEdge);
	if (clipPoints.size() != 2)
	{
		return Manifold{};
	}
	
	//auto manifold = Manifold::GetForFaceB(shapeB.GetNormal(ref_face.GetIndex1()), shapeB.GetVertex(ref_face.GetIndex1()));
	auto manifold = Manifold::GetForFaceB(shapeB.GetNormal(ref_face.GetIndex1()), ref_face.GetIndex1(), shapeB.GetVertex(ref_face.GetIndex1()));
	const auto totalRadius = edgeInfo.GetVertexRadius() + GetVertexRadius(shapeB);
	for (auto i = decltype(clipPoints.size()){0}; i < clipPoints.size(); ++i)
	{
		const auto separation = Dot(ref_face.GetNormal(), clipPoints[i].v - ref_face.GetVertex1());
		if (separation <= totalRadius)
		{
			manifold.AddPoint(clipPoints[i].cf.typeA, clipPoints[i].cf.indexA, clipPoints[i].v);
			//manifold.AddPoint(Manifold::Point{clipPoints[i].v, Flip(clipPoints[i].cf)});
		}
	}
	return manifold;
}

/// @param shape1 Shape 1. This should be shape A for face-A type manifold or shape B for face-B type manifold.
/// @param xf1 Transform 1. This should be transform A for face-A type manifold or transform B for face-B type manifold.
/// @param idx1 Index 1. This should be the index of the vertex and normal of shape1 that had the maximal
///    separation distance from any vertex in shape2.
/// @param idx2 Index 2. This is the index of the vertex of shape2 that had the maximal separation distance
//     from the edge of shape1 identified by idx1.
static inline Manifold GetFaceManifold(const Manifold::Type type,
									   const DistanceProxy& shape1, const Transformation& xf1,
									   const IndexSeparation::index_type idx1,
									   const DistanceProxy& shape2, const Transformation& xf2,
									   const IndexSeparation::index_type idx2,
									   const Manifold::Conf conf)
{
	assert(type == Manifold::e_faceA || type == Manifold::e_faceB);
	assert(shape1.GetVertexCount() > 1 && shape2.GetVertexCount() > 1);

	const auto r1 = shape1.GetVertexRadius();
	const auto r2 = shape2.GetVertexRadius();
	const auto totalRadius = Length{r1 + r2};
	
	const auto idx1Next = GetModuloNext(idx1, shape1.GetVertexCount());
	
	const auto shape1_rel_vertex1 = shape1.GetVertex(idx1);
	const auto shape1_rel_vertex2 = shape1.GetVertex(idx1Next);
	const auto shape1_abs_vertex1 = Transform(shape1_rel_vertex1, xf1);
	const auto shape1_abs_vertex2 = Transform(shape1_rel_vertex2, xf1);
	
	const auto shape1_rel_edge1 = shape1_rel_vertex2 - shape1_rel_vertex1;
	assert(IsValid(shape1_rel_edge1));
	auto shape1_len_edge1 = Length{0};
	const auto shape1_rel_edge1_dir = GetUnitVector(shape1_rel_edge1, shape1_len_edge1, UnitVec2::GetZero());
	assert(IsValid(shape1_rel_edge1_dir));
	const auto shape1_edge1_abs_dir = Rotate(shape1_rel_edge1_dir, xf1.q);
	
	// Clip incident edge against extruded edge1 side edges.
	// Side offsets, extended by polytope skin thickness.
	
	const auto shape1_rel_normal = InverseRotate(Rotate(shape1.GetNormal(idx1), xf1.q), xf2.q);
	const auto shape2_idx0 = GetModuloPrev(idx2, shape2.GetVertexCount());
	const auto shape2_idx1 = idx2;
	const auto shape2_normal0 = shape2.GetNormal(shape2_idx0);
	const auto shape2_normal1 = shape2.GetNormal(shape2_idx1);
	const auto shape2_s0 = Dot(shape1_rel_normal, shape2_normal0);
	const auto shape2_s1 = Dot(shape1_rel_normal, shape2_normal1);
	const auto shape2_i1 = (shape2_s0 < shape2_s1)? shape2_idx0: shape2_idx1;
	const auto shape2_i2 = GetModuloNext(shape2_i1, shape2.GetVertexCount()); /// XXX is this correct?
	const auto incidentEdge = ClipList{
		ClipVertex{Transform(shape2.GetVertex(shape2_i1), xf2), GetFaceVertexContactFeature(idx1, shape2_i1)},
		ClipVertex{Transform(shape2.GetVertex(shape2_i2), xf2), GetFaceVertexContactFeature(idx1, shape2_i2)}
	};
	const auto clipPoints = [&]()
	{
		// Gets the two vertices in world coordinates and their face-vertex contact features
		// of the incident edge of shape2
		//const auto incidentEdge = GetIncidentEdgeClipList(idx1, shape1.GetNormal(idx1), xf1, shape2, xf2, idx2);
		assert(incidentEdge[0].cf.indexB == idx2 || incidentEdge[1].cf.indexB == idx2);
		const auto shape1_dp_v1_e1 = Dot(shape1_edge1_abs_dir, shape1_abs_vertex1);
		const auto shape1_dp_v2_e1 = Dot(shape1_edge1_abs_dir, shape1_abs_vertex2);
		return GetClipPoints(idx1, -shape1_dp_v1_e1, -shape1_edge1_abs_dir,
							 idx1Next, +shape1_dp_v2_e1, shape1_edge1_abs_dir,
							 incidentEdge);
	}();
	if (clipPoints.size() == 2)
	{
		const auto abs_normal = GetFwdPerpendicular(shape1_edge1_abs_dir); // Normal points from 1 to 2
		const auto rel_midpoint = (shape1_rel_vertex1 + shape1_rel_vertex2) / RealNum{2};
		const auto abs_offset = Dot(abs_normal, shape1_abs_vertex1); ///< Face offset.
		
		auto manifold = Manifold{};
		switch (type)
		{
			case Manifold::e_faceA:
			{
				//auto manifold = Manifold::GetForFaceA(GetFwdPerpendicular(rel_tangent), idx1, rel_midpoint);
				manifold = Manifold::GetForFaceA(GetFwdPerpendicular(shape1_rel_edge1_dir), rel_midpoint);
				for (auto&& cp: clipPoints)
				{
					if ((Dot(abs_normal, cp.v) - abs_offset) <= totalRadius)
					{
						manifold.AddPoint(Manifold::Point{InverseTransform(cp.v, xf2), cp.cf});
						//manifold.AddPoint(cp.cf.typeB, cp.cf.indexB, InverseTransform(cp.v, xf2));
					}
				}
				break;
			}
			case Manifold::e_faceB:
			{
				manifold = Manifold::GetForFaceB(GetFwdPerpendicular(shape1_rel_edge1_dir), rel_midpoint);
				for (auto&& cp: clipPoints)
				{
					if ((Dot(abs_normal, cp.v) - abs_offset) <= totalRadius)
					{
						manifold.AddPoint(Manifold::Point{InverseTransform(cp.v, xf2), Flip(cp.cf)});
					}
				}
				break;
			}
			default:
				break;
		}
		if (manifold.GetPointCount() > 0)
		{
			return manifold;
		}
	}
	
	// If the shapes are colliding, then they're colliding with each others corners.
	// Using a circles manifold, means these corners will repell each other with a normal
	// that's in the direction between the two vertices.
	// That's problematic though for things like polygons sliding over edges where a face
	// manifold that favors the primary edge can work better.
	// Use a threshold against the ratio of the square of the vertex radius to the square
	// of the length of the primary edge to determine whether to return a circles manifold
	// or a face manifold.
	const auto shape2_rel_vertex1 = shape2.GetVertex(shape2_i1);
	const auto shape2_abs_vertex1 = Transform(shape2_rel_vertex1, xf2);
	const auto shape2_rel_vertex2 = shape2.GetVertex(shape2_i2);
	const auto shape2_abs_vertex2 = Transform(shape2_rel_vertex2, xf2);
	const auto totalRadiusSquared = Square(totalRadius);
	const auto mustUseFaceManifold = (shape1_len_edge1 / r1) > conf.maxCirclesRatio;
	if (GetLengthSquared(shape1_abs_vertex1 - shape2_abs_vertex1) <= totalRadiusSquared)
	{
		// shape 1 vertex 1 is colliding with shape 2 vertex 1
		// shape 1 vertex 1 is the vertex at index idx1, or one before idx1Next.
		// shape 2 vertex 1 is the vertex at index shape2_i1, or one before shape2_i2.
		switch (type)
		{
			case Manifold::e_faceA:
				// shape 1 is shape A.
				if (mustUseFaceManifold)
				{
					return Manifold::GetForFaceA(GetFwdPerpendicular(shape1_rel_edge1_dir), idx1,
												 shape1_rel_vertex1, ContactFeature::e_vertex,
												 shape2_i1, shape2_rel_vertex1);
				}
				return Manifold::GetForCircles(shape1_rel_vertex1, idx1, shape2_rel_vertex1, shape2_i1);
			case Manifold::e_faceB:
				// shape 2 is shape A.
				if (mustUseFaceManifold)
				{
					return Manifold::GetForFaceB(GetFwdPerpendicular(shape1_rel_edge1_dir), idx1,
												 shape1_rel_vertex1, ContactFeature::e_vertex,
												 shape2_i1, shape2_rel_vertex1);
				}
				return Manifold::GetForCircles(shape2_rel_vertex1, shape2_i1, shape1_rel_vertex1, idx1);
			default:
				break;
		}
	}
	else if (GetLengthSquared(shape1_abs_vertex1 - shape2_abs_vertex2) <= totalRadiusSquared)
	{
		// shape 1 vertex 1 is colliding with shape 2 vertex 2
		switch (type)
		{
			case Manifold::e_faceA:
				if (mustUseFaceManifold)
				{
					return Manifold::GetForFaceA(GetFwdPerpendicular(shape1_rel_edge1_dir), idx1,
												 shape1_rel_vertex1, ContactFeature::e_vertex,
												 shape2_i2, shape2_rel_vertex2);
				}
				return Manifold::GetForCircles(shape1_rel_vertex1, idx1, shape2_rel_vertex2, shape2_i2);
			case Manifold::e_faceB:
				if (mustUseFaceManifold)
				{
					return Manifold::GetForFaceB(GetFwdPerpendicular(shape1_rel_edge1_dir), idx1,
												 shape1_rel_vertex1, ContactFeature::e_vertex,
												 shape2_i2, shape2_rel_vertex2);
				}
				return Manifold::GetForCircles(shape2_rel_vertex2, shape2_i2, shape1_rel_vertex1, idx1);
			default:
				break;
		}
	}
	else if (GetLengthSquared(shape1_abs_vertex2 - shape2_abs_vertex2) <= totalRadiusSquared)
	{
		// shape 1 vertex 2 is colliding with shape 2 vertex 2
		switch (type)
		{
			case Manifold::e_faceA:
				if (mustUseFaceManifold)
				{
					return Manifold::GetForFaceA(GetFwdPerpendicular(shape1_rel_edge1_dir), idx1Next,
												 shape1_rel_vertex2, ContactFeature::e_vertex,
												 shape2_i2, shape2_rel_vertex2);
				}
				return Manifold::GetForCircles(shape1_rel_vertex2, idx1Next, shape2_rel_vertex2, shape2_i2);
			case Manifold::e_faceB:
				if (mustUseFaceManifold)
				{
					return Manifold::GetForFaceB(GetFwdPerpendicular(shape1_rel_edge1_dir), idx1Next,
												 shape1_rel_vertex2, ContactFeature::e_vertex,
												 shape2_i2, shape2_rel_vertex2);
				}
				return Manifold::GetForCircles(shape2_rel_vertex2, shape2_i2, shape1_rel_vertex2, idx1Next);
			default:
				break;
		}
	}
	else if (GetLengthSquared(shape1_abs_vertex2 - shape2_abs_vertex1) <= totalRadiusSquared)
	{
		// shape 1 vertex 2 is colliding with shape 2 vertex 1
		switch (type)
		{
			case Manifold::e_faceA:
				if (mustUseFaceManifold)
				{
					return Manifold::GetForFaceA(GetFwdPerpendicular(shape1_rel_edge1_dir), idx1Next,
												 shape1_rel_vertex2, ContactFeature::e_vertex,
												 shape2_i1, shape2_rel_vertex1);
				}
				return Manifold::GetForCircles(shape1_rel_vertex2, idx1Next, shape2_rel_vertex1, shape2_i1);
			case Manifold::e_faceB:
				return Manifold::GetForCircles(shape2_rel_vertex1, shape2_i1, shape1_rel_vertex2, idx1Next);
			default:
				break;
		}
	}
	return Manifold{};
}

static Manifold CollideShapes(const DistanceProxy& shapeA, const Transformation& xfA,
							  Length2D locationB, Length radiusB, const Transformation& xfB)
{
	// Computes the center of the circle in the frame of the polygon.
	const auto cLocal = InverseTransform(Transform(locationB, xfB), xfA); ///< Center of circle in frame of polygon.
	
	const auto totalRadius = shapeA.GetVertexRadius() + radiusB;
	const auto vertexCount = shapeA.GetVertexCount();
	
	// Find edge that circle is closest to.
	auto indexOfMax = decltype(vertexCount){0};
	auto maxSeparation = -MaxFloat * Meter;
	{
		for (auto i = decltype(vertexCount){0}; i < vertexCount; ++i)
		{
			// Get circle's distance from vertex[i] in direction of normal[i].
			const auto s = Dot(shapeA.GetNormal(i), cLocal - shapeA.GetVertex(i));
			if (s > totalRadius)
			{
				// Early out - no contact.
				return Manifold{};
			}
			if (maxSeparation < s)
			{
				maxSeparation = s;
				indexOfMax = i;
			}
		}
	}
	const auto indexOfMax2 = GetModuloNext(indexOfMax, vertexCount);
	assert(maxSeparation <= totalRadius);
	
	// Vertices that subtend the incident face.
	const auto v1 = shapeA.GetVertex(indexOfMax);
	const auto v2 = shapeA.GetVertex(indexOfMax2);
	
	if (maxSeparation < Length{0})
	{
		// Circle's center is inside the polygon and closest to edge[indexOfMax].
		return Manifold::GetForFaceA(shapeA.GetNormal(indexOfMax), indexOfMax, (v1 + v2) / RealNum{2},
									 ContactFeature::e_vertex, 0, locationB);
	}
	
	// Circle's center is outside polygon and closest to edge[indexOfMax].
	// Compute barycentric coordinates.
	
	const auto cLocalV1 = cLocal - v1;
	if (Dot(cLocalV1, v2 - v1) <= Area{0})
	{
		// Circle's center right of v1 (in direction of v1 to v2).
		if (GetLengthSquared(cLocalV1) > Square(totalRadius))
		{
			return Manifold{};
		}
		return Manifold::GetForCircles(v1, indexOfMax, locationB, 0);
	}
	
	const auto ClocalV2 = cLocal - v2;
	if (Dot(ClocalV2, v1 - v2) <= Area{0})
	{
		// Circle's center left of v2 (in direction of v2 to v1).
		if (GetLengthSquared(ClocalV2) > Square(totalRadius))
		{
			return Manifold{};
		}
		return Manifold::GetForCircles(v2, indexOfMax2, locationB, 0);
	}
	
	// Circle's center is between v1 and v2.
	const auto faceCenter = (v1 + v2) / RealNum{2};
	if (Dot(cLocal - faceCenter, shapeA.GetNormal(indexOfMax)) > totalRadius)
	{
		return Manifold{};
	}
	return Manifold::GetForFaceA(shapeA.GetNormal(indexOfMax), indexOfMax, faceCenter,
								 ContactFeature::e_vertex, 0, locationB);
}

static Manifold CollideShapes(Length2D locationA, Length radiusA, const Transformation& xfA,
							  Length2D locationB, Length radiusB, const Transformation& xfB)
{
	const auto pA = Transform(locationA, xfA);
	const auto pB = Transform(locationB, xfB);
	const auto totalRadius = radiusA + radiusB;
	return (GetLengthSquared(pB - pA) > Square(totalRadius))?
		Manifold{}: Manifold::GetForCircles(locationA, 0, locationB, 0);
}

/*
 * Definition of public CollideShapes functions.
 * All CollideShapes functions return a Manifold object.
 */

Manifold box2d::CollideShapes(const CircleShape& shapeA, const Transformation& xfA,
							  const CircleShape& shapeB, const Transformation& xfB,
							  const Manifold::Conf)
{
	return ::CollideShapes(shapeA.GetLocation(), GetVertexRadius(shapeA), xfA,
						   shapeB.GetLocation(), GetVertexRadius(shapeB), xfB);
}

Manifold box2d::CollideShapes(const PolygonShape& shapeA, const Transformation& xfA,
							  const CircleShape& shapeB, const Transformation& xfB,
							  const Manifold::Conf)
{
	return ::CollideShapes(GetDistanceProxy(shapeA, 0), xfA, shapeB.GetLocation(), shapeB.GetVertexRadius(), xfB);
}

Manifold box2d::CollideShapes(const EdgeShape& shapeA, const Transformation& xfA,
							  const CircleShape& shapeB, const Transformation& xfB,
							  const Manifold::Conf)
{
	/*
	 * Determine the collision manifold between the edge and the circle. This accounts for
	 * edge connectivity.
	 *
	 * To do this, this code treats the edge like an end-rounded strait-line drawn by an air
	 * brush having the radius that GetVertexRadius(shapeA) returns. As such, the collision is
	 * categorized as either a collision with the first end-point, the second end-point, or the
	 * conceptual rectangle between the end-points.
	 *
	 * Definitions:
	 *   "-" Is an edge parallel to the horizon bounded by vertex A on the left and vertex B
	 *       on the right.
	 *   "|" Is a line perpendicular to the edge.
	 *   "." Is the center of a circle whose position is called Q.
	 *
	 * Then:
	 *   When Q is anywhere to the left of A (as in ".|-"), it's said to be in region A.
	 *   When Q is anywhere to the right of B (as in "-|."), it's said to be in region B.
	 *   Otheriwse (when Q is between A and B), Q is said to be in region AB.
	 */
	
	// Compute circle in frame of edge
	const auto Q = Length2D{InverseTransform(Transform(shapeB.GetLocation(), xfB), xfA)}; ///< Circle's position in frame of edge.
	
	const auto A = shapeA.GetVertex1(); ///< Edge shape's vertex 1.
	const auto B = shapeA.GetVertex2(); ///< Edge shape's vertex 2.
	const auto e = B - A; ///< Edge shape's primary edge.
	
	// Barycentric coordinates
	
	const auto totalRadius = GetVertexRadius(shapeA) + GetVertexRadius(shapeB);
	
	// Check if circle's center is relatively left of first vertex of edge - this is "Region A"
	const auto deltaQA = Q - A;
	const auto v = Area{Dot(e, deltaQA)};
	if (v <= Area{0})
	{
		if (GetLengthSquared(deltaQA) > Square(totalRadius))
		{
			return Manifold{};
		}
		
		// Is there an edge connected to A?
		if (shapeA.HasVertex0())
		{
			// Is the circle in Region AB of the previous edge?
			if (Dot(A - shapeA.GetVertex0(), A - Q) > Area{0})
			{
				return Manifold{};
			}
		}
		return Manifold::GetForCircles(A, 0, shapeB.GetLocation(), 0);
	}
	
	// Check if circle's center is relatively right of second vertex of edge - this is "Region B"
	const auto u = Area{Dot(e, B - Q)};
	if (u <= Area{0})
	{
		const auto deltaQB = Q - B;
		if (GetLengthSquared(deltaQB) > Square(totalRadius))
		{
			return Manifold{};
		}
		
		// Is there an edge connected to B?
		if (shapeA.HasVertex3())
		{
			// Is the circle in Region AB of the next edge?
			if (Area{Dot(shapeA.GetVertex3() - B, deltaQB)} > Area{0})
			{
				return Manifold{};
			}
		}
		return Manifold::GetForCircles(B, 1, shapeB.GetLocation(), 0);
	}
	
	// Region AB
	const auto eLenSquared = GetLengthSquared(e);
	assert(eLenSquared > RealNum{0} * SquareMeter);

	// Compute P in an order less likely to overflow
	const auto P = Length2D{((A / eLenSquared) * u) + ((B / eLenSquared) * v)};
	
	if (GetLengthSquared(Q - P) > Square(totalRadius))
	{
		return Manifold{};
	}
	
	const auto ln = GetUnitVector([=]() {
		const auto e_perp = Length2D{GetRevPerpendicular(e)};
		return (Dot(e_perp, deltaQA) < Area{0})? -e_perp: e_perp;
	}(), UnitVec2::GetZero());
	
	return Manifold::GetForFaceA(ln, 0, A, ContactFeature::e_vertex, 0, shapeB.GetLocation());
}

Manifold box2d::CollideShapes(const EdgeShape& shapeA, const Transformation& xfA,
							  const EdgeShape& shapeB, const Transformation& xfB,
							  const Manifold::Conf)
{
	const auto shapeA_v1 = shapeA.GetVertex1(); // p
	const auto shapeA_v2 = shapeA.GetVertex2();
	const auto shapeB_v1 = InverseTransform(Transform(shapeB.GetVertex1(), xfB), xfA); // q
	const auto shapeB_v2 = InverseTransform(Transform(shapeB.GetVertex2(), xfB), xfA);
	const auto shapeA_edge = shapeA_v2 - shapeA_v1; // r
	const auto shapeB_edge = shapeB_v2 - shapeB_v1; // s
	const auto shapeA_normal = GetRevPerpendicular(GetUnitVector(shapeA_edge));
	const auto shapeA_len_squared = GetLengthSquared(shapeA_edge); // r . r
	const auto shapeB_len_squared = GetLengthSquared(shapeB_edge); // s . s
	const auto shapeA_extent = GetVertexRadius(shapeA) / Sqrt(shapeA_len_squared);
	const auto shapeB_extent = GetVertexRadius(shapeB) / Sqrt(shapeB_len_squared);
	const auto totalRadius = GetVertexRadius(shapeA) + GetVertexRadius(shapeB);

	// Now solve for:
	// shapeA_v1 + shapeA_c * shapeA_edge == shapeB_v1 + shapeB_c * shapeB_edge

	const auto shapeB_v1_sub_shapeA_v1 = shapeB_v1 - shapeA_v1; // q - p
	const auto cross_edge_A_B = Cross(shapeA_edge, shapeB_edge); // (r × s)
	const auto shapeA_n = Cross(shapeB_v1_sub_shapeA_v1, shapeB_edge); // (q − p) × s
	const auto shapeB_n = Cross(shapeB_v1_sub_shapeA_v1, shapeA_edge); // (q − p) × r
	if (almost_zero(StripUnit(cross_edge_A_B)))
	{
		// The two lines are parallel.
		if (almost_zero(StripUnit(shapeB_n)))
		{
			// The two lines are collinear (and parallel).
			const auto shapeA_v1_p = shapeA_v1 - (shapeA_extent * shapeA_edge);
			const auto shapeA_v2_p = shapeA_v2 + (shapeA_extent * shapeA_edge);
			const auto shapeA_edge_p = shapeA_v2_p - shapeA_v1_p;
			const auto shapeA_len_squared_p = GetLengthSquared(shapeA_edge_p);
			const auto shapeB_v1_p = shapeB_v1 - (shapeB_extent * shapeB_edge);
			const auto shapeB_v2_p = shapeB_v2 + (shapeB_extent * shapeB_edge);
			const auto shapeB_edge_p = shapeB_v2_p - shapeB_v1_p;
			const auto shapeB_v1_sub_shapeA_v1_p = shapeB_v1_p - shapeA_v1_p;
			const auto dot_edge_B_A = Dot(shapeB_edge_p, shapeA_edge_p);
			const auto shapeA_c0 = Dot(shapeB_v1_sub_shapeA_v1_p, shapeA_edge_p) / shapeA_len_squared_p; // t0
			const auto shapeA_c1 = shapeA_c0 + (dot_edge_B_A / shapeA_len_squared_p); // t1
			const auto interval = (dot_edge_B_A < Area{0})?
				std::array<RealNum, 2>{{shapeA_c1, shapeA_c0}}:
				std::array<RealNum, 2>{{shapeA_c0, shapeA_c1}};
			if ((interval[1] >= 0) && (interval[0] <= 1))
			{
				// The line segments are overlapping (and collinear).
				const auto contact_pt = shapeA_v1_p + interval[0] * shapeA_edge_p;
				const auto len_squared_from_shapeA_v1 = GetLengthSquared(shapeA_v1 - contact_pt);
				const auto len_squared_from_shapeA_v2 = GetLengthSquared(shapeA_v2 - contact_pt);
				if (len_squared_from_shapeA_v1 < len_squared_from_shapeA_v2)
				{
					if (len_squared_from_shapeA_v1 >= Area{0})
					{
						return Manifold::GetForCircles(shapeA_v1, 0, shapeB_v1, 0);						
					}
				}
				else
				{
					if (len_squared_from_shapeA_v2 >= Area{0})
					{
						return Manifold::GetForCircles(shapeA_v2, 1, shapeB_v1, 0);						
					}
				}
				const auto ln = GetRevPerpendicular(GetUnitVector(shapeA_edge));
				return Manifold::GetForFaceA(ln, 0, contact_pt, ContactFeature::e_face, 0, shapeB_v1);
			}
			// The line segments are disjoint (and collinear).
			return Manifold{};
		}
		// The two lines are not collinear (but they are parallel).
		const auto s = Dot(shapeA_normal, shapeB_v1);
		if (Abs(s) > totalRadius)
		{
			return Manifold{};
		}
		GetLengthSquared(shapeA_v1 - shapeB_v1);
		GetLengthSquared(shapeA_v1 - shapeB_v2);
		GetLengthSquared(shapeA_v2 - shapeB_v1);
		GetLengthSquared(shapeA_v2 - shapeB_v2);
	}
	else
	{
		// The two lines are NOT parallel.
		const auto shapeA_c = shapeA_n / cross_edge_A_B; // t = (q − p) × s / (r × s)
		const auto shapeB_c = shapeB_n / cross_edge_A_B; // u = (q − p) × r / (r × s)
		const auto shapeA_c_valid = ((shapeA_c >= 0) && (shapeA_c <= 1));
		const auto shapeB_c_valid = ((shapeB_c >= 0) && (shapeB_c <= 1));
		if (shapeA_c_valid && shapeB_c_valid)
		{
			// The two line segments meet at the point shapeA_v1 + shapeA_c * shapeA_edge
			const auto lp = shapeA_v1 + shapeA_c * shapeA_edge;
			return Manifold::GetForFaceA(shapeA_normal, 0, (shapeA_v1 + shapeA_v2) / RealNum{2},
										 ContactFeature::e_face, 0, lp);
		}
		else
		{
			// The two line segments are not parallel but do not intersect.
		}
	}
	return Manifold{};
}

Manifold box2d::CollideShapes(const EdgeShape& shapeA, const Transformation& xfA,
							  const PolygonShape& shapeB, const Transformation& xfB,
							  const Manifold::Conf)
{
	// Algorithm:
	// 1. Classify v1 and v2
	// 2. Classify polygon centroid as front or back
	// 3. Flip normal if necessary
	// 4. Initialize normal range to [-pi, pi] about face normal
	// 5. Adjust normal range according to adjacent edges
	// 6. Visit each separating axes, only accept axes within the range
	// 7. Return if _any_ axis indicates separation
	// 8. Clip

	const auto xf = MulT(xfA, xfB);
	const auto localShapeB = Transform(shapeB, xf);
	const auto edgeInfo = EdgeInfo{shapeA, localShapeB.GetCentroid()};
	const auto totalRadius = GetVertexRadius(shapeA) + GetVertexRadius(shapeB);

	// If no valid normal can be found then this edge should not collide.
	const auto refDir = edgeInfo.GetNormal();
	const auto edgeAxis = GetMostAntiParallelSeparation(localShapeB.GetVertices(),
														GetVec2(refDir), edgeInfo.GetVertex1());
	if (edgeAxis.separation * Meter > totalRadius)
	{
		return Manifold{};
	}
	
	const auto polygonAxis = GetPolygonSeparation(localShapeB, edgeInfo);
	if (polygonAxis.separation * Meter > totalRadius)
	{
		return Manifold{};
	}
	
	// Use hysteresis for jitter reduction.
	constexpr auto k_relativeTol = RealNum(0.98);
	constexpr auto k_absoluteTol = DefaultLinearSlop / RealNum{5}; // 0.001
	
	// Now:
	//   (edgeAxis.separation <= MaxSeparation) AND
	//   (polygonAxis.index == EPAxis::InvalidIndex OR polygonAxis.separation <= MaxEPSeparation)
	
	if ((polygonAxis.index != IndexSeparation::InvalidIndex) &&
		(polygonAxis.separation * Meter > ((k_relativeTol * edgeAxis.separation * Meter) + k_absoluteTol)))
	{
		// polygonAxis
		return GetManifoldFaceB(edgeInfo, shapeB, localShapeB, polygonAxis.index);
	}
	// edgeAxis
	return GetManifoldFaceA(edgeInfo, localShapeB, xf);	
}

Manifold box2d::CollideShapes(const DistanceProxy& shapeA, const Transformation& xfA,
							  const DistanceProxy& shapeB, const Transformation& xfB,
							  const Manifold::Conf conf)
{
	// Find edge normal of max separation on A - return if separating axis is found
	// Find edge normal of max separation on B - return if separation axis is found
	// Choose reference edge as min(minA, minB)
	// Find incident edge
	// Clip

	const auto vertexCountShapeA = shapeA.GetVertexCount();
	const auto vertexCountShapeB = shapeB.GetVertexCount();
	if (vertexCountShapeA == 1)
	{
		if (vertexCountShapeB > 1)
		{
			return ::CollideShapes(shapeB, xfB, shapeA.GetVertex(0), shapeA.GetVertexRadius(), xfA);
		}
		return ::CollideShapes(shapeA.GetVertex(0), shapeA.GetVertexRadius(), xfA,
							   shapeB.GetVertex(0), shapeB.GetVertexRadius(), xfB);
	}
	if (vertexCountShapeB == 1)
	{
		if (vertexCountShapeA > 1)
		{
			return ::CollideShapes(shapeA, xfA, shapeB.GetVertex(0), shapeB.GetVertexRadius(), xfB);
		}
		return ::CollideShapes(shapeA.GetVertex(0), shapeA.GetVertexRadius(), xfA,
							   shapeB.GetVertex(0), shapeB.GetVertexRadius(), xfB);
	}

	const auto totalRadius = shapeA.GetVertexRadius() + shapeB.GetVertexRadius();
	
	const auto edgeSepA = ::GetMaxSeparation(shapeA, xfA, shapeB, xfB, totalRadius);
	if (edgeSepA.separation * Meter > totalRadius)
	{
		return Manifold{};
	}
	
	const auto edgeSepB = ::GetMaxSeparation(shapeB, xfB, shapeA, xfA, totalRadius);
	if (edgeSepB.separation * Meter > totalRadius)
	{
		return Manifold{};
	}
	
	constexpr auto k_tol = BOX2D_MAGIC(DefaultLinearSlop / RealNum{10});
	return (edgeSepB.separation * Meter > (edgeSepA.separation * Meter + k_tol))?
		GetFaceManifold(Manifold::e_faceB,
						shapeB, xfB, edgeSepB.index1,
						shapeA, xfA, edgeSepB.index2,
						conf):
		GetFaceManifold(Manifold::e_faceA,
						shapeA, xfA, edgeSepA.index1,
						shapeB, xfB, edgeSepA.index2,
						conf);
}

Manifold box2d::CollideShapes(const PolygonShape& shapeA, const Transformation& xfA,
							  const PolygonShape& shapeB, const Transformation& xfB,
							  const Manifold::Conf)
{
	return CollideShapes(GetDistanceProxy(shapeA, 0), xfA, GetDistanceProxy(shapeB, 0), xfB);
}
