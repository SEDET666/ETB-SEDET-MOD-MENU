#include <windows.h>
#include <tlhelp32.h>
#include <bits/stdc++.h>

using namespace std;

bool InjectDLL(DWORD processID, const char* dllPath) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (hProcess == NULL) {
		std::cerr << "Failed to open process." << std::endl;
		return false;
	}
	
	LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (pDllPath == NULL) {
		std::cerr << "Failed to allocate memory in target process." << std::endl;
		CloseHandle(hProcess);
		return false;
	}
	
	if (!WriteProcessMemory(hProcess, pDllPath, (LPVOID)dllPath, strlen(dllPath) + 1, NULL)) {
		std::cerr << "Failed to write DLL path to target process memory." << std::endl;
		VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pDllPath, 0, NULL);
	if (hThread == NULL) {
		std::cerr << "Failed to create remote thread in target process." << std::endl;
		VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	
	WaitForSingleObject(hThread, INFINITE);
	
	VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	
	return true;
}

DWORD GetPIDByProcessName(const string& processName)
{
	// 创建进程快照
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		cerr << "创建进程快照失败，错误码：" << GetLastError() << endl;
		return 0;
	}
	
	PROCESSENTRY32 pe32{0};
	pe32.dwSize = sizeof(PROCESSENTRY32);
	
	// 遍历所有进程
	if (Process32First(hSnapshot, &pe32)) {
		do {
			// 比较进程名（不区分大小写）
			if (_stricmp(pe32.szExeFile, processName.c_str()) == 0) {
				CloseHandle(hSnapshot);
				return pe32.th32ProcessID; // 返回匹配的PID
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	
	CloseHandle(hSnapshot);
	return 0; // 未找到返回0
}

// 获取当前进程所在目录
string GetCurrentProcessDir()
{
	// 定义缓冲区存储路径，MAX_PATH是Windows定义的最大路径长度
	char buffer[MAX_PATH] = {0};
	
	// 获取当前进程的完整路径（包含exe文件名）
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	
	// 找到最后一个反斜杠的位置，分割路径和文件名
	string fullPath = buffer;
	size_t lastSlashPos = fullPath.find_last_of("\\");
	
	// 截取到最后一个反斜杠位置，得到进程所在目录
	if (lastSlashPos != string::npos)
	{
		return fullPath.substr(0, lastSlashPos + 1); // +1保留最后的反斜杠
	}
	
	return ""; // 异常情况返回空字符串
}

int main() {
	DWORD processID = GetPIDByProcessName("Backrooms-Win64-Shipping.exe"); // 替换为目标游戏的进程ID
	string dllFullPath = GetCurrentProcessDir() + "\SEMOD_R.dll";
	const char* dllPath = dllFullPath.c_str (); // 替换为你的DLL路径
	if (InjectDLL(processID, dllPath)) {
		std::cout << "DLL注入成功！" << std::endl;
	} else {
		std::cerr << "DLL注入失败。" << std::endl;
	}
	system("pause");
	return 0;
}
