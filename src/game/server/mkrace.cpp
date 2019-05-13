//mkRace
#include "gamecontext.h"
#include <engine/shared/config.h>
#include <game/server/player.h>
#include <game/server/gamemodes/race.h>
#include <game/version.h>

//bool CheckClientID(int ClientID);

void ToggleSpecPause(IConsole::IResult *pResult, void *pUserData, int PauseType)
{
	//if(!CheckClientID(pResult->GetInteger(0)))
	//	return;

	CGameContext *pSelf = (CGameContext *) pUserData;
	IServer* pServ = pSelf->Server();
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if(!pPlayer)
		return;

	int PauseState = pPlayer->IsPaused();
	if(PauseState > 0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "You are force-paused for %d seconds.", (PauseState - pServ->Tick()) / pServ->TickSpeed());
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "spec", aBuf);
	}
	else if(pResult->NumArguments() > 0)
	{
		if(-PauseState == PauseType && pPlayer->m_SpectatorID != pResult->m_ClientID && pServ->ClientIngame(pPlayer->m_SpectatorID) && !str_comp(pServ->ClientName(pPlayer->m_SpectatorID), pResult->GetString(0)))
		{
			pPlayer->Pause(CPlayer::PAUSE_NONE, false);
		}
		else
		{
			pPlayer->Pause(PauseType, false);
			pPlayer->SpectatePlayerName(pResult->GetString(0));
		}
	}
	else if(-PauseState == PauseType)
	{
		pPlayer->Pause(CPlayer::PAUSE_NONE, false);
	}
	else if(-PauseState != PauseType)
	{
		pPlayer->Pause(PauseType, false);
	}
}

void CGameContext::ConToggleSpec(IConsole::IResult *pResult, void *pUserData)
{
	ToggleSpecPause(pResult, pUserData, g_Config.m_SvPauseable ? CPlayer::PAUSE_SPEC : CPlayer::PAUSE_PAUSED);
}

void CGameContext::ConTogglePause(IConsole::IResult *pResult, void *pUserData)
{
	ToggleSpecPause(pResult, pUserData, CPlayer::PAUSE_PAUSED);
}

void CGameContext::ConSetEyeEmote(IConsole::IResult *pResult,
		void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	//if (!CheckClientID(pResult->m_ClientID))
	//	return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	if(pResult->NumArguments() == 0) {
		pSelf->m_pChatConsole->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"emote",
				(pPlayer->m_EyeEmote) ?
						"You can now use the preset eye emotes." :
						"You don't have any eye emotes, remember to bind some. (until you die)");
		return;
	}
	else if(str_comp_nocase(pResult->GetString(0), "on") == 0)
		pPlayer->m_EyeEmote = true;
	else if(str_comp_nocase(pResult->GetString(0), "off") == 0)
		pPlayer->m_EyeEmote = false;
	else if(str_comp_nocase(pResult->GetString(0), "toggle") == 0)
		pPlayer->m_EyeEmote = !pPlayer->m_EyeEmote;
	pSelf->m_pChatConsole->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"emote",
			(pPlayer->m_EyeEmote) ?
					"You can now use the preset eye emotes." :
					"You don't have any eye emotes, remember to bind some. (until you die)");
}

void CGameContext::ConEyeEmote(IConsole::IResult *pResult, void *pUserData, int Emote, int Duration)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (g_Config.m_SvEmotionalTees == -1)
	{
		pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "emote",
				"Emotes are disabled.");
		return;
	}

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if(pPlayer->m_LastEyeEmote + g_Config.m_SvEyeEmoteChangeDelay * pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;

	pPlayer->m_DefEmote = Emote;
	
	pPlayer->m_DefEmoteReset = pSelf->Server()->Tick() + Duration * pSelf->Server()->TickSpeed();
	pPlayer->m_LastEyeEmote = pSelf->Server()->Tick();
}

void CGameContext::ConEmote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;

	//if (!CheckClientID(pResult->m_ClientID))
	//	return;

	if (pResult->NumArguments() == 0)
	{
		pSelf->m_pChatConsole->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"emote",
				"Emote commands are: /emote surprise /emote blink /emote close /emote angry /emote happy /emote pain");
		pSelf->m_pChatConsole->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"emote",
				"Example: /emote surprise 10 for 10 seconds or /emote surprise (default 5 seconds)");
	}
	else
	{
		int Emote = -1;
		if (!str_comp(pResult->GetString(0), "angry"))
			Emote = EMOTE_ANGRY;
		else if (!str_comp(pResult->GetString(0), "blink"))
			Emote = EMOTE_BLINK;
		else if (!str_comp(pResult->GetString(0), "close"))
			Emote = EMOTE_BLINK;
		else if (!str_comp(pResult->GetString(0), "happy"))
			Emote = EMOTE_HAPPY;
		else if (!str_comp(pResult->GetString(0), "pain"))
			Emote = EMOTE_PAIN;
		else if (!str_comp(pResult->GetString(0), "surprise"))
			Emote = EMOTE_SURPRISE;
		else if (!str_comp(pResult->GetString(0), "normal"))
			Emote = EMOTE_NORMAL;
		else
			pSelf->m_pChatConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD,
					"emote", "Unknown emote... Say /emote");

		int Duration = 5;
		if (pResult->NumArguments() > 1)
			Duration = pResult->GetInteger(1);

		if(Emote != -1)
			ConEyeEmote(pResult, pUserData, Emote, Duration);
	}
}

void CGameContext::ConEmoteLove(IConsole::IResult *pResult, void *pUserData)
{
	ConEyeEmote(pResult, pUserData, EMOTE_HAPPY, 3600);	
}

void CGameContext::ConEmoteHate(IConsole::IResult *pResult, void *pUserData)
{
	ConEyeEmote(pResult, pUserData, EMOTE_ANGRY, 3600);	
}

void CGameContext::ConEmotePain(IConsole::IResult *pResult, void *pUserData)
{
	ConEyeEmote(pResult, pUserData, EMOTE_PAIN, 3600);	
}

void CGameContext::ConEmoteBlink(IConsole::IResult *pResult, void *pUserData)
{
	ConEyeEmote(pResult, pUserData, EMOTE_BLINK, 3600);	
}

void CGameContext::ConEmoteSurprise(IConsole::IResult *pResult, void *pUserData)
{
	ConEyeEmote(pResult, pUserData, EMOTE_SURPRISE, 3600);	
}

void CGameContext::ConEmoteReset(IConsole::IResult *pResult, void *pUserData)
{
	ConEyeEmote(pResult, pUserData, EMOTE_NORMAL, 0);	
}

void CGameContext::ConMe(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	//if (!CheckClientID(pResult->m_ClientID))
	//	return;

	char aBuf[256 + 24];

	str_format(aBuf, 256 + 24, "'%s' %s",
			pSelf->Server()->ClientName(pResult->m_ClientID),
			pResult->GetString(0));
	if (g_Config.m_SvSlashMe)
		pSelf->SendChat(-1, CHAT_ALL, -1, aBuf);
	else
		pSelf->m_pChatConsole->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"me",
				"/me is disabled on this server");
}