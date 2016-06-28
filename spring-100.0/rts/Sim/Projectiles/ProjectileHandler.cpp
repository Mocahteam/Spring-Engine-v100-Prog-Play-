/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include <algorithm>

#include "Projectile.h"
#include "ProjectileHandler.h"
#include "Game/GlobalUnsynced.h"
#include "Game/TraceRay.h"
#include "Map/Ground.h"
#include "Rendering/GlobalRendering.h"
#include "Rendering/GroundFlash.h"
#include "Sim/Features/Feature.h"
#include "Sim/Features/FeatureDef.h"
#include "Sim/Misc/CollisionHandler.h"
#include "Sim/Misc/CollisionVolume.h"
#include "Sim/Misc/QuadField.h"
#include "Sim/Misc/TeamHandler.h"
#include "Sim/Projectiles/Unsynced/FlyingPiece.h"
#include "Sim/Projectiles/Unsynced/NanoProjectile.h"
#include "Sim/Units/Unit.h"
#include "Sim/Units/UnitDef.h"
#include "System/Config/ConfigHandler.h"
#include "System/EventHandler.h"
#include "System/Log/ILog.h"
#include "System/TimeProfiler.h"
#include "System/creg/STL_Deque.h"


// reserve 5% of maxNanoParticles for important stuff such as capture and reclaim other teams' units
#define NORMAL_NANO_PRIO 0.95f
#define HIGH_NANO_PRIO 1.0f


using namespace std;

CONFIG(int, MaxParticles).defaultValue(3000).headlessValue(1).minimumValue(1);
CONFIG(int, MaxNanoParticles).defaultValue(2000).headlessValue(1).minimumValue(1);

CProjectileHandler* projectileHandler = NULL;


CR_BIND(CProjectileHandler, )
CR_REG_METADATA(CProjectileHandler, (
	CR_MEMBER(syncedProjectiles),
	CR_MEMBER(unsyncedProjectiles),
	CR_MEMBER_UN(flyingPieces3DO),
	CR_MEMBER_UN(flyingPiecesS3O),
	CR_MEMBER_UN(groundFlashes),
	CR_MEMBER_UN(resortFlyingPieces3DO),
	CR_MEMBER_UN(resortFlyingPiecesS3O),

	CR_MEMBER(maxParticles),
	CR_MEMBER(maxNanoParticles),
	CR_MEMBER(currentNanoParticles),
	CR_MEMBER_UN(lastCurrentParticles),
	CR_MEMBER_UN(lastSyncedProjectilesCount),
	CR_MEMBER_UN(lastUnsyncedProjectilesCount),

	CR_MEMBER(freeSyncedIDs),
	CR_MEMBER(freeUnsyncedIDs),
	CR_MEMBER(syncedProjectileIDs),
	CR_MEMBER(unsyncedProjectileIDs),

	CR_SERIALIZER(Serialize)
))



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProjectileHandler::CProjectileHandler()
: currentNanoParticles(0)
, lastCurrentParticles(0)
, lastSyncedProjectilesCount(0)
, lastUnsyncedProjectilesCount(0)
, resortFlyingPieces3DO(false)
, resortFlyingPiecesS3O(false)
, syncedProjectileIDs(1024, nullptr)
#if UNSYNCED_PROJ_NOEVENT
, unsyncedProjectileIDs(0, nullptr)
#else
, unsyncedProjectileIDs(8192, nullptr)
#endif
{
	maxParticles     = configHandler->GetInt("MaxParticles");
	maxNanoParticles = configHandler->GetInt("MaxNanoParticles");

	// preload some IDs
	for (int i = 0; i < syncedProjectileIDs.size(); i++) {
		freeSyncedIDs.push_back(i);
	}
	std::random_shuffle(freeSyncedIDs.begin(), freeSyncedIDs.end(), gs->rng);

	for (int i = 0; i < unsyncedProjectileIDs.size(); i++) {
		freeUnsyncedIDs.push_back(i);
	}
	std::random_shuffle(freeUnsyncedIDs.begin(), freeUnsyncedIDs.end(), gu->rng);

	// register ConfigNotify()
	configHandler->NotifyOnChange(this);
}

CProjectileHandler::~CProjectileHandler()
{
	configHandler->RemoveObserver(this);

	for (CProjectile* p: syncedProjectiles) {
		delete p;
	}
	syncedProjectiles.clear(); // synced first, to avoid callback crashes
	for (CProjectile* p: unsyncedProjectiles) {
		delete p;
	}
	unsyncedProjectiles.clear();

	freeSyncedIDs.clear();
	freeUnsyncedIDs.clear();

	syncedProjectileIDs.clear();
	unsyncedProjectileIDs.clear();

	CCollisionHandler::PrintStats();
}

