/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/vmath.h>

enum
{
	PHYSICSFLAG_STOPPER=1,
	PHYSICSFLAG_TELEPORT=2,
	PHYSICSFLAG_SPEEDUP=4,

	PHYSICSFLAG_RACE_ALL=PHYSICSFLAG_STOPPER|PHYSICSFLAG_TELEPORT|PHYSICSFLAG_SPEEDUP,
};

typedef void (*FPhysicsStepCallback)(vec2 Pos, float IntraTick, void *pUserData);

struct CCollisionData
{
	FPhysicsStepCallback m_pfnPhysicsStepCallback;
	void *m_pPhysicsStepUserData;
	int m_PhysicsFlags;
	ivec2 m_LastSpeedupTilePos;
	bool m_Teleported;
};

class CCollision
{
	class CTile *m_pTiles;
	class CTeleTile *m_pTele;
	class CSpeedupTile *m_pSpeedup;
	class CTile *m_pFront;
	class CTuneTile *m_pTune;
	class CDoorTile *m_pDoor;

	int m_Width;
	int m_Height;
	class CLayers *m_pLayers;

	vec2 m_aTeleporter[1<<8];

	void InitTeleporter();

	bool IsTileSolid(int x, int y) const;
	int GetTile(int x, int y) const;

	bool IsRaceTile(int TilePos, int Mask);

public:
	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_NOHOOK=4,
	};

	CCollision();
	void Init(class CLayers *pLayers);
	bool CheckPoint(float x, float y) const { return IsTileSolid(round_to_int(x), round_to_int(y)); }
	bool CheckPoint(vec2 Pos) const { return CheckPoint(Pos.x, Pos.y); }
	int GetCollisionAt(float x, float y) const { return GetTile(round_to_int(x), round_to_int(y)); }
	int GetWidth() const { return m_Width; };
	int GetHeight() const { return m_Height; };
	int IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision) const;
	int IntersectLineTeleWeapon(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, int *pTeleNr);
	int IntersectLineTeleHook(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, int *pTeleNr);
	void MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces) const;
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity, CCollisionData *pCollisionData = 0) const;
	bool TestBox(vec2 Pos, vec2 Size) const;

	// race
	vec2 GetPos(int TilePos) const;
	int GetTilePosLayer(const class CMapItemLayerTilemap *pLayer, vec2 Pos) const;

	bool CheckIndexEx(vec2 Pos, int Index) const;
	int CheckIndexExRange(vec2 Pos, int MinIndex, int MaxIndex) const;

	int CheckCheckpoint(vec2 Pos) const;
	vec2 GetTeleportDestination(int Tele) const { return m_aTeleporter[Tele - 1]; };

	class CTeleTile *TeleLayer() { return m_pTele; }

	int GetIndex(int x, int y);
	int GetIndex(vec2 PrevPos, vec2 Pos);
	int GetFIndex(int x, int y);
	int IsNoLaser(int x, int y);
	int IsFNoLaser(int x, int y);
	int GetTileIndex(int Index);
	int GetFTileIndex(int Index);
	int GetTileFlags(int Index);
	int GetFTileFlags(int Index);
	int IntersectNoLaser(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision);
	int IntersectNoLaserNW(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision);
	int GetPureMapIndex(float x, float y);
	int GetPureMapIndex(vec2 Pos) { return GetPureMapIndex(Pos.x, Pos.y); }
	int IsTeleport(int Index);
	bool TileExists(int Index);
	bool TileExistsNext(int Index);
	vec2 CpSpeed(int index, int Flags = 0);
	int GetMapIndex(vec2 Pos);
	int IsEvilTeleport(int Index);
	int IsCheckTeleport(int Index);
	int IsCheckEvilTeleport(int Index);
	int IsTeleportHook(int Index);
	int IsMover(int x, int y, int *pFlags);
	bool IsThrough(int x, int y, int xoff, int yoff, vec2 pos0, vec2 pos1);
	bool IsHookBlocker(int x, int y, vec2 pos0, vec2 pos1);
	int GetFCollisionAt(float x, float y) { return GetFTile(round_to_int(x), round_to_int(y)); }
	int GetFTile(int x, int y);
	int IsSpeedup(int Index);
	void GetSpeedup(int Index, vec2 *Dir, int *Force, int *MaxSpeed);
};

void ThroughOffset(vec2 Pos0, vec2 Pos1, int *Ox, int *Oy);
#endif
