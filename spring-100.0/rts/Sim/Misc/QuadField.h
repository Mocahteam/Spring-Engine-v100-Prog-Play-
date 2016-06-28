/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef QUAD_FIELD_H
#define QUAD_FIELD_H

#include <vector>
#include <boost/noncopyable.hpp>

#include "System/creg/creg_cond.h"
#include "System/float3.h"
#include "System/type2.h"

class CUnit;
class CFeature;
class CProjectile;
class CSolidObject;


class CQuadField : boost::noncopyable
{
	CR_DECLARE_STRUCT(CQuadField)
	CR_DECLARE_SUB(Quad)

public:

/*
needed to support dynamic resizing (not used yet)
      in large games the average loading factor (number of objects per quad)
      can grow too large to maintain amortized constant performance so more
      quads are needed
*/
//	static void Resize(int quad_size);

	CQuadField(int2 mapDims, int quad_size);
	~CQuadField();

	std::vector<int> GetQuads(float3 pos, const float radius);
	std::vector<int> GetQuadsRectangle(const float3 mins, const float3 maxs);
	std::vector<int> GetQuadsOnRay(const float3 start, const float3 dir, const float length);

	void GetUnitsAndFeaturesColVol(
		const float3& pos,
		const float radius,
		std::vector<CUnit*>& units,
		std::vector<CFeature*>& features,
		unsigned int* numUnitsPtr = NULL,
		unsigned int* numFeaturesPtr = NULL
	);

	/**
	 * Returns all units within @c radius of @c pos,
	 * and treats each unit as a 3D point object
	 */
	std::vector<CUnit*> GetUnits(const float3& pos, float radius);
	/**
	 * Returns all units within @c radius of @c pos,
	 * takes the 3D model radius of each unit into account,
 	 * and performs the search within a sphere or cylinder depending on @c spherical
	 */
	std::vector<CUnit*> GetUnitsExact(const float3& pos, float radius, bool spherical = true);
	/**
	 * Returns all units within the rectangle defined by
	 * mins and maxs, which extends infinitely along the y-axis
	 */
	std::vector<CUnit*> GetUnitsExact(const float3& mins, const float3& maxs);
	/**
	 * Returns all features within @c radius of @c pos,
	 * takes the 3D model radius of each feature into account,
	 * and performs the search within a sphere or cylinder depending on @c spherical
	 */
	std::vector<CFeature*> GetFeaturesExact(const float3& pos, float radius, bool spherical = true);
	/**
	 * Returns all features within the rectangle defined by
	 * mins and maxs, which extends infinitely along the y-axis
	 */
	std::vector<CFeature*> GetFeaturesExact(const float3& mins, const float3& maxs);

	std::vector<CProjectile*> GetProjectilesExact(const float3& pos, float radius);
	std::vector<CProjectile*> GetProjectilesExact(const float3& mins, const float3& maxs);

	std::vector<CSolidObject*> GetSolidsExact(
		const float3& pos,
		const float radius,
		const unsigned int physicalStateBits = 0xFFFFFFFF,
		const unsigned int collisionStateBits = 0xFFFFFFFF
	);

	void MovedUnit(CUnit* unit);
	void RemoveUnit(CUnit* unit);

	void AddFeature(CFeature* feature);
	void RemoveFeature(CFeature* feature);

	void MovedProjectile(CProjectile* projectile);
	void AddProjectile(CProjectile* projectile);
	void RemoveProjectile(CProjectile* projectile);

	struct Quad {
		CR_DECLARE_STRUCT(Quad)
		Quad();
		std::vector<CUnit*> units;
		std::vector< std::vector<CUnit*> > teamUnits;
		std::vector<CFeature*> features;
		std::vector<CProjectile*> projectiles;
	};

	const Quad& GetQuad(unsigned i) const {
		assert(i < baseQuads.size());
		return baseQuads[i];
	}
	const Quad& GetQuadAt(unsigned x, unsigned z) const {
		assert(unsigned(numQuadsX * z + x) < baseQuads.size());
		return baseQuads[numQuadsX * z + x];
	}


	int GetNumQuadsX() const { return numQuadsX; }
	int GetNumQuadsZ() const { return numQuadsZ; }

	int GetQuadSizeX() const { return quadSizeX; }
	int GetQuadSizeZ() const { return quadSizeZ; }

	const static unsigned int BASE_QUAD_SIZE =  128;

private:
	// optimized functions, somewhat less userfriendly
	//
	// when calling these, <begQuad> and <endQuad> are both expected
	// to point to the *start* of an array of int's of size at least
	// numQuadsX * numQuadsZ (eg. tempQuads) -- GetQuadsOnRay ensures
	// this by itself, for GetQuads the callers take care of it
	//
	void GetQuads(float3 pos, float radius, std::vector<int>* quads) const;

	int2 WorldPosToQuadField(const float3 p) const;
	int WorldPosToQuadFieldIdx(const float3 p) const;

private:
	std::vector<Quad> baseQuads;

	int numQuadsX;
	int numQuadsZ;

	int quadSizeX;
	int quadSizeZ;
};

extern CQuadField* quadField;

#endif /* QUAD_FIELD_H */