void CProjectileHandler::Serialize(creg::ISerializer* s)
{
	if (s->IsWriting()) {
		int ssize = int(syncedProjectiles.size());
		int usize = int(unsyncedProjectiles.size());

		s->SerializeInt(&ssize);
		for (CProjectile* p: syncedProjectiles) {
			void** ptr = (void**) &p;
			s->SerializeObjectPtr(ptr, p->GetClass());
		}

		s->SerializeInt(&usize);
		for (CProjectile* p: unsyncedProjectiles) {
			void** ptr = (void**) &p;
			s->SerializeObjectPtr(ptr, p->GetClass());
		}
	} else {
		int ssize, usize;

		s->SerializeInt(&ssize);
		syncedProjectiles.resize(ssize);

		for (CProjectile* p: syncedProjectiles) {
			void** ptr = (void**) &p;
			s->SerializeObjectPtr(ptr, 0/*FIXME*/);
		}


		s->SerializeInt(&usize);
		unsyncedProjectiles.resize(usize);

		for (CProjectile* p: unsyncedProjectiles) {
			void** ptr = (void**) &p;
			s->SerializeObjectPtr(ptr, 0/*FIXME*/);
		}
	}
}


void CProjectileHandler::ConfigNotify(const std::string& key, const std::string& value)
{
	if (key != "MaxParticles" && key != "MaxNanoParticles")
		return;

	maxParticles     = configHandler->GetInt("MaxParticles");
	maxNanoParticles = configHandler->GetInt("MaxNanoParticles");
}


static void MAPPOS_SANITY_CHECK(const float3 v)
{
	v.AssertNaNs();
	assert(v.x >= -(float3::maxxpos * 16.0f));
	assert(v.x <=  (float3::maxxpos * 16.0f));
	assert(v.z >= -(float3::maxzpos * 16.0f));
	assert(v.z <=  (float3::maxzpos * 16.0f));
	assert(v.y >= -MAX_PROJECTILE_HEIGHT);
	assert(v.y <=  MAX_PROJECTILE_HEIGHT);
}


void CProjectileHandler::UpdateProjectileContainer(ProjectileContainer& pc, bool synced)
{
	// WARNING: we can't use iters here cause ProjectileCreated and ProjectileDestroyed events
	// may add new projectiles to the container!
	for (size_t i=0; i<pc.size();) {
		CProjectile* p = pc[i];
		assert(p);
		assert(p->synced == synced);
		assert(p->synced == !!(p->GetClass()->binder->flags & creg::CF_Synced));
		if (p->callEvent) {
		#if UNSYNCED_PROJ_NOEVENT
			if (synced) {
				eventHandler.ProjectileCreated(p, p->GetAllyteamID());
			} else {
				eventHandler.UnsyncedProjectileCreated(p);
			}
		#else
			eventHandler.ProjectileCreated(p, p->GetAllyteamID());
		#endif
			p->callEvent = false;
		}
		if (!p->deleteMe) {
			++i;
			continue;
		}
		pc[i] = pc.back();
		pc.pop_back();
		if (synced) { //FIXME move outside of loop!
			eventHandler.ProjectileDestroyed(p, p->GetAllyteamID());
			syncedProjectileIDs[p->id] = nullptr;
			freeSyncedIDs.push_back(p->id);
			ASSERT_SYNCED(p->pos);
			ASSERT_SYNCED(p->id);
		} else {
		#if UNSYNCED_PROJ_NOEVENT
			eventHandler.UnsyncedProjectileDestroyed(p);
		#else
			eventHandler.ProjectileDestroyed(p, p->GetAllyteamID());
			unsyncedProjectileIDs[p->id] = nullptr;
			freeUnsyncedIDs.push_back(p->id);
		#endif
		}
		delete p;
	}

	SCOPED_TIMER("ProjectileHandler::Update::PP");

	//WARNING: we can't use iters here cause p->Update() may add new projectiles to the container!
	for (size_t i=0; i<pc.size(); ++i) {
		CProjectile* p = pc[i];
		assert(p);

		MAPPOS_SANITY_CHECK(p->pos);

		p->Update();
		quadField->MovedProjectile(p);

		MAPPOS_SANITY_CHECK(p->pos);
	}
}


