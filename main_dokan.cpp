#ifdef _MSC_VER

#include "common.h"
#include "FHCAPI.h"
#include "XLStringUtil.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define SLASH	'\\'

#include "C:\\Program Files (x86)\\Dokan\\DokanLibrary\\dokan.h"
#pragma comment(lib, "C:\\Program Files (x86)\\Dokan\\DokanLibrary\\dokan.lib")

static FHCAPI  g_API;



//このクラスはロックだ!
//そして、これを読んでいるキミ達もロックだ!
//by キユ(w
class XLLock  
{
private:
	CRITICAL_SECTION CriticalSection;
public:
	XLLock()
	{
		InitializeCriticalSection(&CriticalSection);
	}
	virtual ~XLLock()
	{
		DeleteCriticalSection(&CriticalSection);
	}

	//クリティカルセクションに入る
	void Lock()
	{
		EnterCriticalSection(&CriticalSection);
	}

	//クリティカルセクションから抜ける
	void UnLock()
	{
		 LeaveCriticalSection(&CriticalSection);
	}
};

//自動ロック.
//このクラスは自動ロックだ!
//そして、これを読んでいるキミ達もロックだ!
//並列を知らない子供が嫌い。並列をなくした大人が嫌い。優しいロックが好き。バイバイ
class XLAutoLock
{
public:
	XLAutoLock(XLLock * l)
	{
		Lock = l;
		Lock->Lock();
	}
	~XLAutoLock()
	{
		Lock->UnLock();
	}
private:
	XLLock*	Lock;
};

XLLock g_Lock;


BOOL g_UseStdErr;
BOOL g_DebugMode;

static void DbgPrint(LPCWSTR format, ...)
{
	if (g_DebugMode) {
		WCHAR buffer[512];
		va_list argp;
		va_start(argp, format);
		vswprintf_s(buffer, sizeof(buffer)/sizeof(WCHAR), format, argp);
		va_end(argp);
		if (g_UseStdErr) {
			fwprintf(stderr, buffer);
		} else {
			OutputDebugStringW(buffer);
		}
	}
}


