#include <windows.h>
#include "CoreTypes.h"

extern int32 GuardedMain(const TCHAR* CmdLine);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	int32 ErrorLevel = 0;

	const TCHAR* CmdLine = TEXT("");

	ErrorLevel = GuardedMain(CmdLine);

	return ErrorLevel;
}