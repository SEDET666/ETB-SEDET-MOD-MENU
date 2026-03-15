#include <iostream>
#include <filesystem>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

using namespace std;
namespace fs = std::filesystem;

// 获取当前程序运行目录
fs::path GetCurrentDir() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	return fs::path(buffer).parent_path();
}

// 等待指定进程出现并返回其PID
DWORD WaitForProcess(const wstring& processName) {
	while (true) {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32W pe; // 显式使用宽字符版本
			pe.dwSize = sizeof(PROCESSENTRY32W);
			if (Process32FirstW(hSnapshot, &pe)) { // 使用宽字符API
				do {
					if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0) {
						CloseHandle(hSnapshot);
						return pe.th32ProcessID;
					}
				} while (Process32NextW(hSnapshot, &pe)); // 使用宽字符API
			}
			CloseHandle(hSnapshot);
		}
		Sleep(500);
	}
}

// 获取进程完整路径
fs::path GetProcessPath(DWORD pid) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (hProcess) {
		wchar_t buffer[MAX_PATH];
		if (GetModuleFileNameExW(hProcess, NULL, buffer, MAX_PATH)) {
			CloseHandle(hProcess);
			return fs::path(buffer);
		}
		CloseHandle(hProcess);
	}
	return L"";
}

// 终止进程
bool TerminateProcessById(DWORD pid) {
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (hProcess) {
		BOOL result = TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
		return result != 0;
	}
	return false;
}

int main() {
	// 1. 检查文件
	fs::path currentDir = GetCurrentDir();
	fs::path ue4ssDir = currentDir / L"UE4SS";
	fs::path pakFile = currentDir / L"SEMOD_P.pak";
	
	if (!fs::exists(ue4ssDir) || !fs::exists(pakFile)) {
		cout << "请完整解压后重试!" << endl;
		system("pause");
		return 1;
	}
	
	// 2. 启动游戏
	cout << "正在启动游戏..." << endl;
	ShellExecuteW(NULL, L"open", L"steam://rungameid/1943950", NULL, NULL, SW_SHOWNORMAL);
	
	cout << "等待游戏进程启动..." << endl;
	DWORD gamePid = WaitForProcess(L"Backrooms-Win64-Shipping.exe");
	
	// 3. 获取路径
	fs::path process_path = GetProcessPath(gamePid);
	if (process_path.empty()) {
		cout << "无法获取游戏进程路径!" << endl;
		system("pause");
		return 1;
	}
	fs::path processDir = process_path.parent_path();
	
	// 4. 终止进程
	cout << "正在关闭游戏进程..." << endl;
	if (!TerminateProcessById(gamePid)) {
		cout << "无法终止游戏进程，请手动关闭后重试!" << endl;
		system("pause");
		return 1;
	}
	Sleep(1000);
	
	// 5. 复制UE4SS
	cout << "正在安装UE4SS..." << endl;
	try {
		fs::copy(ue4ssDir / L"ue4ss", processDir / L"ue4ss", 
			fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		fs::copy(ue4ssDir / L"dwmapi.dll", processDir / L"dwmapi.dll", 
			fs::copy_options::overwrite_existing);
	} catch (const fs::filesystem_error& e) {
		cout << "复制UE4SS文件失败: " << e.what() << endl;
		system("pause");
		return 1;
	}
	
	// 6. 创建LogicMods目录
	fs::path logicmods_path = processDir / L".." / L".." / L"Content" / L"Paks" / L"LogicMods";
	logicmods_path = fs::absolute(logicmods_path);
	
	try {
		if (!fs::exists(logicmods_path)) {
			fs::create_directories(logicmods_path);
		}
	} catch (const fs::filesystem_error& e) {
		cout << "创建LogicMods目录失败: " << e.what() << endl;
		system("pause");
		return 1;
	}
	
	// 7. 复制模组
	cout << "正在安装模组..." << endl;
	try {
		fs::copy(pakFile, logicmods_path / L"SEMOD_P.pak", 
			fs::copy_options::overwrite_existing);
	} catch (const fs::filesystem_error& e) {
		cout << "复制模组文件失败: " << e.what() << endl;
		system("pause");
		return 1;
	}
	
	// 8. 完成
	cout << "\nUE4SS和模组均已安装成功，快去享受模组吧~" << endl;
	system("pause");
	return 0;
}