#define FHCFSCheckFlag(val, flag) if (val&flag) { DbgPrint(L"\t" L#flag L"\n"); }
static int __stdcall
FHCFSCreateFile(
	LPCWSTR					FileName,
	DWORD					AccessMode,
	DWORD					ShareMode,
	DWORD					CreationDisposition,
	DWORD					FlagsAndAttributes,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	std::string fullpath = _W2A(FileName);
	const char* path = fullpath.c_str();

	DbgPrint(L"CreateFile : %s\n", FileName);

	if (CreationDisposition == CREATE_NEW)
		DbgPrint(L"\tCREATE_NEW\n");
	if (CreationDisposition == OPEN_ALWAYS)
		DbgPrint(L"\tOPEN_ALWAYS\n");
	if (CreationDisposition == CREATE_ALWAYS)
		DbgPrint(L"\tCREATE_ALWAYS\n");
	if (CreationDisposition == OPEN_EXISTING)
		DbgPrint(L"\tOPEN_EXISTING\n");
	if (CreationDisposition == TRUNCATE_EXISTING)
		DbgPrint(L"\tTRUNCATE_EXISTING\n");

	/*
	if (ShareMode == 0 && AccessMode & FILE_WRITE_DATA)
		ShareMode = FILE_SHARE_WRITE;
	else if (ShareMode == 0)
		ShareMode = FILE_SHARE_READ;
	*/

	DbgPrint(L"\tShareMode = 0x%x\n", ShareMode);

	FHCFSCheckFlag(ShareMode, FILE_SHARE_READ);
	FHCFSCheckFlag(ShareMode, FILE_SHARE_WRITE);
	FHCFSCheckFlag(ShareMode, FILE_SHARE_DELETE);

	DbgPrint(L"\tAccessMode = 0x%x\n", AccessMode);

	FHCFSCheckFlag(AccessMode, GENERIC_READ);
	FHCFSCheckFlag(AccessMode, GENERIC_WRITE);
	FHCFSCheckFlag(AccessMode, GENERIC_EXECUTE);
	
	FHCFSCheckFlag(AccessMode, DELETE);
	FHCFSCheckFlag(AccessMode, FILE_READ_DATA);
	FHCFSCheckFlag(AccessMode, FILE_READ_ATTRIBUTES);
	FHCFSCheckFlag(AccessMode, FILE_READ_EA);
	FHCFSCheckFlag(AccessMode, READ_CONTROL);
	FHCFSCheckFlag(AccessMode, FILE_WRITE_DATA);
	FHCFSCheckFlag(AccessMode, FILE_WRITE_ATTRIBUTES);
	FHCFSCheckFlag(AccessMode, FILE_WRITE_EA);
	FHCFSCheckFlag(AccessMode, FILE_APPEND_DATA);
	FHCFSCheckFlag(AccessMode, WRITE_DAC);
	FHCFSCheckFlag(AccessMode, WRITE_OWNER);
	FHCFSCheckFlag(AccessMode, SYNCHRONIZE);
	FHCFSCheckFlag(AccessMode, FILE_EXECUTE);
	FHCFSCheckFlag(AccessMode, STANDARD_RIGHTS_READ);
	FHCFSCheckFlag(AccessMode, STANDARD_RIGHTS_WRITE);
	FHCFSCheckFlag(AccessMode, STANDARD_RIGHTS_EXECUTE);

	// When FileName is a directory, needs to change the flag so that the file can be opened.

	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_ARCHIVE);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_ENCRYPTED);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_HIDDEN);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_NORMAL);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_OFFLINE);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_READONLY);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_SYSTEM);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_TEMPORARY);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_WRITE_THROUGH);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_OVERLAPPED);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_NO_BUFFERING);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_RANDOM_ACCESS);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_SEQUENTIAL_SCAN);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_DELETE_ON_CLOSE);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_BACKUP_SEMANTICS);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_POSIX_SEMANTICS);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_OPEN_REPARSE_POINT);
	FHCFSCheckFlag(FlagsAndAttributes, FILE_FLAG_OPEN_NO_RECALL);
	FHCFSCheckFlag(FlagsAndAttributes, SECURITY_ANONYMOUS);
	FHCFSCheckFlag(FlagsAndAttributes, SECURITY_IDENTIFICATION);
	FHCFSCheckFlag(FlagsAndAttributes, SECURITY_IMPERSONATION);
	FHCFSCheckFlag(FlagsAndAttributes, SECURITY_DELEGATION);
	FHCFSCheckFlag(FlagsAndAttributes, SECURITY_CONTEXT_TRACKING);
	FHCFSCheckFlag(FlagsAndAttributes, SECURITY_EFFECTIVE_ONLY);
	FHCFSCheckFlag(FlagsAndAttributes, SECURITY_SQOS_PRESENT);
	DbgPrint(L"\n");

	if ( path[0] != SLASH )
	{//謎
		return -ERROR_ACCESS_DENIED;
	}

	if ( path[1] == '\0' )
	{// トップディレクトリ
		DokanFileInfo->Context = 1;
		return 0;
	}

	const char * splitSlash = XLStringUtil::strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{//トップ直下には、家電名のディレクトリしか無い.
		splitSlash = path + strlen(path);
	}

	XLAutoLock al(&g_Lock);

	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ERROR_ACCESS_DENIED;
	}

	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		DokanFileInfo->Context = 3;
		return 0;
	}

	{
		for(auto ait = elec->action.begin(); ait!=elec->action.end() ; ait++)
		{
			if(splitSlash+1 == (*ait)->name)
			{
				DokanFileInfo->Context = 2;
				return 0;
			}
		}
	}

	DokanFileInfo->Context = 1;
	return 0;
}


static int __stdcall
FHCFSCreateDirectory(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	return -ERROR_ACCESS_DENIED;
}


