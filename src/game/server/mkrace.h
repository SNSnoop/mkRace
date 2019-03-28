
#ifndef CHAT_COMMAND
#define CHAT_COMMAND(name, params, flags, callback, userdata, help)
#endif

CHAT_COMMAND("pause", "?r[player name]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConTogglePause, this, "Toggles pause")
CHAT_COMMAND("spec", "?r[player name]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConToggleSpec, this, "Toggles spec (if not available behaves as /pause)")
CHAT_COMMAND("emote", "?s[emote name] i[duration in seconds]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEyeEmote, this, "Sets your tee's eye emote")

#undef CHAT_COMMAND