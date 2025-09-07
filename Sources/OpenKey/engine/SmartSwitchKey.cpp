//
//  SmartSwitchKey.cpp
//  OpenKey
//
//  Created by Tuyen on 8/13/19.
//  Copyright © 2019 Tuyen Mai. All rights reserved.
//

#include "SmartSwitchKey.h"
#include <map>
#include <set>
#include <iostream>
#include <memory.h>

//main data, i use `map` because it has O(Log(n))
static map<string, Int8> _smartSwitchKeyData;
static string _cacheKey = ""; //use cache for faster
static Int8 _cacheData = 0; //use cache for faster

//exclusion list data
static set<string> _excludedApps;
static string _excludeCacheKey = ""; //use cache for faster
static bool _excludeCacheData = false; //use cache for faster

void initSmartSwitchKey(const Byte* pData, const int& size) {
    _smartSwitchKeyData.clear();
    if (pData == NULL) return;
    Uint16 count = 0;
    Uint32 cursor = 0;
    if (size >= 2) {
        memcpy(&count, pData + cursor, 2);
        cursor+=2;
    }
    Uint8 bundleIdSize;
    Uint8 value;
    for (int i = 0; i < count; i++) {
        bundleIdSize = pData[cursor++];
        string bundleId((char*)pData + cursor, bundleIdSize);
        cursor += bundleIdSize;
        value = pData[cursor++];
        _smartSwitchKeyData[bundleId] = value;
    }
}

void getSmartSwitchKeySaveData(vector<Byte>& outData) {
    outData.clear();
    Uint16 count = (Uint16)_smartSwitchKeyData.size();
    outData.push_back((Byte)count);
    outData.push_back((Byte)(count>>8));
    
    for (std::map<string, Int8>::iterator it = _smartSwitchKeyData.begin(); it != _smartSwitchKeyData.end(); ++it) {
        outData.push_back((Byte)it->first.length());
        for (int j = 0; j < it->first.length(); j++) {
            outData.push_back(it->first[j]);
        }
        outData.push_back(it->second);
    }
}

int getAppInputMethodStatus(const string& bundleId, const int& currentInputMethod) {
    if (_cacheKey.compare(bundleId) == 0) {
        return _cacheData;
    }
    if (_smartSwitchKeyData.find(bundleId) != _smartSwitchKeyData.end()) {
        _cacheKey = bundleId;
        _cacheData = _smartSwitchKeyData[bundleId];
        return _cacheData;
    }
    _cacheKey = bundleId;
    _cacheData = currentInputMethod;
    _smartSwitchKeyData[bundleId] = _cacheData;
    return -1;
}

void setAppInputMethodStatus(const string& bundleId, const int& language) {
    _smartSwitchKeyData[bundleId] = language;
    _cacheKey = bundleId;
    _cacheData = language;
}

bool isAppExcluded(const string& bundleId) {
    if (_excludeCacheKey.compare(bundleId) == 0) {
        return _excludeCacheData;
    }
    _excludeCacheKey = bundleId;
    _excludeCacheData = _excludedApps.find(bundleId) != _excludedApps.end();
    return _excludeCacheData;
}

void addAppToExclusionList(const string& bundleId) {
    _excludedApps.insert(bundleId);
    if (_excludeCacheKey.compare(bundleId) == 0) {
        _excludeCacheData = true;
    }
}

void removeAppFromExclusionList(const string& bundleId) {
    _excludedApps.erase(bundleId);
    if (_excludeCacheKey.compare(bundleId) == 0) {
        _excludeCacheData = false;
    }
}

vector<string> getExcludedApps() {
    vector<string> result;
    for (const auto& app : _excludedApps) {
        result.push_back(app);
    }
    return result;
}

void initExclusionList(const Byte* pData, const int& size) {
    _excludedApps.clear();
    if (pData == NULL) return;
    Uint16 count = 0;
    Uint32 cursor = 0;
    if (size >= 2) {
        memcpy(&count, pData + cursor, 2);
        cursor += 2;
    }
    Uint8 bundleIdSize;
    for (int i = 0; i < count; i++) {
        bundleIdSize = pData[cursor++];
        string bundleId((char*)pData + cursor, bundleIdSize);
        cursor += bundleIdSize;
        _excludedApps.insert(bundleId);
    }
}

void getExclusionListSaveData(vector<Byte>& outData) {
    outData.clear();
    Uint16 count = (Uint16)_excludedApps.size();
    outData.push_back((Byte)count);
    outData.push_back((Byte)(count >> 8));
    
    for (const auto& app : _excludedApps) {
        outData.push_back((Byte)app.length());
        for (int j = 0; j < app.length(); j++) {
            outData.push_back(app[j]);
        }
    }
}
