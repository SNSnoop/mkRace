/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <generated/server_data.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#include "character.h"
#include "pickup.h"

CPickup::CPickup(CGameWorld *pGameWorld, int Type, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP, Pos, PickupPhysSize)
{
	m_Type = Type;

	Reset();

	GameWorld()->InsertEntity(this);
}

void CPickup::Reset()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if (g_pData->m_aPickups[m_Type].m_Spawndelay > 0)
			m_SpawnTick[i] = Server()->Tick() + Server()->TickSpeed() * g_pData->m_aPickups[m_Type].m_Spawndelay;
		else
			m_SpawnTick[i] = -1;
	}
}

void CPickup::Tick()
{
	// wait for respawn
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_SpawnTick[i] > 0 && Server()->Tick() > m_SpawnTick[i] && g_Config.m_SvPickupRespawn > -1)
		{
			// respawn
			m_SpawnTick[i] = -1;

			if(m_Type == PICKUP_GRENADE || m_Type == PICKUP_SHOTGUN || m_Type == PICKUP_LASER)
				GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SPAWN, CmaskOne(i));
		}
	}
	// Check if a player intersected us
	CCharacter *apChrs[MAX_CLIENTS];
	int Num = GameServer()->m_World.FindEntities(m_Pos, 20.0f, (CEntity**)apChrs, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	for(int j = 0; j < Num; j++)
	{
		CCharacter *pChr = apChrs[j];
		int ClientID = pChr->GetPlayer()->GetCID();

		if(!pChr->IsAlive() || m_SpawnTick[ClientID] != -1)
			continue;

		// player picked us up, is someone was hooking us, let them go
		bool Picked = false;
		switch (m_Type)
		{
			case PICKUP_HEALTH:
				if(pChr->Freeze()) GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, CmaskOne(ClientID));
				break;

			case PICKUP_ARMOR:
				if(!pChr->m_FreezeTime && pChr->GetActiveWeapon() >= WEAPON_SHOTGUN)
					pChr->SetActiveWeapon(WEAPON_HAMMER);
				for(int i = WEAPON_SHOTGUN; i < NUM_WEAPONS; i++)
				{
					if(pChr->GetWeaponGot(i))
					{
						if(!(pChr->m_FreezeTime && i == WEAPON_NINJA))
						{
							pChr->SetWeaponGot(i, false);
							pChr->SetWeaponAmmo(i, 0);
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, CmaskOne(ClientID));
						}
					}
				}
				pChr->SetNinjaActivationDir(vec2(0,0));
				pChr->SetNinjaActivationTick(-500);
				pChr->SetNinjaCurrentMoveTime(0);
				break;

			case PICKUP_GRENADE:
				if(pChr->GiveWeapon(WEAPON_GRENADE, g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Maxammo))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, CmaskOne(ClientID));
					if(pChr->GetPlayer())
						GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), WEAPON_GRENADE);
				}
				break;
			case PICKUP_SHOTGUN:
				if(pChr->GiveWeapon(WEAPON_SHOTGUN, g_pData->m_Weapons.m_aId[WEAPON_SHOTGUN].m_Maxammo))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, CmaskOne(ClientID));
					if(pChr->GetPlayer())
						GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), WEAPON_SHOTGUN);
				}
				break;
			case PICKUP_LASER:
				if(pChr->GiveWeapon(WEAPON_LASER, g_pData->m_Weapons.m_aId[WEAPON_LASER].m_Maxammo))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, CmaskOne(ClientID));
					if(pChr->GetPlayer())
						GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), WEAPON_LASER);
				}
				break;

			case PICKUP_NINJA:
				{
					// activate ninja on target player
					pChr->GiveNinja();

					// loop through all players, setting their emotes
					/*
					CCharacter *pC = static_cast<CCharacter *>(GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER));
					for(; pC; pC = (CCharacter *)pC->TypeNext())
					{
						if (pC != pChr)
							pC->SetEmote(EMOTE_SURPRISE, Server()->Tick() + Server()->TickSpeed());
					}
					*/

					pChr->SetEmote(EMOTE_ANGRY, Server()->Tick() + 1200 * Server()->TickSpeed() / 1000);
					break;
				}

			default:
				break;
		};

		if(Picked)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "pickup player='%d:%s' item=%d",
				pChr->GetPlayer()->GetCID(), Server()->ClientName(pChr->GetPlayer()->GetCID()), m_Type);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
			/*int RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
			if(RespawnTime >= 0)
				m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * RespawnTime;*/

			if(g_Config.m_SvPickupRespawn > -1)
				m_SpawnTick[ClientID] = Server()->Tick() + Server()->TickSpeed() * g_Config.m_SvPickupRespawn;
			else
				m_SpawnTick[ClientID] = 1;
		}
	}
}

void CPickup::TickPaused()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(m_SpawnTick[i] != -1)
			++m_SpawnTick[i];
}

void CPickup::Snap(int SnappingClient)
{
	int SpecID = SnappingClient == -1 ? -1 : GameServer()->m_apPlayers[SnappingClient]->GetSpectatorID();
	int OwnerID = SpecID == -1 ? SnappingClient : SpecID;
	if((OwnerID != -1 && m_SpawnTick[OwnerID] != -1) || NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetID(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
}
