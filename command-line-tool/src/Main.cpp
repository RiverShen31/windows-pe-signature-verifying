#include <Windows.h>
#include <fltUser.h>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <wintrust.h>
#include <softpub.h>
#include <tchar.h>
#include <psapi.h>
#include <unordered_map>

#include "cliUtils.h"

enum ACTION {
    Connect,
    Disconnect,
    Read,
    Write,
    Create,
    Open,
    Delete,
    ProcessPath
};

enum COMMAND_TYPE {
    ADD_PROCESS_LIST,
    ADD_BLOCK_LIST,
};

//enum TYPE_MESSAGE_KM_TO_UM {
//    ProcessPath,
//    LOG
//};

struct HEADER {
    ACTION Action;
    ULONG Size;
    LARGE_INTEGER Time;
};

struct MESSAGE_ACTION {
    COMMAND_TYPE Command;
    UINT32 Length;
    WCHAR Buffer[260];
};

typedef struct _TRACKED_ACTION : HEADER {
    //TYPE_MESSAGE_KM_TO_UM Type;
    UINT32 DeviceIdOffset;
    UINT32 DeviceIdLength;
    UINT32 CanonicalFilePathOffset;
    UINT32 CanonicalFilePathLength;
    UINT32 Pid;
    UINT32 ProcessPathOffset;
    UINT32 ProcessPathLength;
} TRACKED_ACTION, * PTRACKED_ACTION;


typedef struct _PID_BOOK_ENTRY : HEADER {
    //ULONG Size;
    //TYPE_MESSAGE_KM_TO_UM Type;
	COMMAND_TYPE CommandType;
    ULONG Pid;
    UINT32 ProcessPathOffset;
    UINT32 ProcessPathLength;
} PID_BOOK_ENTRY, * PPID_BOOK_ENTRY;