static int __stdcall
FHCFSOpenDirectory(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	DbgPrint(L"OpenDirectory : %s\n", FileName);

	std::string fullpath = _W2A(FileName);
	const char* path = fullpath.c_str();

	if ( path[0] != SLASH )
	{//謎
		return -ENOENT;
	}

	XLAutoLock al(&g_Lock);
	g_API.update();

	if ( path[1] == '\0' )
	{// トップディレクトリ
		DokanFileInfo->Context = 1;
		return 0;
	}

	const char * splitSlash = strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{
		splitSlash = path+strlen(path);
	}

	{
		const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
		const Elec* elec = g_API.findElec(elecname);
		if (!elec)
		{
			return -ENOENT;
		}
		
	}
	DokanFileInfo->Context = 1;
	return 0;
}


static int __stdcall
FHCFSCloseFile(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	if (DokanFileInfo->Context) {
		DbgPrint(L"CloseFile: %s\n", FileName);
		DbgPrint(L"\terror : not cleanuped file\n\n");
//		CloseHandle((HANDLE)DokanFileInfo->Context);
		DokanFileInfo->Context = 0;
	} else {
		//DbgPrint(L"Close: %s\n\tinvalid handle\n\n", FileName);
		DbgPrint(L"Close: %s\n\n", FileName);
		return 0;
	}

	//DbgPrint(L"\n");
	return 0;
}


static int __stdcall
FHCFSCleanup(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	if (DokanFileInfo->Context) {
		DbgPrint(L"Cleanup: %s\n\n", FileName);
		DokanFileInfo->Context = 0;
	} else {
		DbgPrint(L"Cleanup: %s\n\tinvalid handle\n\n", FileName);
		return -1;
	}

	return 0;
}


static int __stdcall
FHCFSReadFile(
	LPCWSTR				FileName,
	LPVOID				buf,
	DWORD				size,
	LPDWORD				ReadLength,
	LONGLONG			Offset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	BOOL	opened = FALSE;
	ULONG	offset = (ULONG)Offset;

	DbgPrint(L"ReadFile : %s\n", FileName);

	std::string fullpath = _W2A(FileName);
	const char* path = fullpath.c_str();

	const char * splitSlash = strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{//トップ直下には、家電名のディレクトリしか無い.
	 //ディレクトリは開けないのでエラーを返す
		return -ERROR_ACCESS_DENIED;
	}

	XLAutoLock al(&g_Lock);
	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ERROR_ACCESS_DENIED;
	}

	std::string result;
	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		result = elec->status;
	}
	else if ( splitSlash+1 == elec->status)
	{//家電の状態ファイル. このステータスが有効な場合
		result = "1";
	}
	else
	{//家電の状態ファイル.
		result = "";
	}
	
	if ( offset >= result.size() )
	{//書き込みバッファがたりなさすぎ
		return 0;
	}
	if ( (offset + size) > result.size() )
	{//ファイルサイズに、バッファを補正する.
		size = (result.size() - offset);
	}
	memcpy(buf, result.c_str() + offset, size);
	*ReadLength=size;
	return 0;
}


