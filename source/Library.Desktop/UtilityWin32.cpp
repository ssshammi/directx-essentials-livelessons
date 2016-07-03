#include "pch.h"
#include "UtilityWin32.h"
#include <Shlwapi.h>

namespace Library
{
	std::string UtilityWin32::CurrentDirectory()
	{
		WCHAR buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buffer);
		std::wstring currentDirectoryW(buffer);

		return std::string(currentDirectoryW.begin(), currentDirectoryW.end());
	}

	std::wstring UtilityWin32::ExecutableDirectory()
	{
		WCHAR buffer[MAX_PATH];
		GetModuleFileName(nullptr, buffer, MAX_PATH);
		PathRemoveFileSpec(buffer);

		return std::wstring(buffer);
	}

	void UtilityWin32::PathJoin(std::wstring& dest, const std::wstring& sourceDirectory, const std::wstring& sourceFile)
	{
		WCHAR buffer[MAX_PATH];

		PathCombine(buffer, sourceDirectory.c_str(), sourceFile.c_str());
		dest = buffer;
	}

	void UtilityWin32::GetPathExtension(const std::wstring& source, std::wstring& dest)
	{
		dest = PathFindExtension(source.c_str());
	}
}