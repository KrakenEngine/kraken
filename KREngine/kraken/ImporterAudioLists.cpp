//
// ImporterAudioLists.cpp
//

#include "ImporterAudioLists.h"

//
// (1) UTILITIES
void IMPORTER_AUDIO_UTILITIES() { }

//
// Utility to convert a file code from a resource file into an object id
//      <n> will convert to <n>
//      <n>H will convert to <n> << 8
//      <n>A will convert to <n> << 16
// A result of 0 implies a bad input code
//

unsigned long codeToID(char *code) 
{
	static char decode[64];
	size_t l = strlen(code);
	if (l <= 0) return 0;	// no code string
	if (l > 63) l = 63;
	int type = 0;
	switch (code[l-1]) {
		case 'A' :
		case 'a' :
			type = 8;
		case 'H' :
		case 'h' :
			type += 8;
			strncpy(decode, code, l-1);         // (dst, src, maxlength)
            decode[l-1] = 0;
			break;
		default :
			strncpy(decode, code, l);
            decode[l] = 0;
			break;
	};
	unsigned long id = strtol(decode, 0, 0);    // (str, endptr, base)
	id = id << type;
	return id;
}

//
// (2) ZONE CORNERS
void IMPORTER_AUDIO_ZONE_CORNERS() { }

//
// (2a) A corners list for a given room/space

void zoneRoomCornersList::addCorner(zoneCorner *c) 
{
}

void zoneRoomCornersList::generateZones(simpleList *zonelist) 
{
}

//
// (2b) A list of all the corners lists

void zoneAllCornersList::addCorner(zoneCorner *c) 
{
}

void zoneAllCornersList::generateSpheresFor(unsigned long id, simpleList *zonelist) 
{
}

void zoneAllCornersList::generateSpheres(simpleList *zonelist) 
{
}

//
// (3) RESOURCES
void IMPORTER_AUDIO_RESOURCES() { }

bool zoneResourceList::load(char *path1, char *path2) 
{
    return true;
}

//
// (4) ZONE NODE MANAGER
void IMPORTER_AUDIO_MANAGER() { }

zoneNodeManager::zoneNodeManager() 
{
}

zoneNodeManager::~zoneNodeManager() 
{
}

// (1) parse the resource files and create the resource list
bool zoneNodeManager::loadResources(char *path1, char *path2)
{   
    return resources.load(path1, path2);
}

// (2) given a locator add it to the correct list

//
// insert vector table list into this space!

bool addLocator_audioNode(zoneNodeManager *manager, char *idstring, KRVector3 where, double radius)
{
    return true;
}

bool addLocator_ambientZone(zoneNodeManager *manager, char *idstring, KRVector3 where, double radius)
{
    return true;
}

bool addLocator_reverbZone(zoneNodeManager *manager, char *idstring, KRVector3 where, double radius)
{
    return true;
}

bool addLocator_ambientCorner(zoneNodeManager *manager, char *idstring, KRVector3 where, double radius)
{
    return true;
}

bool addLocator_reverbCorner(zoneNodeManager *manager, char *idstring, KRVector3 where, double radius)
{
    return true;
}

typedef bool (*addLocatorFunction)(zoneNodeManager *, char *, KRVector3, double);

typedef struct {
    const char *prefix;
    unsigned long prefix_length;
    addLocatorFunction function;
} addLocatorTableItem;

addLocatorTableItem addLocatorVectorTable[] = {
    { "AN_", 3, addLocator_audioNode },
    { "AZpoint_", 8, addLocator_ambientZone },
    { "RZpoint_", 8, addLocator_reverbZone },
    { "AZcorner_", 9, addLocator_ambientCorner },
    { "RZcorner_", 9, addLocator_reverbCorner },
    { NULL, 0, NULL },
};

addLocatorTableItem *addLocator_findType(char *name)
{
    addLocatorTableItem *result = addLocatorVectorTable;
    while (NULL != result->prefix)
    {
        if (0 == strncmp(result->prefix, name, result->prefix_length)) return result;
        result++;
    }
    return NULL;
}


bool zoneNodeManager::addLocatorToAList(char *name, KRVector3 point, double radius)
{
    // (1) parse the name and add it to a list where appropriate
    //      AN_<node_name>_<unique> is an audio node
    //      AZpoint_<n>_<unique> is an audio zone point with a radius
    //      RZpoint_<n><p>_<unique> is a reverb zone point with a radius
    //      AZcorner_<n>_<order> is a corner for an ambient zone
    //      RZcorner_<n><p>_<order> is a corner for a reverb zone
    //
    
    addLocatorTableItem *item = addLocator_findType(name);
    if (NULL != item)
    {
    //**** IN HERE we want to parse the args into the ID arg and the unique/order arg
    //**** UPDATE (*addLocatorFunction) to handle the 'order' arg
        char arg[AUDIO_IMPORTER_STRLEN];
        strncpy(arg, &name[item->prefix_length], AUDIO_IMPORTER_STRLEN-1);
        arg[AUDIO_IMPORTER_STRLEN-1] = 0;
        item->function(this, arg, point, radius);
        return true;    // return true if this is an audio locator that was added to a list
    }

    return false;       // return false if the FBX importer should handle the locator itself
}

// (3) turn corner definitions into spheres
void zoneNodeManager::generateSpheresForAllZones()
{
    ambientCornersList.generateSpheres(&ambientZoneList);
    reverbCornersList.generateSpheres(&reverbZoneList);
}

// (4) output ambient zones
// (5) output reverb zones
// (6) output audio nodes (props)
bool zoneNodeManager::outputAmbientZones()
{
    return true;
}

bool zoneNodeManager::outputReverbZones()
{
    return true;
}

bool zoneNodeManager::outputAudioNodes()
{
    return true;
}

// END OF ImporterAudioLists.cpp