static int __stdcall
FHCFSWriteFile(
	LPCWSTR		FileName,
	LPCVOID		buf,
	DWORD		size,
	LPDWORD		NumberOfBytesWritten,
	LONGLONG			Offset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	ULONG	offset = (ULONG)Offset;
	BOOL	opened = FALSE;

	DbgPrint(L"WriteFile : %s, offset %I64d, length %d\n", FileName, offset, size);

	std::string fullpath = _W2A(FileName);
	const char* path = fullpath.c_str();

	const char * splitSlash = XLStringUtil::strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{//トップ直下には、家電名のディレクトリしか無い.
	 //ディレクトリは開けないのでエラーを返す
		return -ERROR_ACCESS_DENIED;
	}
	if(offset!=0)
	{//ごめんね。まとめての読み書きしかサポートしないんだ.
		return -ERROR_ACCESS_DENIED;
	}

	XLAutoLock al(&g_Lock);
	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ERROR_ACCESS_DENIED;
	}

	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		const std::string actionname = XLStringUtil::chop( std::string( (const char*)buf,0,size) );

		//この状態名があるかどうか調べようか.
		if (! g_API.checkStatus(elecname,actionname) )
		{//そんな状態ないです。
			return -ERROR_ACCESS_DENIED;
		}

		g_API.fire(elecname,actionname);
	}
	else
	{//家電の状態ファイル.
		//ONにしたいんだよね・・・？
		bool isON = stringbool( std::string((const char*)buf,0,size) );
		if (!isON)
		{//ステータスファイル名でoffと言われてもどうしろと.
			*NumberOfBytesWritten=size;
			return 0;
		}

		//念のため、パスが有効な状態名かを確認する.
		if (! g_API.checkStatus(elecname,splitSlash+1) )
		{//そんな状態ないです。
			return -ERROR_ACCESS_DENIED;
		}

		g_API.fire(elecname,splitSlash+1);
	}
	g_API.update();

	*NumberOfBytesWritten=size;
	return 0;
}


static int __stdcall
FHCFSFlushFileBuffers(
	LPCWSTR		FileName,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;

	DbgPrint(L"FlushFileBuffers : %s\n", FileName);

	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return 0;
	}

	return 0;
}


static int __stdcall
FHCFSGetFileInformation(
	LPCWSTR							FileName,
	LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation,
	PDOKAN_FILE_INFO				DokanFileInfo)
{
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	BOOL	opened = FALSE;

	DbgPrint(L"GetFileInfo : %s\n", FileName);
	
	std::string fullpath = _W2A(FileName);
	const char* path = fullpath.c_str();
	
	memset(HandleFileInformation,0,sizeof(BY_HANDLE_FILE_INFORMATION));
	
	XLAutoLock al(&g_Lock);
	if ( path[0] == SLASH && path[1] == '\0')
	{//トップディレクトリ
		const std::vector<Elec*>* elec = g_API.getElec();

		HandleFileInformation->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		return 0;
	}

	const char * splitSlash = XLStringUtil::strchr(path+1,SLASH);
	if (splitSlash == NULL)
	{// トップ直下には、家電名のディレクトリしか無い.
		const Elec* elec = g_API.findElec(path+1);
		if(!elec)
		{
			return -ERROR_ACCESS_DENIED;
		}

		HandleFileInformation->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		return 0;
	}

	//サブディレクトリには、家電の状態名とかがある.
	const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
	const Elec* elec = g_API.findElec(elecname);
	if(!elec)
	{
		return -ERROR_ACCESS_DENIED;
	}
	
	if ( strcmp(splitSlash+1,".status") == 0 )
	{
		HandleFileInformation->dwFileAttributes = 0;
		HandleFileInformation->nFileSizeLow = elec->status.size();
	}
	else if ( splitSlash+1 == elec->status)
	{//家電の状態ファイル. このステータスが有効な場合
		HandleFileInformation->dwFileAttributes = 0;
		HandleFileInformation->nFileSizeLow = 1;	//"1"の長さ.
	}
	else
	{//家電の状態ファイル.
		HandleFileInformation->dwFileAttributes = 0;
		HandleFileInformation->nFileSizeLow = 0; //""の長さ
	}
	

	DbgPrint(L"\tFindFiles OK, file size = %d\n", HandleFileInformation->nFileSizeLow);
	DbgPrint(L"\n");
	return 0;
}


