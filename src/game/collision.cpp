/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <math.h>
#include <engine/map.h>
#include <engine/kernel.h>

#include <game/mapitems.h>
#include <game/layers.h>
#include <game/collision.h>

CCollision::CCollision()
{
	m_pTiles = 0;
	m_Width = 0;
	m_Height = 0;
	m_pLayers = 0;

	m_pTilesEx = 0;
	m_pTele = 0;
	m_pSpeedupForce = 0;
	m_pSpeedupAngle = 0;
	m_pTeleporter = 0;
	m_MainTiles = false;
	m_StopTiles = false;
}

CCollision::~CCollision()
{
	delete[] m_pTeleporter;
}

void CCollision::Init(class CLayers *pLayers)
{
	m_pTilesEx = 0;
	m_pTele = 0;
	m_pSpeedupForce = 0;
	m_pSpeedupAngle = 0;
	m_MainTiles = false;
	m_StopTiles = false;

	delete[] m_pTeleporter;
	m_pTeleporter = 0x0;

	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));

	if(m_pLayers->GameExLayer())
		m_pTilesEx = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameExLayer()->m_Data));

	if(m_pLayers->TeleLayer())
	{
		m_pTele = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->TeleLayer()->m_Data));
		InitTeleporter();
	}

	if(m_pLayers->SpeedupForceLayer() && m_pLayers->SpeedupAngleLayer() &&
		m_pLayers->SpeedupForceLayer()->m_Width == m_pLayers->SpeedupAngleLayer()->m_Width &&
		m_pLayers->SpeedupForceLayer()->m_Height == m_pLayers->SpeedupAngleLayer()->m_Height)
	{
		m_pSpeedupForce = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->SpeedupForceLayer()->m_Data));
		m_pSpeedupAngle = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->SpeedupAngleLayer()->m_Data));
	}

	for(int i = 0; i < m_Width*m_Height; i++)
	{
		int Index = m_pTiles[i].m_Index;

		if(Index > 128)
			continue;

		switch(Index)
		{
		case TILE_DEATH:
			m_pTiles[i].m_Index = COLFLAG_DEATH;
			break;
		case TILE_SOLID:
			m_pTiles[i].m_Index = COLFLAG_SOLID;
			break;
		case TILE_NOHOOK:
			m_pTiles[i].m_Index = COLFLAG_SOLID|COLFLAG_NOHOOK;
			break;
		case TILE_FREEZE:
			m_pTiles[i].m_Index = COLFLAG_FREEZE;
			break;
		case TILE_UNFREEZE:
			m_pTiles[i].m_Index = COLFLAG_UNFREEZE;
			break;
		default:
			m_pTiles[i].m_Index = 0;
		}

		// race tiles
		if(Index >= TILE_TELEIN_STOP && Index <= 59)
			m_pTiles[i].m_Index = Index;
		if(CheckIndexExRange(i, TILE_BEGIN, 59) != -1)
			m_MainTiles = true;
		if(CheckIndexExRange(i, TILE_STOPL, TILE_STOPT) != -1)
			m_StopTiles = true;
	}
}

void CCollision::InitTeleporter()
{
	int ArraySize = 0;
	int Width = m_pLayers->TeleLayer()->m_Width;
	int Height = m_pLayers->TeleLayer()->m_Height;

	for(int i = 0; i < Width * Height; i++)
		ArraySize = max(ArraySize, (int)m_pTele[i].m_Index);

	if(!ArraySize)
		return;

	m_pTeleporter = new vec2[ArraySize];
	mem_zero(m_pTeleporter, ArraySize*sizeof(vec2));

	// assign the values
	for(int i = 0; i < m_Width * m_Height; i++)
	{
		int TilePosTele = GetTilePosLayer(m_pLayers->TeleLayer(), i);
		if(TilePosTele >= 0 && m_pTele[TilePosTele].m_Index > 0 && CheckIndexEx(i, TILE_TELEOUT))
			m_pTeleporter[m_pTele[TilePosTele].m_Index - 1] = vec2(i % Width * 32 + 16, i / Width * 32 + 16);
	}
}

int CCollision::GetTile(int x, int y) const
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

	int Index = m_pTiles[Ny*m_Width+Nx].m_Index;
	if(Index == COLFLAG_SOLID || Index == (COLFLAG_SOLID|COLFLAG_NOHOOK) || Index == COLFLAG_DEATH 
		|| Index == COLFLAG_FREEZE  || Index == COLFLAG_UNFREEZE)
		return Index;
	else
		return 0;
}

