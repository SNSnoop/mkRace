#include <base/math.h>
#include <base/color.h>

#include <engine/shared/config.h>
#include <engine/storage.h>
#include <game/race.h>
#include <game/version.h>

#include "gamecontext.h"
#include "player.h"

#include "gamemodes/race.h"
#include "score.h"

void CGameContext::ChatConInfo(IConsole::IResult *pResult, void *pUser)
{
	CGameContext *pSelf = (CGameContext *)pUser;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Simple DDRace mod, modified from Race %s (C)Rajh, Redix and Sushi.", RACE_VERSION);
	pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", aBuf);
}

void CGameContext::ChatConTop5(IConsole::IResult *pResult, void *pUser)
{
	CGameContext *pSelf = (CGameContext *)pUser;

	if(!g_Config.m_SvShowTimes)
	{
		pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "Showing the Top5 is not allowed on this server.");
		return;
	}

	if(pResult->NumArguments() > 0)
		pSelf->Score()->ShowTop5(pSelf->m_ChatConsoleClientID, max(1, pResult->GetInteger(0)));
	else
		pSelf->Score()->ShowTop5(pSelf->m_ChatConsoleClientID);
}

void CGameContext::ChatConRank(IConsole::IResult *pResult, void *pUser)
{
	CGameContext *pSelf = (CGameContext *)pUser;

	if(g_Config.m_SvShowTimes && pResult->NumArguments() > 0)
	{
		char aStr[256];
		str_copy(aStr, pResult->GetString(0), sizeof(aStr));

		// strip trailing spaces
		int i = str_length(aStr);
		while(i >= 0)
		{
			if (aStr[i] < 0 || aStr[i] > 32)
				break;
			aStr[i] = 0;
			i--;
		}

		pSelf->Score()->ShowRank(pSelf->m_ChatConsoleClientID, aStr);
	}
	else
		pSelf->Score()->ShowRank(pSelf->m_ChatConsoleClientID);
}

void CGameContext::ChatConShowOthers(IConsole::IResult *pResult, void *pUser)
{
	CGameContext *pSelf = (CGameContext *)pUser;

	if(!g_Config.m_SvShowOthers)
		pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "This command is not allowed on this server.");
	else
		pSelf->m_apPlayers[pSelf->m_ChatConsoleClientID]->ToggleShowOthers();
}

void CGameContext::ChatConHelp(IConsole::IResult *pResult, void *pUser)
{
	CGameContext *pSelf = (CGameContext *)pUser;

	pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "---Command List---");
	pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "\"/info\" information about the mod");
	pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "\"/rank\" shows your rank");
	pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "\"/rank NAME\" shows the rank of a specific player");
	pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "\"/top5 X\" shows the top 5");
	// pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chat", "\"/show_others\" show other players?");
}

void CGameContext::InitChatConsole()
{
	if(m_pChatConsole)
		return;

	m_pChatConsole = CreateConsole(CFGFLAG_SERVERCHAT);
	m_ChatConsoleClientID = -1;

	m_pChatConsole->RegisterPrintCallback(IConsole::OUTPUT_LEVEL_STANDARD, SendChatResponse, this);

	m_pChatConsole->Register("info", "", CFGFLAG_SERVERCHAT, ChatConInfo, this, "");
	m_pChatConsole->Register("top5", "?i", CFGFLAG_SERVERCHAT, ChatConTop5, this, "");
	m_pChatConsole->Register("rank", "?r", CFGFLAG_SERVERCHAT, ChatConRank, this, "");
	// m_pChatConsole->Register("show_others", "", CFGFLAG_SERVERCHAT, ChatConShowOthers, this, "");
	m_pChatConsole->Register("help", "", CFGFLAG_SERVERCHAT, ChatConHelp, this, "");
}

void CGameContext::SendChatResponse(const char *pLine, void *pUser, bool Highlighted)
{
	CGameContext *pSelf = (CGameContext *)pUser;
	if(pSelf->m_ChatConsoleClientID == -1)
		return;

	static volatile int ReentryGuard = 0;
	if(ReentryGuard)
		return;
	ReentryGuard++;

	while(*pLine && *pLine != ' ')
		pLine++;
	if(*pLine && *(pLine + 1))
		pSelf->SendChat(-1, CHAT_ALL, pSelf->m_ChatConsoleClientID, pLine + 1);

	ReentryGuard--;
}

void CGameContext::LoadMapSettings()
{
	if(m_Layers.SettingsLayer())
	{
		CMapItemLayerTilemap *pLayer = m_Layers.SettingsLayer();
		CTile *pTiles = static_cast<CTile *>(m_Layers.Map()->GetData(pLayer->m_Data));
		char *pCommand = new char[pLayer->m_Width+1];
		pCommand[pLayer->m_Width] = 0;

		for(int i = 0; i < pLayer->m_Height; i++)
		{
			for(int j = 0; j < pLayer->m_Width; j++)
				pCommand[j] = pTiles[i*pLayer->m_Width+j].m_Index;
			Console()->ExecuteLineFlag(pCommand, CFGFLAG_MAPSETTINGS);
		}

		delete[] pCommand;
		m_Layers.Map()->UnloadData(pLayer->m_Data);
	}
}

int64 CmaskRace(CGameContext *pGameServer, int Owner)
{
	int64 Mask = CmaskOne(Owner);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pGameServer->m_apPlayers[i] && pGameServer->m_apPlayers[i]->ShowOthers())
			Mask = Mask | CmaskOne(i);
	}

	return Mask;
}
