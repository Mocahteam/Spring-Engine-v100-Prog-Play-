#ifndef CSPRINGMAP_H
#define CSPRINGMAP_H

class CSpringMap : public IMap {
public:
	//
	CSpringMap(springai::OOAICallback* callback, CSpringGame* game);
	virtual ~CSpringMap();


	virtual std::string MapName();

	virtual int SpotCount();
	virtual Position GetSpot(int idx);
	virtual std::vector<Position>& GetMetalSpots();

	virtual Position MapDimensions();

	virtual std::vector<IMapFeature*> GetMapFeatures();
	virtual std::vector<IMapFeature*> GetMapFeaturesAt(Position p, double radius);

	virtual double MinimumWindSpeed();
	virtual double MaximumWindSpeed();
	virtual double AverageWind();

	virtual float MaximumHeight();
	virtual float MinimumHeight();

	virtual double TidalStrength();

	virtual Position FindClosestBuildSite(IUnitType* t, Position builderPos, double searchRadius, double minimumDistance);
	virtual Position FindClosestBuildSiteFacing(IUnitType* t, Position builderPos, double searchRadius, double minimumDistance,int facing);

	virtual bool CanBuildHere(IUnitType* t, Position pos);
	virtual bool CanBuildHereFacing(IUnitType* t, Position pos,int facing);

	springai::Resource* GetMetalResource();

protected:
	std::vector<IMapFeature*>::iterator GetMapFeatureIteratorById(int id);
	void UpdateMapFeatures();

	springai::OOAICallback* callback;
	CSpringGame* game;

	std::vector<Position> metalspots;
	springai::Resource* metal;
	springai::Map* map;
	std::vector< IMapFeature*> mapFeatures;
	int lastMapFeaturesUpdate;
};

#endif


