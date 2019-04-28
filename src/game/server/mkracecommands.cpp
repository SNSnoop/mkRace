#include "gamecontext.h"
#include "entities/character.h"
#include <engine/shared/config.h>
#include <game/version.h>
#include <game/server/player.h>
#include <game/server/gamemodes/race.h>

//bool CheckClientID(int ClientID);

void CGameContext::ConGoLeft(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	//if (!CheckClientID(pResult->m_ClientID))
	//	return;
	pSelf->MoveCharacter(pResult->m_ClientID, -1, 0);
}

void CGameContext::ConGoRight(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	//if (!CheckClientID(pResult->m_ClientID))
	//	return;
	pSelf->MoveCharacter(pResult->m_ClientID, 1, 0);
}

void CGameContext::ConGoDown(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	//if (!CheckClientID(pResult->m_ClientID))
	//	return;
	pSelf->MoveCharacter(pResult->m_ClientID, 0, 1);
}

void CGameContext::ConGoUp(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	//if (!CheckClientID(pResult->m_ClientID))
	//	return;
	pSelf->MoveCharacter(pResult->m_ClientID, 0, -1);
}

void CGameContext::MoveCharacter(int ClientID, int X, int Y, bool Raw)
{
	CCharacter *pChr = GetPlayerChar(ClientID);

	if (!pChr)
		return;
	pChr->m_Pos.x += ((Raw) ? 1 : 32) * X;
	pChr->m_Pos.y += ((Raw) ? 1 : 32) * Y;
}

