//
// ImporterAudioLists.h
//

#ifndef _IMPORTER_AUDIO_LISTS_H_
#define _IMPORTER_AUDIO_LISTS_H_

#include "KRVector3.h"
#include "simpleList.h"

enum {
    AUDIO_IMPORTER_STRLEN = 128
};

//
// (1) Nodes and Dummy Props

// A locator point
//      AN_<node_name>_<unique>

struct audioPoint : public simpleListObject {
	KRVector3 point;
    char name[AUDIO_IMPORTER_STRLEN];     // this could change to a std::string
};

//
// (2) Zone Locators - for ambient zones and reverb zones

// A zone sphere object
// 		RZpoint_<n><p>_<unique> locRadius and AZpoint_<n>_<unique> locRadius

struct zone : public simpleListObject {
	KRVector3 point;
	double radius;
	unsigned long id;
};

// A zone corner object
// 		RZcorner_<n><p>_<order_clockwise> and AZcorner_<n>_<order_clockwise>

struct zoneCorner : public simpleListObject {
	KRVector3 point;
    unsigned long id;
};

//
// A list of corners for a particular location

struct zoneRoomCornersList : public simpleListObject {
	unsigned long id;
	simpleList corners;		// the zoneCorner objects for a given id
	
	void addCorner(zoneCorner *c);
        // add it into the list in presorted place/location_number order starting bottom left and going clockwise
	void generateZones(simpleList *zonelist);
        // create a set of spheres that fill in the space defined by the corners
        // and add these spheres to the 'zonelist'
};

//
// A master list of lists of corners

struct zoneAllCornersList : public simpleList {
	void addCorner(zoneCorner *c);
        // find or create a zoneRoomCornersList for the incoming id, then use that list's addCorner() method
    
	void generateSpheresFor(unsigned long id, simpleList *zonelist);
	void generateSpheres(simpleList *zonelist);
        // routines to generate circle points for the geometery of a given place + location_number
        // .. and to place these cirlces into a given zone list
};

// A resource object
//		<n><p> <resource file name> or <n> <resource file name>

struct zoneResource : public simpleListObject {
	unsigned long id;
	char resource_name[AUDIO_IMPORTER_STRLEN];
};

struct zoneResourceList : public simpleList {
	bool load(char *path1, char *path2 = NULL);
        // all resources are loaded into the same list from either 1 or 2 input files
        // we can either put all the reverb and ambient info in 1 file, or split it across 2 files
};

//
// (3) Zone and Node Manager

struct zoneNodeManager {
    zoneResourceList resources;
    
    simpleList ambientZoneList;                 //  list of 'zone' objects
    zoneAllCornersList ambientCornersList;
    
    simpleList reverbZoneList;                  //  list of 'zone' objects
    zoneAllCornersList reverbCornersList;
    
    simpleList audioNodesList;                  // a list of 'audioPoint' objects
    
    zoneNodeManager();
    virtual ~zoneNodeManager();
    
    // (1) parse the resource files and create the resource list
    bool loadResources(char *path1, char *path2=NULL);  // passed through to resources.load()
    
    // (2) given a locator add it to the correct list
    bool addLocatorToAList(char *name, KRVector3 point, double radius);
    
    // (3) turn corner definitions into spheres
    void generateSpheresForAllZones();
    
    // (4) output ambient zones
    // (5) output reverb zones
    // (6) output audio nodes (props)
    virtual bool outputAmbientZones();
    virtual bool outputReverbZones();
    virtual bool outputAudioNodes();
};

#endif

// END OF ImporterAudioLists.h