void DisplayTime(const LARGE_INTEGER& time) {
    FILETIME localTime;
    FileTimeToLocalFileTime((FILETIME*)&time, &localTime);
    SYSTEMTIME st;
    FileTimeToSystemTime(&localTime, &st);
    printf("[%02d:%02d:%02d:%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

std::wstring GetDosNameFromNTName(const std::wstring& ntPath) {
    if (ntPath.empty()) return L"";

    // Lặp qua các ổ đĩa từ C: -> Z:
    for (wchar_t drive = L'C'; drive <= L'Z'; ++drive) {
        wchar_t drivePath[3] = { drive, L':', L'\0' };
        wchar_t devicePath[MAX_PATH] = { 0 };

        // Lấy tên thiết bị tương ứng với ổ đĩa
        if (QueryDosDeviceW(drivePath, devicePath, MAX_PATH)) {
            std::wstring deviceString(devicePath);
            if (ntPath.find(deviceString) == 0) {
                // Thay thế phần đầu "\Device\HarddiskVolumeX" bằng "C:\"
                return drivePath + ntPath.substr(deviceString.length());
            }
        }
    }
    return ntPath; // Trả về NT Path nếu không tìm thấy
}

// ?? X? lý message và ki?m tra ch? ký s?
//void HandleMessagePidProcessPath(BYTE* buffer) {
//    auto info = (PID_BOOK_ENTRY*)buffer;
//    /*DWORD pid = info->Pid;
//
//    std::wcout << L"PID: " << pid << L"\n";*/
//
//    if (info->Type == ProcessPath) {
//        if (info->ProcessPathLength > 0) {
//            std::wstring ntProcessPath((wchar_t*)(buffer + info->ProcessPathOffset), info->ProcessPathLength / sizeof(wchar_t));
//            std::wcout << L"NT Process Path: " << ntProcessPath << std::endl;
//
//            std::wstring win32ProcessPath = GetDosNameFromNTName(ntProcessPath);
//            std::wcout << L"Win32 Process Path: " << win32ProcessPath << std::endl;
//
//            auto lRetVal = CheckFile(win32ProcessPath);
//            if (lRetVal == 0) {
//                std::wcout << L"Verified Signer: YES\n\n";
//				info->CommandType = ADD_PROCESS_LIST;
//            }
//            else {
//                std::wcout << L"Verified Signer: NO\n\n";
//				info->CommandType = ADD_BLOCK_LIST;
//            }
//        }
//    }
//}

void DisplayInfo(BYTE* buffer, DWORD size) {
    while (size > 0) {
        auto info = (PID_BOOK_ENTRY*)buffer;
        DWORD pid = info->Pid;

        std::wcout << L"PID: " << pid << L"\n";

        if (info->ProcessPathLength > 0) {
            std::wstring ntProcessPath((wchar_t*)(buffer + info->ProcessPathOffset), info->ProcessPathLength / sizeof(wchar_t));
            std::wcout << L"NT Process Path: " << ntProcessPath << std::endl;

            std::wstring win32ProcessPath = GetDosNameFromNTName(ntProcessPath);
            std::wcout << L"Win32 Process Path: " << win32ProcessPath << std::endl;

            //printf("Test1\n");

            /*if (!isFileExists(win32ProcessPath)) {
                std::wcout << L"File not found\n\n";
                return;
            }*/

            //printf("Test2\n");

            /*if (isFileAFolder(win32ProcessPath)) {
                std::wcout << L"Folder\n\n";
                return;
            }*/

            /*printf("Test\n");

            std::cout << "Press any key to continue...\n";
            _getch();*/


            /*auto lRetVal = CheckFile(win32ProcessPath);
            if (lRetVal == 0) {
                std::wcout << L"Verified Signer: YES\n\n";
            }
            else {
                std::wcout << L"Verified Signer: NO\n\n";
            }*/

            /*std::wstring win32ProcessPath = GetDosNameFromNTName(ntProcessPath.c_str());

            std::wcout << L"DOS Process Path: " << win32ProcessPath << std::endl;

            bool isSigned = VerifyEmbeddedSignature(win32ProcessPath);
            std::wcout << L"Verified Signer: " << (isSigned ? L"YES" : L"NO") << L"\n\n";*/
		}

		buffer += info->Size;
		size -= info->Size;
        printf("\n");
    }
    printf("----------------------------------\n");
}

int wmain() {

    DWORD bytesReturned;
    HANDLE hPort;
    HANDLE hPort2;
    HRESULT hr;
    HRESULT hr2;
    int choice;

    printf("Choose 1 to send message or 2, 3 to receive logs: ");
    scanf_s("%d", &choice);

    switch (choice) {
    case 1: {
        hr = FilterConnectCommunicationPort(L"\\SendMessage", 0, nullptr, 0, nullptr, &hPort);
        if (FAILED(hr)) {
            printf("Error connecting to port (HR=0x%08X)\n", hr);
            return 1;
        }

        ULONG command;
        printf("Enter command to send: ");
        scanf_s("%d", &command);

        for (int i = 0; i < 100; i++) {
            LARGE_INTEGER sendTime;
            GetSystemTimeAsFileTime((FILETIME*)&sendTime);

            hr = FilterSendMessage(hPort, &command, sizeof(command), &sendTime, sizeof(sendTime), &bytesReturned);
            if (FAILED(hr)) {
                printf("Send failed\n");
            }
            else {
                DisplayTime(sendTime);
                printf("Message sent successfully\n");
            }
        }

        CloseHandle(hPort);
        break;
    }

    case 2: {
        hr = FilterConnectCommunicationPort(L"\\SendMessage", 0, nullptr, 0, nullptr, &hPort);
        if (FAILED(hr)) {
            printf("Error connecting to port (HR=0x%08X)\n", hr);
            return 1;
        }

		DWORD currentPid = GetCurrentProcessId();
        printf("Current PID : % d\n", currentPid);

		hr = FilterSendMessage(hPort, &currentPid, sizeof(currentPid), nullptr, 0, &bytesReturned);
        if (FAILED(hr)) {
            printf("Error sending PID to KM (HR=0x%08X)\n", hr);
        }

        printf("Test");

        hr2 = FilterConnectCommunicationPort(L"\\CommunicationPort", 0, nullptr, 0, nullptr, &hPort2);
        if (FAILED(hr2)) {
            printf("Error connecting to port (HR=0x%08X)\n", hr2);
            return 1;
        }

        BYTE buffer[8192]; // Buffer c? ??nh
        FILTER_MESSAGE_HEADER* message = (FILTER_MESSAGE_HEADER*)buffer;

        printf("Receiving logs...\n");

        while (true) {
            hr2 = FilterGetMessage(hPort2, message, sizeof(buffer), nullptr);
            if (FAILED(hr2)) {
                printf("Error receiving message (HR=0x%08X)\n", hr2);
                break;
            }

            
            //HandleMessage(buffer + sizeof(FILTER_MESSAGE_HEADER));
            MESSAGE_ACTION messageActionProcessList;

            //FILTER_MESSAGE_HEADER* messageHeader = (FILTER_MESSAGE_HEADER*)buffer;
            HEADER* header = (HEADER*)(buffer + sizeof(FILTER_MESSAGE_HEADER));

            if (header->Action == ProcessPath) {
                //printf("Test\n");

                PID_BOOK_ENTRY* info = (PID_BOOK_ENTRY*)(buffer + sizeof(FILTER_MESSAGE_HEADER));

                if (info->ProcessPathLength > 0) {
                    std::wstring ntProcessPath((wchar_t*)(buffer + sizeof(FILTER_MESSAGE_HEADER) + info->ProcessPathOffset), info->ProcessPathLength / sizeof(wchar_t));
                    //std::wcout << L"NT Process Path: " << ntProcessPath << std::endl;

                    std::wstring win32ProcessPath = GetDosNameFromNTName(ntProcessPath);
                    //std::wcout << L"Win32 Process Path: " << win32ProcessPath << std::endl;

                    auto lRetVal = CheckFile(win32ProcessPath);
                    if (lRetVal == 0) {
                        //std::wcout << L"Verified Signer: YES\n\n";
                        info->CommandType = ADD_PROCESS_LIST;
                    }
                    else {
                        //std::wcout << L"Verified Signer: NO\n\n";
                        //info->CommandType = ADD_BLOCK_LIST;
                        std::wcout << L"Win32 Process Path: " << win32ProcessPath << std::endl;
                        char userChoice;
                        std::cout << "Do you want to add this to whitelist? (Y/N): ";
                        std::cin >> userChoice;

                        if (userChoice == 'Y' || userChoice == 'y') {
                            info->CommandType = ADD_PROCESS_LIST;  // Thêm vào whitelist
                        }
                        else {
                            std::cout << "Skip this process." << std::endl;
                            continue;
                        }
                    }

                    messageActionProcessList.Command = info->CommandType;
                    messageActionProcessList.Length = static_cast<USHORT>(info->ProcessPathLength);
                    memcpy(messageActionProcessList.Buffer, (wchar_t*)(buffer + sizeof(FILTER_MESSAGE_HEADER) + info->ProcessPathOffset), info->ProcessPathLength);

                    hr = FilterSendMessage(hPort, &messageActionProcessList, sizeof(messageActionProcessList), nullptr, 0, &bytesReturned);
                    if (FAILED(hr)) {
                        //printf("Error sending verification result to KM (HR=0x%08X)\n", hr);
                    }
                }
            }
            else if (header->Action != ProcessPath) {
                TRACKED_ACTION* info = (TRACKED_ACTION*)(buffer + sizeof(FILTER_MESSAGE_HEADER));

                DisplayTime(info->Time);
                printf("Action: %s\n",
                    info->Action == Connect ? "Connect" :
                    info->Action == Disconnect ? "Disconnect" :
                    info->Action == Read ? "Read" :
                    info->Action == Write ? "Write" :
                    info->Action == Create ? "Create" :
                    info->Action == Open ? "Open" :
                    info->Action == Delete ? "Delete" : "Unknown");

                if (info->CanonicalFilePathLength > 0) {
                    std::wstring filePath((wchar_t*)(buffer + sizeof(FILTER_MESSAGE_HEADER) + info->CanonicalFilePathOffset), info->CanonicalFilePathLength / sizeof(wchar_t));
                    wprintf(L"File: %s\n", filePath.c_str());
                }

                if (info->DeviceIdLength > 0) {
                    std::wstring deviceId((wchar_t*)(buffer + sizeof(FILTER_MESSAGE_HEADER) + info->DeviceIdOffset), info->DeviceIdLength / sizeof(wchar_t));
                    wprintf(L"Device ID: %s\n", deviceId.c_str());
                }

                printf("PID: %u\n", info->Pid);

                if (info->ProcessPathLength > 0) {
                    std::wstring processPath((wchar_t*)(buffer + sizeof(FILTER_MESSAGE_HEADER) + info->ProcessPathOffset), info->ProcessPathLength / sizeof(wchar_t));
                    wprintf(L"Process: %s\n\n", processPath.c_str());
                }
            }

			
        }

        CloseHandle(hPort);
		CloseHandle(hPort2);
        break;
    }

   // case 3: {
   //     auto hFile = CreateFile(L"\\\\.\\ExternalDeviceMonitor", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
   //     if (hFile == INVALID_HANDLE_VALUE) {
			//printf("Failed to open file\n");
   //         return 1;
   //     }

   //     int size = 1 << 16; //64 KB
   //     auto buffer = std::make_unique<BYTE[]>(size);

   //     while (true) {
   //         DWORD bytes = 0;
   //         if (!ReadFile(hFile, buffer.get(), size, &bytes, nullptr)) {

   //             //printf("(%u)\n", GetLastError());
   //         }

   //         if (bytes) {
   //             DisplayInfo(buffer.get(), bytes);
   //             printf("------------------------------------------------------\n\n\n");
   //         }
   //         //Sleep(1000);

   //     }
   //     break;
   // }

    default:
        printf("Invalid choice!\n");
        break;
    }

    return 0;
}

