
#ifndef CHAT_COMMAND
#define CHAT_COMMAND(name, params, flags, callback, userdata, help)
#endif

CHAT_COMMAND("pause", "?r[player name]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConTogglePause, this, "Toggles pause")
CHAT_COMMAND("spec", "?r[player name]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConToggleSpec, this, "Toggles spec (if not available behaves as /pause)")
CHAT_COMMAND("emote", "?s[emote name] i[duration in seconds]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEmote, this, "Sets your tee's eye emote")
CHAT_COMMAND("me", "r[message]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConMe, this, "Like the famous irc command '/me says hi' will display '<yourname> says hi'")

CHAT_COMMAND("love", "", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEmoteLove, this, "Alias for Happy emote")
CHAT_COMMAND("hate", "", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEmoteHate, this, "Alias for Angry emote")
CHAT_COMMAND("pain", "", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEmotePain, this, "Alias for Pain emote")
CHAT_COMMAND("blink", "", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEmoteBlink, this, "Alias for Blink emote")
CHAT_COMMAND("surprise", "", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEmoteSurprise, this, "Alias for Surprise emote")
CHAT_COMMAND("reset_emote", "", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConEmoteReset, this, "Alias for Default emote")

CHAT_COMMAND("dr", "", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConDisconnectRescue, this, "Rescue to location before disconnect")
CHAT_COMMAND("swap", "?r[playername]", CFGFLAG_SERVERCHAT|CFGFLAG_SERVER, ConSwap, this, "Swap places with any player")

#undef CHAT_COMMAND