template<class T>
static void UPDATE_CONTAINER(T& cont) {
#ifdef DEBUG
	const auto* origStart = cont.empty() ? nullptr : &(*cont.begin());
#endif

	auto pti = cont.begin();
	while (pti != cont.end()) {
		auto* p = *pti;
		if (!p->Update()) {
			*pti = cont.back();
			cont.pop_back();
			delete p;
		} else {
			++pti;
		}
	}

	//WARNING: check if the vector got enlarged while iterating, in that case
	// all iterators got invalidated in the loop, causing crashes
	assert(cont.empty() || &(*cont.begin()) == origStart);
}


void CProjectileHandler::Update()
{
	{
		SCOPED_TIMER("ProjectileHandler::Update");

		// particles
		CheckCollisions(); // before :Update() to check if the particles move into stuff
		UpdateProjectileContainer(syncedProjectiles, true);
		UpdateProjectileContainer(unsyncedProjectiles, false);

		// groundflashes
		UPDATE_CONTAINER(groundFlashes);

		// flying pieces
		UPDATE_CONTAINER(flyingPiecesS3O);
		UPDATE_CONTAINER(flyingPieces3DO);

		// sort those every now and then
		FlyingPieceComparator fsort;
		if (resortFlyingPiecesS3O) std::sort(flyingPiecesS3O.begin(), flyingPiecesS3O.end(), fsort);
		if (resortFlyingPieces3DO) std::sort(flyingPieces3DO.begin(), flyingPieces3DO.end(), fsort);
	}

	// precache part of particles count calculation that else becomes very heavy
	lastCurrentParticles = 0;
	for (const CProjectile* p: syncedProjectiles) {
		lastCurrentParticles += p->GetProjectilesCount();
	}
	for (const CProjectile* p: unsyncedProjectiles) {
		lastCurrentParticles += p->GetProjectilesCount();
	}
	lastSyncedProjectilesCount = syncedProjectiles.size();
	lastUnsyncedProjectilesCount = unsyncedProjectiles.size();
}



void CProjectileHandler::AddProjectile(CProjectile* p)
{
	// already initialized?
	assert(p->id < 0);
	assert(p->callEvent);
	std::deque<int>* freeIDs = NULL;
	ProjectileMap* proIDs = NULL;

	if (p->synced) {
		syncedProjectiles.push_back(p);
		freeIDs = &freeSyncedIDs;
		proIDs = &syncedProjectileIDs;
		ASSERT_SYNCED(freeIDs->size());
	} else {
		unsyncedProjectiles.push_back(p);
#if UNSYNCED_PROJ_NOEVENT
		return;
#endif
		freeIDs = &freeUnsyncedIDs;
		proIDs = &unsyncedProjectileIDs;
	}

	if (freeIDs->empty()) {
		const size_t oldSize = proIDs->size();
		const size_t newSize = oldSize + 256;
		for (int i = oldSize; i < newSize; i++) {
			freeIDs->push_back(i);
		}
		if (p->synced) {
			std::random_shuffle(freeIDs->begin(), freeIDs->end(), gs->rng);
		} else{
			std::random_shuffle(freeIDs->begin(), freeIDs->end(), gu->rng);
		}
		proIDs->resize(newSize, nullptr);
	}

	p->id = freeIDs->front();
	freeIDs->pop_front();
	(*proIDs)[p->id] = p;

	if ((p->id) > (1 << 24)) {
		LOG_L(L_WARNING, "Lua %s projectile IDs are now out of range", (p->synced? "synced": "unsynced"));
	}

	if (p->synced) {
		ASSERT_SYNCED(p->id);
#ifdef TRACE_SYNC
		tracefile << "New projectile id: " << p->id << ", ownerID: " << p->GetOwnerID();
		tracefile << ", type: " << p->GetProjectileType() << " pos: <" << p->pos.x << ", " << p->pos.y << ", " << p->pos.z << ">\n";
#endif
	}
}




