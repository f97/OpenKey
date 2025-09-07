/*----------------------------------------------------------
OpenKey - The Cross platform Open source Vietnamese Keyboard application.

Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey
Fanpage: https://www.facebook.com/OpenKeyVN

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#include "ExclusionDialog.h"
#include "AppDelegate.h"
#include "../../../engine/SmartSwitchKey.h"
#include <Shlobj.h>
#include <Commdlg.h>
#include <CommCtrl.h>
#include <Psapi.h>

// Static member initialization
ExclusionDialog* ExclusionDialog::instance = nullptr;

ExclusionDialog::ExclusionDialog(const HINSTANCE& hInstance, const int& resourceId)
    : BaseDialog(hInstance, resourceId), hMouseHook(NULL), isAppSelectionMode(false) {
    instance = this;
}

ExclusionDialog::~ExclusionDialog() {
    if (hMouseHook) {
        UnhookWindowsHookEx(hMouseHook);
        hMouseHook = NULL;
    }
    instance = nullptr;
}

void ExclusionDialog::initDialog() {
    HINSTANCE hIns = GetModuleHandleW(NULL);
    
    // Set dialog icon
    SET_DIALOG_ICON(IDI_APP_ICON);
    
    // Initialize ListView
    hListView = GetDlgItem(hDlg, IDC_LIST_EXCLUDED_APPS);
    
    // Set ListView style
    ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    
    // Add column to ListView
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 280;
    lvc.pszText = (LPWSTR)_T("Tên ứng dụng");
    ListView_InsertColumn(hListView, 0, &lvc);
    
    // Load exclusion list
    refreshExclusionList();
}

void ExclusionDialog::refreshExclusionList() {
    // Clear existing items
    ListView_DeleteAllItems(hListView);
    
    // Get excluded apps
    vector<string> excludedApps = getExcludedApps();
    
    // Add items to ListView
    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iSubItem = 0;
    
    for (int i = 0; i < excludedApps.size(); i++) {
        // Convert UTF-8 to wide string
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, excludedApps[i].c_str(), (int)excludedApps[i].length(), NULL, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, excludedApps[i].c_str(), (int)excludedApps[i].length(), &wstr[0], size_needed);
        
        lvi.iItem = i;
        lvi.pszText = (LPWSTR)wstr.c_str();
        ListView_InsertItem(hListView, &lvi);
    }
}

void ExclusionDialog::onAddApp() {
    OPENFILENAME ofn;
    TCHAR szFile[260] = { 0 };
    
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = _T("Chọn ứng dụng để loại trừ");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    // Display the Open dialog box
    if (GetOpenFileName(&ofn) == TRUE) {
        // Extract just the filename from the full path
        TCHAR* filename = _tcsrchr(szFile, _T('\\'));
        if (filename != NULL) {
            filename++; // Move past the backslash
        } else {
            filename = szFile; // Use the whole string if no backslash found
        }
        
        // Convert to UTF-8 for the engine
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, filename, (int)_tcslen(filename), NULL, 0, NULL, NULL);
        std::string exeName(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, filename, (int)_tcslen(filename), &exeName[0], size_needed, NULL, NULL);
        
        // Check if already excluded
        if (isAppExcluded(exeName)) {
            WCHAR msg[512];
            wsprintf(msg, _T("'%s' đã có trong danh sách."), filename);
            MessageBox(hDlg, msg, _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
        } else {
            // Add to exclusion list
            addAppToExclusionList(exeName);
            
            // Save the exclusion list
            saveExclusionListData();
            
            // Refresh the list
            refreshExclusionList();
            
            // Show confirmation message
            WCHAR msg[512];
            wsprintf(msg, _T("Đã thêm '%s' vào danh sách loại trừ."), filename);
            MessageBox(hDlg, msg, _T("Thành công"), MB_OK | MB_ICONINFORMATION);
        }
    }
}

void ExclusionDialog::onRemoveApp() {
    int selectedIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (selectedIndex == -1) {
        MessageBox(hDlg, _T("Vui lòng chọn ứng dụng cần xóa."), _T("Thông báo"), MB_OK | MB_ICONWARNING);
        return;
    }
    
    // Get the selected item text
    TCHAR itemText[256];
    ListView_GetItemText(hListView, selectedIndex, 0, itemText, 256);
    
    // Convert to UTF-8
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, itemText, (int)_tcslen(itemText), NULL, 0, NULL, NULL);
    std::string exeName(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, itemText, (int)_tcslen(itemText), &exeName[0], size_needed, NULL, NULL);
    
    // Remove from exclusion list
    removeAppFromExclusionList(exeName);
    
    // Save the exclusion list
    saveExclusionListData();
    
    // Refresh the list
    refreshExclusionList();
    
    MessageBox(hDlg, _T("Đã xóa ứng dụng khỏi danh sách loại trừ."), _T("Thành công"), MB_OK | MB_ICONINFORMATION);
}

void ExclusionDialog::onClearAll() {
    vector<string> excludedApps = getExcludedApps();
    if (excludedApps.empty()) {
        MessageBox(hDlg, _T("Danh sách đã trống."), _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    int result = MessageBox(hDlg, 
        _T("Bạn có chắc chắn muốn xóa tất cả ứng dụng khỏi danh sách loại trừ?"), 
        _T("Xác nhận"), 
        MB_YESNO | MB_ICONQUESTION);
        
    if (result == IDYES) {
        // Clear all exclusions
        for (const auto& app : excludedApps) {
            removeAppFromExclusionList(app);
        }
        
        // Save the exclusion list
        saveExclusionListData();
        
        // Refresh the list
        refreshExclusionList();
        
        MessageBox(hDlg, _T("Đã xóa tất cả ứng dụng khỏi danh sách loại trừ."), _T("Thành công"), MB_OK | MB_ICONINFORMATION);
    }
}

void ExclusionDialog::onAddAppDragDrop() {
    startAppSelection();
}

void ExclusionDialog::startAppSelection() {
    // Show instructions
    int result = MessageBox(hDlg, 
        _T("Kéo con trỏ đến ứng dụng bạn muốn loại trừ rồi bấm chuột trái.\n\nBấm OK để bắt đầu, Cancel để hủy."), 
        _T("Chọn ứng dụng để loại trừ"), 
        MB_OKCANCEL | MB_ICONINFORMATION);
        
    if (result == IDOK) {
        isAppSelectionMode = true;
        
        // Change cursor to crosshair
        SetCursor(LoadCursor(NULL, IDC_CROSS));
        
        // Install mouse hook
        hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, GetModuleHandle(NULL), 0);
        
        // Set timer to auto-cleanup after 10 seconds
        SetTimer(hDlg, 1, 10000, NULL);
    }
}

void ExclusionDialog::stopAppSelection() {
    if (hMouseHook) {
        UnhookWindowsHookEx(hMouseHook);
        hMouseHook = NULL;
    }
    isAppSelectionMode = false;
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    KillTimer(hDlg, 1);
}

string ExclusionDialog::getAppNameAtPoint(POINT pt) {
    HWND hwnd = WindowFromPoint(pt);
    if (!hwnd) return "";
    
    // Get the root window
    HWND rootHwnd = GetAncestor(hwnd, GA_ROOTOWNER);
    if (!rootHwnd) rootHwnd = hwnd;
    
    // Get process ID
    DWORD processId;
    GetWindowThreadProcessId(rootHwnd, &processId);
    
    // Open process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) return "";
    
    // Get executable name
    TCHAR exeName[MAX_PATH];
    if (GetModuleBaseName(hProcess, NULL, exeName, MAX_PATH)) {
        CloseHandle(hProcess);
        
        // Convert to UTF-8
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, exeName, (int)_tcslen(exeName), NULL, 0, NULL, NULL);
        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, exeName, (int)_tcslen(exeName), &result[0], size_needed, NULL, NULL);
        
        return result;
    }
    
    CloseHandle(hProcess);
    return "";
}

LRESULT CALLBACK ExclusionDialog::MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance && instance->isAppSelectionMode) {
        if (wParam == WM_LBUTTONDOWN) {
            PMSLLHOOKSTRUCT pMouseStruct = (PMSLLHOOKSTRUCT)lParam;
            POINT pt = pMouseStruct->pt;
            
            string appName = instance->getAppNameAtPoint(pt);
            
            if (!appName.empty()) {
                // Check if already excluded
                if (isAppExcluded(appName)) {
                    WCHAR msg[512];
                    // Convert to wide string for display
                    int size_needed = MultiByteToWideChar(CP_UTF8, 0, appName.c_str(), (int)appName.length(), NULL, 0);
                    std::wstring wstr(size_needed, 0);
                    MultiByteToWideChar(CP_UTF8, 0, appName.c_str(), (int)appName.length(), &wstr[0], size_needed);
                    
                    wsprintf(msg, _T("'%s' đã có trong danh sách."), wstr.c_str());
                    MessageBox(instance->hDlg, msg, _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
                } else {
                    // Add to exclusion list
                    addAppToExclusionList(appName);
                    saveExclusionListData();
                    instance->refreshExclusionList();
                    
                    // Show confirmation
                    WCHAR msg[512];
                    // Convert to wide string for display
                    int size_needed = MultiByteToWideChar(CP_UTF8, 0, appName.c_str(), (int)appName.length(), NULL, 0);
                    std::wstring wstr(size_needed, 0);
                    MultiByteToWideChar(CP_UTF8, 0, appName.c_str(), (int)appName.length(), &wstr[0], size_needed);
                    
                    wsprintf(msg, _T("Đã thêm '%s' vào danh sách loại trừ."), wstr.c_str());
                    MessageBox(instance->hDlg, msg, _T("Thành công"), MB_OK | MB_ICONINFORMATION);
                }
            } else {
                MessageBox(instance->hDlg, _T("Không thể xác định ứng dụng tại vị trí này."), _T("Lỗi"), MB_OK | MB_ICONWARNING);
            }
            
            instance->stopAppSelection();
            return 1; // Block the mouse click
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

INT_PTR ExclusionDialog::eventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        this->hDlg = hDlg;
        initDialog();
        return TRUE;
    case WM_TIMER:
        if (wParam == 1) {
            // Timeout for app selection mode
            stopAppSelection();
            MessageBox(hDlg, _T("Thời gian chọn ứng dụng đã hết. Vui lòng thử lại."), _T("Hết thời gian"), MB_OK | MB_ICONINFORMATION);
        }
        break;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDOK:
            stopAppSelection(); // Clean up if dialog is closed during selection
            EndDialog(hDlg, IDOK);
            break;
        case IDC_BUTTON_ADD_APP:
            onAddApp();
            break;
        case IDC_BUTTON_ADD_APP_DRAGDROP:
            onAddAppDragDrop();
            break;
        case IDC_BUTTON_REMOVE_APP:
            onRemoveApp();
            break;
        case IDC_BUTTON_CLEAR_ALL:
            onClearAll();
            break;
        }
        break;
    }
    }
    return FALSE;
}

void ExclusionDialog::fillData() {
    // This method is called when the dialog needs to be populated with data
    // For ExclusionDialog, the data is loaded in initDialog() via refreshExclusionList()
    // No additional filling is needed as the data is loaded dynamically
}