bool CCollision::IsTileSolid(int x, int y) const
{
	return GetTile(x, y)&COLFLAG_SOLID;
}

// race
int CCollision::GetTilePos(vec2 Pos)
{
	int Nx = clamp((int)Pos.x/32, 0, m_Width-1);
	int Ny = clamp((int)Pos.y/32, 0, m_Height-1);
	
	return Ny*m_Width+Nx;
}

int CCollision::GetTilePosLayer(const CMapItemLayerTilemap *pLayer, int TilePos)
{
	int x = TilePos % m_Width;
	int y = TilePos / m_Width;

	if(TilePos < 0 || !pLayer || x < 0 || y < 0 || x >= pLayer->m_Width || y >= pLayer->m_Height)
		return -1;

	return y*pLayer->m_Width+x;
}

vec2 CCollision::GetPos(int TilePos)
{
	int x = TilePos%m_Width;
	int y = TilePos/m_Width;
	
	return vec2(x*32+16, y*32+16);
}

bool CCollision::CheckIndexEx(int TilePos, int Index)
{
	if(TilePos >= 0 && m_pTiles[TilePos].m_Index == Index)
		return true;
	int TilePosEx = GetTilePosLayer(m_pLayers->GameExLayer(), TilePos);
	if(TilePosEx != -1 && m_pTilesEx[TilePosEx].m_Index == Index)
		return true;
	return false;
}

int CCollision::CheckIndexExRange(int TilePos, int MinIndex, int MaxIndex)
{
	if(TilePos >= 0 && m_pTiles[TilePos].m_Index >= MinIndex && m_pTiles[TilePos].m_Index <= MaxIndex)
		return m_pTiles[TilePos].m_Index;
	int TilePosEx = GetTilePosLayer(m_pLayers->GameExLayer(), TilePos);
	if(TilePosEx >= 0 && m_pTilesEx[TilePosEx].m_Index >= MinIndex && m_pTilesEx[TilePosEx].m_Index <= MaxIndex)
		return m_pTilesEx[TilePosEx].m_Index;
	return -1;
}

bool CCollision::IsRaceTile(int TilePos, int Mask)
{
	if(Mask&RACECHECK_TILES_MAIN && CheckIndexExRange(TilePos, TILE_BEGIN, 59) != -1)
		return true;
	if(Mask&RACECHECK_TILES_STOP && CheckIndexExRange(TilePos, TILE_STOPL, TILE_STOPT) != -1)
		return true;
	if(Mask&RACECHECK_TELE && CheckIndexExRange(TilePos, TILE_TELEIN_STOP, TILE_TELEIN) != -1)
		return true;
	if(Mask&RACECHECK_SPEEDUP && CheckIndexEx(TilePos, TILE_BOOST))
		return true;
	return false;
}

int CCollision::CheckRaceTile(vec2 PrevPos, vec2 Pos, int Mask)
{
	if(Mask&RACECHECK_TILES_MAIN && !m_MainTiles)
		Mask ^= RACECHECK_TILES_MAIN;
	if(Mask&RACECHECK_TILES_STOP && !m_StopTiles)
		Mask ^= RACECHECK_TILES_STOP;
	if(Mask&RACECHECK_TELE && !m_pTeleporter)
		Mask ^= RACECHECK_TELE;
	if(Mask&RACECHECK_SPEEDUP && !m_pSpeedupForce)
		Mask ^= RACECHECK_SPEEDUP;

	if(!Mask)
		return -1;

	float Distance = distance(PrevPos, Pos);
	int End = Distance+1;

	for(int i = 0; i <= End; i++)
	{
		float a = i/float(End);
		vec2 Tmp = mix(PrevPos, Pos, a);
		int TilePos = GetTilePos(Tmp);
		if(IsRaceTile(TilePos, Mask))
			return TilePos;
	}

	return -1;
}

int CCollision::CheckCheckpoint(int TilePos)
{
	int Cp = CheckIndexExRange(TilePos, 35, 59);
	if(Cp != -1)
		return Cp-35;
	return -1;
}