void CProjectileHandler::CheckUnitCollisions(
	CProjectile* p,
	std::vector<CUnit*>& tempUnits,
	const float3 ppos0,
	const float3 ppos1)
{
	CollisionQuery cq;

	for (CUnit* unit: tempUnits) {
		if (unit == NULL)
			break;

		// if this unit fired this projectile, always ignore
		if (unit == p->owner())
			continue;
		if (!unit->HasCollidableStateBit(CSolidObject::CSTATE_BIT_PROJECTILES))
			continue;
		
		if (p->GetAllyteamID() >= 0) {
			if (p->GetCollisionFlags() & Collision::NOFRIENDLIES) {
				if ( teamHandler->AlliedAllyTeams(p->GetAllyteamID(), unit->allyteam)) { continue; }
			}
			if (p->GetCollisionFlags() & Collision::NOENEMIES) {
				if (!teamHandler->AlliedAllyTeams(p->GetAllyteamID(), unit->allyteam)) { continue; }
			}
			if (p->GetCollisionFlags() & Collision::NONEUTRALS) {
				if (unit->IsNeutral()) { continue; }
			}
		}

		if (CCollisionHandler::DetectHit(unit, ppos0, ppos1, &cq)) {
			if (cq.GetHitPiece() != NULL) {
				unit->SetLastAttackedPiece(cq.GetHitPiece(), gs->frameNum);
			}

			if (!cq.InsideHit()) {
				p->SetPosition(cq.GetHitPos());
				p->Collision(unit);
				p->SetPosition(ppos0);
			} else {
				p->Collision(unit);
			}

			break;
		}
	}
}

void CProjectileHandler::CheckFeatureCollisions(
	CProjectile* p,
	std::vector<CFeature*>& tempFeatures,
	const float3 ppos0,
	const float3 ppos1)
{
	// already collided with unit?
	if (!p->checkCol)
		return;

	if ((p->GetCollisionFlags() & Collision::NOFEATURES) != 0)
		return;

	CollisionQuery cq;

	for (CFeature* feature: tempFeatures) {
		if (feature == NULL)
			break;

		if (!feature->HasCollidableStateBit(CSolidObject::CSTATE_BIT_PROJECTILES))
			continue;

		if (CCollisionHandler::DetectHit(feature, ppos0, ppos1, &cq)) {
			if (!cq.InsideHit()) {
				p->SetPosition(cq.GetHitPos());
				p->Collision(feature);
				p->SetPosition(ppos0);
			} else {
				p->Collision(feature);
			}

			break;
		}
	}
}

void CProjectileHandler::CheckUnitFeatureCollisions(ProjectileContainer& pc)
{
	static std::vector<CUnit*> tempUnits(unitHandler->MaxUnits(), NULL);
	static std::vector<CFeature*> tempFeatures(unitHandler->MaxUnits(), NULL);

	for (size_t i=0; i<pc.size(); ++i) {
		CProjectile* p = pc[i];
		if (!p->checkCol) continue;
		if ( p->deleteMe) continue;

		const float3 ppos0 = p->pos;
		const float3 ppos1 = p->pos + p->speed;

		quadField->GetUnitsAndFeaturesColVol(p->pos, p->radius + p->speed.w, tempUnits, tempFeatures);

		CheckUnitCollisions(p, tempUnits, ppos0, ppos1);
		CheckFeatureCollisions(p, tempFeatures, ppos0, ppos1);
	}
}

void CProjectileHandler::CheckGroundCollisions(ProjectileContainer& pc)
{
	for (size_t i=0; i<pc.size(); ++i) {
		CProjectile* p = pc[i];
		if (!p->checkCol)
			continue;

		// NOTE: if <p> is a MissileProjectile and does not
		// have selfExplode set, it will never be removed (!)
		if (p->GetCollisionFlags() & Collision::NOGROUND)
			continue;

		// NOTE: don't add p->radius to groundHeight, or most
		// projectiles will collide with the ground too early
		const float groundHeight = CGround::GetHeightReal(p->pos.x, p->pos.z);
		const bool belowGround = (p->pos.y < groundHeight);
		const bool insideWater = (p->pos.y <= 0.0f && !belowGround);
		const bool ignoreWater = p->ignoreWater;

		if (belowGround || (insideWater && !ignoreWater)) {
			// if position has dropped below terrain or into water
			// where we cannot live, adjust it and explode us now
			// (if the projectile does not set deleteMe = true, it
			// will keep hugging the terrain)
			p->pos.y = groundHeight * belowGround;
			p->Collision();
		}
	}
}

void CProjectileHandler::CheckCollisions()
{
	SCOPED_TIMER("ProjectileHandler::Update::CheckCollisions");

	CheckUnitFeatureCollisions(syncedProjectiles); //! changes simulation state
	CheckUnitFeatureCollisions(unsyncedProjectiles); //! does not change simulation state

	CheckGroundCollisions(syncedProjectiles); //! changes simulation state
	CheckGroundCollisions(unsyncedProjectiles); //! does not change simulation state
}



void CProjectileHandler::AddGroundFlash(CGroundFlash* flash)
{
	groundFlashes.push_back(flash);
}