static int __stdcall
FHCFSFindFiles(
	LPCWSTR				FileName,
	PFillFindData		FillFindData, // function pointer
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WIN32_FIND_DATAW	findData = {0};
	int count = 0;

	DbgPrint(L"FindFiles :%s\n", FileName);

	std::string fullpath = _W2A(FileName);
	const char* path = fullpath.c_str();

	XLAutoLock al(&g_Lock);
	g_API.update();

	if ( path[1] == '\0' )
	{// トップディレクトリ
		const std::vector<Elec*>* elec = g_API.getElec();
		for(auto eit = elec->begin(); eit!=elec->end() ; eit++)
		{
			findData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
			lstrcpyW(findData.cFileName, _A2W( (*eit)->name).c_str() );
			FillFindData(&findData, DokanFileInfo);
		}
		return 0;
	}

	const char * splitSlash = strchr(path+1,'/');
	if (splitSlash == NULL)
	{
		splitSlash = path+strlen(path);
	}

	{
		const std::string elecname = std::string(path+1,0,splitSlash - (path+1));
		const Elec* elec = g_API.findElec(elecname);
		
		findData.dwFileAttributes = 0; //現在の状態 
		lstrcpyW(findData.cFileName, _A2W( std::string(".status")).c_str() );
		FillFindData(&findData, DokanFileInfo);

		for(auto ait = elec->action.begin(); ait!=elec->action.end() ; ait++)
		{
			findData.dwFileAttributes = 0; //現在の状態 
			lstrcpyW(findData.cFileName, _A2W( (*ait)->name).c_str() );
			FillFindData(&findData, DokanFileInfo);
		}
	}
	return 0;
}


static int __stdcall
FHCFSDeleteFile(
	LPCWSTR				FileName,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;

	DbgPrint(L"DeleteFile %s\n", FileName);

	return -ERROR_ACCESS_DENIED;
}


static int __stdcall
FHCFSDeleteDirectory(
	LPCWSTR				FileName,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;

	DbgPrint(L"DeleteDirectory %s\n", FileName);

	return -ERROR_ACCESS_DENIED;
}


static int __stdcall
FHCFSMoveFile(
	LPCWSTR				FileName, // existing file name
	LPCWSTR				NewFileName,
	BOOL				ReplaceIfExisting,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	DbgPrint(L"MoveFile %s -> %s\n\n", FileName, NewFileName);

	return -ERROR_ACCESS_DENIED;
}


static int __stdcall
FHCFSLockFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	LONGLONG			Length,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle;

	DbgPrint(L"LockFile %s\n", FileName);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	return 0;
}


static int __stdcall
FHCFSSetEndOfFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE			handle;

	DbgPrint(L"SetEndOfFile %s, %I64d\n", FileName, ByteOffset);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	return -ERROR_ACCESS_DENIED;
}


static int __stdcall
FHCFSSetAllocationSize(
	LPCWSTR				FileName,
	LONGLONG			AllocSize,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE			handle;

	DbgPrint(L"SetAllocationSize %s, %I64d\n", FileName, AllocSize);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	return -ERROR_ACCESS_DENIED;
}


static int __stdcall
FHCFSSetFileAttributes(
	LPCWSTR				FileName,
	DWORD				FileAttributes,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	DbgPrint(L"SetFileAttributes %s\n", FileName);

	DbgPrint(L"\n");
	return 0;
}


static int __stdcall
FHCFSSetFileTime(
	LPCWSTR				FileName,
	CONST FILETIME*		CreationTime,
	CONST FILETIME*		LastAccessTime,
	CONST FILETIME*		LastWriteTime,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle;

	DbgPrint(L"SetFileTime %s\n", FileName);

	handle = (HANDLE)DokanFileInfo->Context;

	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	DbgPrint(L"\n");
	return 0;
}


static int __stdcall
FHCFSUnlockFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	LONGLONG			Length,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle;

	DbgPrint(L"UnlockFile %s\n", FileName);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	return 0;
}


static int __stdcall
FHCFSGetFileSecurity(
	LPCWSTR					FileName,
	PSECURITY_INFORMATION	SecurityInformation,
	PSECURITY_DESCRIPTOR	SecurityDescriptor,
	ULONG				BufferLength,
	PULONG				LengthNeeded,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle;

	DbgPrint(L"GetFileSecurity %s\n", FileName);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	return 0;
}


