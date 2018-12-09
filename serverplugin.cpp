#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <math.h>

#define PAGE_SIZE			4096
#define PAGE_ALIGN_UP(x)	((x + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(x)	(x & ~(PAGE_SIZE-1))

typedef enum
{
	PLUGIN_CONTINUE = 0,
	PLUGIN_OVERRIDE,
	PLUGIN_STOP,
} PLUGIN_RESULT;

typedef enum
{
    eQueryCvarValueStatus_ValueIntact=0,
    eQueryCvarValueStatus_CvarNotFound=1,
    eQueryCvarValueStatus_NotACvar=2,
    eQueryCvarValueStatus_CvarProtected=3
} EQueryCvarValueStatus;

typedef int QueryCvarCookie_t;
class CCommand;
class IServerPluginCallbacks;
struct edict_t;
typedef void* QueryValveInterface;

void *FindSignature(char *pBaseAddress, size_t baseLength, const char *pSignature) {
	char *pEndPtr = pBaseAddress + baseLength;
	unsigned int len = strlen(pSignature);
	
	while (pBaseAddress < pEndPtr)
	{
		bool found = true;
		for (register size_t i = 0; i < len; i++)
		{
			if (pSignature[i] != '?' && pSignature[i] != pBaseAddress[i])
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			return pBaseAddress;
		}

		++pBaseAddress;
	}
	return 0;
}

class ServerPlugin
{
public:
	ServerPlugin()
	{
	}
	~ServerPlugin()
	{
	}
	virtual bool Load(QueryValveInterface engineFactory, QueryValveInterface gsFactory)
	{
		Dl_info info;
		
		if(!dladdr(gsFactory, &info))
			return false;
		
		if(!info.dli_fbase || !info.dli_fname)
			return false;
		
		char* pBaseAddr = (char*)info.dli_fbase;
		
		struct stat buf;
		
		if(stat(info.dli_fname, &buf) != 0)
			return false;
		
		size_t memLen = buf.st_size;
		
		printf("\nOBJECT INFO: %p img_base, %.2fMB in memory\n", pBaseAddr, memLen / (1024.0f*1024.0f));
		/*CBasePlayer::OnTakeDamage_Alive 
		Look for vector ApplyAbsVelocity
		Subtract from func base 
		*/
		//void* velfunc = FindSignature(pBaseAddr, memLen, "\x55\x89\xE5\x83\xEC\x58\x89\x5D\xF4\x89\x75\xF8\x89\x7D\xFC\x8B\x5D\x0C\xF3\x0F\x10\x05????");
		void* compfunc = FindSignature(pBaseAddr, memLen, "\x55\x89\xE5\x57\x56\x53\x83\xEC\x4C\x8B\x75\x0C\x8B\x5D\x08\x8B\x46\x3C\x09\x83\xF4\x20\x00\x00");
		if(compfunc != NULL) {
			printf("Got OnTakeDamage_Alive(): %p\n", compfunc);
			mprotect((void*)PAGE_ALIGN_DOWN((int)compfunc), PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
			mprotect((void*)PAGE_ALIGN_UP((int)compfunc), PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
			unsigned char n_code[5];
			printf("Hijacking call to ApplyAbsVelocity...");
			// call addr (1b OP + 4b addr) -> nop 5b
			memcpy(n_code, "\x90\x90\x90\x90\x90", 5);

			memcpy((unsigned char*)compfunc + 0x52C, n_code, 5);
		} else {
			printf("WARN: Cannot locate OnTakeDamage_Alive, dying now!");
			printf("\n");
			return false;
		}
		
		printf("\n"); //flush??
		
		return true;
	}
	virtual void Unload()
	{

	}
	virtual void Pause()
	{
	}
	virtual void UnPause()
	{
	}
	virtual const char *GetPluginDescription()
	{
		return "No knockback 2: sigpatcheroo";
	}
	virtual void LevelInit(char const *pMapName)
	{
	}
	virtual void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
	{
	}
	virtual void GameFrame(bool simulating)
	{
	}
	virtual void LevelShutdown()
	{
	}
	virtual void ClientActive(edict_t *pEntity)
	{
	}
	virtual void ClientFullyConnect(edict_t *pEntity)
	{
	}
	virtual void ClientDisconnect(edict_t *pEntity)
	{
	}
	virtual void ClientPutInServer(edict_t *pEntity, char const *playername)
	{
	}
	virtual void SetCommandClient(int index)
	{
	}
	virtual void ClientSettingsChanged(edict_t *pEdict)
	{
	}
	virtual PLUGIN_RESULT ClientConnect(bool *bAllowConnect,
										edict_t *pEntity,
										const char *pszName,
										const char *pszAddress,
										char *reject,
										int maxrejectlen) 
	{	
		
		return PLUGIN_CONTINUE;
	}
	virtual PLUGIN_RESULT ClientCommand(edict_t *pEntity)
	{
		return PLUGIN_CONTINUE;
	}
	virtual PLUGIN_RESULT NetworkIDValidated(const char *pszUserName, const char *pszNetworkID)
	{
		return PLUGIN_CONTINUE;
	}
	virtual void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie,
										  edict_t *pPlayerEntity,
										  EQueryCvarValueStatus eStatus,
										  const char *pCvarName,
										  const char *pCvarValue)
	{
	}
	virtual void OnEdictAllocated( edict_t *edict )
	{
	}
	virtual void OnEdictFreed( const edict_t *edict  )
	{
	}
};

ServerPlugin vsp;

#define EXPORT extern "C" __attribute__ ((visibility("default")))

EXPORT void *
CreateInterface(const char *name, int *ret)
{
	void *ptr;
	if (strncmp(name, "ISERVERPLUGINCALLBACKS", 22) == 0)
	{
		ptr = &vsp;
	}

	if (ret != NULL)
		*ret = (ptr != NULL) ? 0 : 1;

	return ptr;
}


