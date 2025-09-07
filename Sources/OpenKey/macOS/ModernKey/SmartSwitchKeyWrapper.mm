//
//  SmartSwitchKeyWrapper.mm
//  ModernKey
//
//  C wrapper for C++ SmartSwitchKey functions
//

#include "../../engine/SmartSwitchKey.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

using namespace std;

extern "C" {
    bool isAppExcluded(const char* bundleId) {
        if (!bundleId) return false;
        string id(bundleId);
        return ::isAppExcluded(id);
    }
    
    void addAppToExclusionList(const char* bundleId) {
        if (!bundleId) return;
        string id(bundleId);
        ::addAppToExclusionList(id);
    }
    
    void removeAppFromExclusionList(const char* bundleId) {
        if (!bundleId) return;
        string id(bundleId);
        ::removeAppFromExclusionList(id);
    }
    
    char** getExcludedApps(int* count) {
        vector<string> apps = ::getExcludedApps();
        *count = (int)apps.size();
        
        if (*count == 0) {
            return nullptr;
        }
        
        char** result = (char**)malloc(*count * sizeof(char*));
        for (int i = 0; i < *count; i++) {
            result[i] = (char*)malloc((apps[i].length() + 1) * sizeof(char));
            strcpy(result[i], apps[i].c_str());
        }
        
        return result;
    }
    
    void freeExcludedApps(char** apps, int count) {
        if (!apps) return;
        
        for (int i = 0; i < count; i++) {
            if (apps[i]) {
                free(apps[i]);
            }
        }
        free(apps);
    }
}