void CProjectileHandler::AddFlyingPiece(
	const float3 pos,
	const float3 speed,
	int team,
	const S3DOPiece* piece,
	const S3DOPrimitive* chunk)
{
	FlyingPiece* fp = new S3DOFlyingPiece(pos, speed, team, piece, chunk);
	flyingPieces3DO.push_back(fp);
	resortFlyingPieces3DO = true;
}

void CProjectileHandler::AddFlyingPiece(
	const float3 pos,
	const float3 speed,
	int team,
	int textureType,
	const SS3OVertex* chunk)
{
	assert(textureType > 0);

	FlyingPiece* fp = new SS3OFlyingPiece(pos, speed, team, textureType, chunk);
	flyingPiecesS3O.push_back(fp);
	resortFlyingPiecesS3O = true;
}


void CProjectileHandler::AddNanoParticle(
	const float3 startPos,
	const float3 endPos,
	const UnitDef* unitDef,
	int teamNum,
	bool highPriority)
{
	const float priority = highPriority? HIGH_NANO_PRIO: NORMAL_NANO_PRIO;

	if (currentNanoParticles >= (maxNanoParticles * priority))
		return;
	if (!unitDef->showNanoSpray)
		return;

	float3 dif = (endPos - startPos);
	const float l = fastmath::apxsqrt2(dif.SqLength());

	dif /= l;
	dif += gu->RandVector() * 0.15f;

	SColor color(unitDef->nanoColor.r, unitDef->nanoColor.g, unitDef->nanoColor.b);
	if (globalRendering->teamNanospray) {
		const CTeam* team = teamHandler->Team(teamNum);
		color = SColor(team->color);
	}
	color.a = 20;
	new CNanoProjectile(startPos, dif, int(l), color);
}

void CProjectileHandler::AddNanoParticle(
	const float3 startPos,
	const float3 endPos,
	const UnitDef* unitDef,
	int teamNum,
	float radius,
	bool inverse,
	bool highPriority)
{
	const float priority = highPriority? HIGH_NANO_PRIO: NORMAL_NANO_PRIO;

	if (currentNanoParticles >= (maxNanoParticles * priority))
		return;
	if (!unitDef->showNanoSpray)
		return;

	float3 dif = (endPos - startPos);
	const float l = fastmath::apxsqrt2(dif.SqLength());
	dif /= l;

	float3 error = gu->RandVector() * (radius / l);

	SColor color(unitDef->nanoColor.r, unitDef->nanoColor.g, unitDef->nanoColor.b);
	if (globalRendering->teamNanospray) {
		const CTeam* team = teamHandler->Team(teamNum);
		color = SColor(team->color);
	}
	color.a = 20;

	if (!inverse) {
		new CNanoProjectile(startPos, (dif + error) * 3, int(l / 3), color);
	} else {
		new CNanoProjectile(startPos + (dif + error) * l, -(dif + error) * 3, int(l / 3), color);
	}
}


CProjectile* CProjectileHandler::GetProjectileBySyncedID(int id)
{
	if ((size_t)id < syncedProjectileIDs.size())
		return syncedProjectileIDs[id];
	return nullptr;
}


CProjectile* CProjectileHandler::GetProjectileByUnsyncedID(int id)
{
	if (UNSYNCED_PROJ_NOEVENT)
		return nullptr; // unsynced projectiles have no IDs if UNSYNCED_PROJ_NOEVENT

	if ((size_t)id < unsyncedProjectileIDs.size())
		return unsyncedProjectileIDs[id];
	return nullptr;
}


float CProjectileHandler::GetParticleSaturation(const bool withRandomization) const
{
	// use the random mult to weaken the max limit a little
	// so the chance is better spread when being close to the limit
	// i.e. when there are rockets that spam CEGs this gives smaller CEGs still a chance
	return (GetCurrentParticles() / float(maxParticles)) * (1.f + int(withRandomization) * 0.3f * gu->RandFloat());
}

int CProjectileHandler::GetCurrentParticles() const
{
	// use precached part of particles count calculation that else becomes very heavy
	// example where it matters: (in ZK) /cheat /give 20 armraven -> shoot ground
	int partCount = lastCurrentParticles;
	for (size_t i = lastSyncedProjectilesCount, e = syncedProjectiles.size(); i < e; ++i) {
		partCount += syncedProjectiles[i]->GetProjectilesCount();
	}
	for (size_t i = lastUnsyncedProjectilesCount, e = unsyncedProjectiles.size(); i < e; ++i) {
		partCount += unsyncedProjectiles[i]->GetProjectilesCount();
	}
	partCount += flyingPieces3DO.size();
	partCount += flyingPiecesS3O.size();
	partCount += groundFlashes.size();
	return partCount;
}