static int __stdcall
FHCFSSetFileSecurity(
	LPCWSTR					FileName,
	PSECURITY_INFORMATION	SecurityInformation,
	PSECURITY_DESCRIPTOR	SecurityDescriptor,
	ULONG				SecurityDescriptorLength,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	HANDLE	handle;

	DbgPrint(L"SetFileSecurity %s\n", FileName);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	return 0;
}

static int __stdcall
FHCFSGetVolumeInformation(
	LPWSTR		VolumeNameBuffer,
	DWORD		VolumeNameSize,
	LPDWORD		VolumeSerialNumber,
	LPDWORD		MaximumComponentLength,
	LPDWORD		FileSystemFlags,
	LPWSTR		FileSystemNameBuffer,
	DWORD		FileSystemNameSize,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	wcscpy_s(VolumeNameBuffer, VolumeNameSize / sizeof(WCHAR), L"HOMEFS");
	*VolumeSerialNumber = 0x19831116;
	*MaximumComponentLength = 256;
	*FileSystemFlags = FILE_CASE_SENSITIVE_SEARCH | 
						FILE_CASE_PRESERVED_NAMES | 
						FILE_SUPPORTS_REMOTE_STORAGE |
						FILE_UNICODE_ON_DISK |
						FILE_PERSISTENT_ACLS;

	wcscpy_s(FileSystemNameBuffer, FileSystemNameSize / sizeof(WCHAR), L"FHCFS");

	return 0;
}


static int __stdcall
FHCFSUnmount(
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	DbgPrint(L"Unmount\n");
	return 0;
}

static void usage()
{
	printf("-家mount- HOMEFS (FHCFS)  with dokan\n");
	printf("\n");
	printf("homefs.exe ip@apikey driveletter [-o debug]\n");
	printf("\n");
	printf("mount.\n");
	printf(" 192.168.10.21のFHC鯖を m: にmountする。 デバッグモードでログを出力する.\n");
	printf(" homefs.exe 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX m -o debug\n");
	printf("\n");
	printf("unmount.\n");
	printf(" dokanctl.exe /u m\n");
	printf("\n");
	printf("実行には、 dokan が必要です.\n");
	printf("\n");
}

class WinSockInit
{
public:
	WinSockInit()
	{
#if _MSC_VER
		WSADATA wsaData;
		::WSAStartup(2 , &wsaData);
#else
#endif
	}
	virtual ~WinSockInit()
	{
#if _MSC_VER
		::WSACleanup();
#else
#endif
	}
};

