# <kb>.so

A small SRCDS server plugin to remove knockback that is present upon all types of damage in the source engine. 
Compile with ```make``` and treat like a normal valve server plugin. See [here](https://developer.valvesoftware.com/wiki/Server_plugins) for more details. A server plugin config is available in the root of the repository. 

# Method of operation
Uses a sigscan (byte pattern matching) to find CBasePlayer::OnTakeDamageAlive, which contains code to apply knockback and do other housekeeping when a player takes damage. A static offset is used from the function base to replace the call to the velocity function with no-ops. A disassembler was used to generate this information. OnTakeDamageAlive is never changed between game updates, so an offset and sigscan is sufficient to persistently solve this problem. Recompilation should never be necessary. 

# Application
This addon is useful in Garry's Mod, where TakeDamage() calls from Lua can introduce unwanted velocity changes. 