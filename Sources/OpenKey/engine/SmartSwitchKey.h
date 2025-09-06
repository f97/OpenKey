//
//  SmartSwitchKey.h
//  OpenKey
//
//  Created by Tuyen on 8/13/19.
//  Copyright © 2019 Tuyen Mai. All rights reserved.
//

#ifndef SmartSwitchKey_h
#define SmartSwitchKey_h

#include "DataType.h"
#include <string>

using namespace std;

void initSmartSwitchKey(const Byte* pData, const int& size);

/**
 * convert all data to save on disk
 */
void getSmartSwitchKeySaveData(vector<Byte>& outData);

/**
 * find and get language input method, if don't has set @currentInputMethod value for this app
 * return:
 * -1: don't have this bundleId
 * 0: English
 * 1: Vietnamese
 */
int getAppInputMethodStatus(const string& bundleId, const int& currentInputMethod);

/**
 * Set default language for this @bundleId
 */
void setAppInputMethodStatus(const string& bundleId, const int& language);

/**
 * Check if application is excluded from Vietnamese input
 * @param bundleId: application identifier (exe name)
 * @return true if excluded, false otherwise
 */
bool isAppExcluded(const string& bundleId);

/**
 * Add application to exclusion list
 * @param bundleId: application identifier (exe name)
 */
void addAppToExclusionList(const string& bundleId);

/**
 * Remove application from exclusion list
 * @param bundleId: application identifier (exe name)
 */
void removeAppFromExclusionList(const string& bundleId);

/**
 * Get all excluded applications
 * @return vector of excluded application identifiers
 */
vector<string> getExcludedApps();

/**
 * Initialize exclusion list from saved data
 * @param pData: pointer to saved data
 * @param size: size of data
 */
void initExclusionList(const Byte* pData, const int& size);

/**
 * Get exclusion list data for saving
 * @param outData: output vector to store data
 */
void getExclusionListSaveData(vector<Byte>& outData);

#endif /* SmartSwitchKey_h */
