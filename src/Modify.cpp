#include "Modify.h"
#include "Logger.h"

extern Logger g_Logger;

// https://www.unknowncheats.me/forum/1064672-post23.html
bool DataCompare (BYTE* pData, BYTE* bSig, char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == NULL;
}

BYTE* FindPattern (BYTE* dwAddress, DWORD dwSize, BYTE* pbSig, char* szMask)
{
	DWORD length = strlen (szMask);
	for (DWORD i = NULL; i < dwSize - length; i++)
	{
		__try
		{
			if (DataCompare (dwAddress + i, pbSig, szMask))
				return dwAddress + i;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return nullptr;
		}
	}
	return 0;
}

DWORD WINAPI KillBanner (LPVOID conf)
{
	auto config = static_cast<Config*>(conf);
	auto* logger = new Logger (config);
	HMODULE hModule = GetModuleHandle (NULL);
	MODULEINFO mInfo = { 0 };
	if (GetModuleInformation (GetCurrentProcess (), hModule, &mInfo, sizeof (MODULEINFO))) {
		logger->Log ("GetModuleInformation OK!");
		LPVOID skipPod = FindPattern ((uint8_t*)hModule, mInfo.SizeOfImage, (BYTE*)"\x83\xC4\x08\x84\xC0\x0F\x84\x00\x04\x00\x00", "xxxxxxxxxxx");

		if (skipPod)
		{
			DWORD oldProtect;
			VirtualProtect ((char*)skipPod + 5, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			memset ((char*)skipPod + 5, 0x90, 1);
			VirtualProtect ((char*)skipPod + 5, 1, oldProtect, &oldProtect);

			VirtualProtect ((char*)skipPod + 6, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			memset ((char*)skipPod + 6, 0xE9, 1);
			VirtualProtect ((char*)skipPod + 6, 1, oldProtect, &oldProtect);
			logger->Log ("main process - patch success!");
		}
		else {
			logger->Log ("main process - patch failed!");
		}
	} 
	else {
		logger->Log ("GetModuleInformation failed!");
	}
	
	return 0;
}
