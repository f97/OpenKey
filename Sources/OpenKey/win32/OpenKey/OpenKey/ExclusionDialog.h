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
#ifndef ExclusionDialog_h
#define ExclusionDialog_h

#include "BaseDialog.h"
#include <vector>
#include <string>

using namespace std;

class ExclusionDialog : public BaseDialog {
private:
    HWND hListView;
    HHOOK hMouseHook;
    bool isAppSelectionMode;
    
    void initDialog();
    void refreshExclusionList();
    void onAddApp();
    void onAddAppDragDrop();
    void onRemoveApp();
    void onClearAll();
    void startAppSelection();
    void stopAppSelection();
    string getAppNameAtPoint(POINT pt);
    
    static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static ExclusionDialog* instance;
    
public:
    ExclusionDialog(const HINSTANCE& hInstance, const int& resourceId);
    ~ExclusionDialog();
    INT_PTR eventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void fillData() override; 
};

#endif