int main(int argc, char *argv[])
{
	WinSockInit winsock;

	if(argc<=2)	
	{
		usage();
		return 0;
	}
	
	char *p = strstr(argv[1],"@");
	if(!p)
	{
		usage();
		return 0;
	}
	std::string mountpoint = std::string(argv[1],0,(int)(p-argv[1]));
	std::string apikey = p+1;
	WCHAR mountDrive[MAX_PATH] = L"M:";
	wcscpy_s(mountDrive, sizeof(mountDrive)/sizeof(WCHAR), _A2W(argv[2]).c_str());
	
	//パスワード(apikey)をps aux で見られないようにマスクする.
//	memset(p,'#',apikey.size());

	if(!g_API.setInit(mountpoint,apikey))
	{
		fprintf(stderr,"g_API.parseLine error!\n");
		return -1;
	}
	if(!g_API.update())
	{
		fprintf(stderr,"g_API.update error!\n");
		return -1;
	}



	g_DebugMode = FALSE;
	g_UseStdErr = FALSE;

	for (int command = 3; command < argc; command++)
	{
		if ( strcmp(argv[command],"-o")  == 0 )
		{
			if (command +1 < argc)
			{
				if ( strcmp(argv[command+1],"debug")  == 0 )
				{
					g_DebugMode = TRUE;
					g_UseStdErr = TRUE;
				}
				else
				{
					fprintf(stderr, "unknown command: %s\n",__argv[command+1]);
					usage();
					return 0;
				}
				command++;
			}
			else
			{
				fprintf(stderr, "unknown command: %s\n",__argv[command]);
				usage();
				return 0;
			}
		}
	}

	int status;
	PDOKAN_OPERATIONS dokanOperations =
			(PDOKAN_OPERATIONS)malloc(sizeof(DOKAN_OPERATIONS));
	PDOKAN_OPTIONS dokanOptions =
			(PDOKAN_OPTIONS)malloc(sizeof(DOKAN_OPTIONS));

	ZeroMemory(dokanOptions, sizeof(DOKAN_OPTIONS));
	dokanOptions->Version = DOKAN_VERSION;
	dokanOptions->ThreadCount = 0; // use default
	dokanOptions->MountPoint = mountDrive;

	if (g_DebugMode) {
		dokanOptions->Options |= DOKAN_OPTION_DEBUG;
	}
	if (g_UseStdErr) {
		dokanOptions->Options |= DOKAN_OPTION_STDERR;
	}

	dokanOptions->Options |= DOKAN_OPTION_KEEP_ALIVE;

	ZeroMemory(dokanOperations, sizeof(DOKAN_OPERATIONS));
	dokanOperations->CreateFile = FHCFSCreateFile;
	dokanOperations->OpenDirectory = FHCFSOpenDirectory;
	dokanOperations->CreateDirectory = FHCFSCreateDirectory;
	dokanOperations->Cleanup = FHCFSCleanup;
	dokanOperations->CloseFile = FHCFSCloseFile;
	dokanOperations->ReadFile = FHCFSReadFile;
	dokanOperations->WriteFile = FHCFSWriteFile;
	dokanOperations->FlushFileBuffers = FHCFSFlushFileBuffers;
	dokanOperations->GetFileInformation = FHCFSGetFileInformation;
	dokanOperations->FindFiles = FHCFSFindFiles;
	dokanOperations->FindFilesWithPattern = NULL;
	dokanOperations->SetFileAttributes = FHCFSSetFileAttributes;
	dokanOperations->SetFileTime = FHCFSSetFileTime;
	dokanOperations->DeleteFile = FHCFSDeleteFile;
	dokanOperations->DeleteDirectory = FHCFSDeleteDirectory;
	dokanOperations->MoveFile = FHCFSMoveFile;
	dokanOperations->SetEndOfFile = FHCFSSetEndOfFile;
	dokanOperations->SetAllocationSize = FHCFSSetAllocationSize;	
	dokanOperations->LockFile = FHCFSLockFile;
	dokanOperations->UnlockFile = FHCFSUnlockFile;
	dokanOperations->GetFileSecurity = FHCFSGetFileSecurity;
	dokanOperations->SetFileSecurity = FHCFSSetFileSecurity;
	dokanOperations->GetDiskFreeSpace = NULL;
	dokanOperations->GetVolumeInformation = FHCFSGetVolumeInformation;
	dokanOperations->Unmount = FHCFSUnmount;

	status = DokanMain(dokanOptions, dokanOperations);
	switch (status) {
	case DOKAN_SUCCESS:
		fprintf(stderr, "Success\n");
		break;
	case DOKAN_ERROR:
		fprintf(stderr, "Error\n");
		break;
	case DOKAN_DRIVE_LETTER_ERROR:
		fprintf(stderr, "Bad Drive letter\n");
		break;
	case DOKAN_DRIVER_INSTALL_ERROR:
		fprintf(stderr, "Can't install driver\n");
		break;
	case DOKAN_START_ERROR:
		fprintf(stderr, "Driver something wrong\n");
		break;
	case DOKAN_MOUNT_ERROR:
		fprintf(stderr, "Can't assign a drive letter\n");
		break;
	case DOKAN_MOUNT_POINT_ERROR:
		fprintf(stderr, "Mount point error\n");
		break;
	default:
		fprintf(stderr, "Unknown error: %d\n", status);
		break;
	}

	free(dokanOptions);
	free(dokanOperations);
	return 0;
}


#else
#endif
