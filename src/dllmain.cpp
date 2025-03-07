// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Config.h"
#include "hosts.h"
#include "Logger.h"
#include "Modify.h"

extern _getaddrinfo getaddrinfo_orig;

BOOL APIENTRY DllMain (HMODULE hModule,
					   DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls (hModule);
	std::string_view procname = GetCommandLine ();
	// only Spotify process - this help avoid false positive
	if (std::string_view::npos != procname.find ("Spotify.exe")) {
		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
			auto *config = new Config();
			if (std::string_view::npos == procname.find ("--type=") && false == config->getConfig("Block_BannerOnly")) {
				// block ads request - main process
				CreateThread (NULL, NULL, KillBanner, config, 0, NULL);
				//g_Logger.setLogfile ("main_log.txt");
			}
			else if (std::string_view::npos != procname.find ("--type=utility")) {
				// block ads banner by hostname - utility process
				//g_Logger.setLogfile ("utility_log.txt");
				auto *log = new Logger (config);
				getaddrinfo_orig = getaddrinfo;
				if (getaddrinfo_orig) {
					Mhook_SetHook ((PVOID*)&getaddrinfo_orig, getaddrinfo_hook);
					log->Log ("Mhook_SetHook - getaddrinfo success!");
				}
				else {
					log->Log ("Mhook_SetHook - getaddrinfo failed!");
				}
				log->~Logger ();
			}
			break;
		}
	}
	return TRUE;
}

