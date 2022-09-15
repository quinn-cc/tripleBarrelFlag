/*	
 Custom flag: Triple Barrel (+TB)
 Tank shoots a spray of three shots.
 
 Server Variables:
 _tripleBarrelAngle - the angle at which the middle bullet and a side bullet are at.
 
 Extra notes:
 - The two extra shots are world weapons, which contain metadata 'type' and 'owner'.
 
 Copyright 2022 Quinn Carmack
 May be redistributed under either the LGPL or MIT licenses.
 
 ./configure --enable-custom-plugins=tripleBarrelFlag
*/
#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

using namespace std;

class TripleBarrelFlag : public bz_Plugin
{
    virtual const char* Name ()
    {
        return "Triple Barrel Flag";
    }
    virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~TripleBarrelFlag();

	virtual void Cleanup(void)
	{
		Flush();
	}
};

BZ_PLUGIN(TripleBarrelFlag)

void TripleBarrelFlag::Init(const char*) {
	
	bz_RegisterCustomFlag("TB", "Triple Barrel", "Tank shoots a spray of three shots.", 0, eGoodFlag);
	bz_registerCustomBZDBDouble("_tripleBarrelAngle", 0.12);
	Register(bz_eShotFiredEvent);
	Register(bz_ePlayerDieEvent);
}

TripleBarrelFlag::~TripleBarrelFlag() {}

void TripleBarrelFlag::Event(bz_EventData *ed)
{
	switch (ed->eventType)
	{
		case bz_eShotFiredEvent:
		{
			bz_ShotFiredEventData_V1* data = (bz_ShotFiredEventData_V1*) ed;
			bz_BasePlayerRecord *playerRecord = bz_getPlayerByIndex(data->playerID);

			if (playerRecord && playerRecord->currentFlag == "Triple Barrel (+TB)")
			{
				float pos[3];
				pos[0] = playerRecord->lastKnownState.pos[0] + cos(playerRecord->lastKnownState.rotation)*bz_getBZDBDouble("_muzzleFront");
				pos[1] = playerRecord->lastKnownState.pos[1] + sin(playerRecord->lastKnownState.rotation)*bz_getBZDBDouble("_muzzleFront");
				pos[2] = playerRecord->lastKnownState.pos[2] + bz_getBZDBDouble("_muzzleHeight");
				
				// Left shot
				float vel1[3];
				vel1[0] = cos(playerRecord->lastKnownState.rotation - bz_getBZDBDouble("_tripleBarrelAngle")) + playerRecord->lastKnownState.velocity[0]/bz_getBZDBDouble("_shotSpeed");
				vel1[1] = sin(playerRecord->lastKnownState.rotation - bz_getBZDBDouble("_tripleBarrelAngle")) + playerRecord->lastKnownState.velocity[1]/bz_getBZDBDouble("_shotSpeed");
				vel1[2] = 0;
				uint32_t shot1GUID = bz_fireServerShot("TB", pos, vel1, playerRecord->team);
				bz_setShotMetaData(shot1GUID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(shot1GUID, "owner", data->playerID);
				
				// Right shot
				float vel2[3];
				vel2[0] = cos(playerRecord->lastKnownState.rotation + bz_getBZDBDouble("_tripleBarrelAngle")) + playerRecord->lastKnownState.velocity[0]/bz_getBZDBDouble("_shotSpeed");
				vel2[1] = sin(playerRecord->lastKnownState.rotation + bz_getBZDBDouble("_tripleBarrelAngle")) + playerRecord->lastKnownState.velocity[1]/bz_getBZDBDouble("_shotSpeed");
				vel2[2] = 0;
				uint32_t shot2GUID = bz_fireServerShot("TB", pos, vel2, playerRecord->team);
				bz_setShotMetaData(shot2GUID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(shot2GUID, "owner", data->playerID);
			}
			
			bz_freePlayerRecord(playerRecord);
		} break;
		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V1* data = (bz_PlayerDieEventData_V1*) ed;
			uint32_t shotGUID = bz_getShotGUID(data->killerID, data->shotID);

			if (bz_shotHasMetaData(shotGUID, "type") && bz_shotHasMetaData(shotGUID, "owner"))
			{
			    std::string flagType = bz_getShotMetaDataS(shotGUID, "type");

			    if (flagType == "TB")
			    {
			        data->killerID = bz_getShotMetaDataI(shotGUID, "owner");
			        data->killerTeam = bz_getPlayerTeam(data->killerID);
			    }
			}
		} break;
		default:
			break;
	}
}