int CCollision::CheckSpeedup(int TilePos)
{
	int TilePosTele = GetTilePosLayer(m_pLayers->SpeedupForceLayer(), TilePos);
	if(TilePosTele < 0 || !CheckIndexEx(TilePos, TILE_BOOST) || m_pSpeedupForce[TilePosTele].m_Index == 0)
		return -1;
	return TilePosTele;
}

void CCollision::GetSpeedup(int SpeedupPos, vec2 *Dir, int *Force)
{
	int SpeedupAngle = m_pSpeedupAngle[SpeedupPos].m_Index % 90;
	unsigned char Flags = m_pSpeedupAngle[SpeedupPos].m_Flags;
	// TODO: handle all cases
	if(Flags == ROTATION_90)
		SpeedupAngle += 90;
	else if(Flags == ROTATION_180)
		SpeedupAngle += 180;
	else if(Flags == ROTATION_270)
		SpeedupAngle += 270;
	float Angle = SpeedupAngle * pi / 180.0f;
	*Force = m_pSpeedupForce[SpeedupPos].m_Index;
	*Dir = vec2(cos(Angle), sin(Angle));
}

int CCollision::CheckTeleport(int TilePos, bool *pStop)
{
	int TilePosTele = GetTilePosLayer(m_pLayers->TeleLayer(), TilePos);
	if(TilePosTele >= 0)
	{
		int Index = CheckIndexExRange(TilePos, TILE_TELEIN_STOP, TILE_TELEIN);
		*pStop = Index == TILE_TELEIN_STOP;
		if(Index != -1)
			return m_pTele[TilePosTele].m_Index;
	}
	return 0;
}

vec2 CCollision::GetTeleportDestination(int Number)
{
	if(m_pTeleporter && Number > 0)
		return m_pTeleporter[Number - 1];
	return vec2(0,0);
}

// TODO: rewrite this smarter!
int CCollision::IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision) const
{
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);
	vec2 Last = Pos0;

	for(int i = 0; i <= End; i++)
	{
		float a = i/float(End);
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y))
		{
			if(pOutCollision)
				*pOutCollision = Pos;
			if(pOutBeforeCollision)
				*pOutBeforeCollision = Last;
			return GetCollisionAt(Pos.x, Pos.y);
		}
		Last = Pos;
	}
	if(pOutCollision)
		*pOutCollision = Pos1;
	if(pOutBeforeCollision)
		*pOutBeforeCollision = Pos1;
	return 0;
}

// TODO: OPT: rewrite this smarter!
void CCollision::MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces) const
{
	if(pBounces)
		*pBounces = 0;

	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;
	if(CheckPoint(Pos + Vel))
	{
		int Affected = 0;
		if(CheckPoint(Pos.x + Vel.x, Pos.y))
		{
			pInoutVel->x *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
		}

		if(CheckPoint(Pos.x, Pos.y + Vel.y))
		{
			pInoutVel->y *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
		}

		if(Affected == 0)
		{
			pInoutVel->x *= -Elasticity;
			pInoutVel->y *= -Elasticity;
		}
	}
	else
	{
		*pInoutPos = Pos + Vel;
	}
}

bool CCollision::TestBox(vec2 Pos, vec2 Size) const
{
	Size *= 0.5f;
	if(CheckPoint(Pos.x-Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x-Size.x, Pos.y+Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y+Size.y))
		return true;
	return false;
}

void CCollision::MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity) const
{
	// do the move
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;

	float Distance = length(Vel);
	int Max = (int)Distance;

	if(Distance > 0.00001f)
	{
		//vec2 old_pos = pos;
		float Fraction = 1.0f/(float)(Max+1);
		for(int i = 0; i <= Max; i++)
		{
			//float amount = i/(float)max;
			//if(max == 0)
				//amount = 0;

			vec2 NewPos = Pos + Vel*Fraction; // TODO: this row is not nice

			if(TestBox(vec2(NewPos.x, NewPos.y), Size))
			{
				int Hits = 0;

				if(TestBox(vec2(Pos.x, NewPos.y), Size))
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					Hits++;
				}

				if(TestBox(vec2(NewPos.x, Pos.y), Size))
				{
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
					Hits++;
				}

				// neither of the tests got a collision.
				// this is a real _corner case_!
				if(Hits == 0)
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
				}
			}

			Pos = NewPos;
		}
	}

	*pInoutPos = Pos;
	*pInoutVel = Vel;
}
