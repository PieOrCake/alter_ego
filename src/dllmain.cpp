#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <set>
#include <mutex>
#include <thread>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <atomic>
#include <functional>

#include "nexus/Nexus.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "GW2API.h"
#include "IconManager.h"
#include "ChatLink.h"
#include "HoardAndSeekAPI.h"
#include "SpecDescriptions.h"
#include "SkinCache.h"
#include "OwnedSkins.h"
#include "Commerce.h"
#include "WikiImage.h"
#include "HttpClient.h"
#include <nlohmann/json.hpp>
#include "AddonShared.h"
#include "Clears.h"

// Version constants
#define V_MAJOR 1
#define V_MINOR 0
#define V_BUILD 1
#define V_REVISION 2

// Quick Access icon identifiers
#define QA_ID "QA_ALTER_EGO"
#define TEX_ICON "TEX_ALTER_EGO_ICON"
#define TEX_ICON_HOVER "TEX_ALTER_EGO_ICON_HOVER"

// --- Normal icon ---
static const unsigned char ICON_NORMAL[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a,
    0xf4, 0x00, 0x00, 0x0a, 0xc6, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x9d, 0x97, 0x79, 0x6c, 0x1c,
    0xe7, 0x79, 0xc6, 0x7f, 0x73, 0xef, 0x7d, 0x73, 0x97, 0xbb, 0x3c, 0x44, 0x91, 0x22, 0x75, 0x50,
    0xb7, 0x28, 0x09, 0x92, 0xe5, 0xd8, 0x91, 0x9d, 0xb8, 0x75, 0x2c, 0xb5, 0x29, 0x5c, 0xdb, 0xe8,
    0x11, 0xa4, 0x05, 0xea, 0xc0, 0x41, 0x8a, 0xb4, 0x41, 0xdc, 0x22, 0x01, 0xdc, 0x04, 0x6d, 0xd0,
    0xa0, 0x49, 0xfd, 0x47, 0x81, 0x1a, 0x4e, 0xdd, 0xa2, 0x40, 0x5d, 0x34, 0x6e, 0xdd, 0xba, 0x68,
    0x62, 0xa0, 0x02, 0xa2, 0xb6, 0x72, 0x54, 0x59, 0x52, 0xac, 0x9a, 0x92, 0x45, 0xea, 0xa0, 0x2c,
    0x92, 0x22, 0x29, 0x92, 0x4b, 0x72, 0xcf, 0xd9, 0xdd, 0xd9, 0x63, 0x8e, 0xaf, 0x7f, 0xd0, 0x12,
    0xa4, 0x38, 0x0e, 0xdc, 0xbe, 0xff, 0xcd, 0x7c, 0x33, 0xf3, 0x3c, 0xf3, 0xbc, 0xef, 0xf7, 0xbe,
    0xcf, 0xa7, 0xf0, 0xf1, 0xa3, 0x0f, 0xc8, 0x02, 0xa5, 0x7b, 0x6f, 0xee, 0xdc, 0xb2, 0x61, 0x78,
    0xa5, 0x50, 0x75, 0xbf, 0xf5, 0xfc, 0xaf, 0xbf, 0x7a, 0xfc, 0xd3, 0x07, 0xf6, 0x8e, 0x4f, 0x4c,
    0xb7, 0x1b, 0xcd, 0x76, 0xe3, 0xe9, 0x63, 0x47, 0x7e, 0xef, 0xd0, 0xbe, 0xcd, 0xc7, 0x3e, 0xff,
    0xab, 0x47, 0xff, 0x78, 0xd7, 0xe8, 0xc0, 0xc2, 0x99, 0x0b, 0xd7, 0xa6, 0xbf, 0xf9, 0x95, 0x67,
    0x5e, 0x79, 0xeb, 0xdc, 0xe4, 0x9b, 0xf7, 0xbe, 0x2f, 0xfd, 0x2c, 0xa4, 0xaf, 0x7f, 0xe9, 0xc9,
    0x47, 0x0f, 0xec, 0x1e, 0xfe, 0xfd, 0x6a, 0xcd, 0xea, 0x91, 0x65, 0xa9, 0x21, 0x04, 0x31, 0x5d,
    0x53, 0xbb, 0x0d, 0x43, 0xb3, 0x54, 0x45, 0x0e, 0xf9, 0x0c, 0xfd, 0xf5, 0xb3, 0xef, 0x5e, 0x3f,
    0xf5, 0x8d, 0x17, 0x5f, 0xbb, 0xfc, 0x95, 0xdf, 0x39, 0xfe, 0xbb, 0x8f, 0x1c, 0xd9, 0x35, 0x5a,
    0xb7, 0x5a, 0x25, 0x4d, 0x55, 0xc6, 0x6c, 0xdb, 0xcd, 0x3a, 0x8e, 0x23, 0x5a, 0x6d, 0x7b, 0xce,
    0x71, 0xdd, 0x54, 0xb3, 0xd5, 0x29, 0x86, 0x83, 0xfe, 0x64, 0xb9, 0x5a, 0xff, 0xe6, 0x40, 0x5f,
    0xfa, 0xd9, 0x27, 0xbf, 0xf0, 0x9d, 0x51, 0xc0, 0xfb, 0x48, 0x02, 0x2f, 0xbe, 0xf0, 0x5b, 0x5f,
    0xdd, 0x3d, 0xba, 0xf1, 0x6b, 0x86, 0xae, 0x25, 0x82, 0x01, 0x03, 0x49, 0x92, 0xe8, 0xd8, 0x2e,
    0x8a, 0x22, 0x13, 0x0e, 0xfa, 0xa8, 0xd5, 0x9b, 0x34, 0x5b, 0x1d, 0x34, 0x4d, 0x75, 0x13, 0xb1,
    0xd0, 0xc4, 0x62, 0xbe, 0x94, 0xba, 0x38, 0x39, 0x1d, 0x38, 0x3c, 0xb6, 0x35, 0x11, 0x0e, 0xfa,
    0x88, 0x47, 0x43, 0xf8, 0x0c, 0x9d, 0xb5, 0x52, 0x95, 0x7a, 0xa3, 0xc5, 0xc9, 0xd3, 0x97, 0x18,
    0x1e, 0xcc, 0x11, 0x0c, 0xf8, 0xac, 0xe1, 0x81, 0x6c, 0xe9, 0x6f, 0x5e, 0x3b, 0xf9, 0xa5, 0x3f,
    0xfd, 0xcb, 0x7f, 0xf9, 0xc1, 0x1d, 0x3c, 0xe5, 0x5e, 0xf0, 0x17, 0xbe, 0xfc, 0xd4, 0xb3, 0x7b,
    0x46, 0x07, 0xff, 0x2c, 0x16, 0x09, 0x46, 0x1a, 0x56, 0x9b, 0xeb, 0xd3, 0x8b, 0xd8, 0xb6, 0x8b,
    0x24, 0x81, 0xe3, 0xb8, 0x14, 0x4a, 0x35, 0xf2, 0x6b, 0x15, 0x8a, 0xa5, 0x1a, 0xb7, 0x97, 0x0b,
    0x72, 0xb1, 0x54, 0xeb, 0x56, 0x55, 0x39, 0x32, 0xb4, 0xa1, 0xdb, 0x5f, 0x6f, 0xb4, 0xb0, 0x6d,
    0x87, 0xe5, 0xd5, 0x0a, 0x85, 0x92, 0x89, 0x27, 0x04, 0x7d, 0xd9, 0x14, 0xdb, 0x46, 0xfa, 0x59,
    0x58, 0x2a, 0xf0, 0xd0, 0xa1, 0xed, 0x9a, 0xcf, 0xd0, 0x22, 0xdb, 0x86, 0xfb, 0xf6, 0xd6, 0x1a,
    0xcd, 0x0b, 0x17, 0xaf, 0xcc, 0x2e, 0x7e, 0x88, 0xc0, 0x6f, 0xfe, 0xca, 0x43, 0xdf, 0x0d, 0xf8,
    0x7d, 0xc3, 0xa9, 0x44, 0x84, 0x4b, 0x57, 0x67, 0xd1, 0x35, 0x8d, 0xfc, 0x5a, 0x99, 0x66, 0xbb,
    0x83, 0xe7, 0x7a, 0x54, 0x6b, 0x16, 0xb3, 0xf3, 0x2b, 0xac, 0x16, 0xab, 0x54, 0x4d, 0x8b, 0x56,
    0xc7, 0x66, 0x71, 0xb9, 0x88, 0x22, 0x2b, 0x24, 0xe3, 0x61, 0x34, 0x4d, 0x25, 0x11, 0x0b, 0x91,
    0x5f, 0xab, 0xe0, 0x7a, 0x1e, 0xae, 0xeb, 0x91, 0x4e, 0x45, 0x19, 0x19, 0xec, 0xe1, 0xdd, 0x89,
    0x69, 0x7a, 0xb3, 0x29, 0xba, 0x92, 0xd1, 0x64, 0x4f, 0x77, 0x62, 0xe4, 0xaf, 0xbf, 0x7f, 0xf2,
    0xef, 0x00, 0xe4, 0xfb, 0xf4, 0x97, 0x24, 0xc3, 0xac, 0x5b, 0x34, 0x5b, 0x1d, 0x02, 0x3e, 0x83,
    0x4a, 0xb5, 0xc1, 0xfe, 0x5d, 0xc3, 0x1c, 0xde, 0xb7, 0x85, 0x68, 0x24, 0x48, 0x3c, 0x1a, 0xe2,
    0x33, 0x8f, 0x8c, 0xb1, 0x6b, 0xeb, 0x46, 0xc2, 0x21, 0x3f, 0x55, 0xb3, 0x01, 0xc0, 0xe4, 0xd4,
    0x1c, 0x85, 0xb2, 0x89, 0xcf, 0xd0, 0x11, 0x42, 0x60, 0x35, 0xdb, 0x54, 0x4d, 0x8b, 0xf3, 0xe3,
    0x53, 0xfc, 0xfd, 0x1b, 0x6f, 0xa1, 0xc8, 0x32, 0xfd, 0xb9, 0x14, 0xb2, 0x2c, 0xd1, 0xee, 0xd8,
    0x0c, 0x0f, 0xe4, 0x76, 0x03, 0x7b, 0x00, 0xd4, 0x7b, 0xf1, 0x03, 0x7e, 0xe3, 0xea, 0xfe, 0x9d,
    0x9b, 0x0e, 0x67, 0x33, 0x09, 0xf6, 0x6c, 0x1f, 0xc4, 0xb2, 0xda, 0xfc, 0xf0, 0xe4, 0x3b, 0x4c,
    0x5c, 0x9f, 0xe3, 0xe1, 0x43, 0xdb, 0x99, 0x99, 0x5f, 0xa1, 0xdd, 0x71, 0xe8, 0xcd, 0xa5, 0xa8,
    0x5b, 0x2d, 0x64, 0x59, 0xa6, 0xd3, 0xb1, 0xe9, 0xcf, 0x75, 0x31, 0xbf, 0x58, 0xe0, 0xfd, 0xd9,
    0x65, 0xb2, 0xe9, 0x38, 0xd1, 0x70, 0x80, 0xc1, 0xfe, 0x6e, 0x8e, 0x7d, 0x6a, 0x3f, 0xff, 0x79,
    0xe6, 0x32, 0xe3, 0x93, 0xd3, 0xec, 0xd8, 0x3a, 0x40, 0xad, 0xde, 0xa4, 0x50, 0x36, 0x19, 0xe8,
    0xcd, 0x84, 0x24, 0x49, 0x92, 0x85, 0x10, 0xf7, 0x2b, 0x70, 0x64, 0xff, 0xd6, 0xde, 0xe1, 0x9d,
    0x43, 0x84, 0x32, 0x71, 0xf4, 0x70, 0x90, 0x70, 0xc8, 0xcf, 0xd8, 0xae, 0x4d, 0x00, 0x2c, 0x2c,
    0x15, 0x88, 0x84, 0xfc, 0x54, 0xaa, 0x75, 0xa2, 0xe1, 0x00, 0xe9, 0x64, 0x94, 0xa0, 0xdf, 0x20,
    0xe8, 0x37, 0x88, 0x46, 0x02, 0x8c, 0x6c, 0xcc, 0x31, 0xb2, 0x31, 0x87, 0x2c, 0x4b, 0x68, 0xaa,
    0x4a, 0x22, 0x1e, 0x66, 0x61, 0xa9, 0x40, 0x2e, 0x93, 0x60, 0xb5, 0x58, 0x45, 0x96, 0x24, 0xfc,
    0x7e, 0x83, 0x62, 0xb9, 0x86, 0xd5, 0x6c, 0x15, 0x84, 0x10, 0xf3, 0x3f, 0xad, 0xc0, 0x40, 0x7f,
    0x4f, 0xd7, 0xde, 0xbb, 0x57, 0xb6, 0x83, 0x24, 0x41, 0xc0, 0x67, 0x90, 0x4d, 0xc7, 0x59, 0xcc,
    0x17, 0xe9, 0x4a, 0x44, 0xf0, 0x84, 0x60, 0x72, 0x6a, 0x8e, 0x5c, 0x26, 0xc1, 0xc8, 0x60, 0x0e,
    0x5d, 0xd7, 0x30, 0x6b, 0x16, 0xaa, 0xaa, 0xa0, 0x69, 0xeb, 0x25, 0x75, 0xe6, 0xc2, 0x55, 0x5c,
    0xcf, 0xa3, 0xbf, 0xa7, 0x8b, 0x74, 0x32, 0x42, 0xa9, 0x5c, 0x63, 0x72, 0x6a, 0x8e, 0x2d, 0x43,
    0xbd, 0x04, 0xfc, 0x06, 0xef, 0x5c, 0x7a, 0x7f, 0x19, 0x58, 0xbd, 0x8f, 0xc0, 0x3f, 0xbe, 0xf4,
    0xd5, 0x07, 0xf0, 0xe9, 0x29, 0x3a, 0x0e, 0xe8, 0x2a, 0x9e, 0xed, 0x20, 0x04, 0x14, 0xcb, 0xe6,
    0xfa, 0x87, 0x52, 0x51, 0x6c, 0xdb, 0xc1, 0x71, 0x3d, 0x82, 0xc0, 0x99, 0x0b, 0xd7, 0x68, 0xb5,
    0x3a, 0xec, 0x1e, 0x1d, 0xc4, 0x6a, 0xb6, 0xb9, 0x39, 0xbb, 0x44, 0xdb, 0x76, 0x70, 0x5d, 0x8f,
    0x66, 0xab, 0x43, 0xc7, 0x76, 0xc8, 0xa6, 0xe3, 0x04, 0xfc, 0x06, 0xe1, 0xb0, 0x9f, 0x9b, 0xb3,
    0xcb, 0x0c, 0xf6, 0x67, 0x58, 0x2d, 0x54, 0xf9, 0xc9, 0xc5, 0x1b, 0x43, 0x77, 0x70, 0xef, 0x12,
    0x08, 0x87, 0xfc, 0xb9, 0x2b, 0xef, 0xdd, 0x94, 0xcd, 0x7a, 0x93, 0xb1, 0x9d, 0x43, 0xc8, 0xf2,
    0x7a, 0x76, 0x4a, 0x95, 0x3a, 0x9a, 0xa6, 0x62, 0xe8, 0x1a, 0xad, 0xb6, 0x4d, 0xab, 0x6d, 0x93,
    0x8c, 0x87, 0xd9, 0xd8, 0x97, 0x61, 0xe2, 0xfa, 0x2d, 0x54, 0x45, 0xe6, 0xe1, 0x43, 0xdb, 0x99,
    0x5b, 0x5c, 0x65, 0x31, 0x5f, 0x22, 0x1e, 0x09, 0x32, 0xd0, 0x97, 0x46, 0x51, 0x14, 0xa6, 0xe7,
    0xf2, 0xe8, 0x9a, 0x8a, 0x04, 0x3c, 0x78, 0x70, 0x94, 0x8a, 0x69, 0x51, 0x31, 0x1b, 0x7c, 0xe6,
    0x91, 0xb1, 0xb3, 0x7f, 0xf2, 0x17, 0xaf, 0xdf, 0x4f, 0xc0, 0xac, 0x59, 0xc5, 0x9b, 0xb7, 0x96,
    0x89, 0x46, 0x82, 0x8c, 0x4f, 0xce, 0x30, 0x3a, 0xd2, 0x8f, 0xdf, 0xa7, 0x73, 0x60, 0xf7, 0x08,
    0xff, 0x7a, 0xe2, 0x1c, 0xb5, 0x46, 0x93, 0x43, 0xfb, 0x36, 0x13, 0x8b, 0x84, 0x18, 0x9f, 0x98,
    0xc6, 0x71, 0x5c, 0x22, 0xe1, 0x00, 0x85, 0xb2, 0x89, 0x59, 0xb7, 0xc8, 0xa4, 0x62, 0x20, 0x40,
    0x00, 0xa7, 0x7f, 0x72, 0x15, 0x49, 0x02, 0xcf, 0x13, 0x6c, 0xd9, 0xd4, 0x4b, 0x4f, 0x77, 0x92,
    0xaa, 0xd9, 0x60, 0x66, 0x3e, 0x8f, 0xe7, 0x79, 0x6c, 0xe8, 0xe9, 0xba, 0xdb, 0x00, 0xef, 0x14,
    0x61, 0xf0, 0xc1, 0x83, 0xa3, 0xcf, 0xee, 0xdf, 0x3d, 0x4c, 0x26, 0x15, 0x65, 0xfb, 0xe6, 0x7e,
    0x42, 0xf1, 0x30, 0x8a, 0xa6, 0x12, 0xf0, 0x1b, 0xe8, 0xba, 0x4a, 0x2a, 0x11, 0xc1, 0x67, 0xe8,
    0xa4, 0x12, 0x61, 0x06, 0xfa, 0xd2, 0x9c, 0x1b, 0xbf, 0x8e, 0xe7, 0x09, 0xc6, 0x27, 0xa6, 0xb9,
    0x7a, 0x63, 0x81, 0x50, 0xd0, 0xc7, 0xf4, 0x7c, 0x9e, 0xfe, 0x5c, 0x0a, 0x5d, 0x53, 0xd1, 0x35,
    0x8d, 0x81, 0xde, 0x34, 0x01, 0xbf, 0xc1, 0x52, 0xbe, 0xc8, 0xc5, 0x2b, 0x33, 0x58, 0xad, 0x0e,
    0xdd, 0x5d, 0x71, 0xf1, 0xfe, 0xec, 0xf2, 0xc9, 0xfb, 0x52, 0xf0, 0xc6, 0x2b, 0x7f, 0xf8, 0x4c,
    0x2a, 0x11, 0xde, 0xa7, 0x2a, 0x0a, 0xb9, 0xa1, 0x9e, 0xf5, 0x15, 0x01, 0xb5, 0x92, 0xc9, 0x5a,
    0xc9, 0xa4, 0xd5, 0xea, 0xb0, 0x77, 0xc7, 0x10, 0x9e, 0x27, 0xa8, 0x98, 0x0d, 0xcc, 0x9a, 0x45,
    0xbb, 0xe3, 0xb0, 0xbc, 0x52, 0xa2, 0x50, 0x32, 0xb9, 0xfa, 0xfe, 0x02, 0x07, 0xf7, 0x8c, 0xa0,
    0x2a, 0x0a, 0x53, 0x33, 0x8b, 0xb8, 0x9e, 0xc7, 0x9e, 0xed, 0x83, 0xf4, 0xe7, 0x52, 0x04, 0xe3,
    0x11, 0x90, 0x20, 0x11, 0x0f, 0x93, 0x5f, 0x2d, 0x33, 0xb6, 0x6b, 0x93, 0x78, 0xf9, 0xd5, 0x13,
    0x93, 0xf7, 0x2a, 0x20, 0x3f, 0x78, 0x60, 0xf4, 0x8f, 0x0c, 0x9f, 0xa1, 0x0a, 0x21, 0xc0, 0x13,
    0x60, 0xbb, 0x54, 0x96, 0x0a, 0x98, 0x35, 0x8b, 0xa5, 0x7c, 0x91, 0xd5, 0x62, 0x15, 0x45, 0x91,
    0x89, 0x47, 0x83, 0x24, 0x63, 0x61, 0xde, 0xbe, 0x70, 0x0d, 0x21, 0xd6, 0xc9, 0x80, 0xc4, 0xf2,
    0x4a, 0x89, 0xd7, 0xdf, 0x3c, 0xc3, 0xd1, 0x07, 0x76, 0x12, 0x8f, 0x86, 0x48, 0x27, 0xa3, 0x98,
    0x35, 0x8b, 0x95, 0x42, 0xf5, 0x83, 0x86, 0xaf, 0x30, 0xb4, 0xb9, 0x9f, 0x07, 0x0e, 0xef, 0xc0,
    0x08, 0x05, 0xe4, 0x5f, 0x78, 0x78, 0xef, 0x2f, 0xdd, 0x37, 0x0b, 0x92, 0xf1, 0xf0, 0x9e, 0x1b,
    0x37, 0x6f, 0x0f, 0x74, 0x77, 0xc5, 0x7d, 0x91, 0x48, 0x10, 0x5c, 0x8f, 0x95, 0xb5, 0x32, 0x15,
    0xb3, 0xc1, 0xf4, 0xad, 0x3c, 0xa1, 0xa0, 0x9f, 0x62, 0xc9, 0xe4, 0xf0, 0xd8, 0x56, 0xc6, 0x27,
    0x67, 0x50, 0x35, 0x85, 0xd9, 0xf9, 0x15, 0x84, 0x10, 0xa8, 0xaa, 0x42, 0xc3, 0x6a, 0xd3, 0xee,
    0xd8, 0x4c, 0x4e, 0xcd, 0x93, 0x4b, 0xc7, 0xf1, 0xfb, 0x0d, 0xac, 0x66, 0x9b, 0x74, 0x32, 0x42,
    0xc3, 0xb4, 0x08, 0xc7, 0xc3, 0xe0, 0x7a, 0x20, 0x49, 0xe0, 0xba, 0xf8, 0x0d, 0xbd, 0xeb, 0xad,
    0xf3, 0x93, 0xff, 0xb4, 0xb0, 0x54, 0xa8, 0x2b, 0x00, 0xd9, 0x4c, 0xbc, 0x9d, 0x4e, 0x46, 0x7f,
    0x3b, 0x1e, 0x0d, 0x12, 0xf1, 0xfb, 0xd0, 0x14, 0x85, 0x1b, 0x33, 0x4b, 0x58, 0x56, 0x0b, 0xc3,
    0xd0, 0x59, 0x2d, 0x56, 0x58, 0x2b, 0x99, 0x2c, 0x2c, 0xad, 0xd1, 0xd3, 0x9d, 0x40, 0x51, 0x14,
    0x72, 0x99, 0x04, 0xb9, 0x4c, 0x92, 0xff, 0xb9, 0x7c, 0x93, 0x58, 0x24, 0xc8, 0x17, 0x7e, 0xe3,
    0x31, 0x7e, 0x7c, 0x6e, 0x92, 0x72, 0xb5, 0xc1, 0xc6, 0xbe, 0x34, 0xad, 0x56, 0x87, 0x4c, 0x57,
    0x0c, 0xc7, 0xf1, 0x50, 0x3d, 0x81, 0xee, 0xd3, 0x01, 0x58, 0x5b, 0x29, 0x55, 0x2f, 0x5d, 0x9d,
    0x5d, 0x68, 0x34, 0x5a, 0xff, 0x75, 0x7e, 0x7c, 0x2a, 0xaf, 0x00, 0x5c, 0xbe, 0x76, 0x6b, 0xf5,
    0x85, 0x2f, 0x3f, 0xfd, 0x39, 0xc7, 0xf5, 0xc2, 0xa5, 0x4a, 0x0d, 0xb3, 0xde, 0xc4, 0xf3, 0x3c,
    0x8a, 0xe5, 0x1a, 0xb1, 0x68, 0x90, 0xf9, 0xdb, 0x6b, 0xac, 0x16, 0x4d, 0x86, 0x37, 0xe6, 0x48,
    0x25, 0xa3, 0xd4, 0x1b, 0x2d, 0x7c, 0x86, 0xc6, 0x8e, 0x2d, 0x1b, 0x08, 0xf8, 0x0c, 0x04, 0xd0,
    0x68, 0xb4, 0x70, 0x5c, 0x97, 0x46, 0xb3, 0x4d, 0x2a, 0x11, 0x41, 0x96, 0x65, 0x92, 0xb1, 0x30,
    0xed, 0x8e, 0x83, 0x40, 0x10, 0x8e, 0x04, 0xb1, 0xaa, 0x75, 0x16, 0xf3, 0xc5, 0x95, 0x83, 0xc7,
    0x9e, 0x7f, 0xf2, 0xfc, 0xf8, 0xd4, 0xe4, 0xbd, 0xdb, 0xd0, 0xea, 0xd8, 0x8e, 0xd2, 0x97, 0x4b,
    0x91, 0x5f, 0x2d, 0x53, 0x31, 0xeb, 0xd4, 0x1b, 0x2d, 0x6c, 0xc7, 0xa5, 0xde, 0x68, 0xe1, 0x7a,
    0x1e, 0xb5, 0xba, 0x45, 0x3c, 0x1a, 0x42, 0x08, 0x41, 0x22, 0x16, 0x26, 0x1c, 0xf2, 0xb3, 0xb2,
    0x56, 0xc6, 0xef, 0xd7, 0xd9, 0x32, 0xd4, 0x4b, 0x3c, 0x16, 0xa2, 0x2b, 0x15, 0x65, 0xad, 0x58,
    0x65, 0xe2, 0xfa, 0x1c, 0x5d, 0xc9, 0x28, 0xb1, 0x48, 0x10, 0x43, 0xd7, 0x50, 0x15, 0xf9, 0x83,
    0x59, 0x27, 0x59, 0xb7, 0x16, 0x56, 0xff, 0x01, 0xb8, 0x76, 0xdf, 0x2e, 0x78, 0xe6, 0xf8, 0x91,
    0xfd, 0x89, 0x58, 0xc8, 0xbf, 0x56, 0xac, 0x52, 0xab, 0x37, 0x69, 0x77, 0x6c, 0x3c, 0x21, 0x10,
    0x9e, 0xe0, 0xf6, 0x72, 0x01, 0xab, 0xd9, 0x66, 0xcb, 0xa6, 0x5e, 0x82, 0x7e, 0x83, 0x54, 0x3c,
    0x82, 0xed, 0x38, 0xeb, 0x33, 0xdf, 0x13, 0xeb, 0xf9, 0x6f, 0xdb, 0xc8, 0xb2, 0x44, 0x36, 0x1d,
    0x67, 0x78, 0x20, 0xcb, 0x8d, 0x99, 0x25, 0x4e, 0xfe, 0xf7, 0x25, 0x96, 0x56, 0x4a, 0xec, 0xde,
    0xb6, 0x91, 0xa1, 0x81, 0xee, 0x3b, 0xbd, 0x46, 0x7e, 0xe2, 0xf3, 0xdf, 0xfa, 0xee, 0xbd, 0xf3,
    0x47, 0x05, 0xd8, 0x36, 0xd2, 0x9f, 0x6c, 0x77, 0xec, 0x88, 0x2c, 0xcb, 0x14, 0x4a, 0x26, 0xa5,
    0x4a, 0x0d, 0x55, 0x53, 0x70, 0x5d, 0x8f, 0x6c, 0x3a, 0x41, 0x34, 0x1c, 0x64, 0xdb, 0x48, 0x1f,
    0x3d, 0xd9, 0x24, 0x55, 0xd3, 0x42, 0x92, 0x40, 0xd7, 0x54, 0xe6, 0x6e, 0xaf, 0x72, 0x7e, 0x7c,
    0x8a, 0xe3, 0x9f, 0x3a, 0xc0, 0xe8, 0xe6, 0x7e, 0xe6, 0x6e, 0xaf, 0x92, 0xe9, 0x8a, 0x81, 0x24,
    0xd1, 0x9b, 0x4d, 0xf2, 0xf2, 0xab, 0x27, 0xb8, 0xb5, 0xb0, 0x52, 0xde, 0x33, 0x3a, 0x78, 0xda,
    0xb6, 0xdd, 0x87, 0x3c, 0x21, 0x5c, 0xa0, 0x76, 0x2f, 0x01, 0x05, 0xe0, 0xd4, 0xd9, 0x89, 0xd0,
    0x13, 0x8f, 0x8e, 0x1d, 0x73, 0x1d, 0x37, 0x7c, 0x73, 0x2e, 0x8f, 0xa2, 0xc8, 0x48, 0x92, 0x44,
    0xc5, 0x6c, 0x08, 0x21, 0x84, 0x74, 0x78, 0x6c, 0x2b, 0xfd, 0x3d, 0x5d, 0x58, 0x56, 0x0b, 0x00,
    0x4f, 0x78, 0x5c, 0x78, 0xef, 0x26, 0xe7, 0xc6, 0xa7, 0x78, 0xe2, 0xd1, 0xfd, 0xe4, 0x32, 0x09,
    0x12, 0xf1, 0x30, 0x42, 0x08, 0x3c, 0x01, 0xae, 0xeb, 0xad, 0x77, 0xc7, 0xae, 0x38, 0x17, 0x27,
    0x67, 0xf4, 0x17, 0x5f, 0xf9, 0xc1, 0xb6, 0xf9, 0xc5, 0xb5, 0x7f, 0x4f, 0xc4, 0x42, 0xc5, 0x1f,
    0xfe, 0xe8, 0x9d, 0xd3, 0x1f, 0x52, 0x00, 0xb8, 0xf8, 0xde, 0xd5, 0x59, 0x73, 0xef, 0xf6, 0xa1,
    0x1c, 0x02, 0x6e, 0x2f, 0x17, 0x48, 0x25, 0x22, 0xf5, 0x52, 0xb9, 0xfe, 0x1f, 0xd9, 0x74, 0xe2,
    0x97, 0xbb, 0xd3, 0xf1, 0xbb, 0xe0, 0x56, 0xab, 0xcd, 0x99, 0x0b, 0x57, 0x59, 0x2b, 0x9a, 0x3c,
    0x7e, 0x74, 0x0c, 0xcf, 0x5b, 0x1f, 0x3e, 0x00, 0xc1, 0x80, 0x8f, 0xf9, 0xc5, 0x35, 0x34, 0x55,
    0x41, 0xd7, 0x55, 0xfa, 0x73, 0x29, 0xba, 0xbb, 0xe2, 0x79, 0x80, 0x13, 0xa7, 0xc6, 0x27, 0x4e,
    0x9c, 0x1a, 0x9f, 0xf8, 0x69, 0x0f, 0x7a, 0xa7, 0x15, 0x8b, 0x52, 0xa5, 0xfe, 0xfc, 0x5a, 0xc9,
    0x2c, 0x55, 0x6a, 0x8d, 0x9a, 0xed, 0xb8, 0xff, 0x3c, 0x71, 0x6d, 0xee, 0xf1, 0x1f, 0x9f, 0x9f,
    0xfc, 0x8e, 0x4f, 0xd7, 0x6a, 0x96, 0xd5, 0xc2, 0x13, 0x82, 0xf9, 0xa5, 0x02, 0xa7, 0xde, 0xbe,
    0xcc, 0xa5, 0x2b, 0xb3, 0xe8, 0x9a, 0xca, 0x62, 0xbe, 0xc8, 0x96, 0x4d, 0xbd, 0xec, 0xdc, 0x3a,
    0x40, 0x22, 0x16, 0x66, 0xd3, 0x70, 0x1f, 0x0f, 0x7f, 0x72, 0x1f, 0x03, 0x7d, 0x69, 0x6e, 0xcc,
    0x2c, 0x61, 0x18, 0x3a, 0x7b, 0xb6, 0x0f, 0x7e, 0xd8, 0x79, 0xfd, 0x1c, 0x5b, 0x9e, 0x01, 0x0c,
    0x60, 0x1e, 0xe0, 0xa9, 0x27, 0x1e, 0x78, 0xe9, 0xf8, 0xa7, 0x0f, 0x7c, 0x71, 0x43, 0x4f, 0x17,
    0x33, 0xf3, 0x2b, 0xd4, 0x1a, 0x4d, 0x2e, 0x4e, 0xce, 0x60, 0xe8, 0x1a, 0x87, 0xf6, 0x6d, 0x66,
    0xff, 0xae, 0x61, 0xaa, 0x35, 0x0b, 0xab, 0xd9, 0x5e, 0xef, 0x94, 0x91, 0x20, 0xb9, 0xee, 0x24,
    0x89, 0x6c, 0x92, 0x4e, 0xb5, 0xc1, 0x8f, 0x4e, 0x5f, 0xe4, 0xf0, 0xbe, 0x2d, 0xcd, 0x3f, 0xff,
    0xab, 0x7f, 0x7b, 0xfa, 0xdb, 0x2f, 0xbd, 0xf1, 0xe6, 0xc7, 0x21, 0x70, 0x37, 0x1e, 0x7b, 0x68,
    0xf7, 0xce, 0xed, 0x9b, 0x37, 0x9c, 0x7f, 0xfc, 0xe8, 0x98, 0x7f, 0x7a, 0x6e, 0x19, 0x21, 0xe0,
    0xca, 0xd4, 0x1c, 0x1d, 0xdb, 0x61, 0xc7, 0xd6, 0x01, 0x82, 0x7e, 0x03, 0xdb, 0x76, 0x68, 0x77,
    0x1c, 0x6c, 0xc7, 0x01, 0x40, 0xd3, 0x54, 0x74, 0x55, 0xc5, 0x30, 0x34, 0x0e, 0xec, 0x1e, 0x26,
    0x9b, 0x49, 0x50, 0x2c, 0x99, 0x2c, 0xaf, 0x95, 0x5f, 0x3b, 0xf2, 0xd9, 0xaf, 0xfd, 0xda, 0xcf,
    0xc2, 0x51, 0x3f, 0x8a, 0x40, 0xa9, 0x52, 0xdf, 0xff, 0x89, 0x83, 0xa3, 0xfe, 0xe5, 0xd5, 0xf5,
    0x83, 0xd0, 0x62, 0xbe, 0x40, 0x28, 0xe8, 0x27, 0x11, 0x0f, 0x63, 0xdb, 0x0e, 0xb7, 0x8a, 0x55,
    0x4a, 0x95, 0x3a, 0x8a, 0x2c, 0xa1, 0xeb, 0x1a, 0x92, 0x24, 0xe1, 0xba, 0x2e, 0x9e, 0x27, 0x48,
    0xc6, 0xc3, 0x9c, 0x3a, 0xdb, 0x22, 0xe0, 0x37, 0xd8, 0xd8, 0x97, 0xa1, 0xd6, 0x68, 0xa6, 0x3e,
    0x6e, 0x0a, 0xee, 0x8d, 0xf8, 0xf7, 0xbe, 0xfd, 0xdc, 0xf7, 0x8b, 0x65, 0xf3, 0x31, 0x59, 0x96,
    0xa5, 0x8e, 0xed, 0xd0, 0x6c, 0xb6, 0x29, 0x94, 0x6b, 0x84, 0x83, 0x3e, 0xb2, 0x99, 0x04, 0xb1,
    0x70, 0x10, 0xf5, 0x03, 0x1b, 0xa6, 0xab, 0x2a, 0x9a, 0xa6, 0xe2, 0x09, 0x8f, 0x52, 0xa5, 0x4e,
    0xa1, 0x64, 0xa2, 0x2a, 0xca, 0x1b, 0xae, 0xe7, 0x4d, 0x7e, 0xe3, 0xc5, 0xd7, 0xfe, 0xf6, 0x8e,
    0x07, 0xfc, 0xd8, 0x0a, 0xfc, 0xc1, 0x73, 0x9f, 0xfd, 0xe4, 0x86, 0x9e, 0xae, 0x4d, 0xef, 0x5c,
    0xba, 0x21, 0x09, 0x01, 0xb2, 0x2c, 0x21, 0xcb, 0x32, 0xba, 0xa6, 0x92, 0x4e, 0xc5, 0xe8, 0x4a,
    0x46, 0x91, 0x00, 0x21, 0xc4, 0xdd, 0x5f, 0x91, 0x24, 0xf0, 0x69, 0x1a, 0xbd, 0xdd, 0x49, 0x7a,
    0xbb, 0x93, 0xe4, 0x32, 0x89, 0x87, 0x6e, 0xdd, 0x5e, 0x1d, 0xff, 0x28, 0xf0, 0x9f, 0xa7, 0x80,
    0xfe, 0xfa, 0xcb, 0xcf, 0xaf, 0xf6, 0x66, 0x93, 0xd1, 0x50, 0xc0, 0x4f, 0xab, 0xd3, 0xc1, 0xb6,
    0x5d, 0x3a, 0xb6, 0x83, 0x10, 0x82, 0xde, 0x6c, 0x8a, 0x50, 0xd0, 0xb7, 0x6e, 0x7f, 0x10, 0x48,
    0x92, 0x84, 0xa2, 0xc8, 0x08, 0x01, 0x86, 0xae, 0xe2, 0x33, 0x74, 0x8c, 0x0f, 0x86, 0x4f, 0xb3,
    0xd9, 0x36, 0x03, 0xc3, 0x4f, 0x1d, 0x05, 0xde, 0xfd, 0xbf, 0x28, 0xe0, 0x58, 0xcd, 0xf6, 0x8d,
    0x86, 0xd5, 0x1e, 0x6b, 0x58, 0xed, 0x0f, 0x91, 0xd4, 0x34, 0x85, 0x56, 0xdb, 0x46, 0xba, 0x67,
    0x25, 0x97, 0x4e, 0x20, 0xcb, 0x12, 0x92, 0x24, 0x21, 0x84, 0xc0, 0xb1, 0x1d, 0x00, 0xb1, 0xb4,
    0x52, 0x2a, 0x02, 0x6b, 0xff, 0x9f, 0x1a, 0x08, 0x7d, 0xf1, 0x73, 0xbf, 0xf8, 0x89, 0x6d, 0xc3,
    0x7d, 0x7b, 0x7d, 0x86, 0xb6, 0x4d, 0xd3, 0xd4, 0x0d, 0xb2, 0x2c, 0x85, 0x75, 0x4d, 0x4d, 0xf7,
    0xf7, 0x74, 0xc5, 0x64, 0x59, 0xd6, 0xee, 0x3c, 0x28, 0x84, 0x70, 0x33, 0xa9, 0x58, 0xd5, 0x76,
    0x9c, 0x82, 0x84, 0x54, 0x6f, 0x75, 0xec, 0x99, 0x72, 0xa5, 0x7e, 0xeb, 0xca, 0x8d, 0xf9, 0xb3,
    0xcf, 0x7d, 0xfd, 0x7b, 0x6f, 0x03, 0x85, 0x8f, 0x02, 0xf9, 0x5f, 0xcf, 0x74, 0xe2, 0xa0, 0x84,
    0xee, 0x83, 0x34, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};
static const unsigned int ICON_NORMAL_size = 2815;

// --- Hover icon ---
static const unsigned char ICON_HOVER[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a,
    0xf4, 0x00, 0x00, 0x0c, 0x78, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x6d, 0x97, 0xe9, 0x8f, 0x9d,
    0xe7, 0x59, 0xc6, 0x7f, 0xcf, 0xbb, 0x9e, 0xe5, 0x3d, 0xfb, 0x3a, 0x67, 0xc6, 0xb3, 0x78, 0xec,
    0x19, 0x8f, 0x3d, 0xb6, 0xe3, 0x3a, 0x71, 0x1c, 0x5b, 0x4d, 0x9d, 0xb4, 0xd0, 0xa8, 0x69, 0xc8,
    0x02, 0x76, 0x52, 0x4a, 0x89, 0xa8, 0x40, 0x11, 0x85, 0x2a, 0xca, 0xa7, 0x1a, 0xa8, 0xda, 0x04,
    0x84, 0xc4, 0x17, 0xe8, 0x07, 0x84, 0x50, 0x04, 0x6a, 0x10, 0x48, 0x14, 0x8d, 0x2b, 0x10, 0x22,
    0x88, 0x24, 0x24, 0x22, 0x83, 0x13, 0xe1, 0x38, 0x4e, 0x26, 0xf6, 0xd8, 0xe3, 0xf1, 0x3e, 0xfb,
    0x72, 0xce, 0x9c, 0x7d, 0x7d, 0xcf, 0xbb, 0x3c, 0x7c, 0x48, 0xfc, 0x85, 0x72, 0xfd, 0x03, 0xf7,
    0xef, 0xbe, 0x75, 0xe9, 0xd6, 0x75, 0x09, 0xbe, 0xd0, 0xcc, 0xe9, 0xd3, 0xea, 0x99, 0x73, 0xe7,
    0x3c, 0xc0, 0x7a, 0xf3, 0x8d, 0x3f, 0x7a, 0x7e, 0x20, 0x9b, 0x78, 0x72, 0xbb, 0xdc, 0x50, 0x02,
    0x86, 0x6e, 0xfb, 0x90, 0xd2, 0x15, 0x35, 0x67, 0x18, 0x6a, 0x37, 0x18, 0x30, 0xc2, 0x91, 0x70,
    0x60, 0x76, 0x6e, 0x71, 0x79, 0xf6, 0xd7, 0x7e, 0xe7, 0xcf, 0x3e, 0x3c, 0xb8, 0x7b, 0xf0, 0xe0,
    0x6b, 0x67, 0x7f, 0xe3, 0x5b, 0x87, 0xa7, 0xc7, 0xf5, 0xe5, 0xb5, 0x62, 0x28, 0x1c, 0x30, 0x0f,
    0xb4, 0x3b, 0xdd, 0x90, 0xe2, 0xfb, 0x76, 0xbd, 0x6d, 0xd7, 0x5c, 0x64, 0x52, 0xfa, 0xb2, 0x67,
    0x85, 0xcc, 0x54, 0xb3, 0xdd, 0xfb, 0xab, 0x68, 0x38, 0x90, 0x4f, 0x44, 0xc2, 0xde, 0xf1, 0xe7,
    0xfe, 0xe0, 0x7b, 0x6f, 0xfe, 0xe4, 0xf7, 0x4c, 0x01, 0x30, 0x33, 0x33, 0xa3, 0x9e, 0x39, 0x73,
    0xc6, 0x3b, 0xfb, 0xdb, 0xcf, 0x4c, 0x3c, 0xf9, 0x8d, 0x87, 0xcf, 0x8e, 0x14, 0x32, 0xcf, 0x85,
    0x83, 0x66, 0x2c, 0x14, 0x36, 0x71, 0x5c, 0x17, 0xd7, 0x83, 0xa0, 0xa9, 0x13, 0x88, 0x04, 0x91,
    0x7d, 0x87, 0x72, 0xa5, 0xe9, 0x6a, 0x8a, 0x52, 0x37, 0xa3, 0xa1, 0xf9, 0x7e, 0xcf, 0x91, 0x9f,
    0x7c, 0xba, 0xb8, 0xdf, 0x97, 0x98, 0xa7, 0x8e, 0x1f, 0xd0, 0x84, 0xae, 0x59, 0x6a, 0x3e, 0x09,
    0xaa, 0x46, 0x77, 0x75, 0x1b, 0xc5, 0xee, 0xf3, 0xaf, 0x6f, 0x5f, 0x70, 0xd3, 0xd9, 0x94, 0x96,
    0x89, 0x5b, 0xb5, 0x7c, 0x3e, 0x2e, 0x7c, 0x45, 0x7f, 0x27, 0x7f, 0xf0, 0xdb, 0x2f, 0x4b, 0x29,
    0xb7, 0x85, 0x94, 0x52, 0x00, 0xbc, 0xf8, 0xc2, 0xe3, 0xbb, 0x7f, 0xeb, 0xb9, 0xc7, 0x5e, 0x9b,
    0xda, 0x3b, 0x7c, 0x26, 0x16, 0x09, 0xeb, 0x9b, 0xa5, 0x0a, 0xad, 0x4e, 0x97, 0xa1, 0x5c, 0x1a,
    0x4d, 0xd3, 0x90, 0xf8, 0xd2, 0x93, 0x88, 0x76, 0xbb, 0xeb, 0xdb, 0x3d, 0x47, 0x71, 0x3d, 0x0f,
    0x21, 0x14, 0x22, 0x91, 0x80, 0xe7, 0xb9, 0x9e, 0xec, 0xd9, 0x8e, 0x96, 0x8e, 0x5b, 0xa8, 0x9a,
    0x4a, 0x57, 0x0a, 0x3f, 0x18, 0x0a, 0x10, 0x52, 0x50, 0xd4, 0x54, 0x8c, 0x52, 0xb5, 0xc9, 0xbd,
    0x4f, 0xae, 0xcb, 0xc1, 0x89, 0x51, 0x31, 0xb8, 0x6f, 0x12, 0xaf, 0x5d, 0x2a, 0x2f, 0x5d, 0xbe,
    0xf7, 0xfa, 0x9e, 0x93, 0x2f, 0xfd, 0xa9, 0x7a, 0xe0, 0xc0, 0x01, 0x75, 0x7a, 0x7a, 0xda, 0xff,
    0xc1, 0x4b, 0xcf, 0x7e, 0x27, 0x9f, 0x49, 0x7e, 0x6f, 0xcf, 0xee, 0xc1, 0x60, 0xa9, 0x5a, 0x77,
    0x6e, 0xdc, 0xdd, 0x50, 0xea, 0xad, 0x1e, 0xb7, 0xee, 0xac, 0xd3, 0x6c, 0x75, 0x70, 0x1c, 0x57,
    0x6c, 0x15, 0xab, 0xcc, 0x5d, 0xbd, 0x2b, 0xee, 0x2c, 0x6d, 0xca, 0x7b, 0x6b, 0xdb, 0xb2, 0x5a,
    0x6b, 0x89, 0xad, 0x52, 0x55, 0xf1, 0x7d, 0xa9, 0x14, 0xb2, 0x49, 0xba, 0x76, 0x5f, 0xfa, 0xbe,
    0x2f, 0x34, 0x1f, 0x51, 0xda, 0x2c, 0x89, 0xbe, 0xeb, 0xca, 0x48, 0x28, 0x28, 0xc2, 0xf9, 0x51,
    0x99, 0xcc, 0x24, 0x45, 0xbf, 0xdd, 0x91, 0x52, 0xfa, 0x32, 0x10, 0x1f, 0x0a, 0x5b, 0x21, 0x63,
    0xff, 0xe9, 0x13, 0x93, 0x35, 0x2d, 0x93, 0xb9, 0x26, 0x00, 0x72, 0xa9, 0xf8, 0x44, 0xad, 0xd5,
    0xb1, 0x8a, 0xdb, 0x65, 0xd9, 0xeb, 0xf4, 0xd4, 0xad, 0x8d, 0x92, 0x48, 0x25, 0xe3, 0x1c, 0x98,
    0x1a, 0xc6, 0x0a, 0x87, 0xa8, 0x94, 0x1b, 0x68, 0x9a, 0xc6, 0xd4, 0xde, 0x5d, 0x6c, 0x15, 0x6b,
    0x62, 0xb3, 0x58, 0x15, 0x95, 0x5a, 0x53, 0x2a, 0x42, 0xb0, 0x55, 0xac, 0x89, 0x6a, 0xbd, 0xcd,
    0x89, 0xa3, 0x93, 0x22, 0x96, 0x8c, 0x01, 0x70, 0x6f, 0xbd, 0x48, 0xab, 0xef, 0x88, 0xed, 0xd5,
    0x6d, 0x6c, 0x73, 0x41, 0x1c, 0xfa, 0xea, 0xa3, 0x24, 0x48, 0x8a, 0x7e, 0xb3, 0x8e, 0x8f, 0xf0,
    0x8c, 0x54, 0x3e, 0x35, 0x7e, 0xec, 0xc0, 0x0b, 0x5a, 0xe9, 0xd4, 0x01, 0x09, 0xd0, 0xea, 0xd8,
    0xeb, 0x85, 0x7c, 0xbc, 0x69, 0xa4, 0x23, 0x91, 0xa0, 0x9a, 0xf6, 0x0f, 0x6b, 0x01, 0x7c, 0xa7,
    0x87, 0xa9, 0x6a, 0x98, 0x86, 0xce, 0xd8, 0xe8, 0x00, 0x9b, 0x5b, 0x65, 0xf6, 0xee, 0x2e, 0xd0,
    0x6c, 0x74, 0x98, 0xbb, 0x76, 0x97, 0x3b, 0x2b, 0x5b, 0x42, 0x4a, 0x89, 0x40, 0x50, 0xa9, 0x35,
    0x99, 0xbf, 0xb1, 0x4c, 0x2a, 0x1e, 0x25, 0x12, 0x0e, 0x00, 0x92, 0x64, 0x30, 0x40, 0x23, 0x9e,
    0x64, 0x6d, 0xe1, 0x36, 0xa5, 0x6b, 0x37, 0x18, 0x39, 0xb8, 0x1b, 0xd3, 0x0a, 0x08, 0x7a, 0x45,
    0x08, 0xc4, 0x7c, 0x27, 0x12, 0x1f, 0xd0, 0x38, 0x07, 0x42, 0xc0, 0xd8, 0x78, 0xba, 0xbc, 0x6f,
    0x72, 0xaa, 0xa7, 0x17, 0x46, 0x23, 0x00, 0x87, 0x76, 0x1d, 0xc2, 0x77, 0xd6, 0x39, 0xff, 0xf3,
    0xf7, 0xa8, 0x76, 0x6c, 0x9e, 0xfa, 0xe5, 0x87, 0x08, 0x06, 0x0c, 0xd6, 0x37, 0xcb, 0x14, 0x06,
    0x52, 0x0c, 0x0f, 0x66, 0x69, 0xb6, 0xbb, 0x14, 0xcb, 0x75, 0x62, 0x56, 0x88, 0x7c, 0x2e, 0x41,
    0xa5, 0xda, 0x62, 0x73, 0xab, 0x4a, 0x26, 0x1d, 0x65, 0x6c, 0x28, 0xc7, 0xc8, 0xbe, 0x51, 0x48,
    0x14, 0xd8, 0x3d, 0x90, 0x60, 0x6b, 0xf1, 0x0e, 0x8d, 0x9d, 0x24, 0xd1, 0xfc, 0x2e, 0xec, 0xb5,
    0xbb, 0x68, 0x51, 0x5b, 0x31, 0x03, 0x61, 0x57, 0x48, 0x29, 0x35, 0x21, 0x84, 0x2b, 0x6b, 0x6f,
    0xfd, 0x39, 0xd6, 0xd8, 0xf7, 0x7d, 0x61, 0xe9, 0x8a, 0xec, 0xfa, 0x2e, 0x7d, 0x45, 0x13, 0x1d,
    0xfc, 0xf5, 0x15, 0xde, 0x3b, 0x3f, 0x4f, 0x3e, 0x13, 0x23, 0x9b, 0x8a, 0xd3, 0x68, 0x75, 0x48,
    0x65, 0xe2, 0xa8, 0x42, 0x61, 0x65, 0x65, 0x8b, 0x52, 0xa5, 0x8e, 0x61, 0xe8, 0x64, 0xd3, 0x71,
    0xe2, 0xd1, 0x30, 0xdd, 0x9e, 0x4d, 0xa5, 0xd6, 0xc4, 0xf7, 0x61, 0xe2, 0xc1, 0x7d, 0x84, 0x1c,
    0x07, 0x55, 0x57, 0xb9, 0x75, 0x63, 0x8d, 0x70, 0x26, 0x49, 0x61, 0xef, 0x24, 0xad, 0xca, 0x26,
    0x4a, 0xbd, 0x81, 0x5e, 0x48, 0xbd, 0xad, 0x01, 0xfe, 0xe9, 0xaf, 0x1d, 0x8d, 0x55, 0x48, 0x3c,
    0x90, 0x54, 0xe3, 0x3a, 0x7e, 0xdb, 0x47, 0xf5, 0x84, 0x8a, 0x8f, 0xf4, 0x25, 0xae, 0xa2, 0x30,
    0x32, 0x98, 0xa6, 0x54, 0x69, 0x60, 0x1a, 0x2a, 0x8a, 0xa2, 0xb1, 0xb4, 0xb4, 0x41, 0x3c, 0x16,
    0xc6, 0x8a, 0x04, 0x49, 0x25, 0x63, 0x20, 0xa0, 0x5c, 0x69, 0x50, 0xa9, 0xb5, 0x50, 0x14, 0xe8,
    0xd9, 0x0e, 0xb3, 0x17, 0xae, 0xe2, 0x3b, 0x0e, 0x0f, 0x1c, 0x9e, 0x44, 0xcd, 0x26, 0xc8, 0x0d,
    0x39, 0xd4, 0x96, 0x37, 0x68, 0xc6, 0x02, 0x32, 0x92, 0x4e, 0xb0, 0xb5, 0xb4, 0x2e, 0x16, 0xe7,
    0x6e, 0xb6, 0x35, 0x21, 0x84, 0xff, 0xee, 0xeb, 0xaf, 0x7c, 0x29, 0xac, 0xa9, 0xc3, 0xf8, 0x36,
    0x48, 0x47, 0x02, 0x8a, 0x40, 0xe2, 0xf6, 0x1c, 0x7a, 0x95, 0x06, 0x63, 0xc3, 0x39, 0x06, 0xf2,
    0x49, 0x3a, 0x9d, 0x1e, 0xb6, 0xed, 0x32, 0x3a, 0x94, 0x63, 0x6e, 0x61, 0x89, 0xf9, 0xeb, 0x4b,
    0xec, 0xdb, 0x3d, 0xc8, 0xde, 0x3d, 0x83, 0xd4, 0x5a, 0x1d, 0xb6, 0x37, 0x77, 0xe8, 0xfb, 0x12,
    0xcf, 0xf3, 0x88, 0x58, 0x21, 0x8c, 0xa0, 0x89, 0x92, 0x8d, 0x82, 0x1a, 0xc2, 0x8a, 0x84, 0x58,
    0xe9, 0xf5, 0x91, 0xd5, 0xb6, 0x88, 0x58, 0x21, 0x6f, 0x79, 0x6d, 0x47, 0x9d, 0x5f, 0x5c, 0x29,
    0x68, 0x00, 0xc9, 0xd1, 0x5d, 0x29, 0x51, 0x5c, 0x0a, 0x48, 0xee, 0xd1, 0x09, 0x46, 0xb0, 0xf2,
    0x83, 0x80, 0xc0, 0x75, 0x3d, 0x76, 0x6a, 0x4d, 0x82, 0x8e, 0x87, 0xa1, 0x6b, 0xd8, 0xb6, 0x43,
    0xa3, 0xd5, 0xc3, 0x0c, 0xe8, 0xec, 0x1d, 0xcd, 0xb3, 0x55, 0x2a, 0x63, 0xf7, 0x7a, 0xe4, 0x76,
    0xe5, 0x19, 0x3f, 0x32, 0x41, 0xf1, 0xfa, 0x3d, 0xb6, 0x2b, 0x2d, 0x72, 0x99, 0x18, 0xd9, 0xe1,
    0x1c, 0x18, 0x06, 0x8d, 0xb5, 0x22, 0x3a, 0x25, 0xa4, 0x2f, 0x99, 0x38, 0x32, 0x81, 0x11, 0x8b,
    0xf8, 0xdb, 0x8b, 0xcb, 0xaa, 0xd0, 0x15, 0xe7, 0x77, 0xbf, 0xf3, 0xc4, 0x25, 0x55, 0x4a, 0x29,
    0x96, 0xe7, 0x67, 0xb3, 0xd5, 0x6a, 0xeb, 0x49, 0xd5, 0xd0, 0x92, 0x6e, 0xb3, 0xe9, 0xab, 0x8a,
    0xa3, 0x68, 0x56, 0x18, 0xcd, 0x08, 0x60, 0xc4, 0x4d, 0x2e, 0x7d, 0x78, 0x85, 0xa5, 0xd5, 0x22,
    0xe3, 0xa3, 0x39, 0x86, 0x77, 0x0f, 0xb1, 0xbc, 0xbc, 0x4d, 0xb5, 0xda, 0xc4, 0xf7, 0x7d, 0x1c,
    0xcf, 0x27, 0x19, 0x0b, 0x11, 0xcb, 0xc4, 0x31, 0x4d, 0x83, 0xa8, 0xae, 0x22, 0x10, 0xcc, 0x5d,
    0xbe, 0xcd, 0xfa, 0xf2, 0x06, 0xab, 0x6b, 0x25, 0x42, 0xa6, 0x4e, 0x7c, 0x38, 0x8f, 0x1e, 0x30,
    0x69, 0xae, 0x6c, 0xc9, 0xdb, 0x4b, 0x1b, 0xc2, 0xb4, 0x82, 0xb2, 0x70, 0x64, 0xdf, 0xa7, 0x9a,
    0x10, 0x42, 0xda, 0xa5, 0xff, 0x78, 0x4e, 0xaa, 0x0c, 0x6b, 0xed, 0x1e, 0xd2, 0x50, 0x55, 0x35,
    0x39, 0x80, 0xf4, 0x4d, 0x84, 0xa2, 0xa0, 0x1a, 0x16, 0x9a, 0xa6, 0xa0, 0x1b, 0x01, 0xac, 0x50,
    0x10, 0x25, 0x17, 0x67, 0xc2, 0x1d, 0xe6, 0x1f, 0x7f, 0xf6, 0x36, 0xb6, 0x27, 0x51, 0x14, 0x85,
    0x0f, 0x3e, 0xb8, 0xc2, 0x53, 0xb9, 0x14, 0x7e, 0xc8, 0x60, 0x79, 0x63, 0x87, 0x83, 0x8f, 0x4c,
    0x23, 0xee, 0x6c, 0x60, 0x77, 0xfb, 0x14, 0xd2, 0x31, 0x92, 0x71, 0x0b, 0xd1, 0x6c, 0x72, 0x63,
    0x69, 0x8b, 0xad, 0x9d, 0x86, 0xcc, 0xa5, 0x22, 0x04, 0x0d, 0x63, 0xa7, 0xb1, 0xba, 0x7e, 0x49,
    0x7b, 0xe3, 0x0f, 0x5f, 0x7c, 0x40, 0x9a, 0xc1, 0x6f, 0x98, 0x91, 0xac, 0x8e, 0xe5, 0xfa, 0x68,
    0xa6, 0x82, 0x08, 0x20, 0xa5, 0x8d, 0xa4, 0x86, 0x5f, 0x2d, 0xa3, 0xa9, 0x2a, 0x07, 0xa7, 0xc6,
    0x10, 0x42, 0xc1, 0xaf, 0xd4, 0xd1, 0x15, 0x68, 0x77, 0xfa, 0xec, 0xd4, 0x9b, 0x08, 0x45, 0xc1,
    0x6e, 0x77, 0x39, 0xb4, 0x5a, 0x64, 0xea, 0xf0, 0x5e, 0x2a, 0xc5, 0x0a, 0xc5, 0xe5, 0x4d, 0x24,
    0x3e, 0x47, 0x0e, 0x8e, 0x13, 0x1a, 0x2b, 0xa0, 0x29, 0x51, 0xa4, 0xaf, 0x33, 0x14, 0x8c, 0x62,
    0xdd, 0xbd, 0x25, 0xb3, 0xa3, 0x23, 0xf4, 0x43, 0xe1, 0xd2, 0x1f, 0xbf, 0xf4, 0xc3, 0x8b, 0xea,
    0x3f, 0xbf, 0xfd, 0x17, 0x7f, 0xa9, 0x87, 0x47, 0x4f, 0x82, 0x27, 0x50, 0x75, 0x10, 0x01, 0x01,
    0x1d, 0xa0, 0x8e, 0xe8, 0xb5, 0xd9, 0xbe, 0xbd, 0xca, 0xd5, 0x9b, 0xab, 0x3c, 0x74, 0x74, 0x12,
    0xa9, 0x09, 0xd4, 0x54, 0x96, 0xf7, 0xdf, 0xbb, 0xc8, 0xc2, 0xf5, 0x25, 0x1c, 0xcf, 0xa7, 0xdd,
    0xb1, 0x71, 0x3c, 0x1f, 0xbf, 0xe7, 0x10, 0x8f, 0x86, 0xd9, 0x7f, 0xe2, 0x10, 0x5e, 0xb9, 0x8e,
    0xeb, 0xf9, 0xa4, 0x12, 0x16, 0x66, 0xd0, 0x80, 0x60, 0x1a, 0xa1, 0xc5, 0x30, 0xc2, 0x16, 0xd1,
    0x54, 0x4a, 0xaa, 0xf1, 0x21, 0xc5, 0xd0, 0x55, 0x67, 0x3a, 0xa7, 0x5f, 0x52, 0x9f, 0x7f, 0xfa,
    0xcb, 0x2f, 0x96, 0x56, 0xb7, 0x06, 0xad, 0x90, 0x67, 0xe8, 0x41, 0x4b, 0xf8, 0x12, 0x84, 0x6c,
    0x20, 0xec, 0x06, 0xdd, 0xcd, 0x2a, 0xb7, 0x6e, 0x6f, 0x20, 0xa5, 0x44, 0x57, 0x15, 0x92, 0xfb,
    0x0f, 0xd2, 0xad, 0x56, 0x59, 0xbd, 0xb3, 0xc2, 0xed, 0xa5, 0x4d, 0x76, 0xaa, 0x4d, 0x82, 0x01,
    0x03, 0xd3, 0x34, 0x58, 0x5b, 0x2b, 0xb1, 0xbe, 0xb1, 0x43, 0x2a, 0x15, 0xa3, 0x10, 0xb7, 0xd0,
    0x54, 0x95, 0x56, 0xbb, 0x47, 0x30, 0x68, 0xa0, 0x68, 0x3e, 0x08, 0x03, 0x29, 0x74, 0x84, 0xae,
    0x0b, 0x29, 0x74, 0x21, 0xa4, 0x08, 0x47, 0x46, 0x52, 0xbe, 0x6a, 0xba, 0x9e, 0xd2, 0xad, 0x95,
    0x1f, 0xdb, 0x5d, 0x48, 0x84, 0xc3, 0xb9, 0xa4, 0x8f, 0x08, 0x09, 0xb7, 0x5d, 0xa7, 0xbf, 0xb5,
    0x4d, 0xa7, 0xd5, 0xa3, 0xd1, 0xe9, 0x01, 0xb0, 0xb6, 0x51, 0x46, 0x37, 0x0d, 0x8c, 0x76, 0x9d,
    0x44, 0x24, 0x08, 0x8a, 0x20, 0x12, 0x0a, 0xd2, 0x6c, 0x75, 0xb9, 0xbb, 0xb4, 0xc5, 0xe1, 0xe9,
    0xdd, 0x4c, 0x4f, 0x0c, 0xf3, 0xd6, 0x9b, 0x1f, 0x12, 0x8d, 0x5a, 0xe4, 0x32, 0x31, 0xda, 0x6d,
    0x1b, 0x2b, 0x97, 0x44, 0x15, 0x1e, 0xa8, 0x02, 0xa1, 0x6a, 0x20, 0x85, 0x10, 0xc2, 0x76, 0x3d,
    0xb7, 0xa9, 0x54, 0x57, 0xb7, 0x3d, 0xe5, 0x46, 0xab, 0xfd, 0xee, 0x9e, 0x89, 0xb1, 0xb2, 0xe6,
    0x09, 0xec, 0xf5, 0x35, 0x68, 0x6d, 0xa3, 0x76, 0x3b, 0xb4, 0x6a, 0x2d, 0xfa, 0x9e, 0xc7, 0x60,
    0x2e, 0xc9, 0x4e, 0xa5, 0xc1, 0x76, 0xb9, 0xce, 0xea, 0x95, 0x79, 0x42, 0x61, 0x13, 0xc3, 0x0c,
    0x30, 0x39, 0x32, 0xc0, 0xaf, 0x3e, 0x75, 0x92, 0x13, 0x0f, 0xed, 0x27, 0x16, 0x0d, 0x31, 0x98,
    0x4f, 0x32, 0x39, 0x5e, 0xa0, 0xe3, 0xb8, 0x5c, 0x98, 0xbb, 0x49, 0xcf, 0x76, 0x10, 0xe2, 0x8b,
    0xb4, 0xe3, 0x29, 0xe0, 0x36, 0x40, 0x76, 0x91, 0x42, 0x00, 0x3d, 0xd5, 0xde, 0x5a, 0xbe, 0x73,
    0xfd, 0xc3, 0xcf, 0xfe, 0x5e, 0xbd, 0xb5, 0xb0, 0xdc, 0xfa, 0xc9, 0x8f, 0xbe, 0xfb, 0x74, 0x38,
    0x19, 0x1b, 0xef, 0x54, 0x9b, 0xc8, 0x76, 0x1d, 0x45, 0x3a, 0xa2, 0xde, 0xb6, 0xe9, 0xdb, 0x36,
    0xc1, 0x80, 0xc9, 0xcd, 0xbb, 0x1b, 0x6c, 0x16, 0xcb, 0x1c, 0x7b, 0x68, 0x3f, 0x99, 0x54, 0x9c,
    0x5e, 0xc7, 0x46, 0xd1, 0x55, 0x32, 0xb9, 0x04, 0xe9, 0xb8, 0x45, 0xcc, 0x0a, 0x22, 0x25, 0xb4,
    0x3b, 0x36, 0x91, 0x48, 0x00, 0xc7, 0x75, 0xf1, 0x3d, 0xd0, 0x75, 0x8d, 0x4c, 0x3a, 0x86, 0xe2,
    0xfa, 0x78, 0x4e, 0x5f, 0x2a, 0x7a, 0x44, 0x4a, 0x55, 0xc5, 0xeb, 0x55, 0xfa, 0xdb, 0xf3, 0xf7,
    0x7e, 0x7e, 0xe8, 0xe9, 0x1f, 0xfc, 0x48, 0x1b, 0x82, 0x60, 0xb3, 0xdd, 0x09, 0x25, 0x86, 0x32,
    0xd8, 0xbd, 0xbe, 0x6c, 0x34, 0xda, 0xc2, 0x6b, 0xf7, 0x68, 0xb6, 0xba, 0x08, 0x01, 0xed, 0x52,
    0x0d, 0xc7, 0x75, 0x41, 0x40, 0x2c, 0x14, 0x04, 0x5d, 0x23, 0x9a, 0xb4, 0x48, 0x98, 0x06, 0xbd,
    0x56, 0x97, 0x9d, 0x52, 0x95, 0x70, 0x28, 0x40, 0x36, 0x14, 0x24, 0x9d, 0xb0, 0x18, 0xdb, 0x95,
    0xa5, 0x5c, 0x6d, 0xb0, 0x78, 0x7b, 0x9d, 0x95, 0xf5, 0x22, 0x02, 0xc9, 0xae, 0x42, 0x16, 0x2b,
    0x1e, 0x10, 0xf8, 0xa0, 0x10, 0xa2, 0xd5, 0xe8, 0x5e, 0xd9, 0xdc, 0xda, 0xfe, 0x27, 0x29, 0xa5,
    0xa6, 0x8c, 0x3e, 0x3c, 0xbd, 0x37, 0x95, 0x88, 0x66, 0x68, 0xf7, 0xa8, 0x94, 0xaa, 0x14, 0x77,
    0x6a, 0xa2, 0x58, 0xac, 0xd1, 0xeb, 0xf7, 0x69, 0x77, 0x6c, 0x36, 0xb6, 0xcb, 0x78, 0xae, 0xcf,
    0xd1, 0x83, 0x7b, 0x88, 0xc6, 0x2c, 0x88, 0x84, 0x51, 0x53, 0x31, 0x5c, 0xdb, 0x86, 0xbe, 0x4b,
    0xb3, 0xdd, 0x67, 0x75, 0x63, 0x87, 0x4a, 0xad, 0x41, 0xbb, 0x6b, 0x13, 0x8b, 0x84, 0x38, 0x7a,
    0x7c, 0x9a, 0x93, 0xc7, 0xf6, 0x53, 0x6f, 0xb5, 0xf9, 0x97, 0x77, 0x3e, 0x62, 0xf6, 0xe2, 0x82,
    0x2c, 0xae, 0x97, 0x5c, 0xdf, 0xee, 0x79, 0xa0, 0x08, 0x4d, 0xf6, 0xae, 0x1f, 0xff, 0xd6, 0x9f,
    0x9c, 0x87, 0x6b, 0x8a, 0xf2, 0xd5, 0x13, 0x87, 0x43, 0xed, 0x5e, 0x3f, 0x2c, 0x75, 0x8d, 0x56,
    0xcf, 0x96, 0x4b, 0x6b, 0x45, 0x56, 0x36, 0x76, 0x58, 0x59, 0x2f, 0xb2, 0x53, 0xad, 0x93, 0x88,
    0x47, 0x18, 0xc8, 0x25, 0x38, 0xb4, 0x6f, 0x0c, 0x73, 0x57, 0x06, 0xda, 0x3d, 0x64, 0xa3, 0x8d,
    0x66, 0x18, 0xec, 0xd4, 0x1a, 0xcc, 0x7e, 0x34, 0x4f, 0xa5, 0xda, 0xe2, 0x91, 0x63, 0xfb, 0xd9,
    0x3b, 0x39, 0x42, 0xcb, 0x76, 0x21, 0x19, 0x65, 0x74, 0x74, 0x80, 0xef, 0x7f, 0xf7, 0x57, 0x08,
    0x99, 0x06, 0x6f, 0xfe, 0xe7, 0x47, 0xde, 0x4f, 0xcf, 0x9d, 0x7f, 0x6b, 0x63, 0xf1, 0xda, 0xac,
    0x57, 0xba, 0xdc, 0xef, 0x97, 0x5a, 0xcd, 0xcf, 0xa3, 0xe0, 0x02, 0xea, 0xec, 0x85, 0x79, 0xe3,
    0xd9, 0x27, 0x8e, 0x7f, 0x2d, 0xa4, 0x2b, 0x43, 0xb7, 0xee, 0x6e, 0x88, 0x4a, 0xb5, 0x49, 0x38,
    0x14, 0x10, 0x5d, 0xdb, 0x61, 0xa7, 0xdc, 0x90, 0x91, 0x70, 0x40, 0x9c, 0x3a, 0x79, 0x88, 0xe8,
    0x50, 0x16, 0xd1, 0xe9, 0x21, 0x3d, 0x1f, 0xa1, 0x69, 0x2c, 0xdf, 0xdd, 0x60, 0xe6, 0xdf, 0x3e,
    0x20, 0x9b, 0x8a, 0xf1, 0xdc, 0xd7, 0x8f, 0x13, 0x8c, 0x59, 0x68, 0xd1, 0x10, 0x41, 0xdf, 0x47,
    0x45, 0x80, 0xe3, 0x52, 0x2e, 0x37, 0x18, 0x1e, 0x4c, 0xf7, 0xd7, 0x37, 0xcb, 0xfa, 0x85, 0xb9,
    0x1b, 0x67, 0x37, 0xed, 0xdc, 0x0f, 0xef, 0x2c, 0x5e, 0xad, 0x48, 0xe9, 0x5d, 0x1f, 0xd9, 0xff,
    0xe8, 0xb5, 0x57, 0x0f, 0x9c, 0x46, 0x03, 0x96, 0xe6, 0xae, 0xdd, 0xaa, 0x46, 0xc2, 0x06, 0xa6,
    0x61, 0x78, 0x3b, 0x95, 0xa6, 0xe2, 0xf9, 0x9e, 0x90, 0xb0, 0x52, 0xaf, 0xb7, 0xd7, 0xa3, 0xb1,
    0xc8, 0x23, 0xda, 0x40, 0x5a, 0xca, 0x46, 0x4b, 0x20, 0x25, 0x02, 0xf8, 0xec, 0xe3, 0x05, 0x3e,
    0xbe, 0x7c, 0x8b, 0x23, 0xd3, 0xe3, 0x0c, 0x17, 0x32, 0x54, 0xab, 0x75, 0x0c, 0x2b, 0x80, 0x91,
    0x8e, 0xa0, 0xe8, 0x2a, 0xeb, 0xb7, 0x57, 0x30, 0x0d, 0x93, 0x40, 0x50, 0x63, 0x34, 0x9a, 0x53,
    0xd2, 0xa9, 0x18, 0x21, 0x2b, 0xd4, 0x7d, 0xed, 0xb5, 0xd7, 0xfc, 0x6f, 0x7e, 0xf3, 0xe8, 0x1b,
    0x32, 0x72, 0xd4, 0x01, 0x10, 0x67, 0xce, 0x78, 0x2a, 0x20, 0xc7, 0x47, 0xb3, 0xa5, 0xb1, 0x5d,
    0x03, 0x27, 0x5d, 0xd7, 0x4f, 0xad, 0x6e, 0x16, 0xcb, 0xad, 0x4e, 0xf7, 0xef, 0xce, 0x5f, 0x5c,
    0x7c, 0x35, 0x1a, 0x09, 0x14, 0x0f, 0x4e, 0x8d, 0x3e, 0x99, 0xcd, 0x25, 0xa5, 0x10, 0x88, 0x7a,
    0xb1, 0xc2, 0xe5, 0xcb, 0xb7, 0x78, 0xef, 0xc3, 0x2b, 0x78, 0xbe, 0xc4, 0xd0, 0x54, 0x06, 0xf2,
    0x09, 0x86, 0xbf, 0xb4, 0x0f, 0x33, 0x9b, 0x41, 0xd1, 0x63, 0x48, 0x2b, 0x47, 0x20, 0x91, 0x27,
    0x18, 0xec, 0x33, 0x77, 0x69, 0x91, 0xc2, 0x40, 0xda, 0xf7, 0x5c, 0x57, 0x31, 0xa4, 0xfa, 0x0f,
    0xff, 0xfe, 0xfe, 0xa5, 0xbb, 0xba, 0x9e, 0xf0, 0x5f, 0x7c, 0xf1, 0xac, 0x7b, 0xbf, 0x8f, 0xa8,
    0x00, 0x17, 0xe7, 0x6e, 0x2f, 0x1d, 0xd8, 0x37, 0xf2, 0x3f, 0x1f, 0x7f, 0x76, 0xe3, 0xbd, 0x4a,
    0xad, 0xf6, 0xd3, 0xbe, 0xe3, 0xce, 0xfc, 0xed, 0xcf, 0xde, 0xad, 0x9c, 0x78, 0x70, 0xf2, 0x37,
    0xa7, 0x27, 0x47, 0x8e, 0x84, 0x85, 0x90, 0xd7, 0xaf, 0x2f, 0x89, 0xf5, 0x8d, 0x32, 0xe7, 0x2f,
    0x2e, 0xb0, 0xb1, 0x55, 0x61, 0x7c, 0x24, 0xcf, 0xc9, 0xe3, 0x07, 0x18, 0x1e, 0xca, 0xe1, 0x76,
    0x6d, 0x3a, 0xd5, 0x06, 0x34, 0x9b, 0x18, 0xba, 0x8f, 0x66, 0x45, 0x50, 0x43, 0x05, 0xc2, 0x99,
    0x04, 0xab, 0x0b, 0x37, 0xfd, 0xa9, 0x63, 0x53, 0x8a, 0x99, 0x8a, 0x5e, 0x2d, 0xec, 0x3a, 0xfa,
    0xc9, 0xcc, 0xcc, 0x8c, 0x07, 0x88, 0xd9, 0xd9, 0x59, 0x09, 0x70, 0xff, 0x55, 0xfc, 0x82, 0x5e,
    0x78, 0xe6, 0xd1, 0x33, 0x5f, 0x7e, 0x70, 0xdf, 0xdf, 0x3c, 0xf5, 0x4b, 0xc7, 0xc2, 0xcb, 0x6b,
    0xdb, 0x4a, 0xa7, 0xeb, 0x88, 0xc5, 0xdb, 0x6b, 0xdc, 0x5d, 0xd9, 0x62, 0xcf, 0xd8, 0x00, 0xd3,
    0x13, 0xc3, 0xec, 0x2a, 0xa4, 0x71, 0x3c, 0x9f, 0xca, 0x4e, 0x9d, 0xbe, 0xd3, 0xc7, 0xd0, 0x35,
    0x22, 0x91, 0x20, 0x5a, 0xc4, 0x42, 0xd5, 0xa3, 0x4c, 0x1c, 0x1c, 0xc1, 0x76, 0xba, 0x8e, 0x69,
    0x85, 0x94, 0xf2, 0xcd, 0xa5, 0xff, 0xbe, 0xba, 0xb0, 0xfc, 0xf2, 0xa9, 0x67, 0xcf, 0xce, 0xcb,
    0xff, 0xfa, 0xb1, 0x26, 0x1e, 0x7b, 0xcd, 0x05, 0xd0, 0xbe, 0x98, 0xf7, 0x45, 0x41, 0x79, 0x5f,
    0x81, 0x53, 0xbe, 0x65, 0x59, 0xe9, 0xa9, 0xf1, 0xc1, 0xa7, 0xbe, 0xfe, 0x95, 0x23, 0xd1, 0x46,
    0xb3, 0xe3, 0x54, 0x1a, 0x1d, 0xb5, 0xd9, 0xec, 0xd0, 0xeb, 0xf5, 0xd9, 0xb7, 0x67, 0x88, 0xe1,
    0xa1, 0x0c, 0xcd, 0x76, 0x97, 0x77, 0x66, 0xe7, 0x68, 0xb5, 0x7b, 0x68, 0x86, 0x86, 0xaa, 0x08,
    0x7c, 0x09, 0x9e, 0xe7, 0xa3, 0x01, 0xd9, 0x54, 0x04, 0xa7, 0xbc, 0x41, 0x66, 0xcf, 0xb0, 0x9e,
    0xed, 0xda, 0x28, 0x7d, 0xf7, 0xc4, 0x48, 0x2e, 0xb6, 0x07, 0x98, 0x27, 0x73, 0x40, 0xb9, 0xbf,
    0xe8, 0x7d, 0x00, 0x29, 0x84, 0x90, 0xa7, 0x4f, 0x9f, 0x16, 0x33, 0x33, 0xa7, 0xd0, 0x34, 0xcd,
    0x49, 0x44, 0xc2, 0x57, 0x7a, 0x7d, 0xa7, 0xb8, 0xb4, 0x56, 0x4c, 0x6b, 0xaa, 0xa0, 0x67, 0xdb,
    0xc4, 0xa3, 0x21, 0x50, 0x04, 0xab, 0xeb, 0x25, 0x84, 0x10, 0x28, 0x9a, 0x4a, 0x22, 0x11, 0x41,
    0x37, 0x35, 0x74, 0x55, 0x45, 0x55, 0x54, 0xa4, 0x94, 0xb8, 0xbe, 0x87, 0x8f, 0xc2, 0x85, 0xab,
    0xf7, 0x90, 0x9f, 0xde, 0x5a, 0xdc, 0x3f, 0x39, 0xdc, 0xf2, 0xa4, 0xbc, 0xe2, 0x2b, 0xde, 0xa2,
    0x10, 0xf0, 0xfe, 0xb9, 0x6b, 0xfe, 0xff, 0x05, 0x00, 0xe0, 0xdc, 0xb9, 0x73, 0x3e, 0xbc, 0x2a,
    0xea, 0xf5, 0x7a, 0x35, 0x93, 0x8f, 0x7e, 0xba, 0x70, 0x73, 0xad, 0x79, 0x79, 0xe1, 0x5e, 0x36,
    0x10, 0x30, 0x24, 0x48, 0xe1, 0xf6, 0x5d, 0x76, 0xaa, 0x4d, 0x62, 0xd1, 0x10, 0x13, 0x63, 0x05,
    0x06, 0x32, 0x09, 0x0c, 0x43, 0xc7, 0xc7, 0x47, 0x41, 0x10, 0x0c, 0x98, 0x04, 0x4c, 0x03, 0xe9,
    0xfb, 0x54, 0xea, 0x4d, 0x2a, 0x75, 0x48, 0x0e, 0xa6, 0xaa, 0x4b, 0x2b, 0xab, 0x6f, 0x7d, 0xfb,
    0x95, 0xbf, 0x7e, 0x1d, 0xd8, 0x96, 0x52, 0x0a, 0x21, 0x84, 0xfb, 0xff, 0x02, 0xcc, 0xcc, 0xcc,
    0x28, 0x42, 0x9c, 0xf1, 0x7e, 0xfc, 0xf2, 0xaf, 0x3f, 0x30, 0xbd, 0x77, 0xe4, 0xf7, 0x55, 0x45,
    0xcd, 0xaf, 0x6d, 0x94, 0x7c, 0xdd, 0x50, 0xf1, 0x7d, 0x50, 0x14, 0x45, 0x0c, 0xe4, 0x53, 0x0c,
    0x17, 0xd2, 0xe4, 0x87, 0xb2, 0x68, 0xaa, 0x8a, 0xeb, 0x38, 0x48, 0x24, 0x42, 0x51, 0xd0, 0x75,
    0x05, 0x5d, 0xd5, 0xd0, 0x0d, 0x4d, 0xa6, 0x0a, 0x59, 0x84, 0xa2, 0xca, 0x5c, 0x3a, 0xf1, 0x70,
    0xcb, 0x96, 0xdd, 0x77, 0x26, 0xf7, 0xbc, 0x53, 0x6b, 0x5d, 0xd8, 0x01, 0xe4, 0xfd, 0xab, 0xff,
    0x02, 0xc0, 0x69, 0x6b, 0x53, 0x03, 0xbc, 0xaf, 0x1c, 0x9f, 0x7a, 0x7c, 0xb4, 0x90, 0x7d, 0x26,
    0x18, 0x34, 0x98, 0x7a, 0xe5, 0x05, 0xe8, 0x76, 0x71, 0xfb, 0x1e, 0x76, 0xdf, 0xc1, 0xb6, 0x1d,
    0x92, 0x03, 0x69, 0x48, 0x5a, 0x9f, 0x7b, 0xd8, 0x97, 0x9f, 0x5b, 0x59, 0x55, 0x01, 0xfd, 0x0b,
    0x5f, 0x6b, 0x02, 0x02, 0x80, 0x21, 0xc0, 0x27, 0x41, 0xf4, 0xf1, 0xfa, 0xc7, 0x73, 0x4f, 0x9c,
    0x79, 0xfe, 0xdc, 0xc7, 0xa6, 0x61, 0xf8, 0x80, 0x72, 0x1f, 0xe0, 0x7f, 0x01, 0x92, 0x48, 0xf1,
    0x72, 0xe0, 0x21, 0xaa, 0x32, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
    0x82,
};
static const unsigned int ICON_HOVER_size = 3249;

// Global variables
HMODULE hSelf;
AddonDefinition_t AddonDef{};
AddonAPI_t* APIDefs = nullptr;
bool g_WindowVisible = false;

// UI State
static int g_SelectedCharIdx = -1;
static int g_SelectedTab = 0;          // 0 = Equipment, 1 = Build
static bool RenderChipStrip(const std::vector<std::string>& labels,
                            const std::vector<bool>& activeMarkers,
                            int& selectedIdx,
                            float& scrollOffset,
                            const char* idScope);

static int g_SelectedEquipTab = 0;     // Equipment tab filter
static float g_EquipTabScroll = 0.0f;  // Horizontal scroll offset for equipment tab chip strip
static float g_BuildTabScroll = 0.0f;  // Horizontal scroll offset for build tab chip strip
static int g_SelectedBuildTab = 0;     // Build tab filter
static bool g_ShowQAIcon = true;
static bool g_CompactCharList = false;
static bool g_ShowCraftingIcons = false;
static bool g_ShowAge = false;
static bool g_ShowPlaytime = false;
static bool g_ShowLastLogin = false;
static int g_BirthdayMode = 1;         // 0 = Always, 1 = A week out, 2 = Never
static float g_CharListWidth = 200.0f; // Resizable character list column width
static bool g_DetailsFetched = false;  // Track if we've requested details for current character
static int g_MainTab = 0;             // 0 = Characters, 1 = Build Library, 2 = Skinventory
static std::chrono::steady_clock::time_point g_FetchDoneTime{}; // For status message fade-out

// Skinventory UI state
static int g_SkinActiveTab = 0; // 0 = Browser, 1 = Shopping List
static bool g_SkinSwitchToBrowser = false;
static bool g_SkinScrollToSkin = false;
static std::string g_SkinSelectedType = "Armor";
static std::string g_SkinSelectedWeightClass = "Heavy";
static std::string g_SkinSelectedSubtype = "Helm";
static uint32_t g_SkinSelectedId = 0;
static char g_SkinSearchFilter[256] = "";
static bool g_SkinShowOwned = true;
static bool g_SkinShowUnowned = true;
static int  g_SkinViewMode = 0; // 0 = List, 1 = Grid
static bool g_SkinRefreshOwned = false;
static bool g_SkinInitialized = false;
static bool g_SwitchToSkinventory = false; // set by achievements tab to navigate to Skinventory

// Portrait cache (for character screenshots in equipment panel)
struct PortraitEntry {
    std::string path;
    int64_t modTime; // file last-write time for cache busting
};
static std::unordered_map<std::string, PortraitEntry> s_portraitPathCache;
static std::unordered_set<std::string> s_portraitMissing;

static int64_t GetFileModTime(const std::string& path) {
    try {
        auto ftime = std::filesystem::last_write_time(std::filesystem::u8path(path));
        return ftime.time_since_epoch().count();
    } catch (...) { return 0; }
}

// Shopping list state
struct SkinShopEntry {
    uint32_t skinId;
    int price;      // in copper
    int source;     // 0 = TP, 1 = Vendor
};
static std::vector<SkinShopEntry> g_SkinShopList;
static bool g_SkinShopListDirty = true;

// =========================================================================
// Achievement Tracker
// =========================================================================
#define EV_AE_ACH_PROGRESS_RESPONSE "EV_ALTER_EGO_ACH_PROGRESS_RESP"
static void OnAchProgressResponse(void* eventArgs);

struct AchGroupDef {
    std::string id;            // UUID string
    std::string name;
    std::string description;
    int order = 0;
    std::vector<uint32_t> categories; // category IDs in this group
};

struct AchCategoryDef {
    uint32_t id = 0;
    std::string name;
    std::string description;
    int order = 0;
    std::string icon_url;
    std::vector<uint32_t> achievements; // achievement IDs in this category
};

struct AchBitDef {
    std::string type;    // "Text", "Item", "Minipet", "Skin"
    uint32_t item_id = 0; // optional: item/skin/mini ID
    std::string text;    // display text or hint
};

struct AchDef {
    uint32_t id = 0;
    std::string name;
    std::string description;
    std::string requirement;
    std::string icon_url;
    std::string type;                    // "Default" or "ItemSet"
    std::vector<std::string> flags;      // "Repeatable", "Hidden", "Daily", etc.
    int max_count = 0;                   // from tiers.back().count
    int total_ap = 0;                    // sum of all tier points
    std::vector<AchBitDef> bits;
    std::vector<uint32_t> prerequisites;
};

struct AchProgress {
    uint32_t id = 0;
    int current = 0;
    int max = 0;
    bool done = false;
    int repeated = 0;
    bool unlocked = true;                // absent in API = unlocked
    std::set<uint32_t> completed_bits;   // indices of completed bits
};

// Achievement tracker state
static std::vector<AchGroupDef> g_AchGroups;
static std::unordered_map<uint32_t, AchCategoryDef> g_AchCategories;
static std::unordered_map<uint32_t, AchDef> g_AchDefs;           // cached definitions
static std::unordered_map<uint32_t, AchProgress> g_AchProgress;  // account progress
static std::vector<uint32_t> g_AchPinned;                        // pinned achievement IDs
static std::unordered_map<uint32_t, std::string> g_AchNameIndex; // id -> name (for search)
static std::unordered_map<uint32_t, uint32_t> g_AchIdToCategory; // ach id -> category id
static std::unordered_set<uint32_t> g_AchHiddenCatIds;          // categories to hide (e.g. Character Adventure Guide)

static std::recursive_mutex g_AchMutex;
static bool g_AchGroupsFetched = false;
static std::atomic<bool> g_AchGroupsFetching{false};
static bool g_AchNameIndexReady = false;
static std::atomic<bool> g_AchNameIndexFetching{false};
static std::string g_AchStatusMsg;

#define ACH_NAME_INDEX_URL "https://raw.githubusercontent.com/PieOrCake/alter_ego/main/data/achievement_names.json"
#define ACH_WAYPOINTS_URL "https://raw.githubusercontent.com/PieOrCake/alter_ego/main/data/achievement_waypoints.json"
#define HERO_CHALLENGES_URL "https://raw.githubusercontent.com/PieOrCake/alter_ego/main/data/hero_challenges.json"

// Hero challenge data: id -> { map_name, expansion }
struct HeroChallengeInfo {
    std::string map_name;
    std::string expansion;
};
static std::unordered_map<std::string, HeroChallengeInfo> g_HeroChallenges; // id -> info
static bool g_HeroChallengesReady = false;
static std::atomic<bool> g_HeroChallengesFetching{false};

// Waypoint data: achId -> (bitIndex -> chatCode), bitIndex -1 = achievement-level waypoint
static std::unordered_map<uint32_t, std::unordered_map<int, std::string>> g_AchWaypoints;
static bool g_AchWaypointsReady = false;
static std::atomic<bool> g_AchWaypointsFetching{false};

// UI state
static std::string g_AchSelectedGroupId;   // currently selected group UUID
static uint32_t g_AchSelectedCatId = 0;    // currently selected category ID
static std::atomic<bool> g_AchCatFetching{false};      // fetching defs for selected category
static std::atomic<bool> g_AchProgressFetching{false}; // fetching account progress
static bool g_AchNeedRequery = false;      // deferred requery on account change
static bool g_AchProgressDirty = false;    // persistent cache needs flushing
static std::string g_AchCachedAccount;     // account whose data is currently in g_AchProgress
static std::chrono::steady_clock::time_point g_AchLastSave;
static void SaveAchProgress(const std::string& account);
static void LoadAchProgress(const std::string& account);
static std::atomic<uint32_t> g_AchCatProgressPending{0}; // catId needing progress query (set by bg thread)
static std::atomic<bool> g_AchRetryPending{false};       // PENDING retry for ach progress
static uint64_t g_AchProgressGen = 0;      // incremented when progress updates, triggers popout cache rebuild
static char g_AchSearchBuf[128] = "";
static bool g_AchPopoutVisible = false;    // popout tracker window visibility
static bool g_AchHideEmptyCats = false;    // rail filter: hide categories with no progress data
static bool g_AchHideCompletedCats = false; // rail filter: hide categories where all achievements are done
static std::unordered_map<uint32_t, bool> g_AchExpandedInPopout; // expanded state per pinned ach
static bool g_AchShowCompletedSteps = false; // popout: show completed bits/steps
static std::unordered_set<uint32_t> g_AchExpandedInList; // expanded state per achievement in main list
static float g_AchTreeWidth = 220.0f;      // resizable left tree panel width
static float g_AchPinnedHeight = 200.0f;   // resizable pinned section height
static float g_AchTreeScrollY = 0.0f;      // saved scroll position of left tree panel
static float g_AchListScrollY = 0.0f;      // saved scroll position of right list panel
static bool g_AchRestoreScroll = false;     // flag to restore scroll positions on next frame
static uint32_t g_AchNavigateToId = 0;     // pending navigation: jump to this achievement ID
static bool g_AchPinnedBootQueried = false; // flag: have we queried pinned progress after H&S became available?

// Active special event detection (from /v2/achievements/daily "special" array)
static std::vector<uint32_t> g_AchActiveMainCatIds;   // main/Historical categories for top of tree
static std::vector<uint32_t> g_AchActiveDailyCatIds;   // daily categories for top of Daily group
static std::string g_AchActiveEventName;                // display name for the active event node
static bool g_AchActiveEventFetched = false;
static std::atomic<bool> g_AchActiveEventFetching{false};

// Festival daily category IDs (hidden from Daily group when their festival is not active)
static const std::unordered_set<uint32_t> FESTIVAL_DAILY_CAT_IDS = {
    79,  // Halloween Daily
    98,  // Wintersday Daily
    102, // Lunar New Year Dailies
    162, // Daily Super Adventure Festival
    201, // Daily Lunar New Year
    213, // Daily Festival of the Four Winds
    233, // Daily Dragon Bash
};

// Festival daily category -> display name + keywords to match Historical categories
struct FestivalMapping {
    std::string displayName;
    std::vector<std::string> keywords;  // match Historical category names
};
static const std::unordered_map<uint32_t, FestivalMapping> FESTIVAL_MAPPINGS = {
    {79,  {"Shadow of the Mad King", {"Mad King", "Halloween", "Lunatic"}}},
    {98,  {"Wintersday",             {"Wintersday", "Toymaker Tixx", "Winter's Presence"}}},
    {102, {"Lunar New Year",         {"Lunar New Year"}}},
    {162, {"Super Adventure Festival",{"Super Adventure Box"}}},
    {201, {"Lunar New Year",         {"Lunar New Year"}}},
    {213, {"Festival of the Four Winds", {"Four Winds", "Queen's Gauntlet", "Bazaar"}}},
    {233, {"Dragon Bash",            {"Dragon Bash"}}},
};

// Categories from Bonus Events group that are always junk (never surfaced)
static bool IsBonusEventJunk(const std::string& name) {
    return name.find("Marshaling") != std::string::npos ||
           name.find("Mobilization") != std::string::npos ||
           name.find("Black Lion") != std::string::npos ||
           name.find("Wizard's Vault") != std::string::npos ||
           name.find("Adventure Guide") != std::string::npos;
}

// Optimistic update grace period — protects alert-based increments from stale API snapback
static std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> g_AchOptimisticTime;

// Persistence & caching
static std::chrono::steady_clock::time_point g_LastAchProgressQuery{};
static void SaveAchTrackerState();
static void LoadAchTrackerState();
static void SaveAchGroupCache();
static void LoadAchGroupCache();
static void SaveAchDefCache();
static void LoadAchDefCache();
static void FetchAchGroups();
static void FetchAchCategoryDefs(uint32_t catId);
static void SendAchProgressQuery(const std::vector<uint32_t>& ids);
static void FetchPinnedAchDefs();
static void SaveAchWaypoints();
static void LoadAchWaypoints();
static void FetchAchWaypoints();
static void FetchActiveSpecialEvent();
static void SaveHeroChallenges();
static void LoadHeroChallenges();
static void FetchHeroChallenges();

// Character search
static char g_CharSearchBuf[128] = "";

// MumbleLink identity for current character indicator
struct MumbleIdentity {
    char Name[20];
    unsigned Profession;
    unsigned Specialization;
    unsigned Race;
    unsigned MapID;
    unsigned WorldID;
    unsigned TeamColorID;
    bool IsCommander;
    float FOV;
    unsigned UISize;
};
static std::string g_CurrentCharName; // currently logged-in character name from MumbleLink

// Local login timestamps (character name -> unix epoch)
static std::unordered_map<std::string, int64_t> g_LoginTimestamps;
static bool g_LoginTimestampsDirty = false;
static void SaveLoginTimestamps();
static void LoadLoginTimestamps();

// Build Library UI state
static float g_LibListWidth = 220.0f; // Resizable build list column width
static int g_LibSelectedIdx = -1;
static int g_LibFilterMode = 0;        // 0=All, 1=PvE, 2=WvW, 3=PvP, 4=Raid, 5=Fractal
static char g_LibSearchBuf[128] = "";
static char g_LibImportBuf[4096] = "";
static char g_LibImportName[128] = "";
// Auto-retry pending import once the profession palette finishes loading
static bool g_LibImportPending = false;
static std::string g_LibImportPendingProfession;
// Right-pane edit mode: by default the Name/Mode/Notes inputs are hidden
// behind an Edit button to keep the preview clean.
static bool g_LibEditMode = false;
static int g_LibImportMode = 0;        // GameMode for import
static bool g_LibShowImport = false;
static std::string g_LibImportError;
static std::string g_LibImportFilePath; // path picked from file dialog for library file import
static bool g_LibDetailsFetched = false;
static int g_LibDragIdx = -1;          // drag-and-drop source index
// Inline edit buffers — std::string + ImGui resize callback so long names
// or notes are never silently truncated by a fixed buffer size.
static std::string g_LibEditName;
static std::string g_LibEditNotes;

static int ImGuiInputStringResizeCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto* str = static_cast<std::string*>(data->UserData);
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    }
    return 0;
}

static bool InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputText(label, str.data(), str.capacity() + 1,
        flags, ImGuiInputStringResizeCallback, &str);
}

static bool InputTextMultilineString(const char* label, std::string& str, const ImVec2& size,
                                     ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputTextMultiline(label, str.data(), str.capacity() + 1,
        size, flags, ImGuiInputStringResizeCallback, &str);
}
static std::string g_LibEditBuildId;   // which build is being edited
static int g_LibCtxDeleteIdx = -1;     // deferred delete from context menu
static bool g_RelaySending = false;    // relay POST in progress
static std::string g_RelayResultCode;  // 4-char code on success
static std::string g_RelayResultError; // error message on failure

// =========================================================================
// Events: Chat integration — detect build links in game chat
// Struct definitions from jsantorek/GW2-Chat (used by "Events: Chat" addon)
// =========================================================================
#define EV_CHAT_MESSAGE "EV_CHAT:Message"

typedef char* ChatStringUTF8;

typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} ChatGUID;

typedef struct {
    uint32_t Low;
    uint32_t High;
} ChatTimestamp;

enum ChatMessageType {
    ChatMsg_Error = 0,
    ChatMsg_Guild,
    ChatMsg_GuildMotD,
    ChatMsg_Local,
    ChatMsg_Map,
    ChatMsg_Party,
    ChatMsg_Squad,
    ChatMsg_SquadMessage,
    ChatMsg_SquadBroadcast,
    ChatMsg_TeamPvP,
    ChatMsg_TeamWvW,
    ChatMsg_Whisper,
    ChatMsg_Emote,
    ChatMsg_EmoteCustom
};

enum ChatMetadataFlags {
    ChatFlag_None = 0,
    ChatFlag_Whisper_IsFromMe = 1 << 4,
};

typedef struct {
    ChatGUID       Account;
    ChatStringUTF8 CharacterName;
    ChatStringUTF8 AccountName;
    ChatStringUTF8 Content;
} ChatGenericMessage;

typedef struct {
    ChatTimestamp       DateTime;
    ChatMessageType     Type;
    ChatMetadataFlags   Flags;
    union {
        ChatGenericMessage Whisper;
        ChatGenericMessage Local;
        ChatGenericMessage Map;
        ChatGenericMessage Party;
        ChatGenericMessage Squad;
        ChatStringUTF8     SquadMessage;
        ChatGenericMessage TeamPvP;
        ChatGenericMessage ErrorGeneric;
    };
} EvChatMessage;

// =========================================================================
// Events: Alerts integration — real-time skin unlock detection
// Struct definitions from jsantorek/GW2-Alerts (used by "Events: Alerts" addon)
// =========================================================================
#define EV_ALERT_UNLOCKED_SKIN "EV_ALERT:UnlockedSkin"

struct AlertUnlockPayload {
    unsigned int ID;
};

static void OnEvAlertSkinUnlocked(void* eventArgs) {
    if (!eventArgs) return;
    auto* payload = (AlertUnlockPayload*)eventArgs;
    if (payload->ID != 0) {
        AlterEgo::GW2API::MarkSkinUnlocked(payload->ID);
        Skinventory::OwnedSkins::MarkOwned(payload->ID);
        g_SkinShopListDirty = true;
    }
}

#define EV_ALERT_ACHIEVEMENT_COMPLETED "EV_ALERT:AchievementCompleted"

// When H&S becomes available, signal render thread to query progress + clears.
// This handler may run on H&S's thread — never call Events_Raise here.
static void OnHoardPongForAch(void* eventArgs) {
    if (!eventArgs) return;
    auto* pong = (HoardPongPayload*)eventArgs;
    if (pong->api_version < 2) return;

    // Signal render thread to query pinned achievement progress (once per session)
    if (!g_AchPinnedBootQueried) {
        g_AchPinnedBootQueried = true;
        g_AchRetryPending = true;  // render thread will pick up pinned + selected cat
    }

    // Signal render thread to query clears completion (once per session)
    static bool s_clearsBootQueried = false;
    if (!s_clearsBootQueried && g_ClearsFetched && !g_ClearsFetching) {
        s_clearsBootQueried = true;
        g_ClearsQueryPending = true;
    }
}

static void OnEvAlertAchievementCompleted(void* eventArgs) {
    if (!eventArgs) return;
    auto* payload = (AlertUnlockPayload*)eventArgs;
    if (payload->ID != 0) {
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            auto it = g_AchProgress.find(payload->ID);
            if (it != g_AchProgress.end()) {
                // Increment progress — handle completion based on achievement type
                it->second.current++;
                if (it->second.max > 0 && it->second.current >= it->second.max) {
                    // Check if this is a repeatable achievement
                    auto defIt = g_AchDefs.find(payload->ID);
                    bool isRepeatable = false;
                    bool isDailyOrWeekly = false;
                    if (defIt != g_AchDefs.end()) {
                        for (const auto& f : defIt->second.flags) {
                            if (f == "Repeatable") isRepeatable = true;
                            if (f == "Daily" || f == "Weekly") isDailyOrWeekly = true;
                        }
                    }
                    if (isRepeatable && !isDailyOrWeekly) {
                        // Infinite repeatable — reset to 0, bump repeat count
                        it->second.current = 0;
                        it->second.repeated++;
                        it->second.done = false;
                        it->second.completed_bits.clear();
                    } else {
                        // Non-repeatable or daily/weekly — mark done
                        it->second.done = true;
                    }
                }
            } else {
                // No existing progress — create with current=1, don't assume done
                AchProgress p;
                p.id = payload->ID;
                p.current = 1;
                g_AchProgress[payload->ID] = std::move(p);
            }
            g_AchProgressGen++;
            g_AchProgressDirty = true;
            g_AchOptimisticTime[payload->ID] = std::chrono::steady_clock::now();
        }
    }
}

// Build toast notification state
struct BuildToast {
    bool active = false;
    std::string sender;         // Character name of who sent it
    std::string chat_link;      // The [&...] build template link
    std::string ae2_code;       // Full AE2: code if detected (empty = plain chat link)
    std::string profession;     // Decoded profession name
    std::string spec_name;      // Elite spec if available
    std::string channel;        // "Party", "Squad", "Whisper", etc.
    bool has_gear = false;      // True if AE2 with gear data
};
static BuildToast g_BuildToast;
static std::mutex g_BuildToastMutex;
static bool g_ChatBuildDetection = true;  // master toggle
static float g_ToastPosX = -1.0f;        // -1 = auto-center
static float g_ToastPosY = 100.0f;
static bool g_ToastPosInitialized = false;
static bool g_ToastNeedsFocus = false;
static bool g_ChatAddonConnected = false; // true once we receive any EV_CHAT:Message

static void OnEvChatMessage(void* eventArgs);
static void PushGW2Theme();
static void PopGW2Theme();
struct ThemeGuard {
    ThemeGuard()  { PushGW2Theme(); }
    ~ThemeGuard() { PopGW2Theme(); }
};

// Character list sorting
enum CharSortMode { Sort_Custom = 0, Sort_Name, Sort_Profession, Sort_Level, Sort_Age, Sort_Birthday };
static int g_CharSortMode = Sort_Custom;
static bool g_CharSortAscending = true;            // true = default direction, false = reversed
static std::vector<int> g_CharDisplayOrder;        // indices into GetCharacters()
static std::vector<std::string> g_CustomCharOrder;  // saved custom name order
static int g_CharDragIdx = -1;                      // drag-and-drop source index in display order
static size_t g_LastCharCount = 0;                  // detect character list changes

static void SaveCharSortConfig();
static void LoadCharSortConfig();
static int DaysUntilBirthday(const std::string& created);

static void RebuildCharDisplayOrder() {
    const auto& chars = AlterEgo::GW2API::GetCharacters();
    int n = (int)chars.size();
    g_CharDisplayOrder.resize(n);
    for (int i = 0; i < n; i++) g_CharDisplayOrder[i] = i;

    bool asc = g_CharSortAscending;
    switch (g_CharSortMode) {
        case Sort_Name:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    return asc ? chars[a].name < chars[b].name
                               : chars[a].name > chars[b].name;
                });
            break;
        case Sort_Profession:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    if (chars[a].profession != chars[b].profession)
                        return asc ? chars[a].profession < chars[b].profession
                                   : chars[a].profession > chars[b].profession;
                    return chars[a].name < chars[b].name;
                });
            break;
        case Sort_Level:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    if (chars[a].level != chars[b].level)
                        return asc ? chars[a].level > chars[b].level   // asc = highest first
                                   : chars[a].level < chars[b].level;
                    return chars[a].name < chars[b].name;
                });
            break;
        case Sort_Age:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    return asc ? chars[a].created < chars[b].created   // asc = oldest first
                               : chars[a].created > chars[b].created;
                });
            break;
        case Sort_Birthday:
            std::sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
                [&](int a, int b) {
                    int da = DaysUntilBirthday(chars[a].created);
                    int db = DaysUntilBirthday(chars[b].created);
                    if (da < 0) da = 999; // unparseable dates go to end
                    if (db < 0) db = 999;
                    if (da != db)
                        return asc ? da < db : da > db;  // asc = soonest first
                    return chars[a].name < chars[b].name;
                });
            break;
        case Sort_Custom:
        default: {
            // Order by g_CustomCharOrder, unknowns appended at end
            std::unordered_map<std::string, int> nameToIdx;
            for (int i = 0; i < n; i++) nameToIdx[chars[i].name] = i;

            g_CharDisplayOrder.clear();
            // First: characters in custom order
            for (const auto& name : g_CustomCharOrder) {
                auto it = nameToIdx.find(name);
                if (it != nameToIdx.end()) {
                    g_CharDisplayOrder.push_back(it->second);
                    nameToIdx.erase(it);
                }
            }
            // Then: any new characters not yet in custom order
            for (int i = 0; i < n; i++) {
                if (nameToIdx.find(chars[i].name) != nameToIdx.end()) {
                    g_CharDisplayOrder.push_back(i);
                }
            }
            break;
        }
    }
    // Multi-account: group by account (current account first), preserve sort within each group
    if (AlterEgo::GW2API::IsMultiAccount()) {
        const std::string& currentAcct = AlterEgo::GW2API::GetCurrentAccountName();
        // Build ordered account list: current account first, then others in encounter order
        std::vector<std::string> acctOrder;
        if (!currentAcct.empty()) acctOrder.push_back(currentAcct);
        for (const auto& acct : AlterEgo::GW2API::GetAccounts()) {
            if (acct.name != currentAcct)
                acctOrder.push_back(acct.name);
        }
        // Stable sort by account group — preserves the per-sort-key ordering within each group
        std::unordered_map<std::string, int> acctRank;
        for (int i = 0; i < (int)acctOrder.size(); i++) acctRank[acctOrder[i]] = i;
        std::stable_sort(g_CharDisplayOrder.begin(), g_CharDisplayOrder.end(),
            [&](int a, int b) {
                const std::string& aa = chars[a].account_name;
                const std::string& ba = chars[b].account_name;
                if (aa == ba) return false; // preserve existing sort within same account
                int ra = acctRank.count(aa) ? acctRank[aa] : 999;
                int rb = acctRank.count(ba) ? acctRank[ba] : 999;
                return ra < rb;
            });
    }
    g_LastCharCount = chars.size();
}

// Character refresh selection popup state
static bool g_RefreshPopupOpen = false;
static bool g_RefreshListFetching = false;     // waiting for char list from H&S
static bool g_AutoCharListRequested = false;   // auto-fetched char list on H&S connect
static std::vector<std::string> g_RefreshNames; // all character names from API
static std::vector<bool> g_RefreshSelection;    // parallel checkbox state

// Multi-account selector state
static std::string g_SelectedAccountFilter;    // empty = "All Accounts", else specific account_name

// Returns the account name to use for per-account queries (dropdown selection only)
std::string GetEffectiveAccountName() {
    return g_SelectedAccountFilter;
}

bool WriteFileAtomic(const std::string& finalPath, const std::string& content) {
    std::string tmp = finalPath + ".tmp";
    try {
        {
            std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
            if (!f.is_open()) return false;
            f.write(content.data(), (std::streamsize)content.size());
            f.flush();
            if (!f.good()) return false;
        }
        // MoveFileExA with MOVEFILE_REPLACE_EXISTING is atomic on NTFS for
        // same-volume renames (the temp file lives next to the final path).
        if (!MoveFileExA(tmp.c_str(), finalPath.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            std::error_code ec;
            std::filesystem::remove(tmp, ec);
            return false;
        }
        return true;
    } catch (...) {
        std::error_code ec;
        std::filesystem::remove(tmp, ec);
        return false;
    }
}

// Gear customization dialog state
static bool g_GearDialogOpen = false;
static std::string g_GearDialogSlot;          // Which slot is being edited
static std::string g_GearDialogBuildId;       // Which saved build
static char g_GearStatSearch[128] = "";       // Search filter for stat combos
static uint32_t g_GearSelectedStatId = 0;     // Currently selected stat in dialog
static int g_GearSelectorTab = 0;             // 0 = Stats, 1 = Rune/Sigil, 2 = Weapon Type
static bool g_GearEditingSigil2 = false;      // true = editing second sigil slot (two-handed)

// Save to Library dialog state
static bool g_SaveLibDialogOpen = false;
static char g_SaveLibName[128] = "";
static int g_SaveLibMode = 0;              // GameMode index
static bool g_SaveLibIncludeEquip = false;
static int g_SaveLibEquipTab = 0;          // Selected equipment tab number (1-based)
static std::string g_SaveLibChatLink;      // Pre-built chat link
static std::string g_SaveLibCharName;      // Character name (to find equipment)
static std::string g_SaveLibProfession;    // Profession name

// Compute days until next character birthday from ISO date string (e.g. "2013-06-25T...")
// Returns -1 if the date can't be parsed. Returns 0 on the birthday itself.
static int DaysUntilBirthday(const std::string& created) {
    if (created.size() < 10) return -1;
    int year, month, day;
    if (sscanf(created.c_str(), "%d-%d-%d", &year, &month, &day) != 3) return -1;

    time_t now = std::time(nullptr);
    struct tm* today = std::localtime(&now);

    // Build this year's birthday
    struct tm bday = {};
    bday.tm_year = today->tm_year;
    bday.tm_mon = month - 1;
    bday.tm_mday = day;
    bday.tm_hour = 0;
    time_t bdayTime = std::mktime(&bday);

    // Build today at midnight for clean day diff
    struct tm todayMid = *today;
    todayMid.tm_hour = 0;
    todayMid.tm_min = 0;
    todayMid.tm_sec = 0;
    time_t todayTime = std::mktime(&todayMid);

    int diff = (int)std::difftime(bdayTime, todayTime) / 86400;
    if (diff < 0) {
        // Birthday already passed this year — next year
        bday.tm_year = today->tm_year + 1;
        bdayTime = std::mktime(&bday);
        diff = (int)std::difftime(bdayTime, todayTime) / 86400;
    }
    return diff;
}

// Compute character age in years from ISO date string
static int CharacterAgeYears(const std::string& created) {
    if (created.size() < 10) return -1;
    int year, month, day;
    if (sscanf(created.c_str(), "%d-%d-%d", &year, &month, &day) != 3) return -1;

    time_t now = std::time(nullptr);
    struct tm* today = std::localtime(&now);
    int age = (today->tm_year + 1900) - year;
    // Haven't had birthday yet this year
    if ((today->tm_mon + 1) < month || ((today->tm_mon + 1) == month && today->tm_mday < day))
        age--;
    return age;
}

// Strip GW2 markup tags from description text
// Converts <br> to newline, removes <c=@...> and </c> tags
static std::string StripGW2Markup(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '<') {
            // Find closing >
            size_t end = text.find('>', i);
            if (end != std::string::npos) {
                std::string tag = text.substr(i, end - i + 1);
                // Convert <br> to newline
                if (tag == "<br>" || tag == "<br/>" || tag == "<br />") {
                    result += '\n';
                }
                // Skip all other tags (<c=@...>, </c>, etc.)
                i = end + 1;
                continue;
            }
        }
        result += text[i];
        i++;
    }
    return result;
}

// GW2 API weapon type → current in-game display name
static const char* WeaponDisplayName(const std::string& apiName) {
    if (apiName == "Harpoon") return "Spear";
    if (apiName == "Speargun") return "Harpoon Gun";
    return apiName.c_str();
}

// GW2 rarity colors
static ImVec4 GetRarityColor(const std::string& rarity) {
    if (rarity == "Legendary")  return ImVec4(0.63f, 0.39f, 0.78f, 1.0f);
    if (rarity == "Ascended")   return ImVec4(0.90f, 0.39f, 0.55f, 1.0f);
    if (rarity == "Exotic")     return ImVec4(1.00f, 0.65f, 0.00f, 1.0f);
    if (rarity == "Rare")       return ImVec4(1.00f, 0.86f, 0.20f, 1.0f);
    if (rarity == "Masterwork") return ImVec4(0.12f, 0.71f, 0.12f, 1.0f);
    if (rarity == "Fine")       return ImVec4(0.35f, 0.63f, 0.90f, 1.0f);
    return ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
}

// GW2 profession colors
static ImVec4 GetProfessionColor(const std::string& prof) {
    if (prof == "Guardian")     return ImVec4(0.47f, 0.71f, 0.86f, 1.0f);
    if (prof == "Warrior")      return ImVec4(1.00f, 0.83f, 0.22f, 1.0f);
    if (prof == "Engineer")     return ImVec4(0.85f, 0.56f, 0.25f, 1.0f);
    if (prof == "Ranger")       return ImVec4(0.55f, 0.80f, 0.28f, 1.0f);
    if (prof == "Thief")        return ImVec4(0.78f, 0.31f, 0.31f, 1.0f);
    if (prof == "Elementalist") return ImVec4(0.94f, 0.42f, 0.42f, 1.0f);
    if (prof == "Mesmer")       return ImVec4(0.71f, 0.31f, 0.71f, 1.0f);
    if (prof == "Necromancer")  return ImVec4(0.32f, 0.75f, 0.54f, 1.0f);
    if (prof == "Revenant")     return ImVec4(0.63f, 0.22f, 0.22f, 1.0f);
    return ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
}

// Core profession icon URL from API (fetched at startup via FetchProfessionInfoAsync)
static const char* GetProfessionIconUrl(const std::string& prof) {
    const auto* info = AlterEgo::GW2API::GetProfessionInfo(prof);
    if (info && !info->icon_url.empty()) return info->icon_url.c_str();
    return nullptr;
}

// Stable icon ID for profession icons (offset to avoid collision with item/skill IDs)
static uint32_t GetProfessionIconId(const std::string& prof) {
    static const std::unordered_map<std::string, uint32_t> ids = {
        {"Guardian", 9000001}, {"Warrior", 9000002}, {"Engineer", 9000003},
        {"Ranger", 9000004}, {"Thief", 9000005}, {"Elementalist", 9000006},
        {"Mesmer", 9000007}, {"Necromancer", 9000008}, {"Revenant", 9000009}
    };
    auto it = ids.find(prof);
    return it != ids.end() ? it->second : 0;
}

// Crafting discipline icon helpers (wiki tango icons, stable IDs 9100001+)
static uint32_t GetCraftingIconId(const std::string& disc) {
    static const std::unordered_map<std::string, uint32_t> ids = {
        {"Armorsmith", 9100001}, {"Artificer", 9100002}, {"Chef", 9100003},
        {"Huntsman", 9100004}, {"Jeweler", 9100005}, {"Leatherworker", 9100006},
        {"Scribe", 9100007}, {"Tailor", 9100008}, {"Weaponsmith", 9100009}
    };
    auto it = ids.find(disc);
    return it != ids.end() ? it->second : 0;
}

static const char* GetCraftingIconUrl(const std::string& disc) {
    static const std::unordered_map<std::string, std::string> urls = {
        {"Armorsmith",    "https://wiki.guildwars2.com/images/3/32/Armorsmith_tango_icon_20px.png"},
        {"Artificer",     "https://wiki.guildwars2.com/images/b/b7/Artificer_tango_icon_20px.png"},
        {"Chef",          "https://wiki.guildwars2.com/images/8/8f/Chef_tango_icon_20px.png"},
        {"Huntsman",      "https://wiki.guildwars2.com/images/f/f3/Huntsman_tango_icon_20px.png"},
        {"Jeweler",       "https://wiki.guildwars2.com/images/f/f2/Jeweler_tango_icon_20px.png"},
        {"Leatherworker", "https://wiki.guildwars2.com/images/e/e5/Leatherworker_tango_icon_20px.png"},
        {"Scribe",        "https://wiki.guildwars2.com/images/0/0b/Scribe_tango_icon_20px.png"},
        {"Tailor",        "https://wiki.guildwars2.com/images/4/4d/Tailor_tango_icon_20px.png"},
        {"Weaponsmith",   "https://wiki.guildwars2.com/images/4/46/Weaponsmith_tango_icon_20px.png"}
    };
    auto it = urls.find(disc);
    return it != urls.end() ? it->second.c_str() : nullptr;
}

// Copy text to Windows clipboard
void CopyToClipboard(const std::string& text) {
    if (!OpenClipboard(NULL)) return;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem) {
        char* p = (char*)GlobalLock(hMem);
        if (p) {
            memcpy(p, text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
    }
    CloseClipboard();
}

// Settings persistence
static void SaveSettings() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/settings.json";
    nlohmann::json j;
    j["show_qa_icon"] = g_ShowQAIcon;
    j["compact_char_list"] = g_CompactCharList;
    j["show_crafting_icons"] = g_ShowCraftingIcons;
    j["show_age"] = g_ShowAge;
    j["show_playtime"] = g_ShowPlaytime;
    j["show_last_login"] = g_ShowLastLogin;
    j["birthday_mode"] = g_BirthdayMode;
    j["char_list_width"] = g_CharListWidth;
    j["lib_list_width"] = g_LibListWidth;
    j["chat_build_detection"] = g_ChatBuildDetection;
    j["toast_pos_x"] = g_ToastPosX;
    j["toast_pos_y"] = g_ToastPosY;
    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadSettings() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/settings.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    nlohmann::json j;
    try { j = nlohmann::json::parse(file); }
    catch (...) { return; }
    auto getBool = [&](const char* k, bool& dst) {
        if (j.contains(k) && j[k].is_boolean()) dst = j[k].get<bool>();
    };
    auto getInt = [&](const char* k, int& dst) {
        if (j.contains(k) && j[k].is_number_integer()) dst = j[k].get<int>();
    };
    auto getFloat = [&](const char* k, float& dst) {
        if (j.contains(k) && j[k].is_number()) dst = j[k].get<float>();
    };
    getBool("show_qa_icon", g_ShowQAIcon);
    getBool("compact_char_list", g_CompactCharList);
    getBool("show_crafting_icons", g_ShowCraftingIcons);
    getBool("show_age", g_ShowAge);
    getBool("show_playtime", g_ShowPlaytime);
    getBool("show_last_login", g_ShowLastLogin);
    getInt("birthday_mode", g_BirthdayMode);
    getFloat("char_list_width", g_CharListWidth);
    getFloat("lib_list_width", g_LibListWidth);
    getBool("chat_build_detection", g_ChatBuildDetection);
    getFloat("toast_pos_x", g_ToastPosX);
    getFloat("toast_pos_y", g_ToastPosY);
}

// Character sort persistence
static void SaveCharSortConfig() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/char_sort.json";
    nlohmann::json j;
    j["sort_mode"] = g_CharSortMode;
    j["sort_ascending"] = g_CharSortAscending;
    j["custom_order"] = g_CustomCharOrder;
    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadCharSortConfig() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/char_sort.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        if (j.contains("sort_mode")) g_CharSortMode = j["sort_mode"].get<int>();
        if (j.contains("sort_ascending")) g_CharSortAscending = j["sort_ascending"].get<bool>();
        if (j.contains("custom_order")) {
            g_CustomCharOrder.clear();
            for (const auto& name : j["custom_order"])
                g_CustomCharOrder.push_back(name.get<std::string>());
        }
    } catch (...) {}
}

// Session restore persistence
static std::string g_SessionSelectedCharName; // deferred: resolve to index after chars load

static void SaveSession() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/session.json";

    nlohmann::json j;
    j["main_tab"] = g_MainTab;

    // Characters tab state — save by name for stability across sort/order changes
    const auto& chars = AlterEgo::GW2API::GetCharacters();
    if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
        int ri = g_CharDisplayOrder[g_SelectedCharIdx];
        if (ri >= 0 && ri < (int)chars.size())
            j["selected_char"] = chars[ri].name;
    }
    j["detail_tab"] = g_SelectedTab;
    j["equip_tab"] = g_SelectedEquipTab;
    j["build_tab"] = g_SelectedBuildTab;

    // Build Library state — save selected build by ID
    const auto& builds = AlterEgo::GW2API::GetSavedBuilds();
    if (g_LibSelectedIdx >= 0 && g_LibSelectedIdx < (int)builds.size())
        j["lib_selected_id"] = builds[g_LibSelectedIdx].id;
    j["lib_filter"] = g_LibFilterMode;

    // Account selector
    j["selected_account"] = g_SelectedAccountFilter;

    // Skinventory state
    j["skin_active_tab"] = g_SkinActiveTab;
    j["skin_type"] = g_SkinSelectedType;
    j["skin_weight"] = g_SkinSelectedWeightClass;
    j["skin_subtype"] = g_SkinSelectedSubtype;
    j["skin_selected_id"] = g_SkinSelectedId;
    j["skin_show_owned"] = g_SkinShowOwned;
    j["skin_show_unowned"] = g_SkinShowUnowned;
    j["skin_view_mode"] = g_SkinViewMode;

    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadSession() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/session.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);

        // g_WindowVisible intentionally not restored — always start closed
        if (j.contains("main_tab")) g_MainTab = j["main_tab"].get<int>();

        // Defer character selection — store name, resolve after chars load
        if (j.contains("selected_char"))
            g_SessionSelectedCharName = j["selected_char"].get<std::string>();
        if (j.contains("detail_tab")) g_SelectedTab = j["detail_tab"].get<int>();
        if (j.contains("equip_tab")) g_SelectedEquipTab = j["equip_tab"].get<int>();
        if (j.contains("build_tab")) g_SelectedBuildTab = j["build_tab"].get<int>();

        // Build Library — defer selection, resolve after library loads
        if (j.contains("lib_selected_id")) {
            std::string savedId = j["lib_selected_id"].get<std::string>();
            const auto& builds = AlterEgo::GW2API::GetSavedBuilds();
            for (int i = 0; i < (int)builds.size(); i++) {
                if (builds[i].id == savedId) { g_LibSelectedIdx = i; break; }
            }
        }
        if (j.contains("lib_filter")) g_LibFilterMode = j["lib_filter"].get<int>();

        // Account selector
        if (j.contains("selected_account"))
            g_SelectedAccountFilter = j["selected_account"].get<std::string>();

        // Skinventory
        if (j.contains("skin_active_tab")) g_SkinActiveTab = j["skin_active_tab"].get<int>();
        if (j.contains("skin_type")) g_SkinSelectedType = j["skin_type"].get<std::string>();
        if (j.contains("skin_weight")) g_SkinSelectedWeightClass = j["skin_weight"].get<std::string>();
        if (j.contains("skin_subtype")) g_SkinSelectedSubtype = j["skin_subtype"].get<std::string>();
        if (j.contains("skin_selected_id")) g_SkinSelectedId = j["skin_selected_id"].get<uint32_t>();
        if (j.contains("skin_show_owned")) g_SkinShowOwned = j["skin_show_owned"].get<bool>();
        if (j.contains("skin_show_unowned")) g_SkinShowUnowned = j["skin_show_unowned"].get<bool>();
        if (j.contains("skin_view_mode")) g_SkinViewMode = j["skin_view_mode"].get<int>();
    } catch (...) {}
}


// =========================================================================
// Achievement Tracker — Persistence
// =========================================================================

static void SaveAchTrackerState() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/ach_tracked.json";

    nlohmann::json j;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        j["pinned"] = g_AchPinned;
        j["popout_visible"] = g_AchPopoutVisible;
        j["show_completed_steps"] = g_AchShowCompletedSteps;
        j["hide_empty_cats"] = g_AchHideEmptyCats;
        j["hide_completed_cats"] = g_AchHideCompletedCats;
        j["selected_group"] = g_AchSelectedGroupId;
        j["selected_category"] = g_AchSelectedCatId;
        j["tree_width"] = g_AchTreeWidth;
        j["pinned_height"] = g_AchPinnedHeight;
        j["tree_scroll_y"] = g_AchTreeScrollY;
        j["list_scroll_y"] = g_AchListScrollY;

        nlohmann::json expanded = nlohmann::json::array();
        for (const auto& [id, exp] : g_AchExpandedInPopout) {
            if (exp) expanded.push_back(id);
        }
        j["expanded_in_popout"] = expanded;
    }

    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadAchTrackerState() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/ach_tracked.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        if (j.contains("pinned") && j["pinned"].is_array()) {
            g_AchPinned.clear();
            for (const auto& id : j["pinned"]) g_AchPinned.push_back(id.get<uint32_t>());
        }
        g_AchPopoutVisible = j.value("popout_visible", false);
        g_AchShowCompletedSteps = j.value("show_completed_steps", !j.value("show_remaining", true));
        g_AchHideEmptyCats = j.value("hide_empty_cats", false);
        g_AchHideCompletedCats = j.value("hide_completed_cats", false);
        g_AchSelectedGroupId = j.value("selected_group", "");
        g_AchSelectedCatId = j.value("selected_category", 0u);
        g_AchTreeWidth = j.value("tree_width", 220.0f);
        g_AchPinnedHeight = j.value("pinned_height", 200.0f);
        g_AchTreeScrollY = j.value("tree_scroll_y", 0.0f);
        g_AchListScrollY = j.value("list_scroll_y", 0.0f);
        if (j.contains("expanded_in_popout") && j["expanded_in_popout"].is_array()) {
            g_AchExpandedInPopout.clear();
            for (const auto& id : j["expanded_in_popout"]) {
                g_AchExpandedInPopout[id.get<uint32_t>()] = true;
            }
        }
        if (g_AchSelectedCatId > 0) g_AchRestoreScroll = true;
    } catch (...) {}
}

static void SaveAchGroupCache() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/ach_groups.json";

    nlohmann::json j;
    auto epochSecs = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    j["fetch_time"] = epochSecs;

    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);

        nlohmann::json groups = nlohmann::json::array();
        for (const auto& g : g_AchGroups) {
            nlohmann::json gj;
            gj["id"] = g.id;
            gj["name"] = g.name;
            gj["description"] = g.description;
            gj["order"] = g.order;
            gj["categories"] = g.categories;
            groups.push_back(gj);
        }
        j["groups"] = groups;

        nlohmann::json cats = nlohmann::json::object();
        for (const auto& [id, c] : g_AchCategories) {
            nlohmann::json cj;
            cj["id"] = c.id;
            cj["name"] = c.name;
            cj["description"] = c.description;
            cj["order"] = c.order;
            cj["icon_url"] = c.icon_url;
            cj["achievements"] = c.achievements;
            cats[std::to_string(id)] = cj;
        }
        j["categories"] = cats;
    }

    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadAchGroupCache() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/ach_groups.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);

        // Check staleness — daily categories change each reset, so invalidate on daily reset
        int64_t fetchEpoch = j.value("fetch_time", (int64_t)0);
        auto fetchTime = std::chrono::system_clock::from_time_t((time_t)fetchEpoch);
        auto now = std::chrono::system_clock::now();
        auto dailyReset = CalcLastDailyReset(now);
        if (fetchTime < dailyReset) return; // stale — daily categories have rotated

        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);

        if (j.contains("groups") && j["groups"].is_array()) {
            g_AchGroups.clear();
            for (const auto& gj : j["groups"]) {
                AchGroupDef g;
                g.id = gj.value("id", "");
                g.name = gj.value("name", "");
                g.description = gj.value("description", "");
                g.order = gj.value("order", 0);
                if (gj.contains("categories") && gj["categories"].is_array()) {
                    for (const auto& cid : gj["categories"]) g.categories.push_back(cid.get<uint32_t>());
                }
                g_AchGroups.push_back(std::move(g));
            }
        }

        if (j.contains("categories") && j["categories"].is_object()) {
            g_AchCategories.clear();
            for (auto& [key, cj] : j["categories"].items()) {
                AchCategoryDef c;
                c.id = cj.value("id", 0u);
                c.name = cj.value("name", "");
                c.description = cj.value("description", "");
                c.order = cj.value("order", 0);
                c.icon_url = cj.value("icon_url", "");
                if (cj.contains("achievements") && cj["achievements"].is_array()) {
                    for (const auto& aid : cj["achievements"]) c.achievements.push_back(aid.get<uint32_t>());
                }
                g_AchCategories[c.id] = std::move(c);
            }
        }

        // Build reverse lookup
        for (const auto& [catId, cat] : g_AchCategories) {
            for (uint32_t achId : cat.achievements) {
                g_AchIdToCategory[achId] = catId;
            }
        }

        // Build hidden category set
        g_AchHiddenCatIds.clear();
        for (const auto& g : g_AchGroups) {
            if (g.name == "Character Adventure Guide") {
                for (uint32_t cid : g.categories) g_AchHiddenCatIds.insert(cid);
            }
        }

        if (!g_AchGroups.empty()) g_AchGroupsFetched = true;
    } catch (...) {}
}

static void SaveAchDefCache() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/ach_defs.json";

    nlohmann::json j;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        for (const auto& [id, d] : g_AchDefs) {
            nlohmann::json dj;
            dj["id"] = d.id;
            dj["name"] = d.name;
            dj["description"] = d.description;
            dj["requirement"] = d.requirement;
            dj["icon_url"] = d.icon_url;
            dj["type"] = d.type;
            dj["flags"] = d.flags;
            dj["max_count"] = d.max_count;
            dj["total_ap"] = d.total_ap;
            dj["prerequisites"] = d.prerequisites;
            nlohmann::json bits = nlohmann::json::array();
            for (const auto& b : d.bits) {
                nlohmann::json bj;
                bj["type"] = b.type;
                bj["item_id"] = b.item_id;
                bj["text"] = b.text;
                bits.push_back(bj);
            }
            dj["bits"] = bits;
            j[std::to_string(id)] = dj;
        }
    }

    std::ofstream file(path);
    if (file.is_open()) file << j.dump();
}

static void LoadAchDefCache() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/ach_defs.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        for (auto& [key, dj] : j.items()) {
            AchDef d;
            d.id = dj.value("id", 0u);
            d.name = dj.value("name", "");
            d.description = dj.value("description", "");
            d.requirement = dj.value("requirement", "");
            d.icon_url = dj.value("icon_url", "");
            d.type = dj.value("type", "");
            if (dj.contains("flags") && dj["flags"].is_array()) {
                for (const auto& f : dj["flags"]) d.flags.push_back(f.get<std::string>());
            }
            d.max_count = dj.value("max_count", 0);
            d.total_ap = dj.value("total_ap", 0);
            if (dj.contains("prerequisites") && dj["prerequisites"].is_array()) {
                for (const auto& p : dj["prerequisites"]) d.prerequisites.push_back(p.get<uint32_t>());
            }
            if (dj.contains("bits") && dj["bits"].is_array()) {
                for (const auto& bj : dj["bits"]) {
                    AchBitDef b;
                    b.type = bj.value("type", "");
                    b.item_id = bj.value("item_id", 0u);
                    b.text = bj.value("text", "");
                    d.bits.push_back(std::move(b));
                }
            }
            if (d.id > 0) g_AchDefs[d.id] = std::move(d);
        }
    } catch (...) {}
}

static void SaveAchNameIndex() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/ach_name_index.json";

    nlohmann::json j;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        for (const auto& [id, name] : g_AchNameIndex) {
            j[std::to_string(id)] = name;
        }
    }

    std::ofstream file(path);
    if (file.is_open()) file << j.dump();
}

static void LoadAchNameIndex() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/ach_name_index.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        g_AchNameIndex.clear();
        for (auto& [key, val] : j.items()) {
            uint32_t id = (uint32_t)std::stoul(key);
            g_AchNameIndex[id] = val.get<std::string>();
        }
        if (!g_AchNameIndex.empty()) g_AchNameIndexReady = true;
    } catch (...) {}
}

// =========================================================================
// Achievement Tracker — Waypoint Data
// =========================================================================

static void SaveAchWaypoints() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/ach_waypoints.json";

    nlohmann::json j;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        for (const auto& [achId, bits] : g_AchWaypoints) {
            nlohmann::json entry;
            for (const auto& [bitIdx, code] : bits) {
                entry[std::to_string(bitIdx)] = code;
            }
            j[std::to_string(achId)] = entry;
        }
    }

    std::ofstream file(path);
    if (file.is_open()) file << j.dump();
}

static void LoadAchWaypoints() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/ach_waypoints.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        g_AchWaypoints.clear();
        for (auto& [achKey, val] : j.items()) {
            uint32_t achId = (uint32_t)std::stoul(achKey);
            if (val.is_string()) {
                // Single waypoint for the whole achievement
                g_AchWaypoints[achId][-1] = val.get<std::string>();
            } else if (val.is_object()) {
                for (auto& [bitKey, code] : val.items()) {
                    int bitIdx = std::stoi(bitKey);
                    g_AchWaypoints[achId][bitIdx] = code.get<std::string>();
                }
            }
        }
        if (!g_AchWaypoints.empty()) g_AchWaypointsReady = true;
    } catch (...) {}
}

static void FetchAchWaypoints() {
    if (g_AchWaypointsFetching) return;
    g_AchWaypointsFetching = true;

    std::thread([]() {
        std::string json = Skinventory::HttpClient::Get(ACH_WAYPOINTS_URL);
        if (!json.empty()) {
            try {
                auto j = nlohmann::json::parse(json);
                if (j.is_object()) {
                    std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                    g_AchWaypoints.clear();
                    for (auto& [achKey, val] : j.items()) {
                        uint32_t achId = (uint32_t)std::stoul(achKey);
                        if (val.is_string()) {
                            g_AchWaypoints[achId][-1] = val.get<std::string>();
                        } else if (val.is_object()) {
                            for (auto& [bitKey, code] : val.items()) {
                                int bitIdx = std::stoi(bitKey);
                                g_AchWaypoints[achId][bitIdx] = code.get<std::string>();
                            }
                        }
                    }
                    g_AchWaypointsReady = true;
                    SaveAchWaypoints();
                }
            } catch (...) {}
        }
        g_AchWaypointsFetching = false;
    }).detach();
}

// =========================================================================
// Hero Challenges — Data
// =========================================================================

static void SaveHeroChallenges() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/hero_challenges.json";
    nlohmann::json j;
    for (const auto& [id, info] : g_HeroChallenges) {
        nlohmann::json entry;
        entry["map_name"] = info.map_name;
        entry["expansion"] = info.expansion;
        j[id] = entry;
    }
    std::ofstream file(path);
    if (file.is_open()) file << j.dump();
}

static void LoadHeroChallenges() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/hero_challenges.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        if (j.is_object()) {
            g_HeroChallenges.clear();
            for (auto& [id, val] : j.items()) {
                HeroChallengeInfo info;
                info.map_name = val.value("map_name", "");
                info.expansion = val.value("expansion", "");
                g_HeroChallenges[id] = std::move(info);
            }
            g_HeroChallengesReady = !g_HeroChallenges.empty();
        }
    } catch (...) {}
}

static void FetchHeroChallenges() {
    if (g_HeroChallengesFetching) return;
    g_HeroChallengesFetching = true;
    std::thread([]() {
        std::string json = Skinventory::HttpClient::Get(HERO_CHALLENGES_URL);
        if (!json.empty()) {
            try {
                auto j = nlohmann::json::parse(json);
                if (j.is_object()) {
                    g_HeroChallenges.clear();
                    for (auto& [id, val] : j.items()) {
                        HeroChallengeInfo info;
                        info.map_name = val.value("map_name", "");
                        info.expansion = val.value("expansion", "");
                        g_HeroChallenges[id] = std::move(info);
                    }
                    g_HeroChallengesReady = true;
                    SaveHeroChallenges();
                }
            } catch (...) {}
        }
        g_HeroChallengesFetching = false;
    }).detach();
}

// =========================================================================
// Achievement Tracker — API Fetch
// =========================================================================

static void FetchAchGroups() {
    if (g_AchGroupsFetching) return;
    g_AchGroupsFetching = true;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        g_AchStatusMsg = "Fetching achievement groups...";
    }

    std::thread([]() {
        // Fetch all groups
        std::string groupsJson = Skinventory::HttpClient::Get(
            "https://api.guildwars2.com/v2/achievements/groups?ids=all");
        if (groupsJson.empty()) {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchStatusMsg = "Failed to fetch achievement groups";
            g_AchGroupsFetching = false;
            return;
        }

        std::vector<AchGroupDef> groups;
        try {
            auto j = nlohmann::json::parse(groupsJson);
            if (j.is_array()) {
                for (const auto& gj : j) {
                    AchGroupDef g;
                    g.id = gj.value("id", "");
                    g.name = gj.value("name", "");
                    g.description = gj.value("description", "");
                    g.order = gj.value("order", 0);
                    if (gj.contains("categories") && gj["categories"].is_array()) {
                        for (const auto& cid : gj["categories"]) g.categories.push_back(cid.get<uint32_t>());
                    }
                    if (!g.id.empty()) groups.push_back(std::move(g));
                }
            }
        } catch (...) {}

        if (groups.empty()) {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchStatusMsg = "Failed to parse achievement groups";
            g_AchGroupsFetching = false;
            return;
        }

        // Sort groups by order
        std::sort(groups.begin(), groups.end(),
            [](const AchGroupDef& a, const AchGroupDef& b) { return a.order < b.order; });

        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchStatusMsg = "Fetching achievement categories...";
        }

        // Fetch all categories
        std::string catsJson = Skinventory::HttpClient::Get(
            "https://api.guildwars2.com/v2/achievements/categories?ids=all");
        if (catsJson.empty()) {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchStatusMsg = "Failed to fetch categories";
            g_AchGroupsFetching = false;
            return;
        }

        std::unordered_map<uint32_t, AchCategoryDef> cats;
        std::unordered_map<uint32_t, uint32_t> idToCat;
        try {
            auto j = nlohmann::json::parse(catsJson);
            if (j.is_array()) {
                for (const auto& cj : j) {
                    AchCategoryDef c;
                    c.id = cj.value("id", 0u);
                    c.name = cj.value("name", "");
                    c.description = cj.value("description", "");
                    c.order = cj.value("order", 0);
                    c.icon_url = cj.value("icon", "");
                    if (cj.contains("achievements") && cj["achievements"].is_array()) {
                        for (const auto& aid : cj["achievements"]) {
                            uint32_t achId = aid.get<uint32_t>();
                            c.achievements.push_back(achId);
                            idToCat[achId] = c.id;
                        }
                    }
                    if (c.id > 0) cats[c.id] = std::move(c);
                }
            }
        } catch (...) {}

        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchGroups = std::move(groups);
            g_AchCategories = std::move(cats);
            g_AchIdToCategory = std::move(idToCat);
            // Build hidden category set
            g_AchHiddenCatIds.clear();
            for (const auto& g : g_AchGroups) {
                if (g.name == "Character Adventure Guide") {
                    for (uint32_t cid : g.categories) g_AchHiddenCatIds.insert(cid);
                }
            }
            g_AchGroupsFetched = true;
            g_AchGroupsFetching = false;
            g_AchStatusMsg = "";
        }

        SaveAchGroupCache();
        FetchActiveSpecialEvent();
    }).detach();
}

static void FetchActiveSpecialEvent() {
    if (g_AchActiveEventFetching) return;
    g_AchActiveEventFetching = true;
    std::thread([]() {
        // Try disk-cached daily response (1-hour TTL) before hitting the API
        std::string json;
        std::string dailyCachePath = AlterEgo::GW2API::GetDataDirectory() + "/ach_daily_cache.json";
        try {
            std::ifstream cf(dailyCachePath);
            if (cf.is_open()) {
                nlohmann::json wrap;
                cf >> wrap;
                if (wrap.is_object() && wrap.contains("ts") && wrap.contains("body")) {
                    int64_t ts = wrap["ts"].get<int64_t>();
                    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                    if (now - ts < 3600 && wrap["body"].is_string()) {
                        json = wrap["body"].get<std::string>();
                    }
                }
            }
        } catch (...) {}

        if (json.empty()) {
            json = Skinventory::HttpClient::Get("https://api.guildwars2.com/v2/achievements/daily");
            if (!json.empty()) {
                try {
                    nlohmann::json wrap;
                    wrap["ts"] = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                    wrap["body"] = json;
                    WriteFileAtomic(dailyCachePath, wrap.dump());
                } catch (...) {}
            }
        }
        if (json.empty()) {
            g_AchActiveEventFetching = false;
            return;
        }
        std::vector<uint32_t> specialAchIds;
        try {
            auto j = nlohmann::json::parse(json);
            if (j.contains("special") && j["special"].is_array()) {
                for (const auto& entry : j["special"]) {
                    if (entry.contains("id")) specialAchIds.push_back(entry["id"].get<uint32_t>());
                }
            }
        } catch (...) {
            g_AchActiveEventFetching = false;
            return;
        }

        // Resolve special achievement IDs to their category IDs
        std::unordered_set<uint32_t> triggerCatIds;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            for (uint32_t achId : specialAchIds) {
                auto it = g_AchIdToCategory.find(achId);
                if (it != g_AchIdToCategory.end()) triggerCatIds.insert(it->second);
            }
        }

        // Build keyword list and classify trigger categories as daily or main
        std::vector<std::string> keywords;
        std::vector<uint32_t> dailyCats;
        std::vector<uint32_t> mainCats;
        std::string eventName;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);

            for (uint32_t cid : triggerCatIds) {
                // Check if this is a known festival daily
                auto festIt = FESTIVAL_MAPPINGS.find(cid);
                if (festIt != FESTIVAL_MAPPINGS.end()) {
                    dailyCats.push_back(cid);
                    if (eventName.empty()) eventName = festIt->second.displayName;
                    for (const auto& kw : festIt->second.keywords) keywords.push_back(kw);
                    continue;
                }

                // Non-festival: classify by category name
                auto catIt = g_AchCategories.find(cid);
                if (catIt == g_AchCategories.end()) continue;
                const auto& name = catIt->second.name;

                // Skip junk
                if (IsBonusEventJunk(name)) continue;

                if (name.find("Daily") != std::string::npos) {
                    // Daily bonus event (e.g. "Daily Roller Beetle Racing")
                    dailyCats.push_back(cid);
                    // Derive keyword by stripping "Daily " prefix
                    std::string kw = name;
                    if (kw.substr(0, 6) == "Daily ") kw = kw.substr(6);
                    keywords.push_back(kw);
                    if (eventName.empty()) eventName = kw;
                } else {
                    // Main bonus event category (e.g. "Fractal Rush", "Gnashbash")
                    mainCats.push_back(cid);
                    keywords.push_back(name);
                    if (eventName.empty()) eventName = name;
                }
            }

            // Search Historical group for categories matching any keyword
            if (!keywords.empty()) {
                for (const auto& grp : g_AchGroups) {
                    if (grp.name != "Historical") continue;
                    for (uint32_t cid : grp.categories) {
                        if (triggerCatIds.count(cid)) continue; // already classified
                        auto catIt = g_AchCategories.find(cid);
                        if (catIt == g_AchCategories.end()) continue;
                        const auto& name = catIt->second.name;
                        for (const auto& kw : keywords) {
                            if (name.find(kw) != std::string::npos) {
                                mainCats.push_back(cid);
                                break;
                            }
                        }
                    }
                    break;
                }
            }

            // Also search Bonus Events group for non-daily, non-junk matching categories
            if (!keywords.empty()) {
                for (const auto& grp : g_AchGroups) {
                    if (grp.name != "Bonus Events") continue;
                    for (uint32_t cid : grp.categories) {
                        if (triggerCatIds.count(cid)) continue; // already classified
                        auto catIt = g_AchCategories.find(cid);
                        if (catIt == g_AchCategories.end()) continue;
                        const auto& name = catIt->second.name;
                        if (IsBonusEventJunk(name)) continue;
                        if (name.find("Daily") != std::string::npos) continue; // dailies handled above
                        for (const auto& kw : keywords) {
                            if (name.find(kw) != std::string::npos) {
                                mainCats.push_back(cid);
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }

        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchActiveMainCatIds = std::move(mainCats);
            g_AchActiveDailyCatIds = std::move(dailyCats);
            g_AchActiveEventName = std::move(eventName);
            g_AchActiveEventFetched = true;
            g_AchActiveEventFetching = false;
        }
    }).detach();
}

static AchDef ParseAchDef(const nlohmann::json& ach) {
    AchDef d;
    d.id = ach.value("id", 0u);
    d.name = ach.value("name", "");
    d.description = ach.value("description", "");
    d.requirement = ach.value("requirement", "");
    d.icon_url = ach.value("icon", "");
    d.type = ach.value("type", "");
    if (ach.contains("flags") && ach["flags"].is_array()) {
        for (const auto& f : ach["flags"]) d.flags.push_back(f.get<std::string>());
    }
    if (ach.contains("prerequisites") && ach["prerequisites"].is_array()) {
        for (const auto& p : ach["prerequisites"]) d.prerequisites.push_back(p.get<uint32_t>());
    }
    if (ach.contains("bits") && ach["bits"].is_array()) {
        for (const auto& bj : ach["bits"]) {
            AchBitDef b;
            b.type = bj.value("type", "");
            b.item_id = bj.value("id", 0u);
            b.text = bj.value("text", "");
            d.bits.push_back(std::move(b));
        }
    }
    if (ach.contains("tiers") && ach["tiers"].is_array()) {
        auto& tiers = ach["tiers"];
        for (const auto& t : tiers) {
            d.total_ap += t.value("points", 0);
        }
        if (!tiers.empty()) {
            d.max_count = tiers.back().value("count", 0);
        }
    }
    return d;
}

static void FetchAchCategoryDefs(uint32_t catId) {
    if (g_AchCatFetching) return;
    g_AchCatFetching = true;

    std::vector<uint32_t> toFetch;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        auto it = g_AchCategories.find(catId);
        if (it == g_AchCategories.end()) { g_AchCatFetching = false; return; }
        for (uint32_t id : it->second.achievements) {
            if (g_AchDefs.find(id) == g_AchDefs.end()) toFetch.push_back(id);
        }
    }

    if (toFetch.empty()) {
        g_AchCatFetching = false;
        // Still need progress — trigger that
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        auto it = g_AchCategories.find(catId);
        if (it != g_AchCategories.end()) {
            SendAchProgressQuery(it->second.achievements);
        }
        return;
    }

    std::thread([toFetch, catId]() {
        // Fetch in batches of 200
        for (size_t start = 0; start < toFetch.size(); start += 200) {
            size_t end = std::min(start + 200, toFetch.size());
            std::string idStr;
            for (size_t i = start; i < end; i++) {
                if (i > start) idStr += ",";
                idStr += std::to_string(toFetch[i]);
            }

            std::string json = Skinventory::HttpClient::Get(
                "https://api.guildwars2.com/v2/achievements?ids=" + idStr);
            if (!json.empty()) {
                try {
                    auto j = nlohmann::json::parse(json);
                    if (j.is_array()) {
                        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                        for (const auto& ach : j) {
                            AchDef d = ParseAchDef(ach);
                            if (d.id > 0) g_AchDefs[d.id] = std::move(d);
                        }
                    }
                } catch (...) {}
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        g_AchCatFetching = false;
        SaveAchDefCache();

        // Signal render thread to query progress (Events_Raise must be on render thread)
        g_AchCatProgressPending = catId;
    }).detach();
}

static void SendAchProgressQuery(const std::vector<uint32_t>& ids) {
    if (ids.empty() || !APIDefs) return;

    // Send via H&S achievement query (same mechanism as Clears)
    std::string acctName = GetEffectiveAccountName();
    HoardQueryAchievementRequest req{};
    req.api_version = HOARD_API_VERSION;
    strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
    strncpy(req.response_event, EV_AE_ACH_PROGRESS_RESPONSE, sizeof(req.response_event) - 1);
    if (!acctName.empty())
        strncpy(req.account_name, acctName.c_str(), sizeof(req.account_name) - 1);

    // Batch into chunks of 200 (H&S limit)
    for (size_t start = 0; start < ids.size(); start += 200) {
        size_t end = std::min(start + 200, ids.size());
        req.id_count = (uint32_t)(end - start);
        for (size_t i = start; i < end; i++) {
            req.ids[i - start] = ids[i];
        }
        APIDefs->Events_Raise(EV_HOARD_QUERY_ACHIEVEMENT, &req);
    }
    g_AchProgressFetching = true;
}

static void OnAchProgressResponse(void* eventArgs) {
    if (!eventArgs) return;
    auto* resp = (HoardQueryAchievementResponse*)eventArgs;
    if (resp->api_version < 2) { return; }

    if (resp->status != HOARD_STATUS_OK) {
        if (resp->status == HOARD_STATUS_PENDING) {
            g_AchRetryPending = true;
        }
        g_AchProgressFetching = false;
        return;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        for (uint32_t i = 0; i < resp->entry_count; i++) {
            auto& e = resp->entries[i];
            AchProgress p;
            p.id = e.id;
            p.done = e.done;
            p.current = e.current;
            p.max = e.max;
            for (uint32_t b = 0; b < e.bit_count && b < 64; b++) {
                p.completed_bits.insert(e.bits[b]);
            }
            // For infinite repeatables, the API returns done=true if ever completed,
            // even after restart. Override based on current progress.
            if (p.done && p.max > 0 && p.current < p.max) {
                auto defIt = g_AchDefs.find(e.id);
                if (defIt != g_AchDefs.end()) {
                    bool isRepeatable = false, isDailyOrWeekly = false;
                    for (const auto& f : defIt->second.flags) {
                        if (f == "Repeatable") isRepeatable = true;
                        if (f == "Daily" || f == "Weekly") isDailyOrWeekly = true;
                    }
                    if (isRepeatable && !isDailyOrWeekly) {
                        p.done = false;
                    }
                }
            }
            // Grace period: protect optimistic alert increments from stale API data
            auto optIt = g_AchOptimisticTime.find(e.id);
            if (optIt != g_AchOptimisticTime.end()) {
                auto elapsed = std::chrono::steady_clock::now() - optIt->second;
                auto existIt = g_AchProgress.find(e.id);
                if (elapsed < std::chrono::minutes(10) && existIt != g_AchProgress.end()
                    && p.current < existIt->second.current) {
                    // API hasn't caught up — keep our optimistic current/done,
                    // but merge in any newly completed bits from the API
                    for (uint32_t bit : p.completed_bits) {
                        existIt->second.completed_bits.insert(bit);
                    }
                    continue;
                }
                // API caught up or grace period expired — accept and clear
                g_AchOptimisticTime.erase(optIt);
            }
            g_AchProgress[e.id] = std::move(p);
        }
        g_LastAchProgressQuery = std::chrono::steady_clock::now();
        g_AchProgressGen++;
        g_AchProgressDirty = true;
    }
    g_AchProgressFetching = false;
}

static void FetchAchNameIndex() {
    if (g_AchNameIndexFetching) return;
    g_AchNameIndexFetching = true;

    std::thread([]() {
        // Try GitHub hosted index first
        std::string json = Skinventory::HttpClient::Get(ACH_NAME_INDEX_URL);
        if (!json.empty()) {
            try {
                auto j = nlohmann::json::parse(json);
                if (j.is_object() && j.size() > 100) {
                    std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                    g_AchNameIndex.clear();
                    for (auto& [key, val] : j.items()) {
                        uint32_t id = (uint32_t)std::stoul(key);
                        g_AchNameIndex[id] = val.get<std::string>();
                    }
                    g_AchNameIndexReady = true;
                    g_AchNameIndexFetching = false;
                    SaveAchNameIndex();
                    return;
                }
            } catch (...) {}
        }

        // Fallback: build index from GW2 API by fetching all achievement IDs in batches
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchStatusMsg = "Building achievement index...";
        }

        // Get all achievement IDs
        std::string idsJson = Skinventory::HttpClient::Get(
            "https://api.guildwars2.com/v2/achievements");
        if (idsJson.empty()) { g_AchNameIndexFetching = false; return; }

        std::vector<uint32_t> allIds;
        try {
            auto j = nlohmann::json::parse(idsJson);
            if (j.is_array()) {
                for (const auto& id : j) allIds.push_back(id.get<uint32_t>());
            }
        } catch (...) {}

        if (allIds.empty()) { g_AchNameIndexFetching = false; return; }

        std::unordered_map<uint32_t, std::string> nameIndex;
        for (size_t start = 0; start < allIds.size(); start += 200) {
            size_t end = std::min(start + 200, allIds.size());
            std::string idStr;
            for (size_t i = start; i < end; i++) {
                if (i > start) idStr += ",";
                idStr += std::to_string(allIds[i]);
            }

            std::string batchJson = Skinventory::HttpClient::Get(
                "https://api.guildwars2.com/v2/achievements?ids=" + idStr);
            if (!batchJson.empty()) {
                try {
                    auto j = nlohmann::json::parse(batchJson);
                    if (j.is_array()) {
                        for (const auto& ach : j) {
                            uint32_t id = ach.value("id", 0u);
                            std::string name = ach.value("name", "");
                            if (id > 0 && !name.empty()) nameIndex[id] = name;
                        }

                        // Also cache full defs while we have them
                        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                        for (const auto& ach : j) {
                            AchDef d = ParseAchDef(ach);
                            if (d.id > 0 && g_AchDefs.find(d.id) == g_AchDefs.end()) {
                                g_AchDefs[d.id] = std::move(d);
                            }
                        }
                    }
                } catch (...) {}
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchNameIndex = std::move(nameIndex);
            g_AchNameIndexReady = !g_AchNameIndex.empty();
            g_AchStatusMsg = "";
        }
        g_AchNameIndexFetching = false;

        if (g_AchNameIndexReady) {
            SaveAchNameIndex();
            SaveAchDefCache();
        }
    }).detach();
}

static void FetchPinnedAchDefs() {
    std::vector<uint32_t> missing;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        for (uint32_t id : g_AchPinned) {
            if (g_AchDefs.find(id) == g_AchDefs.end()) missing.push_back(id);
        }
    }
    if (missing.empty()) return; // All defs cached; progress queried via OnHoardPongForAch

    std::thread([missing]() {
        for (size_t start = 0; start < missing.size(); start += 200) {
            size_t end = std::min(start + 200, missing.size());
            std::string idStr;
            for (size_t i = start; i < end; i++) {
                if (i > start) idStr += ",";
                idStr += std::to_string(missing[i]);
            }
            std::string json = Skinventory::HttpClient::Get(
                "https://api.guildwars2.com/v2/achievements?ids=" + idStr);
            if (!json.empty()) {
                try {
                    auto j = nlohmann::json::parse(json);
                    if (j.is_array()) {
                        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                        for (const auto& ach : j) {
                            AchDef d = ParseAchDef(ach);
                            if (d.id > 0) g_AchDefs[d.id] = std::move(d);
                        }
                    }
                } catch (...) {}
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        SaveAchDefCache();
        // Progress queried via OnHoardPongForAch when H&S is ready
    }).detach();
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: hSelf = hModule; break;
    case DLL_PROCESS_DETACH: break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    }
    return TRUE;
}

// Forward declarations
void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void ProcessKeybind(const char* aIdentifier, bool aIsRelease);
void AddonRender();
void AddonOptions();
void OnMumbleIdentityUpdated(void* eventArgs);

// Icon size for item icons
static const float ICON_SIZE = 40.0f;
static const float SMALL_ICON_SIZE = 24.0f;
static const float TRAIT_ICON_SIZE = 32.0f;
static const float MINOR_TRAIT_SIZE = 26.0f;
static const float SPEC_PORTRAIT_SIZE = 100.0f;

// Equipment slot display order (paper doll layout)
static const char* EQUIPMENT_SLOTS_LEFT[] = {
    "Helm", "Shoulders", "Coat", "Gloves", "Leggings", "Boots"
};
static const char* EQUIPMENT_SLOTS_RIGHT[] = {
    "Backpack", "Accessory1", "Accessory2", "Amulet", "Ring1", "Ring2"
};
static const char* EQUIPMENT_SLOTS_WEAPONS[] = {
    "WeaponA1", "WeaponA2", "WeaponB1", "WeaponB2",
    "WeaponAquaticA", "WeaponAquaticB"
};

static const char* SlotDisplayName(const std::string& slot) {
    if (slot == "Helm") return "Helm";
    if (slot == "Shoulders") return "Shoulders";
    if (slot == "Coat") return "Chest";
    if (slot == "Gloves") return "Gloves";
    if (slot == "Leggings") return "Leggings";
    if (slot == "Boots") return "Boots";
    if (slot == "Backpack") return "Back";
    if (slot == "Accessory1") return "Accessory 1";
    if (slot == "Accessory2") return "Accessory 2";
    if (slot == "Amulet") return "Amulet";
    if (slot == "Ring1") return "Ring 1";
    if (slot == "Ring2") return "Ring 2";
    if (slot == "WeaponA1") return "Weapon A1";
    if (slot == "WeaponA2") return "Weapon A2";
    if (slot == "WeaponB1") return "Weapon B1";
    if (slot == "WeaponB2") return "Weapon B2";
    if (slot == "WeaponAquaticA") return "Aquatic A";
    if (slot == "WeaponAquaticB") return "Aquatic B";
    if (slot == "HelmAquatic") return "Aqua Helm";
    if (slot == "Sickle") return "Sickle";
    if (slot == "Axe") return "Logging Axe";
    if (slot == "Pick") return "Mining Pick";
    if (slot == "FishingRod") return "Fishing Rod";
    if (slot == "FishingBait") return "Bait";
    if (slot == "FishingLure") return "Lure";
    if (slot == "PowerCore") return "Jade Bot Core";
    if (slot == "Relic") return "Relic";
    if (slot == "SensoryArray") return "Sensory Array";
    if (slot == "ServiceChip") return "Service Chip";
    return slot.c_str();
}

// Fetch item/skin detail data for selected character on-demand
static void FetchDetailsForCharacter(const AlterEgo::Character& ch) {
    std::vector<uint32_t> item_ids;
    std::vector<uint32_t> skin_ids;
    std::vector<uint32_t> spec_ids;
    std::vector<uint32_t> stat_ids;

    for (const auto& eq : ch.equipment) {
        if (eq.id != 0) item_ids.push_back(eq.id);
        if (eq.skin != 0) skin_ids.push_back(eq.skin);
        if (eq.stat_id != 0) stat_ids.push_back(eq.stat_id);
        for (auto u : eq.upgrades) if (u != 0) item_ids.push_back(u);
        for (auto inf : eq.infusions) if (inf != 0) item_ids.push_back(inf);
    }

    std::vector<uint32_t> skill_ids;

    for (const auto& bt : ch.build_tabs) {
        for (int i = 0; i < 3; i++) {
            if (bt.specializations[i].spec_id != 0)
                spec_ids.push_back(bt.specializations[i].spec_id);
        }
        // Collect skill IDs
        auto addSkill = [&](uint32_t id) { if (id != 0) skill_ids.push_back(id); };
        addSkill(bt.terrestrial_skills.heal);
        for (int i = 0; i < 3; i++) addSkill(bt.terrestrial_skills.utilities[i]);
        addSkill(bt.terrestrial_skills.elite);
        addSkill(bt.aquatic_skills.heal);
        for (int i = 0; i < 3; i++) addSkill(bt.aquatic_skills.utilities[i]);
        addSkill(bt.aquatic_skills.elite);
    }

    if (!item_ids.empty()) AlterEgo::GW2API::FetchItemDetailsAsync(item_ids);
    if (!skin_ids.empty()) AlterEgo::GW2API::FetchSkinDetailsAsync(skin_ids);
    if (!spec_ids.empty()) AlterEgo::GW2API::FetchSpecDetailsAsync(spec_ids);
    if (!skill_ids.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(skill_ids);
    if (!stat_ids.empty()) AlterEgo::GW2API::FetchItemStatDetailsAsync(stat_ids);

    // Query skin unlock status from H&S cache (instant, no API call)
    if (!skin_ids.empty()) AlterEgo::GW2API::QuerySkinUnlocks(skin_ids);

    // Fetch profession icon for equipment panel silhouette
    if (!ch.profession.empty()) AlterEgo::GW2API::FetchProfessionInfoAsync(ch.profession);
}

// Reject any string that contains path-traversal characters before joining
// it into a filesystem path. Character names come from MumbleLink / H&S and
// SHOULD never contain these, but we don't want a malformed source to escape
// the portrait directory.
static bool IsSafePathComponent(const std::string& s) {
    if (s.empty()) return false;
    if (s == "." || s == "..") return false;
    for (char c : s) {
        if (c == '/' || c == '\\' || c == ':' || c == '\0') return false;
    }
    return true;
}

// Resolve and load a per-character portrait texture (PNG/JPG from AlterEgo/portraits/<name>.<ext>)
// Returns nullptr if not present; caches result. Same backing data as the Equipment-panel overlay.
static Texture_t* GetCharacterPortraitTexture(const AlterEgo::Character& ch) {
    if (!IsSafePathComponent(ch.name)) return nullptr;
    auto cacheIt = s_portraitPathCache.find(ch.name);
    if (cacheIt == s_portraitPathCache.end()) {
        if (s_portraitMissing.find(ch.name) != s_portraitMissing.end()) return nullptr;
        std::string portraitDir = AlterEgo::GW2API::GetDataDirectory() + "/portraits";
        std::filesystem::create_directories(portraitDir);
        static const char* exts[] = { ".png", ".jpg", ".jpeg" };
        bool found = false;
        for (const char* ext : exts) {
            std::string path = portraitDir + "/" + ch.name + ext;
            std::error_code ec;
            if (std::filesystem::is_regular_file(std::filesystem::u8path(path), ec)) {
                int64_t modTime = GetFileModTime(path);
                s_portraitPathCache[ch.name] = { path, modTime };
                found = true;
                break;
            }
        }
        if (!found) {
            s_portraitMissing.insert(ch.name);
            return nullptr;
        }
        cacheIt = s_portraitPathCache.find(ch.name);
    }
    std::string texId = "AE_PORTRAIT_" + ch.name + "_" + std::to_string(cacheIt->second.modTime);
    Texture_t* tex = APIDefs->Textures_Get(texId.c_str());
    if (!tex || !tex->Resource) {
        try {
            auto fspath = std::filesystem::u8path(cacheIt->second.path);
            std::ifstream ifs(fspath, std::ios::binary | std::ios::ate);
            if (ifs.is_open()) {
                auto sz = ifs.tellg();
                if (sz > 0) {
                    std::vector<uint8_t> buf((size_t)sz);
                    ifs.seekg(0);
                    ifs.read(reinterpret_cast<char*>(buf.data()), sz);
                    tex = APIDefs->Textures_GetOrCreateFromMemory(
                        texId.c_str(), buf.data(), (uint64_t)buf.size());
                }
            }
        } catch (...) {}
    }
    return (tex && tex->Resource) ? tex : nullptr;
}

// Render a right-aligned profession emblem (the wiki "tango" icon fetched via the GW2 API).
// Draws a faint dark backing + gold-ish frame; shows a tooltip with the profession name.
static void RenderProfessionEmblem(const std::string& profession, float size) {
    if (profession.empty()) { ImGui::Dummy(ImVec2(size, size)); return; }

    // Trigger fetch if we don't already have it
    AlterEgo::GW2API::FetchProfessionInfoAsync(profession);
    const auto* prof = AlterEgo::GW2API::GetProfessionInfo(profession);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 maxp(pos.x + size, pos.y + size);

    // Backing card (subtle, so it reads as part of the header)
    dl->AddRectFilled(pos, maxp, IM_COL32(20, 20, 26, 180), 4.0f);
    dl->AddRect(pos, maxp, IM_COL32(140, 113, 55, 160), 4.0f, 0, 1.0f);

    bool drew = false;
    if (prof && !prof->icon_big_url.empty()) {
        // Use a stable synthetic ID so IconManager caches the texture by profession
        uint32_t profIconId = 9100000u + (uint32_t)(std::hash<std::string>{}(profession) % 100000u);
        Texture_t* tex = AlterEgo::IconManager::GetIcon(profIconId);
        if (tex && tex->Resource) {
            float ir = size - 8.0f;
            float ix = pos.x + (size - ir) * 0.5f;
            float iy = pos.y + (size - ir) * 0.5f;
            dl->AddImage(tex->Resource,
                ImVec2(ix, iy), ImVec2(ix + ir, iy + ir),
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32(240, 226, 190, 230));
            drew = true;
        } else {
            AlterEgo::IconManager::RequestIcon(profIconId, prof->icon_big_url);
        }
    }
    if (!drew) {
        // Placeholder: profession initial in dim gold
        std::string init = profession.substr(0, 1);
        ImVec2 ts = ImGui::CalcTextSize(init.c_str());
        dl->AddText(ImVec2(pos.x + (size - ts.x) * 0.5f, pos.y + (size - ts.y) * 0.5f),
            IM_COL32(180, 160, 110, 200), init.c_str());
    }

    ImGui::Dummy(ImVec2(size, size));
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(GetProfessionColor(profession), "%s", profession.c_str());
        ImGui::EndTooltip();
    }
}

// Render a circular character avatar (size px). Uses per-character portrait if available,
// falls back to the profession icon. Draws a gold ring around it.
static void RenderCharacterAvatar(const AlterEgo::Character& ch, float size) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);
    float r = size * 0.5f;

    // Background
    dl->AddCircleFilled(center, r, IM_COL32(20, 20, 26, 255), 32);

    Texture_t* tex = GetCharacterPortraitTexture(ch);
    bool drewImage = false;
    if (tex && tex->Resource) {
        // Use AddImageRounded with rounding == radius for circular crop
        dl->AddImageRounded(tex->Resource,
            ImVec2(center.x - r, center.y - r),
            ImVec2(center.x + r, center.y + r),
            ImVec2(0, 0), ImVec2(1, 1),
            IM_COL32_WHITE, r, ImDrawCornerFlags_All);
        drewImage = true;
    } else {
        // Fallback: profession icon
        const auto* prof = AlterEgo::GW2API::GetProfessionInfo(ch.profession);
        if (prof && !prof->icon_url.empty()) {
            uint32_t profIconId = 9000000 + std::hash<std::string>{}(ch.profession) % 100000;
            Texture_t* ptex = AlterEgo::IconManager::GetIcon(profIconId);
            if (ptex && ptex->Resource) {
                // Tinted, inset slightly so the gold ring shows
                float ir = r * 0.78f;
                dl->AddImageRounded(ptex->Resource,
                    ImVec2(center.x - ir, center.y - ir),
                    ImVec2(center.x + ir, center.y + ir),
                    ImVec2(0, 0), ImVec2(1, 1),
                    IM_COL32(230, 220, 180, 255), ir, ImDrawCornerFlags_All);
                drewImage = true;
            } else {
                AlterEgo::IconManager::RequestIcon(profIconId, prof->icon_url);
            }
        }
    }

    if (!drewImage) {
        // Initial placeholder — first letter
        std::string initial = ch.name.empty() ? "?" : ch.name.substr(0, 1);
        ImVec2 ts = ImGui::CalcTextSize(initial.c_str());
        dl->AddText(ImVec2(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f),
            IM_COL32(180, 160, 110, 220), initial.c_str());
    }

    // Gold ring
    dl->AddCircle(center, r, IM_COL32(197, 161, 85, 220), 32, 1.5f);
    dl->AddCircle(center, r - 1.5f, IM_COL32(0, 0, 0, 90), 32, 1.0f);

    ImGui::Dummy(ImVec2(size, size));
}

// Build a one-line subtitle for an equipped item: "Stat • Sigil/Rune" (truncated where needed).
// Returns empty string if neither piece of info is available.
static std::string BuildEquipSubtitle(const AlterEgo::EquipmentItem* eq) {
    if (!eq || eq->id == 0) return "";
    std::string out;

    // Stat name (e.g. "Berserker", from "Berserker's")
    if (eq->stat_id != 0) {
        const auto* si = AlterEgo::GW2API::GetItemStatInfo(eq->stat_id);
        if (si && !si->name.empty()) {
            out = si->name;
            if (!out.empty() && out.back() == 's' && out.size() > 2 &&
                out[out.size() - 2] == '\'') {
                out.erase(out.size() - 2);
            }
        }
    } else {
        const auto* item_info = AlterEgo::GW2API::GetItemInfo(eq->id);
        if (item_info && item_info->details.is_object() &&
            item_info->details.contains("infix_upgrade") &&
            item_info->details["infix_upgrade"].contains("id")) {
            uint32_t fixedStatId = item_info->details["infix_upgrade"]["id"].get<uint32_t>();
            const auto* si = AlterEgo::GW2API::GetItemStatInfo(fixedStatId);
            if (si && !si->name.empty()) {
                out = si->name;
                if (!out.empty() && out.back() == 's' && out.size() > 2 &&
                    out[out.size() - 2] == '\'') {
                    out.erase(out.size() - 2);
                }
            }
        }
    }

    // First upgrade (rune for armor, sigil for weapon) — short form
    if (!eq->upgrades.empty() && eq->upgrades[0] != 0) {
        const auto* uinfo = AlterEgo::GW2API::GetItemInfo(eq->upgrades[0]);
        if (uinfo && !uinfo->name.empty()) {
            std::string upgradeShort = uinfo->name;
            // Strip "Superior Rune of " / "Superior Sigil of " / "the "
            static const char* prefixes[] = {
                "Superior Rune of the ", "Superior Rune of ",
                "Superior Sigil of the ", "Superior Sigil of ",
                "Major Rune of the ", "Major Rune of ",
                "Major Sigil of the ", "Major Sigil of "
            };
            for (const char* p : prefixes) {
                size_t plen = strlen(p);
                if (upgradeShort.rfind(p, 0) == 0) {
                    upgradeShort = upgradeShort.substr(plen);
                    break;
                }
            }
            if (!out.empty()) out += " \xe2\x80\xa2 "; // bullet
            else out = "";
            out += upgradeShort;
        }
    }
    return out;
}

// Render a single equipment slot with icon and tooltip
static void RenderEquipmentSlot(const AlterEgo::EquipmentItem* eq, const char* slotName) {
    ImGui::PushID(slotName);

    if (!eq || eq->id == 0) {
        // Empty slot
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
            IM_COL32(40, 40, 40, 180));
        ImGui::GetWindowDrawList()->AddRect(
            pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
            IM_COL32(80, 80, 80, 200));
        ImGui::Dummy(ImVec2(ICON_SIZE, ICON_SIZE));
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s (Empty)", slotName);
            ImGui::EndTooltip();
        }
    } else {
        // Show skin icon when available, item icon as fallback
        const AlterEgo::ItemInfo* item_info = AlterEgo::GW2API::GetItemInfo(eq->id);
        const AlterEgo::SkinInfo* skin_info = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;

        // Use offset ID for skin icons to avoid cache collision with item icons
        uint32_t display_id;
        std::string icon_url;
        std::string display_name = slotName;
        std::string rarity = "Basic";

        if (skin_info) {
            display_id = eq->skin + 5000000;
            icon_url = skin_info->icon_url;
            display_name = skin_info->name;
            rarity = skin_info->rarity;
        } else {
            display_id = eq->id;
            if (item_info) {
                icon_url = item_info->icon_url;
                display_name = item_info->name;
                rarity = item_info->rarity;
            }
        }

        // Use item rarity for border color (actual item, not skin)
        if (item_info) rarity = item_info->rarity;

        Texture_t* tex = AlterEgo::IconManager::GetIcon(display_id);
        if (tex && tex->Resource) {
            ImVec4 borderColor = GetRarityColor(rarity);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::Image(tex->Resource, ImVec2(ICON_SIZE, ICON_SIZE));
            ImGui::GetWindowDrawList()->AddRect(
                pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
                ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, 2.0f);
        } else {
            ImVec4 col = GetRarityColor(rarity);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
                ImGui::ColorConvertFloat4ToU32(ImVec4(col.x * 0.3f, col.y * 0.3f, col.z * 0.3f, 0.5f)));
            ImGui::GetWindowDrawList()->AddRect(
                pos, ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE),
                ImGui::ColorConvertFloat4ToU32(col), 0.0f, 0, 1.0f);
            ImGui::Dummy(ImVec2(ICON_SIZE, ICON_SIZE));

            if (!icon_url.empty())
                AlterEgo::IconManager::RequestIcon(display_id, icon_url);
        }

        // Tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();

            if (tex && tex->Resource) {
                ImGui::Image(tex->Resource, ImVec2(48, 48));
                ImGui::SameLine();
            }
            ImGui::BeginGroup();
            ImGui::TextColored(GetRarityColor(rarity), "%s", display_name.c_str());

            // Defense or weapon strength from item details
            if (item_info && item_info->details.is_object()) {
                const auto& det = item_info->details;
                if (det.contains("defense") && det["defense"].get<int>() > 0) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Defense: %d",
                        det["defense"].get<int>());
                }
                if (det.contains("min_power") && det.contains("max_power")) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Weapon Strength: %d - %d",
                        det["min_power"].get<int>(), det["max_power"].get<int>());
                }
            }

            // Stat type name (e.g. "Berserker's", "Minstrel's")
            if (eq->stat_id != 0) {
                const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(eq->stat_id);
                if (statInfo && !statInfo->name.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), "%s", statInfo->name.c_str());
                }
            } else if (item_info && item_info->details.is_object() &&
                       item_info->details.contains("infix_upgrade") &&
                       item_info->details["infix_upgrade"].contains("id")) {
                uint32_t fixedStatId = item_info->details["infix_upgrade"]["id"].get<uint32_t>();
                const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(fixedStatId);
                if (statInfo && !statInfo->name.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), "%s", statInfo->name.c_str());
                }
            }

            // Attribute bonuses
            if (!eq->attributes.empty()) {
                for (const auto& [attr, val] : eq->attributes) {
                    // Format attribute name for display (CamelCase → spaced)
                    std::string displayAttr = attr;
                    if (attr == "BoonDuration") displayAttr = "Concentration";
                    else if (attr == "ConditionDuration") displayAttr = "Expertise";
                    else if (attr == "ConditionDamage") displayAttr = "Condition Damage";
                    else if (attr == "CritDamage") displayAttr = "Ferocity";
                    else if (attr == "HealingPower") displayAttr = "Healing Power";
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "+%d %s", val, displayAttr.c_str());
                }
            } else if (item_info && item_info->details.is_object() &&
                       item_info->details.contains("infix_upgrade") &&
                       item_info->details["infix_upgrade"].contains("attributes")) {
                // Fallback: use fixed stats from item details
                for (const auto& attr : item_info->details["infix_upgrade"]["attributes"]) {
                    std::string name = attr.value("attribute", "");
                    int mod = attr.value("modifier", 0);
                    if (mod > 0 && !name.empty()) {
                        std::string displayAttr = name;
                        if (name == "BoonDuration") displayAttr = "Concentration";
                        else if (name == "ConditionDuration") displayAttr = "Expertise";
                        else if (name == "ConditionDamage") displayAttr = "Condition Damage";
                        else if (name == "CritDamage") displayAttr = "Ferocity";
                        else if (name == "HealingPower") displayAttr = "Healing Power";
                        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "+%d %s", mod, displayAttr.c_str());
                    }
                }
            }

            // Infusions (name only)
            for (auto inf_id : eq->infusions) {
                if (inf_id == 0) continue;
                const auto* iinfo = AlterEgo::GW2API::GetItemInfo(inf_id);
                if (iinfo) {
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "%s", iinfo->name.c_str());
                }
            }

            // Upgrades (Runes/Sigils)
            for (auto upgrade_id : eq->upgrades) {
                if (upgrade_id == 0) continue;
                const auto* uinfo = AlterEgo::GW2API::GetItemInfo(upgrade_id);
                if (uinfo) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.9f, 1.0f), "%s", uinfo->name.c_str());
                    // Show rune bonuses
                    if (uinfo->details.is_object() && uinfo->details.contains("bonuses") &&
                        uinfo->details["bonuses"].is_array()) {
                        int idx = 1;
                        for (const auto& bonus : uinfo->details["bonuses"]) {
                            ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.9f, 1.0f), "(%d): %s",
                                idx++, bonus.get<std::string>().c_str());
                        }
                    }
                    // Show sigil description
                    if (uinfo->details.is_object() && uinfo->details.contains("infix_upgrade") &&
                        uinfo->details["infix_upgrade"].contains("buff") &&
                        uinfo->details["infix_upgrade"]["buff"].contains("description")) {
                        ImGui::PushTextWrapPos(300.0f);
                        std::string udesc = StripGW2Markup(uinfo->details["infix_upgrade"]["buff"]["description"].get<std::string>());
                        ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.9f, 1.0f), "%s", udesc.c_str());
                        ImGui::PopTextWrapPos();
                    }
                }
            }

            // Transmuted skin — show original item name (the item before reskin)
            if (skin_info && item_info && skin_info->name != item_info->name) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Transmuted");
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", item_info->name.c_str());
            }

            // Rarity + type
            if (item_info) {
                ImGui::Spacing();
                ImGui::TextColored(GetRarityColor(rarity), "%s", item_info->rarity.c_str());
                if (item_info->details.is_object()) {
                    std::string subtype;
                    if (item_info->details.contains("weight_class"))
                        subtype = item_info->details["weight_class"].get<std::string>() + " ";
                    if (item_info->details.contains("type"))
                        subtype += WeaponDisplayName(item_info->details["type"].get<std::string>());
                    else
                        subtype += WeaponDisplayName(item_info->type);
                    if (!subtype.empty())
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", subtype.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", WeaponDisplayName(item_info->type));
                }
            }

            // Description (skip for legendary relics — API returns a stale emulated effect)
            bool isLegendaryRelic = item_info && item_info->type == "Relic" && item_info->rarity == "Legendary";
            if (item_info && !item_info->description.empty() && !isLegendaryRelic) {
                ImGui::PushTextWrapPos(300.0f);
                std::string idesc = StripGW2Markup(item_info->description);
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 1.0f), "%s", idesc.c_str());
                ImGui::PopTextWrapPos();
            }

            // Legendary relic API limitation note
            if (isLegendaryRelic) {
                ImGui::Spacing();
                ImGui::PushTextWrapPos(300.0f);
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.4f, 1.0f),
                    "The API does not expose which relic effect\nthis legendary relic is emulating.");
                ImGui::PopTextWrapPos();
            }

            // Skin unlock status — only show if locked (unlocked is implied for equipped items)
            if (eq->skin != 0 && AlterEgo::GW2API::HasSkinUnlockData(eq->skin) &&
                !AlterEgo::GW2API::IsSkinUnlocked(eq->skin)) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Skin Locked");
            }

            // Dyes (limit to skin's actual dye channel count, use material-specific RGB)
            if (!eq->dyes.empty()) {
                int maxDyes = (int)eq->dyes.size();
                if (skin_info && skin_info->dye_slot_count >= 0) {
                    maxDyes = std::min(maxDyes, skin_info->dye_slot_count);
                }
                bool hasAny = false;
                for (int di = 0; di < maxDyes; di++) {
                    if (eq->dyes[di] == 0) continue;
                    const auto* dye = AlterEgo::GW2API::GetDyeColor(eq->dyes[di]);
                    if (dye) {
                        if (!hasAny) { ImGui::Spacing(); hasAny = true; }
                        // Pick material-specific RGB; default to cloth when no skin material info
                        int cr = dye->cloth_r, cg = dye->cloth_g, cb = dye->cloth_b;
                        if (skin_info && di < (int)skin_info->dye_slot_materials.size()) {
                            const auto& mat = skin_info->dye_slot_materials[di];
                            if (mat == "leather") { cr = dye->leather_r; cg = dye->leather_g; cb = dye->leather_b; }
                            else if (mat == "metal") { cr = dye->metal_r; cg = dye->metal_g; cb = dye->metal_b; }
                        }
                        ImVec4 dyeCol((float)cr / 255.0f, (float)cg / 255.0f,
                                      (float)cb / 255.0f, 1.0f);
                        ImGui::ColorButton("##dye", dyeCol, 0, ImVec2(12, 12));
                        ImGui::SameLine();
                        ImGui::Text("%s", dye->name.c_str());
                    }
                }
            }

            ImGui::EndGroup();
            ImGui::EndTooltip();
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextItem("##eq_ctx")) {
            if (item_info) {
                if (ImGui::MenuItem("Copy Item Chat Link")) {
                    std::string link = AlterEgo::ChatLink::EncodeItem(eq->id, 1, eq->skin,
                        eq->upgrades.size() > 0 ? eq->upgrades[0] : 0,
                        eq->upgrades.size() > 1 ? eq->upgrades[1] : 0);
                    CopyToClipboard(link);
                }
                if (!item_info->chat_link.empty()) {
                    if (ImGui::MenuItem("Copy API Chat Link")) {
                        CopyToClipboard(item_info->chat_link);
                    }
                }
            }
            if (skin_info) {
                if (ImGui::MenuItem("Copy Skin Chat Link")) {
                    std::string link = AlterEgo::ChatLink::EncodeSkin(eq->skin);
                    CopyToClipboard(link);
                }
            }
            ImGui::Separator();
            // Find item in H&S (opens H&S search window)
            if (item_info && !item_info->name.empty()) {
                if (ImGui::MenuItem("Find in Hoard & Seek")) {
                    APIDefs->Events_Raise(EV_HOARD_SEARCH, (void*)item_info->name.c_str());
                }
            }
            // Show item locations from H&S cache
            if (item_info) {
                const auto* loc = AlterEgo::GW2API::GetItemLocation(eq->id);
                if (loc) {
                    if (ImGui::BeginMenu("Item Locations")) {
                        ImGui::Text("Total: %d", loc->total_count);
                        ImGui::Separator();
                        for (const auto& entry : loc->locations) {
                            ImGui::Text("%s", entry.location.c_str());
                            ImGui::SameLine(150);
                            if (!entry.sublocation.empty()) {
                                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", entry.sublocation.c_str());
                                ImGui::SameLine(280);
                            }
                            ImGui::Text("x%d", entry.count);
                        }
                        ImGui::EndMenu();
                    }
                } else {
                    if (ImGui::MenuItem("Query Item Locations")) {
                        AlterEgo::GW2API::QueryItemLocation(eq->id);
                    }
                }
            }
            ImGui::EndPopup();
        }
    }

    ImGui::PopID();
}

// Find equipment for a slot + tab
static const AlterEgo::EquipmentItem* FindEquipment(
    const AlterEgo::Character& ch, const std::string& slot, int tab)
{
    for (const auto& eq : ch.equipment) {
        if (eq.slot == slot && (eq.tab == tab || eq.tab == 0)) {
            return &eq;
        }
    }
    return nullptr;
}

// Helper: render one equipment row with label LEFT of icon (for right column)
// Shared resolution of display name + rarity for an equipped slot
static void ResolveEquipDisplay(const AlterEgo::EquipmentItem* eq, const char* slot,
                                std::string& outName, std::string& outRarity) {
    outName = "---";
    outRarity = "Basic";
    if (!eq || eq->id == 0) {
        outName = SlotDisplayName(slot);
        return;
    }
    const auto* info = AlterEgo::GW2API::GetItemInfo(eq->id);
    const auto* skin = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;
    if (info) outRarity = info->rarity;
    if (std::string(slot) == "Relic" && info && info->rarity == "Legendary") {
        outName = "Legendary Relic";
    } else if (skin) {
        outName = skin->name;
    } else if (info) {
        outName = info->name;
    } else {
        outName = SlotDisplayName(slot);
    }
}

// Truncate a string with "..." so its rendered width fits maxW.
static std::string TruncateToWidth(const std::string& s, float maxW) {
    if (ImGui::CalcTextSize(s.c_str()).x <= maxW) return s;
    const char* ell = "\xe2\x80\xa6"; // …
    float ellW = ImGui::CalcTextSize(ell).x;
    int lo = 0, hi = (int)s.size();
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        if (ImGui::CalcTextSize(s.substr(0, mid).c_str()).x + ellW <= maxW) lo = mid;
        else hi = mid - 1;
    }
    return s.substr(0, lo) + ell;
}

// Card-style equipment row. dir = +1 (icon-left), -1 (icon-right / trinkets column).
static void RenderEquipCard(const AlterEgo::Character& ch, const char* slot, int tab, int dir) {
    const auto* eq = FindEquipment(ch, slot, tab);
    std::string name, rarity;
    ResolveEquipDisplay(eq, slot, name, rarity);
    bool hasItem = (eq && eq->id != 0);
    std::string subtitle = hasItem ? BuildEquipSubtitle(eq) : "";

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float availW = ImGui::GetContentRegionAvail().x;
    float cardH = ICON_SIZE + 6.0f;
    ImVec2 cardMin = ImGui::GetCursorScreenPos();
    ImVec2 cardMax(cardMin.x + availW, cardMin.y + cardH);

    // Card background — solid at the outer edge, fades to transparent toward the centre
    // so the character portrait shows through the middle of the panel.
    if (hasItem) {
        ImU32 outerCol = IM_COL32(28, 26, 32, 215);
        ImU32 innerCol = IM_COL32(28, 26, 32, 25);
        if (dir > 0) {
            dl->AddRectFilledMultiColor(cardMin, cardMax, outerCol, innerCol, innerCol, outerCol);
        } else {
            dl->AddRectFilledMultiColor(cardMin, cardMax, innerCol, outerCol, outerCol, innerCol);
        }
    } else {
        // Empty slots: very faint, same fade direction
        ImU32 outerCol = IM_COL32(20, 19, 24, 80);
        ImU32 innerCol = IM_COL32(20, 19, 24, 0);
        if (dir > 0) {
            dl->AddRectFilledMultiColor(cardMin, cardMax, outerCol, innerCol, innerCol, outerCol);
        } else {
            dl->AddRectFilledMultiColor(cardMin, cardMax, innerCol, outerCol, outerCol, innerCol);
        }
    }

    // Rarity accent strip on the outer edge only
    if (hasItem) {
        ImVec4 rcol = GetRarityColor(rarity);
        ImU32 raccent = ImGui::ColorConvertFloat4ToU32(ImVec4(rcol.x, rcol.y, rcol.z, 0.70f));
        if (dir > 0) {
            dl->AddRectFilled(cardMin, ImVec2(cardMin.x + 2.0f, cardMax.y), raccent);
        } else {
            dl->AddRectFilled(ImVec2(cardMax.x - 2.0f, cardMin.y), cardMax, raccent);
        }
    }

    float pad = 4.0f;
    float textGap = 6.0f;
    float textW = availW - ICON_SIZE - pad * 2 - textGap;
    float lineH = ImGui::GetTextLineHeight();

    if (dir > 0) {
        // Icon on left
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + pad, cardMin.y + 3.0f));
        RenderEquipmentSlot(eq, SlotDisplayName(slot));

        if (hasItem) {
            float textX = cardMin.x + pad + ICON_SIZE + textGap;
            std::string nTrunc = TruncateToWidth(name, textW);
            std::string sTrunc = subtitle.empty() ? "" : TruncateToWidth(subtitle, textW);
            // If subtitle would lose more than half its content, drop it entirely
            if (!sTrunc.empty() && sTrunc != subtitle) {
                if ((float)sTrunc.size() < (float)subtitle.size() * 0.5f) sTrunc.clear();
            }
            float blockH = sTrunc.empty() ? lineH : lineH * 2 + 1.0f;
            float textTopY = cardMin.y + (cardH - blockH) * 0.5f;
            dl->AddText(ImVec2(textX, textTopY),
                ImGui::ColorConvertFloat4ToU32(GetRarityColor(rarity)), nTrunc.c_str());
            if (!sTrunc.empty()) {
                dl->AddText(ImVec2(textX, textTopY + lineH + 1.0f),
                    IM_COL32(150, 142, 120, 230), sTrunc.c_str());
            }
        }
    } else {
        // Text on left, icon on right (trinkets column)
        float iconX = cardMax.x - pad - ICON_SIZE;

        if (hasItem) {
            float textRightX = iconX - textGap;
            std::string nTrunc = TruncateToWidth(name, textW);
            std::string sTrunc = subtitle.empty() ? "" : TruncateToWidth(subtitle, textW);
            if (!sTrunc.empty() && sTrunc != subtitle) {
                if ((float)sTrunc.size() < (float)subtitle.size() * 0.5f) sTrunc.clear();
            }
            float nW = ImGui::CalcTextSize(nTrunc.c_str()).x;
            float sW = sTrunc.empty() ? 0.0f : ImGui::CalcTextSize(sTrunc.c_str()).x;
            float blockH = sTrunc.empty() ? lineH : lineH * 2 + 1.0f;
            float textTopY = cardMin.y + (cardH - blockH) * 0.5f;
            dl->AddText(ImVec2(textRightX - nW, textTopY),
                ImGui::ColorConvertFloat4ToU32(GetRarityColor(rarity)), nTrunc.c_str());
            if (!sTrunc.empty()) {
                dl->AddText(ImVec2(textRightX - sW, textTopY + lineH + 1.0f),
                    IM_COL32(150, 142, 120, 230), sTrunc.c_str());
            }
        }
        ImGui::SetCursorScreenPos(ImVec2(iconX, cardMin.y + 3.0f));
        RenderEquipmentSlot(eq, SlotDisplayName(slot));
    }

    // Advance cursor below the card
    ImGui::SetCursorScreenPos(ImVec2(cardMin.x, cardMax.y + 3.0f));
}

static void RenderEquipRowReverse(const AlterEgo::Character& ch, const char* slot, int tab) {
    RenderEquipCard(ch, slot, tab, -1);
}

// Helper: render one equipment row (icon + name) for the paper-doll grid
static void RenderEquipRow(const AlterEgo::Character& ch, const char* slot, int tab) {
    RenderEquipCard(ch, slot, tab, +1);
}

// Draw a race silhouette as a semi-transparent background overlay
static void DrawRaceSilhouette(ImDrawList* dl, const std::string& race, const std::string& gender,
                                ImVec2 center, float height) {
    ImU32 col = IM_COL32(255, 255, 255, 25); // Very faint white
    float s = height; // total height scale
    bool female = (gender == "Female");

    // Helper: offset from center
    auto P = [&](float x, float y) -> ImVec2 {
        return ImVec2(center.x + x * s, center.y + y * s);
    };

    if (race == "Asura") {
        // Large head, big ears, tiny body
        float headR = 0.18f;
        dl->AddCircleFilled(P(0, -0.30f), headR * s, col, 24);
        // Ears (triangles)
        dl->AddTriangleFilled(P(-0.15f, -0.35f), P(-0.28f, -0.48f), P(-0.10f, -0.44f), col);
        dl->AddTriangleFilled(P(0.15f, -0.35f), P(0.28f, -0.48f), P(0.10f, -0.44f), col);
        // Eyes (dark spots)
        ImU32 eyeCol = IM_COL32(0, 0, 0, 40);
        dl->AddCircleFilled(P(-0.07f, -0.32f), 0.035f * s, eyeCol, 12);
        dl->AddCircleFilled(P(0.07f, -0.32f), 0.035f * s, eyeCol, 12);
        // Body (small)
        float bw = female ? 0.09f : 0.11f;
        ImVec2 body[] = { P(-bw, -0.12f), P(bw, -0.12f), P(bw * 0.8f, 0.15f), P(-bw * 0.8f, 0.15f) };
        dl->AddConvexPolyFilled(body, 4, col);
        // Arms
        ImVec2 armL[] = { P(-bw, -0.10f), P(-bw - 0.06f, -0.08f), P(-bw - 0.08f, 0.10f), P(-bw, 0.08f) };
        ImVec2 armR[] = { P(bw, -0.10f), P(bw + 0.06f, -0.08f), P(bw + 0.08f, 0.10f), P(bw, 0.08f) };
        dl->AddConvexPolyFilled(armL, 4, col);
        dl->AddConvexPolyFilled(armR, 4, col);
        // Legs
        ImVec2 legL[] = { P(-bw * 0.7f, 0.15f), P(-0.02f, 0.15f), P(-0.03f, 0.38f), P(-bw * 0.8f, 0.38f) };
        ImVec2 legR[] = { P(0.02f, 0.15f), P(bw * 0.7f, 0.15f), P(bw * 0.8f, 0.38f), P(0.03f, 0.38f) };
        dl->AddConvexPolyFilled(legL, 4, col);
        dl->AddConvexPolyFilled(legR, 4, col);
        // Feet (oversized)
        dl->AddCircleFilled(P(-0.06f, 0.40f), 0.04f * s, col, 12);
        dl->AddCircleFilled(P(0.06f, 0.40f), 0.04f * s, col, 12);
    }
    else if (race == "Charr") {
        // Hunched, horns, snout, tail, digitigrade legs
        float headR = 0.10f;
        ImVec2 headC = P(0.02f, -0.32f);
        dl->AddCircleFilled(headC, headR * s, col, 20);
        // Horns
        dl->AddTriangleFilled(P(-0.06f, -0.40f), P(-0.14f, -0.50f), P(-0.02f, -0.42f), col);
        dl->AddTriangleFilled(P(0.10f, -0.40f), P(0.18f, -0.50f), P(0.06f, -0.42f), col);
        // Snout
        dl->AddTriangleFilled(P(-0.02f, -0.28f), P(0.06f, -0.28f), P(0.02f, -0.22f), col);
        // Hunched torso
        float sw = female ? 0.14f : 0.17f;
        ImVec2 torso[] = { P(-sw, -0.22f), P(sw, -0.22f), P(sw * 0.7f, 0.12f), P(-sw * 0.7f, 0.12f) };
        dl->AddConvexPolyFilled(torso, 4, col);
        // Arms (thick)
        ImVec2 armL[] = { P(-sw, -0.20f), P(-sw - 0.07f, -0.16f), P(-sw - 0.05f, 0.08f), P(-sw + 0.02f, 0.06f) };
        ImVec2 armR[] = { P(sw, -0.20f), P(sw + 0.07f, -0.16f), P(sw + 0.05f, 0.08f), P(sw - 0.02f, 0.06f) };
        dl->AddConvexPolyFilled(armL, 4, col);
        dl->AddConvexPolyFilled(armR, 4, col);
        // Digitigrade legs
        ImVec2 legL[] = { P(-0.10f, 0.12f), P(-0.02f, 0.12f), P(-0.06f, 0.30f), P(-0.14f, 0.28f) };
        ImVec2 legR[] = { P(0.02f, 0.12f), P(0.10f, 0.12f), P(0.14f, 0.28f), P(0.06f, 0.30f) };
        dl->AddConvexPolyFilled(legL, 4, col);
        dl->AddConvexPolyFilled(legR, 4, col);
        // Lower legs (reversed knee)
        ImVec2 lowerL[] = { P(-0.14f, 0.28f), P(-0.06f, 0.30f), P(-0.10f, 0.46f), P(-0.16f, 0.44f) };
        ImVec2 lowerR[] = { P(0.06f, 0.30f), P(0.14f, 0.28f), P(0.16f, 0.44f), P(0.10f, 0.46f) };
        dl->AddConvexPolyFilled(lowerL, 4, col);
        dl->AddConvexPolyFilled(lowerR, 4, col);
        // Tail
        ImVec2 tail[] = { P(-0.08f, 0.08f), P(-0.06f, 0.12f), P(-0.24f, 0.20f), P(-0.26f, 0.16f) };
        dl->AddConvexPolyFilled(tail, 4, col);
    }
    else {
        // Human / Norn / Sylvari — humanoid with race-specific proportions
        float headR, shoulderW, hipW, legLen, neckY;
        if (race == "Norn") {
            headR = 0.09f;
            shoulderW = female ? 0.15f : 0.19f;
            hipW = female ? 0.13f : 0.14f;
            legLen = 0.22f;
            neckY = -0.33f;
        } else if (race == "Sylvari") {
            headR = 0.08f;
            shoulderW = female ? 0.11f : 0.13f;
            hipW = female ? 0.10f : 0.10f;
            legLen = 0.22f;
            neckY = -0.34f;
        } else { // Human
            headR = 0.085f;
            shoulderW = female ? 0.12f : 0.15f;
            hipW = female ? 0.11f : 0.11f;
            legLen = 0.21f;
            neckY = -0.33f;
        }

        // Head
        dl->AddCircleFilled(P(0, neckY - headR * 0.8f), headR * s, col, 24);

        // Sylvari leaf crest
        if (race == "Sylvari") {
            dl->AddTriangleFilled(P(-0.04f, neckY - headR * 2.0f),
                                   P(0.04f, neckY - headR * 2.0f),
                                   P(0, neckY - headR * 3.2f), col);
        }

        // Torso
        float torsoBot = 0.08f;
        ImVec2 torso[] = {
            P(-shoulderW, neckY + 0.02f), P(shoulderW, neckY + 0.02f),
            P(hipW, torsoBot), P(-hipW, torsoBot)
        };
        dl->AddConvexPolyFilled(torso, 4, col);

        // Arms
        float armW = 0.045f;
        ImVec2 armL[] = {
            P(-shoulderW, neckY + 0.03f), P(-shoulderW - armW, neckY + 0.05f),
            P(-shoulderW - armW + 0.01f, 0.14f), P(-shoulderW + 0.02f, 0.12f)
        };
        ImVec2 armR[] = {
            P(shoulderW, neckY + 0.03f), P(shoulderW + armW, neckY + 0.05f),
            P(shoulderW + armW - 0.01f, 0.14f), P(shoulderW - 0.02f, 0.12f)
        };
        dl->AddConvexPolyFilled(armL, 4, col);
        dl->AddConvexPolyFilled(armR, 4, col);

        // Legs
        float legW = hipW * 0.55f;
        ImVec2 legL[] = {
            P(-hipW + 0.01f, torsoBot), P(-0.02f, torsoBot),
            P(-0.03f, torsoBot + legLen), P(-hipW, torsoBot + legLen)
        };
        ImVec2 legR[] = {
            P(0.02f, torsoBot), P(hipW - 0.01f, torsoBot),
            P(hipW, torsoBot + legLen), P(0.03f, torsoBot + legLen)
        };
        dl->AddConvexPolyFilled(legL, 4, col);
        dl->AddConvexPolyFilled(legR, 4, col);

        // Feet
        dl->AddCircleFilled(P(-hipW * 0.5f, torsoBot + legLen + 0.01f), 0.03f * s, col, 10);
        dl->AddCircleFilled(P(hipW * 0.5f, torsoBot + legLen + 0.01f), 0.03f * s, col, 10);
    }
}

// Render the equipment panel (game-matching layout)
static void RenderEquipmentPanel(const AlterEgo::Character& ch) {
    // Add padding so icons don't touch the pane edges
    ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor.x + 6, cursor.y + 4));

    // Equipment tab selector
    int maxTab = 1;
    for (const auto& eq : ch.equipment) {
        if (eq.tab > maxTab) maxTab = eq.tab;
    }
    int tab = (g_SelectedEquipTab > 0) ? g_SelectedEquipTab : ch.active_equipment_tab;
    if (tab == 0) tab = 1;

    if (maxTab > 1) {
        std::vector<std::string> labels;
        std::vector<bool> activeMarkers;
        labels.reserve(maxTab);
        activeMarkers.reserve(maxTab);
        for (int t = 1; t <= maxTab; t++) {
            std::string label;
            auto nameIt = ch.equipment_tab_names.find(t);
            if (nameIt != ch.equipment_tab_names.end() && !nameIt->second.empty()) {
                label = nameIt->second;
            } else {
                label = "Tab " + std::to_string(t);
            }
            labels.push_back(label);
            activeMarkers.push_back(ch.active_equipment_tab == t);
        }
        int selIdx = tab - 1;
        if (RenderChipStrip(labels, activeMarkers, selIdx, g_EquipTabScroll, "##equip_chips")) {
            g_SelectedEquipTab = selIdx + 1;
            g_DetailsFetched = false;
        }
    }
    ImGui::Spacing();

    // Record start position for silhouette overlay
    ImVec2 panelStart = ImGui::GetCursorScreenPos();

    // Split draw list: channel 0 = portrait background, channel 1 = equipment foreground
    ImDrawListSplitter splitter;
    splitter.Split(ImGui::GetWindowDrawList(), 2);
    splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 1); // draw content on top

    // Main layout: Left (Armor + Weapons) | Right (Trinkets)
    {
        float totalW = ImGui::GetContentRegionAvail().x;
        float gap = 8.0f;
        float pad = 6.0f;
        float leftW = totalW * 0.54f;

        // ===== LEFT COLUMN: Armor + Weapons =====
        ImGui::BeginGroup();
        ImGui::Indent(pad);

        RenderSectionHeader("Armor", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
        RenderEquipRow(ch, "Helm", tab);
        RenderEquipRow(ch, "Shoulders", tab);
        RenderEquipRow(ch, "Coat", tab);
        RenderEquipRow(ch, "Gloves", tab);
        RenderEquipRow(ch, "Leggings", tab);
        RenderEquipRow(ch, "Boots", tab);

        ImGui::Spacing();

        RenderSectionHeader("Weapons", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
        {
            auto DrawSetLabel = [](const char* roman, float rowH) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 lp = ImGui::GetCursorScreenPos();
                float w = 28.0f;
                float h = rowH;
                ImU32 bgTop = IM_COL32(26, 22, 18, 255);
                ImU32 bgBot = IM_COL32(20, 17, 13, 255);
                dl->AddRectFilledMultiColor(lp, ImVec2(lp.x + w, lp.y + h), bgTop, bgTop, bgBot, bgBot);
                dl->AddRect(lp, ImVec2(lp.x + w, lp.y + h),
                    IM_COL32(70, 60, 40, 140), 3.0f, 0, 1.0f);
                ImVec2 ts = ImGui::CalcTextSize(roman);
                dl->AddText(
                    ImVec2(lp.x + w * 0.5f - ts.x * 0.5f, lp.y + h * 0.5f - ts.y * 0.5f),
                    IM_COL32(197, 161, 85, 255), roman);
                ImGui::Dummy(ImVec2(w, h));
            };

            float rowH = ICON_SIZE + 10.0f;
            float labelW = 28.0f;
            float innerGap = 6.0f;
            float weaponsW = leftW - pad * 2 - labelW - innerGap;
            float primaryW = (weaponsW - 8.0f) * 0.62f;
            float secondaryW = (weaponsW - 8.0f) - primaryW;

            auto RenderWeaponSet = [&](const char* roman, const char* a, const char* b) {
                ImGui::BeginGroup();
                DrawSetLabel(roman, rowH);
                ImGui::EndGroup();
                ImGui::SameLine(0, innerGap);

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

                ImGui::PushID(a);
                ImGui::BeginChild("##wA", ImVec2(primaryW, rowH), false,
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                RenderEquipRow(ch, a, tab);
                ImGui::EndChild();
                ImGui::PopID();

                ImGui::SameLine(0, 8.0f);

                ImGui::PushID(b);
                ImGui::BeginChild("##wB", ImVec2(secondaryW, rowH), false,
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                RenderEquipRow(ch, b, tab);
                ImGui::EndChild();
                ImGui::PopID();

                ImGui::PopStyleVar(2);
            };

            RenderWeaponSet("I",  "WeaponA1", "WeaponA2");
            RenderWeaponSet("II", "WeaponB1", "WeaponB2");
        }

        ImGui::Unindent(pad);
        ImGui::EndGroup();

        ImGui::SameLine(leftW + gap);

        // ===== RIGHT COLUMN: Trinkets =====
        ImGui::BeginGroup();

        {
            float colW = totalW - leftW - gap;
            float headerW = ImGui::CalcTextSize("Trinkets").x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colW - headerW - pad);
            ImGui::TextColored(ImVec4(0.70f, 0.58f, 0.20f, 1.0f), "Trinkets");
            // Accent underline (right-aligned)
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(pos.x + colW * 0.4f, pos.y),
                ImVec2(pos.x + colW - pad, pos.y),
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.70f, 0.58f, 0.20f, 0.30f)), 1.0f);
            ImGui::Spacing();
        }
        RenderEquipRowReverse(ch, "Backpack", tab);
        RenderEquipRowReverse(ch, "Accessory1", tab);
        RenderEquipRowReverse(ch, "Accessory2", tab);
        RenderEquipRowReverse(ch, "Amulet", tab);
        RenderEquipRowReverse(ch, "Ring1", tab);
        RenderEquipRowReverse(ch, "Ring2", tab);
        RenderEquipRowReverse(ch, "Relic", tab);

        ImGui::EndGroup();
    }

    // Draw character portrait (user screenshot) or race concept art as overlay
    {
        ImVec2 panelEnd = ImGui::GetCursorScreenPos();
        float panelH = panelEnd.y - panelStart.y;
        float panelW = ImGui::GetContentRegionAvail().x;

        Texture_t* overlayTex = nullptr;
        float overlayAspect = 0.75f; // default portrait aspect (w/h)
        ImU32 overlayTint = IM_COL32(255, 255, 255, 35); // faint for race art

        // Check for user portrait: AlterEgo/portraits/<Character Name>.{png,jpg,jpeg}
        {

            auto cacheIt = s_portraitPathCache.find(ch.name);
            if (cacheIt != s_portraitPathCache.end()) {
                // Re-check mod time to detect replaced files
                int64_t curMod = GetFileModTime(cacheIt->second.path);
                if (curMod != cacheIt->second.modTime && curMod != 0) {
                    cacheIt->second.modTime = curMod;
                }
                // Include mod time in texture ID to bust Nexus cache on file change
                std::string texId = "AE_PORTRAIT_" + ch.name + "_" + std::to_string(cacheIt->second.modTime);
                // First check if Nexus already has this texture cached
                overlayTex = APIDefs->Textures_Get(texId.c_str());
                if (!overlayTex || !overlayTex->Resource) {
                    // Load file ourselves (handles UTF-8 paths) and pass bytes to Nexus
                    try {
                        auto fspath = std::filesystem::u8path(cacheIt->second.path);
                        std::ifstream ifs(fspath, std::ios::binary | std::ios::ate);
                        if (ifs.is_open()) {
                            auto sz = ifs.tellg();
                            if (sz > 0) {
                                std::vector<uint8_t> buf((size_t)sz);
                                ifs.seekg(0);
                                ifs.read(reinterpret_cast<char*>(buf.data()), sz);
                                overlayTex = APIDefs->Textures_GetOrCreateFromMemory(
                                    texId.c_str(), buf.data(), (uint64_t)buf.size());
                            }
                        }
                    } catch (...) {}
                }
                if (overlayTex && overlayTex->Resource) {
                    if (overlayTex->Height > 0)
                        overlayAspect = (float)overlayTex->Width / (float)overlayTex->Height;
                    overlayTint = IM_COL32(255, 255, 255, 55); // slightly more visible than race art
                }
            } else if (s_portraitMissing.find(ch.name) == s_portraitMissing.end() &&
                       IsSafePathComponent(ch.name)) {
                // Haven't checked yet — scan for portrait file (ensure dir exists)
                std::string portraitDir = AlterEgo::GW2API::GetDataDirectory() + "/portraits";
                std::filesystem::create_directories(portraitDir);
                static const char* exts[] = { ".png", ".jpg", ".jpeg" };
                bool found = false;
                for (const char* ext : exts) {
                    std::string path = portraitDir + "/" + ch.name + ext;
                    std::error_code ec;
                    if (std::filesystem::is_regular_file(std::filesystem::u8path(path), ec)) {
                        int64_t modTime = GetFileModTime(path);
                        s_portraitPathCache[ch.name] = { path, modTime };
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    s_portraitMissing.insert(ch.name);
                }
            }
        }

        // Fallback: race concept art
        if (!overlayTex || !overlayTex->Resource) {
            static const std::unordered_map<std::string, std::pair<uint32_t, std::string>> RACE_ART = {
                {"Asura",   {8000001, "https://wiki.guildwars2.com/images/3/3f/Asura_01_concept_art_%28white%29.jpg"}},
                {"Charr",   {8000002, "https://wiki.guildwars2.com/images/5/55/Charr_01_concept_art_%28white%29.jpg"}},
                {"Human",   {8000003, "https://wiki.guildwars2.com/images/e/e2/Human_01_concept_art_%28white%29.jpg"}},
                {"Norn",    {8000004, "https://wiki.guildwars2.com/images/a/a7/Norn_01_concept_art_%28white%29.jpg"}},
                {"Sylvari", {8000005, "https://wiki.guildwars2.com/images/5/54/Sylvari_01_concept_art_%28white%29.jpg"}}
            };

            auto artIt = RACE_ART.find(ch.race);
            if (artIt != RACE_ART.end()) {
                uint32_t artId = artIt->second.first;
                const std::string& artUrl = artIt->second.second;
                overlayTex = AlterEgo::IconManager::GetIcon(artId);
                if (!overlayTex || !overlayTex->Resource) {
                    AlterEgo::IconManager::RequestIcon(artId, artUrl);
                }
                overlayAspect = 0.75f;
                overlayTint = IM_COL32(255, 255, 255, 35);
            }
        }

        // Draw the overlay image on the background channel
        splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);
        if (overlayTex && overlayTex->Resource && panelH > 40.0f) {
            float artH = panelH * 0.90f;
            float artW = artH * overlayAspect;
            if (artW > panelW * 0.6f) {
                artW = panelW * 0.6f;
                artH = artW / overlayAspect;
            }
            ImVec2 artPos(
                panelStart.x + panelW * 0.5f - artW * 0.5f,
                panelStart.y + panelH * 0.5f - artH * 0.5f
            );
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 artMax(artPos.x + artW, artPos.y + artH);

            dl->AddImage(
                overlayTex->Resource,
                artPos, artMax,
                ImVec2(0, 0), ImVec2(1, 1),
                overlayTint
            );

        }
        splitter.Merge(ImGui::GetWindowDrawList());
    }

    ImGui::Spacing();

    // ===== AQUATIC: Aquabreather + Aquatic Weapons =====
    ImGui::Indent(6.0f);
    RenderSectionHeader("Aquatic", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
    RenderEquipRow(ch, "HelmAquatic", tab);
    {
        float rowH = ICON_SIZE + 6.0f + 4.0f;
        float halfW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

        ImGui::BeginChild("##aqA", ImVec2(halfW, rowH), false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        RenderEquipRow(ch, "WeaponAquaticA", tab);
        ImGui::EndChild();

        ImGui::SameLine(0, 8.0f);

        ImGui::BeginChild("##aqB", ImVec2(halfW, rowH), false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        RenderEquipRow(ch, "WeaponAquaticB", tab);
        ImGui::EndChild();

        ImGui::PopStyleVar(2);
    }
    ImGui::Unindent(6.0f);
}

// Render the build panel
// Draw a marching ants animated line between two points
static void DrawDottedLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 color, float thickness = 1.5f, float dashLen = 4.0f, float gapLen = 3.0f) {
    float dx = b.x - a.x, dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float nx = dx / len, ny = dy / len;
    float cycle = dashLen + gapLen;
    float offset = fmodf((float)ImGui::GetTime() * 21.0f, cycle);
    float pos = offset - cycle;
    while (pos < len) {
        float t0 = pos > 0.0f ? pos : 0.0f;
        float t1 = pos + dashLen < len ? pos + dashLen : len;
        if (t1 > t0)
            dl->AddLine(ImVec2(a.x + nx * t0, a.y + ny * t0),
                        ImVec2(a.x + nx * t1, a.y + ny * t1), color, thickness);
        pos += cycle;
    }
}

// Helper: render a single trait icon with selected/dimmed state and tooltip
// Returns the center screen position of the rendered icon
static ImVec2 RenderTraitIcon(uint32_t trait_id, bool selected, bool isMinor, float size) {
    const auto* tinfo = AlterEgo::GW2API::GetTraitInfo(trait_id);

    ImGui::PushID((int)(trait_id + 2000000));

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);

    Texture_t* tex = AlterEgo::IconManager::GetIcon(trait_id + 2000000);
    if (tex && tex->Resource) {
        if (selected) {
            ImGui::GetWindowDrawList()->AddRect(
                pos,
                ImVec2(pos.x + size, pos.y + size),
                IM_COL32(100, 220, 255, 255), 0.0f, 0, 1.5f);
        }

        if (!selected && !isMinor) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f);
            ImGui::Image(tex->Resource, ImVec2(size, size));
            ImGui::PopStyleVar();
        } else {
            ImGui::Image(tex->Resource, ImVec2(size, size));
        }
    } else {
        ImVec4 col = selected ? ImVec4(0.3f, 0.5f, 0.7f, 0.8f) : ImVec4(0.15f, 0.15f, 0.15f, 0.5f);
        ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size),
            ImGui::ColorConvertFloat4ToU32(col));
        ImGui::Dummy(ImVec2(size, size));

        if (tinfo && !tinfo->icon_url.empty())
            AlterEgo::IconManager::RequestIcon(trait_id + 2000000, tinfo->icon_url);
    }

    if (ImGui::IsItemHovered() && tinfo) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), "%s", tinfo->name.c_str());
        if (isMinor)
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Minor Trait");
        if (!tinfo->description.empty()) {
            ImGui::PushTextWrapPos(300.0f);
            std::string tdesc = StripGW2Markup(tinfo->description);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", tdesc.c_str());
            ImGui::PopTextWrapPos();
        }
        ImGui::EndTooltip();
    }

    ImGui::PopID();
    return center;
}

// =========================================================================
// Hero Challenges Panel
// =========================================================================

struct HCMapEntry {
    std::string map_name;
    int total = 0;
    int completed = 0;
};

struct HCExpansionEntry {
    std::string expansion;
    std::vector<HCMapEntry> maps;
    int total = 0;
    int completed = 0;
};

static void RenderHeroChallengesPanel(const AlterEgo::Character& ch) {
    // Trigger fetch if not loaded
    if (!g_HeroChallengesReady && !g_HeroChallengesFetching) {
        FetchHeroChallenges();
    }

    if (!g_HeroChallengesReady) {
        RenderSpinner("Loading hero challenge data...");
        return;
    }

    // Cache: rebuild only when character changes
    static std::string s_hcCacheCharName;
    static size_t s_hcCacheHPCount = 0;
    static std::vector<HCExpansionEntry> s_hcCache;
    static int s_hcTotalAll = 0;
    static int s_hcCompletedAll = 0;
    static int s_hcUnknownCompleted = 0; // completed IDs not in our data

    if (s_hcCacheCharName != ch.name || s_hcCacheHPCount != ch.heropoints.size()) {
        s_hcCacheCharName = ch.name;
        s_hcCacheHPCount = ch.heropoints.size();
        s_hcCache.clear();
        s_hcTotalAll = 0;
        s_hcCompletedAll = 0;
        s_hcUnknownCompleted = 0;

        // Build completed set
        std::unordered_set<std::string> completedSet(ch.heropoints.begin(), ch.heropoints.end());

        // Group by expansion -> map
        // expansion -> map_name -> {total, completed}
        std::map<std::string, std::map<std::string, std::pair<int, int>>> grouped;

        // Define expansion order
        static const std::vector<std::string> expansionOrder = {
            "Core", "Living World Season 2", "Heart of Thorns", "Path of Fire"
        };
        // Pre-populate order
        for (const auto& exp : expansionOrder) {
            grouped[exp]; // ensure key exists
        }

        for (const auto& [id, info] : g_HeroChallenges) {
            auto& mapEntry = grouped[info.expansion][info.map_name];
            mapEntry.first++;  // total
            if (completedSet.count(id)) {
                mapEntry.second++;  // completed
            }
        }

        // Count completed IDs not in our data
        for (const auto& hp : ch.heropoints) {
            if (g_HeroChallenges.find(hp) == g_HeroChallenges.end()) {
                s_hcUnknownCompleted++;
            }
        }

        // Build display entries
        for (const auto& [exp, maps] : grouped) {
            if (maps.empty()) continue;
            HCExpansionEntry entry;
            entry.expansion = exp;
            for (const auto& [mapName, counts] : maps) {
                HCMapEntry me;
                me.map_name = mapName;
                me.total = counts.first;
                me.completed = counts.second;
                entry.total += me.total;
                entry.completed += me.completed;
                entry.maps.push_back(std::move(me));
            }
            // Sort maps alphabetically
            std::sort(entry.maps.begin(), entry.maps.end(),
                [](const HCMapEntry& a, const HCMapEntry& b) { return a.map_name < b.map_name; });
            s_hcTotalAll += entry.total;
            s_hcCompletedAll += entry.completed;
            s_hcCache.push_back(std::move(entry));
        }
    }

    // Summary
    ImGui::Text("Hero Challenges: %d / %d", s_hcCompletedAll, s_hcTotalAll);
    if (s_hcUnknownCompleted > 0) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
            "(+ %d from expansions not yet tracked)", s_hcUnknownCompleted);
    }

    // Progress bar
    float progress = s_hcTotalAll > 0 ? (float)s_hcCompletedAll / (float)s_hcTotalAll : 0.0f;
    ImGui::ProgressBar(progress, ImVec2(-1, 0));
    ImGui::Spacing();

    if (ch.heropoints.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
            "No hero challenge data available for this character.\nTry refreshing character data.");
        return;
    }

    // Render expansion trees
    for (const auto& exp : s_hcCache) {
        int remaining = exp.total - exp.completed;
        char label[256];
        if (remaining == 0) {
            snprintf(label, sizeof(label), "%s  (%d/%d) " "\xE2\x9C\x93",
                exp.expansion.c_str(), exp.completed, exp.total);
        } else {
            snprintf(label, sizeof(label), "%s  (%d/%d) — %d remaining",
                exp.expansion.c_str(), exp.completed, exp.total, remaining);
        }

        bool allDone = (exp.completed == exp.total);
        if (allDone) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.7f, 0.5f, 1.0f));

        // Default open if there are incomplete challenges
        if (!allDone) ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode(exp.expansion.c_str(), "%s", label)) {
            for (const auto& m : exp.maps) {
                int mRemaining = m.total - m.completed;
                bool mapDone = (m.completed == m.total);

                if (mapDone) {
                    ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.5f, 1.0f),
                        "  %s  %d/%d " "\xE2\x9C\x93", m.map_name.c_str(), m.completed, m.total);
                } else {
                    ImGui::Text("  %s  %d/%d", m.map_name.c_str(), m.completed, m.total);
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.3f, 1.0f),
                        "— %d remaining", mRemaining);
                }
            }
            ImGui::TreePop();
        }

        if (allDone) ImGui::PopStyleColor();
    }
}

static void RenderSkillIcon(uint32_t skill_id, float size);
static bool DecodeBuildLink(const std::string& link, const std::string& name,
                            AlterEgo::GameMode mode, AlterEgo::SavedBuild& out);

static void RenderBuildPanel(const AlterEgo::Character& ch) {
    if (ch.build_tabs.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No build tabs available.");
        return;
    }

    // Build tab selector — chip strip
    {
        std::vector<std::string> labels;
        std::vector<bool> activeMarkers;
        labels.reserve(ch.build_tabs.size());
        activeMarkers.reserve(ch.build_tabs.size());
        for (const auto& bt : ch.build_tabs) {
            labels.push_back(bt.name.empty() ? ("Tab " + std::to_string(bt.tab)) : bt.name);
            activeMarkers.push_back(bt.is_active);
        }
        RenderChipStrip(labels, activeMarkers, g_SelectedBuildTab, g_BuildTabScroll, "##build_chips");
    }

    int buildIdx = g_SelectedBuildTab;
    if (buildIdx < 0 || buildIdx >= (int)ch.build_tabs.size()) {
        // Find active tab
        for (int i = 0; i < (int)ch.build_tabs.size(); i++) {
            if (ch.build_tabs[i].is_active) { buildIdx = i; break; }
        }
        if (buildIdx < 0) buildIdx = 0;
        g_SelectedBuildTab = buildIdx;
    }

    const auto& bt = ch.build_tabs[buildIdx];
    ImGui::Separator();

    // Small left/right padding so spec hex portraits & trait selection borders
    // don't sit flush against the scroll-child edges (icon borders extend ~1px).
    ImGui::Indent(4.0f);

    // Record start position for profession/elite spec overlay
    ImVec2 buildPanelStart = ImGui::GetCursorScreenPos();

    // Specializations — game-like trait grid with dotted lines
    const float iconSz = TRAIT_ICON_SIZE; // uniform size for all trait icons
    const float colW = iconSz * 2.0f;

    for (int i = 0; i < 3; i++) {
        const auto& spec = bt.specializations[i];
        if (spec.spec_id == 0) {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "  (empty)");
            continue;
        }

        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(spec.spec_id);
        std::string specName = specInfo ? specInfo->name : ("Spec " + std::to_string(spec.spec_id));
        bool isElite = specInfo ? specInfo->elite : false;
        ImVec4 specColor = isElite ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

        ImGui::PushID(spec.spec_id);

        if (specInfo && specInfo->major_traits.size() >= 9) {
            // Positions for dotted lines. specCenter stays at (0,0) — the hex
            // icon has been replaced by the spec name overlay on the banner.
            ImVec2 specCenter(0, 0);
            ImVec2 minorCenters[3] = {};
            ImVec2 selectedCenters[3] = {};
            bool hasSelected[3] = {false, false, false};

            // Row geometry — used for banner backdrop and name overlay.
            ImVec2 rowOrigin = ImGui::GetCursorScreenPos();
            float rowW = ImGui::GetContentRegionAvail().x;
            float rowH = iconSz * 3 + ImGui::GetStyle().CellPadding.y * 6;
            ImDrawList* dl = ImGui::GetWindowDrawList();

            // Spec background banner (panoramic art) behind the trait grid.
            // Scaled to fit row height, native aspect preserved, anchored left,
            // with a fade-to-dark overlay on the right so trait icons stay readable.
            if (!specInfo->background_url.empty()) {
                uint32_t bgKey = spec.spec_id + 1000000;
                Texture_t* bgTex = AlterEgo::IconManager::GetIcon(bgKey);
                if (bgTex && bgTex->Resource && bgTex->Height > 0) {
                    // Zoom the banner and anchor it to the row's bottom-left.
                    // 15 specs ship with banners whose top ~66% is transparent
                    // (opaque art lives only in the bottom band). Those need a
                    // much higher zoom so the opaque content fills the row;
                    // everyone else gets the standard 1.4x.
                    auto IsTransparentTopSpec = [](uint32_t id) {
                        switch (id) {
                            case 4: case 7: case 11: case 20: case 22:
                            case 28: case 35: case 36: case 44: case 51:
                            case 54: case 68: case 70: case 71: case 72:
                            case 73: case 74: case 75: case 76: case 77:
                            case 78: case 79: case 80: case 81:
                                return true;
                            default:
                                return false;
                        }
                    };
                    const float zoom = IsTransparentTopSpec(spec.spec_id) ? 2.94f : 1.4f;
                    float drawH = rowH * zoom;
                    float scale = drawH / (float)bgTex->Height;
                    float drawW = (float)bgTex->Width * scale;
                    float bannerLeft = rowOrigin.x;
                    float bannerTop  = rowOrigin.y + rowH - drawH;
                    dl->PushClipRect(rowOrigin,
                        ImVec2(rowOrigin.x + rowW, rowOrigin.y + rowH), true);
                    dl->AddImage(bgTex->Resource,
                        ImVec2(bannerLeft, bannerTop),
                        ImVec2(bannerLeft + drawW, bannerTop + drawH),
                        ImVec2(0, 0), ImVec2(1, 1),
                        IM_COL32(255, 255, 255, 200));
                    // Fade-to-dark anchored to where the trait grid begins,
                    // not the banner. Trait icons start just past column 0.
                    float fadeStart = rowOrigin.x + SPEC_PORTRAIT_SIZE + 20.0f;
                    float fadeEnd   = rowOrigin.x + SPEC_PORTRAIT_SIZE + 80.0f;
                    dl->AddRectFilledMultiColor(
                        ImVec2(fadeStart, rowOrigin.y),
                        ImVec2(fadeEnd,   rowOrigin.y + rowH),
                        IM_COL32(15, 17, 23, 0),   IM_COL32(15, 17, 23, 200),
                        IM_COL32(15, 17, 23, 200), IM_COL32(15, 17, 23, 0));
                    if (fadeEnd < rowOrigin.x + rowW) {
                        dl->AddRectFilled(
                            ImVec2(fadeEnd, rowOrigin.y),
                            ImVec2(rowOrigin.x + rowW, rowOrigin.y + rowH),
                            IM_COL32(15, 17, 23, 200));
                    }
                    dl->PopClipRect();
                } else {
                    AlterEgo::IconManager::RequestIcon(bgKey, specInfo->background_url);
                }
            }

            // Spec name overlay on the banner's left. Drop shadow underneath
            // for legibility against bright banner art. Tooltip is attached
            // to an invisible button covering the same area.
            {
                const float nameScale = 1.5f;
                float nameSize = ImGui::GetFontSize() * nameScale;
                ImVec2 nameAnchor(rowOrigin.x + 14.0f,
                                  rowOrigin.y + rowH * 0.5f - nameSize * 0.5f);
                dl->AddText(NULL, nameSize,
                    ImVec2(nameAnchor.x + 1.0f, nameAnchor.y + 1.0f),
                    IM_COL32(0, 0, 0, 220), specName.c_str());
                dl->AddText(NULL, nameSize, nameAnchor,
                    ImGui::ColorConvertFloat4ToU32(specColor), specName.c_str());

                ImVec2 hitSz = ImGui::CalcTextSize(specName.c_str());
                hitSz.x *= nameScale; hitSz.y *= nameScale;
                ImVec2 prevPos = ImGui::GetCursorScreenPos();
                ImGui::SetCursorScreenPos(nameAnchor);
                ImGui::InvisibleButton("##specname", hitSz);
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextColored(specColor, "%s", specName.c_str());
                    const auto* specDesc = AlterEgo::GetSpecDescription(specName);
                    if (specDesc) {
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.9f, 1.0f),
                            "\"%s\"", specDesc->flavor);
                        ImGui::Spacing();
                        ImGui::PushTextWrapPos(350.0f);
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
                            "%s", specDesc->description);
                        ImGui::PopTextWrapPos();
                    } else {
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s%s",
                            specInfo->profession.c_str(),
                            isElite ? " (Elite)" : "");
                    }
                    ImGui::EndTooltip();
                }
                ImGui::SetCursorScreenPos(prevPos);
            }

            // Outer 2-column table: [reserved left gutter] [6-col trait grid]
            char outerId[32];
            snprintf(outerId, sizeof(outerId), "##so_%u", spec.spec_id);
            if (ImGui::BeginTable(outerId, 2,
                    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                    SPEC_PORTRAIT_SIZE + 8.0f);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                // Column 0: left gutter (banner art shows through; spec name
                // is overlaid above via the drawlist).
                ImGui::TableNextColumn();
                ImGui::Dummy(ImVec2(SPEC_PORTRAIT_SIZE, rowH - 2.0f));

                // Column 1: 6-column × 3-row trait grid
                ImGui::TableNextColumn();
                char innerId[32];
                snprintf(innerId, sizeof(innerId), "##tr_%u", spec.spec_id);
                if (ImGui::BeginTable(innerId, 6,
                        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                    for (int c = 0; c < 6; c++)
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, colW);

                    for (int row = 0; row < 3; row++) {
                        ImGui::TableNextRow();
                        for (int tier = 0; tier < 3; tier++) {
                            // Minor trait column: render only in middle row
                            ImGui::TableNextColumn();
                            if (row == 1 && tier < (int)specInfo->minor_traits.size()) {
                                minorCenters[tier] = RenderTraitIcon(
                                    specInfo->minor_traits[tier], true, true, iconSz);
                            }

                            // Major trait column
                            ImGui::TableNextColumn();
                            int majorIdx = tier * 3 + row;
                            uint32_t tid = specInfo->major_traits[majorIdx];
                            bool selected = ((uint32_t)spec.traits[tier] == tid);
                            ImVec2 c = RenderTraitIcon(tid, selected, false, iconSz);
                            if (selected) {
                                selectedCenters[tier] = c;
                                hasSelected[tier] = true;
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTable();
            }

            // Draw dotted lines connecting traits (spec→minor0 is skipped
            // because the hex icon is gone). Lines stop at icon edges.
            ImU32 lineCol = IM_COL32(180, 230, 255, 180);
            float minorR = MINOR_TRAIT_SIZE * 0.5f;
            float majorR = iconSz * 0.5f;

            auto InsetLine = [](ImVec2 a, ImVec2 b, float rA, float rB, ImVec2& outA, ImVec2& outB) {
                float dx = b.x - a.x, dy = b.y - a.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len < rA + rB) return false;
                float nx = dx / len, ny = dy / len;
                outA = ImVec2(a.x + nx * rA, a.y + ny * rA);
                outB = ImVec2(b.x - nx * rB, b.y - ny * rB);
                return true;
            };

            // Skip the first connector — the hex spec icon is gone, so there
            // is no anchor on the left for a leading dotted line.
            ImVec2 prev;
            float prevR = 0;
            bool havePrev = false;
            (void)specCenter;
            for (int tier = 0; tier < 3; tier++) {
                if (minorCenters[tier].x > 0) {
                    if (havePrev) {
                        ImVec2 la, lb;
                        if (InsetLine(prev, minorCenters[tier], prevR, minorR, la, lb))
                            DrawDottedLine(dl, la, lb, lineCol);
                    }
                    prev = minorCenters[tier]; prevR = minorR; havePrev = true;
                }
                if (hasSelected[tier]) {
                    if (havePrev) {
                        ImVec2 la, lb;
                        if (InsetLine(prev, selectedCenters[tier], prevR, majorR, la, lb))
                            DrawDottedLine(dl, la, lb, lineCol);
                    }
                    prev = selectedCenters[tier]; prevR = majorR; havePrev = true;
                }
            }
        }

        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::Spacing();

    // Skill bar: Heal / Utility / Elite
    {
        // Fetch skill details if needed
        std::vector<uint32_t> toFetch;
        auto MaybeFetch = [&](uint32_t id) {
            if (id && !AlterEgo::GW2API::GetSkillInfo(id)) toFetch.push_back(id);
        };
        MaybeFetch(bt.terrestrial_skills.heal);
        for (int i = 0; i < 3; i++) MaybeFetch(bt.terrestrial_skills.utilities[i]);
        MaybeFetch(bt.terrestrial_skills.elite);
        if (!toFetch.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(toFetch);

        float skillSz = ICON_SIZE;
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Heal / Utility / Elite");
        ImGui::BeginGroup();
        RenderSkillIcon(bt.terrestrial_skills.heal, skillSz);
        ImGui::SameLine();
        for (int i = 0; i < 3; i++) {
            RenderSkillIcon(bt.terrestrial_skills.utilities[i], skillSz);
            ImGui::SameLine();
        }
        RenderSkillIcon(bt.terrestrial_skills.elite, skillSz);
        ImGui::EndGroup();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Copy build chat link button
    bool hasPalette = AlterEgo::GW2API::HasPaletteData(bt.profession);
    if (!hasPalette)
        AlterEgo::GW2API::FetchProfessionPaletteAsync(bt.profession);
    {
        if (RenderGoldButton(hasPalette ? "Copy Build Chat Link" : "Copy Build Chat Link (loading...)") && hasPalette) {
            // Map profession name to code
            auto ProfCode = [](const std::string& p) -> uint8_t {
                if (p == "Guardian")     return 1;
                if (p == "Warrior")      return 2;
                if (p == "Engineer")     return 3;
                if (p == "Ranger")       return 4;
                if (p == "Thief")        return 5;
                if (p == "Elementalist") return 6;
                if (p == "Mesmer")       return 7;
                if (p == "Necromancer")  return 8;
                if (p == "Revenant")     return 9;
                return 0;
            };

            AlterEgo::DecodedBuildLink link{};
            link.profession = ProfCode(bt.profession);

            // Specializations: spec_id and trait choices are already in the right format
            for (int i = 0; i < 3; i++) {
                link.specs[i].spec_id = (uint8_t)bt.specializations[i].spec_id;
                for (int t = 0; t < 3; t++)
                    link.specs[i].traits[t] = (uint8_t)bt.specializations[i].traits[t];
            }

            // Skills: convert skill IDs to palette IDs
            auto ToPalette = [&](uint32_t skill_id) -> uint16_t {
                if (skill_id == 0) return 0;
                return AlterEgo::GW2API::GetPaletteIdFromSkill(bt.profession, skill_id);
            };
            link.terrestrial_skills[0] = ToPalette(bt.terrestrial_skills.heal);
            link.terrestrial_skills[1] = ToPalette(bt.terrestrial_skills.utilities[0]);
            link.terrestrial_skills[2] = ToPalette(bt.terrestrial_skills.utilities[1]);
            link.terrestrial_skills[3] = ToPalette(bt.terrestrial_skills.utilities[2]);
            link.terrestrial_skills[4] = ToPalette(bt.terrestrial_skills.elite);
            link.aquatic_skills[0] = ToPalette(bt.aquatic_skills.heal);
            link.aquatic_skills[1] = ToPalette(bt.aquatic_skills.utilities[0]);
            link.aquatic_skills[2] = ToPalette(bt.aquatic_skills.utilities[1]);
            link.aquatic_skills[3] = ToPalette(bt.aquatic_skills.utilities[2]);
            link.aquatic_skills[4] = ToPalette(bt.aquatic_skills.elite);

            // Ranger pets
            if (bt.profession == "Ranger") {
                link.pets[0] = (uint8_t)bt.pets.terrestrial[0];
                link.pets[1] = (uint8_t)bt.pets.terrestrial[1];
                link.pets[2] = (uint8_t)bt.pets.aquatic[0];
                link.pets[3] = (uint8_t)bt.pets.aquatic[1];
            }

            // Revenant legends: parse number from "Legend1" etc.
            if (bt.profession == "Revenant") {
                auto LegendByte = [](const std::string& s) -> uint8_t {
                    if (s.size() > 6 && s.substr(0, 6) == "Legend")
                        return (uint8_t)std::atoi(s.c_str() + 6);
                    return 0;
                };
                link.legends[0] = LegendByte(bt.legends.terrestrial[0]);
                link.legends[1] = LegendByte(bt.legends.terrestrial[1]);
                link.legends[2] = LegendByte(bt.legends.aquatic[0]);
                link.legends[3] = LegendByte(bt.legends.aquatic[1]);
            }

            // Weapons
            for (auto w : bt.weapons)
                link.weapons.push_back((uint16_t)w);

            std::string chatLink = AlterEgo::ChatLink::EncodeBuild(link);
            CopyToClipboard(chatLink);
            if (APIDefs) APIDefs->GUI_SendAlert("Build link copied!");
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        if (!hasPalette)
            RenderSpinner("Loading profession data...");
        else
            ImGui::Text("Export this build as a GW2 chat link");
        ImGui::EndTooltip();
    }

    // Save to Library button — opens dialog
    ImGui::SameLine();
    if (RenderGoldButton(hasPalette ? "Save to Library" : "Save to Library (loading...)") && hasPalette) {
        // Build the chat link
        auto ProfCodeLib = [](const std::string& p) -> uint8_t {
            if (p == "Guardian")     return 1;
            if (p == "Warrior")      return 2;
            if (p == "Engineer")     return 3;
            if (p == "Ranger")       return 4;
            if (p == "Thief")        return 5;
            if (p == "Elementalist") return 6;
            if (p == "Mesmer")       return 7;
            if (p == "Necromancer")  return 8;
            if (p == "Revenant")     return 9;
            return 0;
        };

        AlterEgo::DecodedBuildLink saveLink{};
        saveLink.profession = ProfCodeLib(bt.profession);
        for (int i = 0; i < 3; i++) {
            saveLink.specs[i].spec_id = (uint8_t)bt.specializations[i].spec_id;
            const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(bt.specializations[i].spec_id);
            for (int t = 0; t < 3; t++) {
                int traitId = bt.specializations[i].traits[t];
                uint8_t choice = 0;
                if (traitId != 0 && specInfo && specInfo->major_traits.size() >= 9) {
                    for (int r = 0; r < 3; r++) {
                        if ((int)specInfo->major_traits[t * 3 + r] == traitId) {
                            choice = (uint8_t)(r + 1);
                            break;
                        }
                    }
                }
                saveLink.specs[i].traits[t] = choice;
            }
        }
        auto ToPaletteLib = [&](uint32_t skill_id) -> uint16_t {
            if (skill_id == 0) return 0;
            return AlterEgo::GW2API::GetPaletteIdFromSkill(bt.profession, skill_id);
        };
        saveLink.terrestrial_skills[0] = ToPaletteLib(bt.terrestrial_skills.heal);
        saveLink.terrestrial_skills[1] = ToPaletteLib(bt.terrestrial_skills.utilities[0]);
        saveLink.terrestrial_skills[2] = ToPaletteLib(bt.terrestrial_skills.utilities[1]);
        saveLink.terrestrial_skills[3] = ToPaletteLib(bt.terrestrial_skills.utilities[2]);
        saveLink.terrestrial_skills[4] = ToPaletteLib(bt.terrestrial_skills.elite);
        saveLink.aquatic_skills[0] = ToPaletteLib(bt.aquatic_skills.heal);
        saveLink.aquatic_skills[1] = ToPaletteLib(bt.aquatic_skills.utilities[0]);
        saveLink.aquatic_skills[2] = ToPaletteLib(bt.aquatic_skills.utilities[1]);
        saveLink.aquatic_skills[3] = ToPaletteLib(bt.aquatic_skills.utilities[2]);
        saveLink.aquatic_skills[4] = ToPaletteLib(bt.aquatic_skills.elite);
        if (bt.profession == "Ranger") {
            saveLink.pets[0] = (uint8_t)bt.pets.terrestrial[0];
            saveLink.pets[1] = (uint8_t)bt.pets.terrestrial[1];
            saveLink.pets[2] = (uint8_t)bt.pets.aquatic[0];
            saveLink.pets[3] = (uint8_t)bt.pets.aquatic[1];
        }
        if (bt.profession == "Revenant") {
            auto LegByte = [](const std::string& s) -> uint8_t {
                if (s.size() > 6 && s.substr(0, 6) == "Legend")
                    return (uint8_t)std::atoi(s.c_str() + 6);
                return 0;
            };
            saveLink.legends[0] = LegByte(bt.legends.terrestrial[0]);
            saveLink.legends[1] = LegByte(bt.legends.terrestrial[1]);
            saveLink.legends[2] = LegByte(bt.legends.aquatic[0]);
            saveLink.legends[3] = LegByte(bt.legends.aquatic[1]);
        }
        for (auto w : bt.weapons)
            saveLink.weapons.push_back((uint16_t)w);

        g_SaveLibChatLink = AlterEgo::ChatLink::EncodeBuild(saveLink);

        // Pre-fill dialog state
        std::string defaultName = ch.name + " - " +
            (bt.name.empty() ? ("Tab " + std::to_string(bt.tab)) : bt.name);
        snprintf(g_SaveLibName, sizeof(g_SaveLibName), "%s", defaultName.c_str());
        g_SaveLibMode = 0;
        g_SaveLibIncludeEquip = false;
        g_SaveLibEquipTab = ch.active_equipment_tab > 0 ? ch.active_equipment_tab : 1;
        g_SaveLibCharName = ch.name;
        g_SaveLibProfession = ch.profession;
        g_SaveLibDialogOpen = true;
    }

    ImGui::Unindent(4.0f);
}

// --- Build Library helpers ---

// Profession code to name mapping
static const char* ProfessionFromCode(uint8_t code) {
    switch (code) {
        case 1: return "Guardian";
        case 2: return "Warrior";
        case 3: return "Engineer";
        case 4: return "Ranger";
        case 5: return "Thief";
        case 6: return "Elementalist";
        case 7: return "Mesmer";
        case 8: return "Necromancer";
        case 9: return "Revenant";
        default: return "Unknown";
    }
}

static const char* GameModeNames[] = { "All", "PvE", "WvW", "PvP", "Raid", "Fractal", "Other" };
static const char* GameModeImportNames[] = { "PvE", "WvW", "PvP", "Raid", "Fractal", "Other" };

static AlterEgo::GameMode GameModeFromIndex(int idx) {
    switch (idx) {
        case 0: return AlterEgo::GameMode::PvE;
        case 1: return AlterEgo::GameMode::WvW;
        case 2: return AlterEgo::GameMode::PvP;
        case 3: return AlterEgo::GameMode::Raid;
        case 4: return AlterEgo::GameMode::Fractal;
        default: return AlterEgo::GameMode::Other;
    }
}

static bool GameModeMatchesFilter(AlterEgo::GameMode mode, int filter) {
    if (filter == 0) return true; // All
    switch (filter) {
        case 1: return mode == AlterEgo::GameMode::PvE;
        case 2: return mode == AlterEgo::GameMode::WvW;
        case 3: return mode == AlterEgo::GameMode::PvP;
        case 4: return mode == AlterEgo::GameMode::Raid;
        case 5: return mode == AlterEgo::GameMode::Fractal;
        default: return true;
    }
}

static const char* GameModeLabel(AlterEgo::GameMode m) {
    switch (m) {
        case AlterEgo::GameMode::PvE:     return "PvE";
        case AlterEgo::GameMode::WvW:     return "WvW";
        case AlterEgo::GameMode::PvP:     return "PvP";
        case AlterEgo::GameMode::Raid:    return "Raid";
        case AlterEgo::GameMode::Fractal: return "Fractal";
        default:                          return "Other";
    }
}

// =========================================================================
// AE2: Compact binary build format — fits in GW2 chat (~130-150 chars)
// =========================================================================
//
// Format (all multi-byte values are little-endian):
//   [0]     uint8   version = 2
//   [1]     uint8   flags: bits 0-2 = game_mode (0-5), bit 3 = has_gear,
//                          bit 4 = has_rune, bit 5 = has_relic
//   [2]     uint8   build_link_len (N)
//   [3..3+N-1]     raw build link bytes (base64 payload of [&...])
//   If has_gear:
//     uint16  gear_mask (bitmask of populated slots, see AE2_SLOT_*)
//     For each set bit in gear_mask (in bit order):
//       uint16  stat_id
//   If has_rune:
//     uint32  rune_id (GW2 item ID)
//   If has_relic:
//     uint32  relic_id (GW2 item ID)
//   For each weapon slot set in gear_mask (bits 6-9):
//     uint32  sigil_id (0 if none)
//     uint16  weapon_type_id (see WeaponTypeId enum)

static const int AE2_SLOT_Helm        = 0;
static const int AE2_SLOT_Shoulders   = 1;
static const int AE2_SLOT_Coat        = 2;
static const int AE2_SLOT_Gloves      = 3;
static const int AE2_SLOT_Leggings    = 4;
static const int AE2_SLOT_Boots       = 5;
static const int AE2_SLOT_WeaponA1    = 6;
static const int AE2_SLOT_WeaponA2    = 7;
static const int AE2_SLOT_WeaponB1    = 8;
static const int AE2_SLOT_WeaponB2    = 9;
static const int AE2_SLOT_Back        = 10;
static const int AE2_SLOT_Accessory1  = 11;
static const int AE2_SLOT_Accessory2  = 12;
static const int AE2_SLOT_Amulet      = 13;
static const int AE2_SLOT_Ring1       = 14;
static const int AE2_SLOT_Ring2       = 15;

static int AE2SlotIndex(const std::string& slot) {
    if (slot == "Helm")        return AE2_SLOT_Helm;
    if (slot == "Shoulders")   return AE2_SLOT_Shoulders;
    if (slot == "Coat")        return AE2_SLOT_Coat;
    if (slot == "Gloves")      return AE2_SLOT_Gloves;
    if (slot == "Leggings")    return AE2_SLOT_Leggings;
    if (slot == "Boots")       return AE2_SLOT_Boots;
    if (slot == "WeaponA1")    return AE2_SLOT_WeaponA1;
    if (slot == "WeaponA2")    return AE2_SLOT_WeaponA2;
    if (slot == "WeaponB1")    return AE2_SLOT_WeaponB1;
    if (slot == "WeaponB2")    return AE2_SLOT_WeaponB2;
    if (slot == "Backpack")    return AE2_SLOT_Back;
    if (slot == "Accessory1")  return AE2_SLOT_Accessory1;
    if (slot == "Accessory2")  return AE2_SLOT_Accessory2;
    if (slot == "Amulet")      return AE2_SLOT_Amulet;
    if (slot == "Ring1")       return AE2_SLOT_Ring1;
    if (slot == "Ring2")       return AE2_SLOT_Ring2;
    return -1;
}

static const char* AE2SlotName(int idx) {
    static const char* names[] = {
        "Helm", "Shoulders", "Coat", "Gloves", "Leggings", "Boots",
        "WeaponA1", "WeaponA2", "WeaponB1", "WeaponB2",
        "Backpack", "Accessory1", "Accessory2", "Amulet", "Ring1", "Ring2"
    };
    if (idx >= 0 && idx < 16) return names[idx];
    return "";
}

static uint16_t WeaponTypeToId(const std::string& wtype) {
    if (wtype == "Axe")        return 5;
    if (wtype == "Longbow")    return 35;
    if (wtype == "Dagger")     return 47;
    if (wtype == "Focus")      return 49;
    if (wtype == "Greatsword") return 50;
    if (wtype == "Hammer")     return 51;
    if (wtype == "Mace")       return 53;
    if (wtype == "Pistol")     return 54;
    if (wtype == "Rifle")      return 85;
    if (wtype == "Scepter")    return 86;
    if (wtype == "Shield")     return 87;
    if (wtype == "Staff")      return 89;
    if (wtype == "Sword")      return 90;
    if (wtype == "Torch")      return 102;
    if (wtype == "Warhorn")    return 103;
    if (wtype == "Shortbow")   return 107;
    if (wtype == "Spear")      return 265;
    return 0;
}

static const char* WeaponTypeFromId(uint16_t id) {
    switch (id) {
        case 5:   return "Axe";
        case 35:  return "Longbow";
        case 47:  return "Dagger";
        case 49:  return "Focus";
        case 50:  return "Greatsword";
        case 51:  return "Hammer";
        case 53:  return "Mace";
        case 54:  return "Pistol";
        case 85:  return "Rifle";
        case 86:  return "Scepter";
        case 87:  return "Shield";
        case 89:  return "Staff";
        case 90:  return "Sword";
        case 102: return "Torch";
        case 103: return "Warhorn";
        case 107: return "Shortbow";
        case 265: return "Spear";
        default:  return "";
    }
}

static void PushU16LE(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back(v & 0xFF);
    buf.push_back((v >> 8) & 0xFF);
}

static void PushU32LE(std::vector<uint8_t>& buf, uint32_t v) {
    buf.push_back(v & 0xFF);
    buf.push_back((v >> 8) & 0xFF);
    buf.push_back((v >> 16) & 0xFF);
    buf.push_back((v >> 24) & 0xFF);
}

static uint16_t ReadU16LE(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static std::string ExportBuildToAE2(const AlterEgo::SavedBuild& build) {
    std::vector<uint8_t> buf;

    // Extract raw build link bytes from chat link
    std::string payload;
    if (!AlterEgo::ChatLink::ExtractPayload(build.chat_link, payload))
        return "";
    auto linkBytes = AlterEgo::ChatLink::Base64Decode(payload);
    if (linkBytes.empty()) return "";

    // Resolve relic ID from persistent name→ID cache
    uint32_t relicId = build.relic_id;
    if (relicId == 0 && !build.relic_name.empty())
        relicId = AlterEgo::GW2API::FindItemIdByName(build.relic_name);

    // Resolve rune/sigil IDs — try stored IDs first, then persistent name→ID cache
    uint32_t runeId = build.rune_id;
    if (runeId == 0 && !build.rune_name.empty())
        runeId = AlterEgo::GW2API::FindItemIdByName(build.rune_name);

    std::map<std::string, uint32_t> resolvedSigilIds;
    std::map<std::string, uint32_t> resolvedSigil2Ids;
    for (const auto& [slot, gs] : build.gear) {
        if (slot.find("Weapon") == 0) {
            uint32_t sid = gs.sigil_id;
            if (sid == 0 && !gs.sigil.empty())
                sid = AlterEgo::GW2API::FindItemIdByName(gs.sigil);
            resolvedSigilIds[slot] = sid;
            uint32_t sid2 = gs.sigil2_id;
            if (sid2 == 0 && !gs.sigil2.empty())
                sid2 = AlterEgo::GW2API::FindItemIdByName(gs.sigil2);
            resolvedSigil2Ids[slot] = sid2;
        }
    }

    // Determine flags
    bool hasGear = !build.gear.empty();
    bool hasRune = (runeId != 0);
    bool hasRelic = (relicId != 0);
    uint8_t gameMode = (uint8_t)build.game_mode;
    if (gameMode > 5) gameMode = 0;

    uint8_t flags = (gameMode & 0x07);
    if (hasGear)  flags |= (1 << 3);
    if (hasRune)  flags |= (1 << 4);
    if (hasRelic) flags |= (1 << 5);

    // Header
    buf.push_back(3); // version (3 = added sigil2 per weapon slot)
    buf.push_back(flags);
    buf.push_back((uint8_t)linkBytes.size());
    buf.insert(buf.end(), linkBytes.begin(), linkBytes.end());

    // Gear
    if (hasGear) {
        uint16_t gearMask = 0;
        // First pass: determine which slots are present
        for (const auto& [slot, gs] : build.gear) {
            int idx = AE2SlotIndex(slot);
            if (idx >= 0 && gs.stat_id != 0) gearMask |= (1 << idx);
        }
        PushU16LE(buf, gearMask);

        // Second pass: write stat IDs in bit order
        for (int i = 0; i < 16; i++) {
            if (!(gearMask & (1 << i))) continue;
            const char* slotName = AE2SlotName(i);
            auto it = build.gear.find(slotName);
            uint16_t statId = (it != build.gear.end()) ? (uint16_t)it->second.stat_id : 0;
            PushU16LE(buf, statId);
        }
    }

    // Rune
    if (hasRune) PushU32LE(buf, runeId);

    // Relic
    if (hasRelic) PushU32LE(buf, relicId);

    // Sigils + weapon types — for each weapon slot present in gear_mask
    if (hasGear) {
        for (int i = AE2_SLOT_WeaponA1; i <= AE2_SLOT_WeaponB2; i++) {
            const char* slotName = AE2SlotName(i);
            auto it = build.gear.find(slotName);
            if (it != build.gear.end() && it->second.stat_id != 0) {
                auto sigilIt = resolvedSigilIds.find(slotName);
                uint32_t sigilId = (sigilIt != resolvedSigilIds.end()) ? sigilIt->second : 0;
                PushU32LE(buf, sigilId);
                PushU16LE(buf, WeaponTypeToId(it->second.weapon_type));
                // Second sigil for two-handed weapons
                auto sigil2It = resolvedSigil2Ids.find(slotName);
                uint32_t sigil2Id = (sigil2It != resolvedSigil2Ids.end()) ? sigil2It->second : 0;
                PushU32LE(buf, sigil2Id);
            }
        }
    }

    std::string b64 = AlterEgo::ChatLink::Base64Encode(buf.data(), buf.size());
    return "AE2:" + b64;
}

static bool ImportBuildFromAE2(const std::string& ae2code, AlterEgo::SavedBuild& out, std::string& error) {
    if (ae2code.size() < 5 || ae2code.substr(0, 4) != "AE2:") {
        error = "Not an AE2 code.";
        return false;
    }

    auto data = AlterEgo::ChatLink::Base64Decode(ae2code.substr(4));
    if (data.size() < 4) { error = "AE2 code too short."; return false; }

    size_t pos = 0;
    uint8_t version = data[pos++];
    if (version != 2 && version != 3) { error = "Unsupported AE2 version."; return false; }

    uint8_t flags = data[pos++];
    uint8_t gameMode = flags & 0x07;
    bool hasGear  = (flags >> 3) & 1;
    bool hasRune  = (flags >> 4) & 1;
    bool hasRelic = (flags >> 5) & 1;

    uint8_t linkLen = data[pos++];
    if (pos + linkLen > data.size()) { error = "AE2 build link truncated."; return false; }

    // Reconstruct chat link from raw bytes
    std::vector<uint8_t> linkBytes(data.begin() + pos, data.begin() + pos + linkLen);
    pos += linkLen;

    std::string b64 = AlterEgo::ChatLink::Base64Encode(linkBytes.data(), linkBytes.size());
    std::string chatLink = "[&" + b64 + "]";

    // Decode build
    AlterEgo::DecodedBuildLink decoded;
    if (!AlterEgo::ChatLink::DecodeBuild(chatLink, decoded)) {
        error = "Failed to decode build from AE2.";
        return false;
    }

    std::string profession = ProfessionFromCode(decoded.profession);
    if (profession == "Unknown") { error = "Unknown profession in AE2."; return false; }

    // Ensure palette data is available
    if (!AlterEgo::GW2API::HasPaletteData(profession)) {
        AlterEgo::GW2API::FetchProfessionPaletteAsync(profession);
        error = "Loading " + profession + " skill data... try again in a moment.";
        return false;
    }

    // Use DecodeBuildLink to populate the SavedBuild
    AlterEgo::GameMode mode = (AlterEgo::GameMode)gameMode;
    std::string buildName = profession; // placeholder, caller should set
    if (!DecodeBuildLink(chatLink, buildName, mode, out)) {
        error = "Failed to decode AE2 build data.";
        return false;
    }

    // Preserve the original AE2 code so we can re-resolve missing data later
    // if the GW2 API was unavailable or rate-limited during this import.
    out.ae2_code = ae2code;

    // Parse gear
    if (hasGear) {
        if (pos + 2 > data.size()) { error = "AE2 gear data truncated."; return false; }
        uint16_t gearMask = ReadU16LE(&data[pos]); pos += 2;

        int slotCount = 0;
        for (int i = 0; i < 16; i++) {
            if (gearMask & (1 << i)) slotCount++;
        }
        if (pos + slotCount * 2 > data.size()) { error = "AE2 gear stats truncated."; return false; }

        for (int i = 0; i < 16; i++) {
            if (!(gearMask & (1 << i))) continue;
            uint16_t statId = ReadU16LE(&data[pos]); pos += 2;

            AlterEgo::BuildGearSlot gs;
            gs.slot = AE2SlotName(i);
            gs.stat_id = statId;

            // Resolve stat name from cache
            const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(statId);
            if (statInfo) gs.stat_name = statInfo->name;

            out.gear[gs.slot] = gs;
        }

        // Read rune
        if (hasRune) {
            if (pos + 4 > data.size()) { error = "AE2 rune data truncated."; return false; }
            out.rune_id = ReadU32LE(&data[pos]); pos += 4;
            // Resolve rune name from item cache
            const auto* runeInfo = AlterEgo::GW2API::GetItemInfo(out.rune_id);
            if (runeInfo) out.rune_name = runeInfo->name;
        }

        // Read relic
        if (hasRelic) {
            if (pos + 4 > data.size()) { error = "AE2 relic data truncated."; return false; }
            out.relic_id = ReadU32LE(&data[pos]); pos += 4;
            const auto* relicInfo = AlterEgo::GW2API::GetItemInfo(out.relic_id);
            if (relicInfo) out.relic_name = relicInfo->name;
        }

        // Read sigils + weapon types for weapon slots
        for (int i = AE2_SLOT_WeaponA1; i <= AE2_SLOT_WeaponB2; i++) {
            if (!(gearMask & (1 << i))) continue;
            size_t needed = (version >= 3) ? 10 : 6; // v3: +4 for sigil2
            if (pos + needed > data.size()) break;
            uint32_t sigilId = ReadU32LE(&data[pos]); pos += 4;
            uint16_t weaponTypeId = ReadU16LE(&data[pos]); pos += 2;
            uint32_t sigil2Id = 0;
            if (version >= 3) { sigil2Id = ReadU32LE(&data[pos]); pos += 4; }
            const char* slotName = AE2SlotName(i);
            auto it = out.gear.find(slotName);
            if (it != out.gear.end()) {
                it->second.sigil_id = sigilId;
                it->second.sigil2_id = sigil2Id;
                const char* wt = WeaponTypeFromId(weaponTypeId);
                if (wt[0]) it->second.weapon_type = wt;
            }
        }

        // Fetch item details for rune/relic/sigils synchronously so names are available
        std::vector<uint32_t> fetchIds;
        if (out.rune_id != 0) fetchIds.push_back(out.rune_id);
        if (out.relic_id != 0) fetchIds.push_back(out.relic_id);
        for (const auto& [slot, gs] : out.gear) {
            if (gs.sigil_id != 0) fetchIds.push_back(gs.sigil_id);
            if (gs.sigil2_id != 0) fetchIds.push_back(gs.sigil2_id);
        }
        if (!fetchIds.empty()) AlterEgo::GW2API::FetchItemDetails(fetchIds);

        // Now resolve names from the cache (after sync fetch) and populate name→ID cache
        if (out.rune_id != 0 && out.rune_name.empty()) {
            const auto* runeInfo = AlterEgo::GW2API::GetItemInfo(out.rune_id);
            if (runeInfo) out.rune_name = runeInfo->name;
        }
        if (out.rune_id != 0 && !out.rune_name.empty())
            AlterEgo::GW2API::CacheItemNameId(out.rune_name, out.rune_id);
        if (out.relic_id != 0 && out.relic_name.empty()) {
            const auto* relicInfo = AlterEgo::GW2API::GetItemInfo(out.relic_id);
            if (relicInfo) out.relic_name = relicInfo->name;
        }
        if (out.relic_id != 0 && !out.relic_name.empty())
            AlterEgo::GW2API::CacheItemNameId(out.relic_name, out.relic_id);
        for (auto& [slot, gs] : out.gear) {
            if (gs.sigil_id != 0 && gs.sigil.empty()) {
                const auto* sigilInfo = AlterEgo::GW2API::GetItemInfo(gs.sigil_id);
                if (sigilInfo) gs.sigil = sigilInfo->name;
            }
            if (gs.sigil_id != 0 && !gs.sigil.empty())
                AlterEgo::GW2API::CacheItemNameId(gs.sigil, gs.sigil_id);
            if (gs.sigil2_id != 0 && gs.sigil2.empty()) {
                const auto* sigil2Info = AlterEgo::GW2API::GetItemInfo(gs.sigil2_id);
                if (sigil2Info) gs.sigil2 = sigil2Info->name;
            }
            if (gs.sigil2_id != 0 && !gs.sigil2.empty())
                AlterEgo::GW2API::CacheItemNameId(gs.sigil2, gs.sigil2_id);
        }
        // Set rune name on armor gear slots
        if (!out.rune_name.empty()) {
            const char* armorSlots[] = {"Helm","Shoulders","Coat","Gloves","Leggings","Boots"};
            for (const char* s : armorSlots) {
                auto ait = out.gear.find(s);
                if (ait != out.gear.end()) {
                    ait->second.rune = out.rune_name;
                    ait->second.rune_id = out.rune_id;
                }
            }
        }
    } else {
        // No gear — still parse rune/relic if flags set
        if (hasRune && pos + 4 <= data.size()) {
            out.rune_id = ReadU32LE(&data[pos]); pos += 4;
        }
        if (hasRelic && pos + 4 <= data.size()) {
            out.relic_id = ReadU32LE(&data[pos]); pos += 4;
        }
    }

    return true;
}

static AlterEgo::GameMode GameModeFromLabel(const std::string& label) {
    if (label == "PvE")     return AlterEgo::GameMode::PvE;
    if (label == "WvW")     return AlterEgo::GameMode::WvW;
    if (label == "PvP")     return AlterEgo::GameMode::PvP;
    if (label == "Raid")    return AlterEgo::GameMode::Raid;
    if (label == "Fractal") return AlterEgo::GameMode::Fractal;
    return AlterEgo::GameMode::Other;
}

// ---- Mobile relay import ----

static bool IsRelayCode(const std::string& s) {
    if (s.size() != 4) return false;
    for (char c : s) {
        if (!isalnum((unsigned char)c)) return false;
    }
    return true;
}

// Cache the last relay response so a retry (e.g. waiting for palette data) doesn't
// consume another single-use code.
static std::string g_RelayCache_Code;
static std::string g_RelayCache_Body;

static bool ImportBuildFromRelay(const std::string& code, AlterEgo::SavedBuild& out, std::string& error) {
    // Defense in depth: callers validate via IsRelayCode, but reject anything
    // unexpected here too before splicing it into the URL.
    if (!IsRelayCode(code)) {
        error = "Invalid code format.";
        return false;
    }

    std::string body;

    // Use cached response if this is a retry for the same code
    if (code == g_RelayCache_Code && !g_RelayCache_Body.empty()) {
        body = g_RelayCache_Body;
    } else {
        std::string url = "https://pie.rocks.cc/relay.php?code=" + code;
        std::string headers = "X-App-Secret: ae-relay-v1\r\n";
        auto resp = Skinventory::HttpClient::GetEx(url, headers);

        if (resp.status_code == 0) {
            error = "Network error — could not reach relay server.";
            return false;
        }
        if (resp.status_code == 400) { error = "Invalid code format."; return false; }
        if (resp.status_code == 403) { error = "Relay server rejected the request."; return false; }
        if (resp.status_code == 404) { error = "Code not found or expired."; return false; }
        if (resp.status_code != 200) {
            error = "Relay error (HTTP " + std::to_string(resp.status_code) + ")";
            return false;
        }

        body = resp.body;
        // Cache for potential retry
        g_RelayCache_Code = code;
        g_RelayCache_Body = body;
    }

    try {
        auto j = nlohmann::json::parse(body);

        std::string name = j.value("name", "Mobile Build");
        std::string profession = j.value("profession", "");
        std::string gameMode = j.value("gameMode", "PvE");
        std::string chatLink = j.value("chatLink", "");

        if (chatLink.empty()) {
            error = "Relay response missing chat link.";
            return false;
        }

        // Need palette data to resolve skills — kick off async fetch and ask user to retry
        if (!profession.empty() && !AlterEgo::GW2API::HasPaletteData(profession)) {
            AlterEgo::GW2API::FetchProfessionPaletteAsync(profession);
            error = "Loading " + profession + " skill data... click Import again in a moment.";
            return false;
        }

        AlterEgo::GameMode mode = GameModeFromLabel(gameMode);
        if (!DecodeBuildLink(chatLink, name, mode, out)) {
            error = "Failed to decode build from relay.";
            return false;
        }

        // Parse gear from gearJson (JSON string within the response)
        std::string gearJsonStr = j.value("gearJson", "");
        if (!gearJsonStr.empty()) {
            try {
                auto gj = nlohmann::json::parse(gearJsonStr);

                if (gj.contains("slots") && gj["slots"].is_object()) {
                    for (auto& [slot, slotData] : gj["slots"].items()) {
                        AlterEgo::BuildGearSlot gs;
                        gs.slot = slot;
                        gs.stat_id = slotData.value("statId", 0u);
                        gs.stat_name = slotData.value("statName", "");
                        gs.weapon_type = slotData.value("weaponType", "");
                        gs.sigil_id = slotData.value("sigilId", 0u);
                        gs.sigil2_id = slotData.value("sigil2Id", 0u);
                        gs.rune_id = slotData.value("runeId", 0u);
                        out.gear[slot] = gs;
                    }
                }

                // Top-level rune/relic
                out.rune_name = gj.value("rune", "");
                out.rune_id = gj.value("runeId", 0u);
                out.relic_name = gj.value("relic", "");
                out.relic_id = gj.value("relicId", 0u);

                // Fetch item details for rune/relic/sigils so names resolve
                std::vector<uint32_t> fetchIds;
                if (out.rune_id != 0) fetchIds.push_back(out.rune_id);
                if (out.relic_id != 0) fetchIds.push_back(out.relic_id);
                for (const auto& [slot, gs] : out.gear) {
                    if (gs.sigil_id != 0) fetchIds.push_back(gs.sigil_id);
                    if (gs.sigil2_id != 0) fetchIds.push_back(gs.sigil2_id);
                }
                if (!fetchIds.empty()) AlterEgo::GW2API::FetchItemDetails(fetchIds);

                // Resolve names and populate name→ID cache
                if (out.rune_id != 0 && out.rune_name.empty()) {
                    const auto* info = AlterEgo::GW2API::GetItemInfo(out.rune_id);
                    if (info) out.rune_name = info->name;
                }
                if (out.rune_id != 0 && !out.rune_name.empty())
                    AlterEgo::GW2API::CacheItemNameId(out.rune_name, out.rune_id);
                if (out.relic_id != 0 && out.relic_name.empty()) {
                    const auto* info = AlterEgo::GW2API::GetItemInfo(out.relic_id);
                    if (info) out.relic_name = info->name;
                }
                if (out.relic_id != 0 && !out.relic_name.empty())
                    AlterEgo::GW2API::CacheItemNameId(out.relic_name, out.relic_id);
                for (auto& [slot, gs] : out.gear) {
                    if (gs.sigil_id != 0 && gs.sigil.empty()) {
                        const auto* info = AlterEgo::GW2API::GetItemInfo(gs.sigil_id);
                        if (info) gs.sigil = info->name;
                    }
                    if (gs.sigil_id != 0 && !gs.sigil.empty())
                        AlterEgo::GW2API::CacheItemNameId(gs.sigil, gs.sigil_id);
                    if (gs.sigil2_id != 0 && gs.sigil2.empty()) {
                        const auto* info = AlterEgo::GW2API::GetItemInfo(gs.sigil2_id);
                        if (info) gs.sigil2 = info->name;
                    }
                    if (gs.sigil2_id != 0 && !gs.sigil2.empty())
                        AlterEgo::GW2API::CacheItemNameId(gs.sigil2, gs.sigil2_id);
                }

                // Set rune name on armor gear slots
                if (!out.rune_name.empty()) {
                    const char* armorSlots[] = {"Helm","Shoulders","Coat","Gloves","Leggings","Boots"};
                    for (const char* s : armorSlots) {
                        auto ait = out.gear.find(s);
                        if (ait != out.gear.end()) {
                            ait->second.rune = out.rune_name;
                            ait->second.rune_id = out.rune_id;
                        }
                    }
                }
            } catch (...) {
                // Gear parsing failed — build still usable without gear
            }
        }

        // Clear cache on successful import
        g_RelayCache_Code.clear();
        g_RelayCache_Body.clear();

        return true;
    } catch (const std::exception& e) {
        error = std::string("Failed to parse relay response: ") + e.what();
        return false;
    }
}

// Send a saved build to the relay server, returning the 4-char code or an error.
static void SendBuildToRelay(const AlterEgo::SavedBuild& build) {
    g_RelaySending = true;
    g_RelayResultCode.clear();
    g_RelayResultError.clear();

    std::thread([build]() {
        // Build the buildJson string (matches mobile app schema)
        nlohmann::json bj;
        {
            nlohmann::json specs = nlohmann::json::array();
            for (int i = 0; i < 3; i++) {
                if (build.specializations[i].spec_id == 0) continue;
                nlohmann::json s;
                s["specId"] = build.specializations[i].spec_id;
                nlohmann::json traits = nlohmann::json::array();
                for (int t = 0; t < 3; t++)
                    traits.push_back(build.specializations[i].traits[t]);
                s["selectedTraitIds"] = traits;
                specs.push_back(s);
            }
            bj["specializations"] = specs;

            // Skills: reverse-resolve to palette IDs for mobile app compatibility
            auto toPalette = [&](uint32_t skill_id) -> int {
                if (skill_id == 0) return 0;
                uint16_t p = AlterEgo::GW2API::GetPaletteIdFromSkill(build.profession, skill_id);
                return (p != 0) ? (int)p : (int)skill_id;
            };
            bj["healSkillId"] = toPalette(build.terrestrial_skills.heal);
            nlohmann::json utils = nlohmann::json::array();
            for (int i = 0; i < 3; i++)
                utils.push_back(toPalette(build.terrestrial_skills.utilities[i]));
            bj["utilitySkillIds"] = utils;
            bj["eliteSkillId"] = toPalette(build.terrestrial_skills.elite);
        }

        // Build the gearJson string (matches mobile app schema)
        nlohmann::json gj;
        {
            nlohmann::json slots = nlohmann::json::object();
            for (const auto& [slot, gs] : build.gear) {
                nlohmann::json s;
                if (gs.stat_id != 0) s["statId"] = gs.stat_id;
                if (!gs.stat_name.empty()) s["statName"] = gs.stat_name;
                if (!gs.weapon_type.empty()) s["weaponType"] = gs.weapon_type;
                if (gs.sigil_id != 0) s["sigilId"] = gs.sigil_id;
                if (gs.sigil2_id != 0) s["sigil2Id"] = gs.sigil2_id;
                if (gs.rune_id != 0) s["runeId"] = gs.rune_id;
                if (!s.empty()) slots[slot] = s;
            }
            gj["slots"] = slots;
            if (!build.rune_name.empty()) gj["rune"] = build.rune_name;
            if (build.rune_id != 0) gj["runeId"] = build.rune_id;
            if (!build.relic_name.empty()) gj["relic"] = build.relic_name;
            if (build.relic_id != 0) gj["relicId"] = build.relic_id;
        }

        // Assemble relay payload
        nlohmann::json payload;
        payload["name"] = build.name;
        payload["profession"] = build.profession;
        payload["gameMode"] = GameModeLabel(build.game_mode);
        payload["chatLink"] = build.chat_link;
        payload["buildJson"] = bj.dump();
        payload["gearJson"] = gj.dump();

        std::string url = "https://pie.rocks.cc/relay.php";
        std::string headers = "X-App-Secret: ae-relay-v1\r\n";
        auto resp = Skinventory::HttpClient::PostJson(url, payload.dump(), headers);

        if (resp.status_code == 200) {
            try {
                auto r = nlohmann::json::parse(resp.body);
                std::string code = r.value("code", "");
                if (!code.empty()) {
                    g_RelayResultCode = code;
                } else {
                    g_RelayResultError = "Server returned no code.";
                }
            } catch (...) {
                g_RelayResultError = "Failed to parse server response.";
            }
        } else if (resp.status_code == 0) {
            g_RelayResultError = "Network error — could not reach relay server.";
        } else if (resp.status_code == 429) {
            g_RelayResultError = "Rate limited — try again in a minute.";
        } else if (resp.status_code == 413) {
            g_RelayResultError = "Build data too large to send.";
        } else {
            g_RelayResultError = "Relay error (HTTP " + std::to_string(resp.status_code) + ")";
        }
        g_RelaySending = false;
    }).detach();
}

// Returns true if input is a shared build (AE2, chat link, or relay code)
// On success, populates 'out' with the imported build
static bool ImportSharedBuild(const std::string& input, AlterEgo::SavedBuild& out, std::string& error) {
    // AE2 compact binary format
    if (input.size() > 4 && input.substr(0, 4) == "AE2:") {
        return ImportBuildFromAE2(input, out, error);
    }
    // 4-character alphanumeric code — mobile relay import
    if (IsRelayCode(input)) {
        return ImportBuildFromRelay(input, out, error);
    }
    // Chat link
    if (input.size() > 4 && input[0] == '[' && input[1] == '&') {
        std::string name = "Imported Build";
        AlterEgo::GameMode mode = AlterEgo::GameMode::PvE;
        // Detect profession for palette data
        AlterEgo::DecodedBuildLink peek;
        if (AlterEgo::ChatLink::DecodeBuild(input, peek)) {
            std::string prof = ProfessionFromCode(peek.profession);
            if (!prof.empty() && prof != "Unknown" && !AlterEgo::GW2API::HasPaletteData(prof)) {
                AlterEgo::GW2API::FetchProfessionPaletteAsync(prof);
                error = "Loading " + prof + " skill data... try again in a moment.";
                return false;
            }
        }
        if (!DecodeBuildLink(input, name, mode, out)) {
            error = "Failed to decode build chat link.";
            return false;
        }
        return true;
    }
    return false; // Not a recognized format
}

// Decode a build chat link and create a SavedBuild
static bool DecodeBuildLink(const std::string& link, const std::string& name,
                            AlterEgo::GameMode mode, AlterEgo::SavedBuild& out) {
    AlterEgo::DecodedBuildLink decoded;
    if (!AlterEgo::ChatLink::DecodeBuild(link, decoded)) return false;

    std::string profession = ProfessionFromCode(decoded.profession);
    if (profession == "Unknown") return false;

    out.name = name;
    out.chat_link = link;
    out.profession = profession;
    out.game_mode = mode;

    // Copy specializations — trait choices stored as indices (1=top, 2=mid, 3=bot)
    // Need to resolve to actual trait IDs using spec info
    for (int i = 0; i < 3; i++) {
        out.specializations[i].spec_id = decoded.specs[i].spec_id;
        // Store trait choices temporarily as indices; resolve to trait IDs below
        out.specializations[i].traits[0] = 0;
        out.specializations[i].traits[1] = 0;
        out.specializations[i].traits[2] = 0;
    }

    // Resolve trait choices to trait IDs using spec cache
    for (int i = 0; i < 3; i++) {
        if (decoded.specs[i].spec_id == 0) continue;
        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(decoded.specs[i].spec_id);
        if (specInfo && specInfo->major_traits.size() >= 9) {
            for (int tier = 0; tier < 3; tier++) {
                uint8_t choice = decoded.specs[i].traits[tier]; // 1=top, 2=mid, 3=bot
                if (choice >= 1 && choice <= 3) {
                    int majorIdx = tier * 3 + (choice - 1);
                    out.specializations[i].traits[tier] = (int)specInfo->major_traits[majorIdx];
                }
            }
        } else {
            // Spec info not loaded yet — store choice indices negated as placeholder
            for (int tier = 0; tier < 3; tier++) {
                out.specializations[i].traits[tier] = -(int)decoded.specs[i].traits[tier];
            }
        }
    }

    // Resolve skill palette IDs to skill IDs
    auto resolveSkill = [&](uint16_t palette_id) -> uint32_t {
        if (palette_id == 0) return 0;
        return AlterEgo::GW2API::GetSkillIdFromPalette(profession, palette_id);
    };

    out.terrestrial_skills.heal = resolveSkill(decoded.terrestrial_skills[0]);
    out.terrestrial_skills.utilities[0] = resolveSkill(decoded.terrestrial_skills[1]);
    out.terrestrial_skills.utilities[1] = resolveSkill(decoded.terrestrial_skills[2]);
    out.terrestrial_skills.utilities[2] = resolveSkill(decoded.terrestrial_skills[3]);
    out.terrestrial_skills.elite = resolveSkill(decoded.terrestrial_skills[4]);

    out.aquatic_skills.heal = resolveSkill(decoded.aquatic_skills[0]);
    out.aquatic_skills.utilities[0] = resolveSkill(decoded.aquatic_skills[1]);
    out.aquatic_skills.utilities[1] = resolveSkill(decoded.aquatic_skills[2]);
    out.aquatic_skills.utilities[2] = resolveSkill(decoded.aquatic_skills[3]);
    out.aquatic_skills.elite = resolveSkill(decoded.aquatic_skills[4]);

    // Pets (Ranger)
    out.pets.terrestrial[0] = decoded.pets[0];
    out.pets.terrestrial[1] = decoded.pets[1];
    out.pets.aquatic[0] = decoded.pets[2];
    out.pets.aquatic[1] = decoded.pets[3];

    // Legends (Revenant)
    for (int i = 0; i < 4; i++)
        out.legend_codes[i] = decoded.legends[i];

    // Weapons
    out.weapons.clear();
    for (auto w : decoded.weapons)
        out.weapons.push_back((uint32_t)w);

    return true;
}

// Fetch spec/trait/skill details needed for a saved build's preview
static void FetchDetailsForSavedBuild(const AlterEgo::SavedBuild& build) {
    std::vector<uint32_t> spec_ids, skill_ids;

    for (int i = 0; i < 3; i++) {
        if (build.specializations[i].spec_id != 0)
            spec_ids.push_back(build.specializations[i].spec_id);
    }

    auto addSkill = [&](uint32_t id) { if (id != 0) skill_ids.push_back(id); };
    addSkill(build.terrestrial_skills.heal);
    for (int i = 0; i < 3; i++) addSkill(build.terrestrial_skills.utilities[i]);
    addSkill(build.terrestrial_skills.elite);
    addSkill(build.aquatic_skills.heal);
    for (int i = 0; i < 3; i++) addSkill(build.aquatic_skills.utilities[i]);
    addSkill(build.aquatic_skills.elite);

    if (!spec_ids.empty()) AlterEgo::GW2API::FetchSpecDetailsAsync(spec_ids);
    if (!skill_ids.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(skill_ids);
}

// =========================================================================
// Chat Build Detection — event handler + toast notification
// =========================================================================

static const char* ChatChannelName(ChatMessageType type) {
    switch (type) {
        case ChatMsg_Party:         return "Party";
        case ChatMsg_Squad:         return "Squad";
        case ChatMsg_Whisper:       return "Whisper";
        case ChatMsg_Local:         return "Local";
        case ChatMsg_Map:           return "Map";
        case ChatMsg_Guild:         return "Guild";
        case ChatMsg_TeamPvP:       return "PvP";
        case ChatMsg_TeamWvW:       return "WvW";
        default:                    return "Chat";
    }
}

static void OnEvChatMessage(void* eventArgs) {
    if (!eventArgs) return;
    g_ChatAddonConnected = true;
    if (!g_ChatBuildDetection) return;

    EvChatMessage* msg = (EvChatMessage*)eventArgs;

    // Only process message types that have ChatGenericMessage with Content
    if (msg->Type != ChatMsg_Party && msg->Type != ChatMsg_Squad &&
        msg->Type != ChatMsg_Whisper && msg->Type != ChatMsg_Local &&
        msg->Type != ChatMsg_Map && msg->Type != ChatMsg_Guild &&
        msg->Type != ChatMsg_TeamPvP && msg->Type != ChatMsg_TeamWvW)
        return;

    // Skip our own outgoing whispers
    if (msg->Type == ChatMsg_Whisper && (msg->Flags & ChatFlag_Whisper_IsFromMe))
        return;

    // All GenericMessage types share the same union offset
    ChatGenericMessage* gm = &msg->Whisper;
    if (!gm->Content) return;

    std::string content(gm->Content);
    std::string sender = gm->CharacterName ? std::string(gm->CharacterName) : "Unknown";

    // Skip our own messages (no toast for builds we posted ourselves)
    if (!g_CurrentCharName.empty() && sender == g_CurrentCharName)
        return;

    // --- Priority 1: Detect AE2: compact build codes (includes gear) ---
    {
        size_t ae2pos = content.find("AE2:");
        if (ae2pos != std::string::npos) {
            // Extract the AE2 code (runs until whitespace or end of string)
            size_t ae2end = content.find_first_of(" \t\n\r", ae2pos);
            std::string ae2code = (ae2end != std::string::npos)
                ? content.substr(ae2pos, ae2end - ae2pos)
                : content.substr(ae2pos);

            // Quick-decode to extract profession info for the toast
            auto ae2data = AlterEgo::ChatLink::Base64Decode(ae2code.substr(4));
            if (ae2data.size() >= 4 && (ae2data[0] == 2 || ae2data[0] == 3)) {
                uint8_t ae2flags = ae2data[1];
                bool ae2hasGear = (ae2flags >> 3) & 1;
                uint8_t linkLen = ae2data[2];
                if (3 + linkLen <= ae2data.size()) {
                    std::vector<uint8_t> linkBytes(ae2data.begin() + 3, ae2data.begin() + 3 + linkLen);
                    std::string b64 = AlterEgo::ChatLink::Base64Encode(linkBytes.data(), linkBytes.size());
                    std::string chatLink = "[&" + b64 + "]";

                    AlterEgo::DecodedBuildLink decoded;
                    if (AlterEgo::ChatLink::DecodeBuild(chatLink, decoded)) {
                        std::string profession = ProfessionFromCode(decoded.profession);
                        if (profession != "Unknown") {
                            std::string specName;
                            for (int i = 2; i >= 0; i--) {
                                if (decoded.specs[i].spec_id != 0) {
                                    const auto* si = AlterEgo::GW2API::GetSpecInfo(decoded.specs[i].spec_id);
                                    if (si && si->elite) specName = si->name;
                                    break;
                                }
                            }

                            {
                                std::lock_guard<std::mutex> lock(g_BuildToastMutex);
                                g_BuildToast.active = true;
                                g_BuildToast.sender = sender;
                                g_BuildToast.chat_link = chatLink;
                                g_BuildToast.ae2_code = ae2code;
                                g_BuildToast.profession = profession;
                                g_BuildToast.spec_name = specName;
                                g_BuildToast.channel = ChatChannelName(msg->Type);
                                g_BuildToast.has_gear = ae2hasGear;
                                g_ToastNeedsFocus = true;
                            }

                            // Pre-fetch palette data so import is instant
                            if (!AlterEgo::GW2API::HasPaletteData(profession)) {
                                AlterEgo::GW2API::FetchProfessionPaletteAsync(profession);
                            }

                            if (APIDefs) {
                                std::string log = "AE2 build detected in " + std::string(ChatChannelName(msg->Type))
                                    + " from " + sender + ": " + profession;
                                if (!specName.empty()) log += " (" + specName + ")";
                                if (ae2hasGear) log += " [with gear]";
                                APIDefs->Log(LOGL_DEBUG, "AlterEgo", log.c_str());
                            }
                            return; // AE2 found, done
                        }
                    }
                }
            }
        }
    }

    // --- Priority 2: Detect native GW2 build chat links [&...] ---
    {
        size_t pos = 0;
        while ((pos = content.find("[&", pos)) != std::string::npos) {
            size_t end = content.find(']', pos);
            if (end == std::string::npos) break;

            std::string link = content.substr(pos, end - pos + 1);
            // GW2 internal chat format escapes ] as \] — strip the backslash
            size_t bs = link.size() - 2; // position before ]
            if (bs > 0 && link[bs] == '\\') link.erase(bs, 1);
            pos = end + 1;

            auto linkType = AlterEgo::ChatLink::DetectType(link);
            if (linkType != AlterEgo::LINK_BUILD) continue;

            AlterEgo::DecodedBuildLink decoded;
            if (!AlterEgo::ChatLink::DecodeBuild(link, decoded)) continue;

            std::string profession = ProfessionFromCode(decoded.profession);
            if (profession == "Unknown") continue;

            std::string specName;
            for (int i = 2; i >= 0; i--) {
                if (decoded.specs[i].spec_id != 0) {
                    const auto* si = AlterEgo::GW2API::GetSpecInfo(decoded.specs[i].spec_id);
                    if (si && si->elite) specName = si->name;
                    break;
                }
            }

            {
                std::lock_guard<std::mutex> lock(g_BuildToastMutex);
                g_BuildToast.active = true;
                g_BuildToast.sender = sender;
                g_BuildToast.chat_link = link;
                g_BuildToast.ae2_code.clear();
                g_BuildToast.profession = profession;
                g_BuildToast.spec_name = specName;
                g_BuildToast.channel = ChatChannelName(msg->Type);
                g_BuildToast.has_gear = false;
                g_ToastNeedsFocus = true;
            }

            // Pre-fetch palette data so import is instant
            if (!AlterEgo::GW2API::HasPaletteData(profession)) {
                AlterEgo::GW2API::FetchProfessionPaletteAsync(profession);
            }

            if (APIDefs) {
                std::string log = "Build link detected in " + std::string(ChatChannelName(msg->Type))
                                + " from " + sender + ": " + profession;
                if (!specName.empty()) log += " (" + specName + ")";
                APIDefs->Log(LOGL_DEBUG, "AlterEgo", log.c_str());
            }

            break;
        }
    }
}

static void RenderBuildToast() {
    std::lock_guard<std::mutex> lock(g_BuildToastMutex);
    if (!g_BuildToast.active) return;

    ThemeGuard themeGuard;

    const float toastWidth = 380.0f;
    const float toastMinHeight = 140.0f;

    // Position: use saved position, or auto-center horizontally
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float posX = (g_ToastPosX < 0) ? (displaySize.x - toastWidth) * 0.5f : g_ToastPosX;
    float posY = g_ToastPosY;

    if (!g_ToastPosInitialized) {
        ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always);
        g_ToastPosInitialized = true;
    } else {
        ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Appearing);
    }
    ImGui::SetNextWindowSize(ImVec2(toastWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.96f);
    if (g_ToastNeedsFocus) {
        ImGui::SetNextWindowFocus();
        g_ToastNeedsFocus = false;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings;

    // Gold highlight border for attention
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.90f, 0.75f, 0.25f, 0.90f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 12));

    bool open = true;
    if (ImGui::Begin("Build Detected##AE_Toast", &open, flags)) {
        // Track position for drag-to-reposition
        ImVec2 wp = ImGui::GetWindowPos();
        if (wp.x != posX || wp.y != posY) {
            g_ToastPosX = wp.x;
            g_ToastPosY = wp.y;
        }

        // Header — gold accent bar
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        dl->AddRectFilled(
            ImVec2(winPos.x + 1, winPos.y + 1),
            ImVec2(winPos.x + winSize.x - 1, winPos.y + 5),
            IM_COL32(230, 190, 65, 220));

        ImGui::Spacing();

        // Title line
        ImGui::TextColored(ImVec4(0.90f, 0.75f, 0.25f, 1.0f), "BUILD SHARED");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.47f, 0.40f, 1.0f), "via %s", g_BuildToast.channel.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Sender + profession info
        ImGui::Text("From:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", g_BuildToast.sender.c_str());

        ImGui::Text("Build:");
        ImGui::SameLine();
        if (!g_BuildToast.spec_name.empty()) {
            ImGui::TextColored(ImVec4(0.9f, 0.87f, 0.78f, 1.0f), "%s (%s)",
                g_BuildToast.spec_name.c_str(), g_BuildToast.profession.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.9f, 0.87f, 0.78f, 1.0f), "%s",
                g_BuildToast.profession.c_str());
        }

        if (g_BuildToast.has_gear) {
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Includes gear, runes & sigils");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.47f, 0.40f, 1.0f), "Traits & skills only");
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // Action buttons — large and prominent
        float buttonWidth = (toastWidth - 28 - 8) * 0.5f; // half width minus padding and gap
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.45f, 0.18f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.60f, 0.25f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.70f, 0.30f, 1.00f));
        if (ImGui::Button("Import to Library", ImVec2(buttonWidth, 32))) {
            std::string buildName = g_BuildToast.spec_name.empty()
                ? g_BuildToast.profession
                : g_BuildToast.spec_name;
            buildName += " (" + g_BuildToast.sender + ")";

            AlterEgo::SavedBuild sb;
            bool imported = false;

            if (!g_BuildToast.ae2_code.empty()) {
                // AE2 import — includes gear data
                std::string ae2err;
                if (ImportBuildFromAE2(g_BuildToast.ae2_code, sb, ae2err)) {
                    sb.name = buildName;
                    AlterEgo::GW2API::AddSavedBuild(std::move(sb));
                    imported = true;
                    if (APIDefs) APIDefs->GUI_SendAlert("Full build imported from chat!");
                } else if (ae2err.find("Loading") != std::string::npos) {
                    // Palette data still loading — don't dismiss, let user retry
                    if (APIDefs) APIDefs->GUI_SendAlert("Loading skill data... click Import again.");
                } else {
                    if (APIDefs) {
                        std::string alertMsg = "AE2 import failed: " + ae2err;
                        APIDefs->GUI_SendAlert(alertMsg.c_str());
                    }
                }
            } else {
                // Plain chat link — traits & skills only
                std::string profession = g_BuildToast.profession;
                if (!profession.empty() && !AlterEgo::GW2API::HasPaletteData(profession)) {
                    AlterEgo::GW2API::FetchProfessionPaletteAsync(profession);
                    if (APIDefs) APIDefs->GUI_SendAlert("Loading skill data... click Import again.");
                } else if (DecodeBuildLink(g_BuildToast.chat_link, buildName, AlterEgo::GameMode::PvE, sb)) {
                    AlterEgo::GW2API::AddSavedBuild(std::move(sb));
                    imported = true;
                    if (APIDefs) APIDefs->GUI_SendAlert("Build imported from chat!");
                } else {
                    if (APIDefs) APIDefs->GUI_SendAlert("Failed to import build. Try again.");
                }
            }

            if (imported) g_BuildToast.active = false;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.20f, 0.12f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.30f, 0.14f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.38f, 0.16f, 1.00f));
        if (ImGui::Button("Dismiss", ImVec2(buttonWidth, 32))) {
            g_BuildToast.active = false;
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
    }
    ImGui::End();

    // X button also dismisses
    if (!open) {
        g_BuildToast.active = false;
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
}

// Render a skill icon with tooltip
static void RenderSkillIcon(uint32_t skill_id, float size) {
    if (skill_id == 0) {
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetCursorScreenPos(),
            ImVec2(ImGui::GetCursorScreenPos().x + size, ImGui::GetCursorScreenPos().y + size),
            IM_COL32(30, 30, 30, 200));
        ImGui::Dummy(ImVec2(size, size));
        return;
    }

    ImGui::PushID((int)(skill_id + 5000000));
    const auto* sinfo = AlterEgo::GW2API::GetSkillInfo(skill_id);
    Texture_t* tex = AlterEgo::IconManager::GetIcon(skill_id + 5000000);

    if (tex && tex->Resource) {
        ImGui::Image(tex->Resource, ImVec2(size, size));
    } else {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(pos,
            ImVec2(pos.x + size, pos.y + size), IM_COL32(40, 40, 60, 200));
        ImGui::Dummy(ImVec2(size, size));
        if (sinfo && !sinfo->icon_url.empty())
            AlterEgo::IconManager::RequestIcon(skill_id + 5000000, sinfo->icon_url);
    }

    if (ImGui::IsItemHovered() && sinfo) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), "%s", sinfo->name.c_str());
        if (!sinfo->description.empty()) {
            ImGui::PushTextWrapPos(300.0f);
            std::string sdesc = StripGW2Markup(sinfo->description);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", sdesc.c_str());
            ImGui::PopTextWrapPos();
        }
        ImGui::EndTooltip();
    }
    ImGui::PopID();
}

// =========================================================================
// Gear Customization Helpers
// =========================================================================

// Friendly attribute name mapping
static const char* FriendlyAttrName(const std::string& attr) {
    if (attr == "CritDamage") return "Ferocity";
    if (attr == "ConditionDamage") return "Condition Damage";
    if (attr == "ConditionDuration") return "Expertise";
    if (attr == "BoonDuration") return "Concentration";
    if (attr == "Healing") return "Healing Power";
    return attr.c_str();
}

// Curated rune list (name + item_id + summary). IDs from GW2 API.
struct RuneEntry { const char* name; uint32_t item_id; const char* bonus; };
static const RuneEntry RUNE_LIST[] = {
    {"Superior Rune of the Scholar",      91595, "+Power, +Ferocity, +10% dmg >90% HP"},
    {"Superior Rune of the Eagle",        91433, "+Power, +Precision, +Ferocity, +10% dmg >80% HP"},
    {"Superior Rune of Strength",         91423, "+Power, +Ferocity, +5% dmg on might"},
    {"Superior Rune of the Dragonhunter", 91432, "+Power, +Ferocity, +7% vs burning foes"},
    {"Superior Rune of the Berserker",    91397, "+Power, +Ferocity, +5% crit dmg"},
    {"Superior Rune of the Thief",        91579, "+Power, +Precision, +10% dmg from stealth"},
    {"Superior Rune of the Firebrand",    91538, "+Condi Dmg, +Expertise, +7% burning dmg"},
    {"Superior Rune of Balthazar",        91513, "+Condi Dmg, +Burning duration"},
    {"Superior Rune of the Trapper",      91508, "+Condi Dmg, +Expertise, +10% condi dur"},
    {"Superior Rune of the Afflicted",    91460, "+Condi Dmg, +Expertise"},
    {"Superior Rune of Tormenting",       91516, "+Condi Dmg, +Expertise, torment on hit"},
    {"Superior Rune of the Tempest",      91583, "+Ferocity, +Precision, stun on crit"},
    {"Superior Rune of Surging",          91591, "+Power, +Ferocity, swiftness on swap"},
    {"Superior Rune of the Pack",         91592, "+Power, +Precision, might/fury/swiftness"},
    {"Superior Rune of Vampirism",        91545, "+Power, +Ferocity, lifesteal"},
    {"Superior Rune of the Monk",         91501, "+Healing, +Outgoing Heal, +Boon Duration"},
    {"Superior Rune of the Water",        91518, "+Healing, +Boon Duration"},
    {"Superior Rune of Durability",       91444, "+Toughness, +Vitality, +Boon Duration"},
    {"Superior Rune of the Defender",     91404, "+Toughness, +Vitality"},
    {"Superior Rune of the Traveler",     91485, "+All stats, +25% movement"},
    {"Superior Rune of the Rebirth",      91627, "+Healing, +Boon Duration, auto-rez"},
    {"Superior Rune of the Nightmare",    91410, "+Condi Dmg, +Condi Duration"},
    {"Superior Rune of Thorns",           91381, "+Condi Dmg, +Expertise, +bleeding"},
    {"Superior Rune of the Citadel",      91588, "+Power, +Ferocity, +might on kill"},
    {"Superior Rune of Fireworks",        91482, "+Power, +Boon Duration"},
    {"Superior Rune of the Grove",        91475, "+Condi Dmg, +Healing"},
    {"Superior Rune of Sanctuary",        91605, "+Vitality, +Toughness, barrier on heal"},
    {"Superior Rune of Snowfall",         91512, "+Power, +Precision, superspeed"},
    {"Superior Rune of the Sunless",      91515, "+Condi Dmg, +Expertise, +torment"},
    {"Superior Rune of Resistance",       91411, "+Vitality, +Toughness, Resistance"},
    {"Superior Rune of the Weaver",       91451, "+Power, +Condi Dmg, +Ferocity"},
    {"Superior Rune of Scavenging",       91553, "+MF, +Power, +Precision"},
    {"Superior Rune of Antitoxin",        91641, "+Vitality, +Healing, condi cleanse"},
    {"Superior Rune of the Aristocracy",  91602, "+Power, +Precision, +might dur"},
    {"Superior Rune of Rage",             91503, "+Power, +Ferocity, fury on crit"},
    // --- Additional runes ---
    {"Superior Rune of Melandru",         24771, "+Toughness, -Condition Duration on self"},
    {"Superior Rune of the Chronomancer", 73399, "+Power, +Precision, +Boon Duration"},
    {"Superior Rune of Hoelbrak",         24729, "+Power, +Might dur, +Condi dur, +Boon dur"},
    {"Superior Rune of Divinity",         24732, "+All stats, +Crit damage"},
    {"Superior Rune of Dwayna",           24768, "+Healing, +Regeneration, +Vitality"},
    {"Superior Rune of the Undead",       24757, "+Condi Dmg, +Toughness"},
    {"Superior Rune of the Ogre",         24756, "+Power, +Ferocity, +Vitality"},
    {"Superior Rune of Altruism",         38206, "+Healing, +Boon Duration, +Vitality"},
    {"Superior Rune of the Warrior",      24821, "+Vitality, +Power"},
    {"Superior Rune of the Engineer",     24812, "+Toughness, +Condi Dmg"},
    {"Superior Rune of Grenth",           24779, "+Condi Dmg, +Chill duration"},
    {"Superior Rune of the Herald",       76100, "+Toughness, +Boon Duration"},
    {"Superior Rune of the Dolyak",       24699, "+Toughness, +Vitality, stability"},
    {"Superior Rune of the Ranger",       24815, "+Precision, +Ferocity"},
    {"Superior Rune of Leadership",       70600, "+All stats, +Boon Duration"},
    // --- Remaining superior runes ---
    {"Superior Rune of Evasion",          67344, "+Vitality, endurance refill on dodge"},
    {"Superior Rune of Exuberance",       44951, "+Vitality, +Toughness, +Might dur"},
    {"Superior Rune of Infiltration",     24703, "+Power, +Precision, stealth on skill"},
    {"Superior Rune of Lyssa",            24776, "+Condi Dmg, all boons + cleanse on elite"},
    {"Superior Rune of Mercy",            24708, "+Healing, +Vitality, faster revive"},
    {"Superior Rune of Nature's Bounty",  81091, "+Condi Dmg, +Expertise, +Regen dur"},
    {"Superior Rune of Orr",              24860, "+Condi Dmg, +Condi Duration"},
    {"Superior Rune of Perplexity",       44957, "+Condi Dmg, confusion on interrupt"},
    {"Superior Rune of Radiance",         67342, "+Power, +Ferocity, blind on swap"},
    {"Superior Rune of Rata Sum",         24726, "+Precision, +Ferocity, golem at low HP"},
    {"Superior Rune of Speed",            24720, "+Power, +Swiftness dur, +25% swiftness"},
    {"Superior Rune of Svanir",           24794, "+Toughness, +Chill dur, frost armor"},
    {"Superior Rune of the Adventurer",   24830, "+Power, +Precision, endurance on heal"},
    {"Superior Rune of the Air",          24750, "+Precision, +Ferocity, lightning on hit"},
    {"Superior Rune of the Baelfire",     24854, "+Condi Dmg, +Burn dur, condi remove"},
    {"Superior Rune of the Brawler",      24833, "+Power, +Ferocity, might on heal"},
    {"Superior Rune of the Cavalier",     83367, "+Toughness, +Ferocity, +25% movement"},
    {"Superior Rune of the Centaur",      24788, "+Power, +Swiftness dur, swiftness on heal"},
    {"Superior Rune of the Daredevil",    72852, "+Power, +Ferocity, daze on dodge"},
    {"Superior Rune of the Deadeye",      82791, "+Precision, +Ferocity, fury on stealth"},
    {"Superior Rune of the Druid",        70450, "+Healing, +Boon dur, heal on dodge"},
    {"Superior Rune of the Earth",        24744, "+Toughness, +Bleed dur, bleed on hit"},
    {"Superior Rune of the Elementalist", 24800, "+Power, +Ferocity, aura on swap"},
    {"Superior Rune of the Fire",         24747, "+Power, +Burn dur, fire aura on hit"},
    {"Superior Rune of the Flame Legion", 24797, "+Power, +Burn dur, burn on hit"},
    {"Superior Rune of the Flock",        24696, "+Power, +Vitality, heal on hit"},
    {"Superior Rune of the Forgeman",     24851, "+Toughness, +Burn dur, fire shield"},
    {"Superior Rune of the Golemancer",   24785, "+Power, +Ferocity, mini golem at low HP"},
    {"Superior Rune of the Guardian",     24824, "+Power, +Boon dur, aegis on hit"},
    {"Superior Rune of the Holosmith",    82633, "+Power, +Ferocity, lightning on crit"},
    {"Superior Rune of the Ice",          24753, "+Toughness, +Chill dur, chill on hit"},
    {"Superior Rune of the Krait",        24762, "+Condi Dmg, +Bleed dur, torment on bleed"},
    {"Superior Rune of the Lich",         24688, "+Condi Dmg, +Ferocity, lifesteal on crit"},
    {"Superior Rune of the Mad King",     36044, "+Power, +Ferocity, plague at low HP"},
    {"Superior Rune of the Mesmer",       24803, "+Condi Dmg, +Confusion dur"},
    {"Superior Rune of the Mirage",       84127, "+Condi Dmg, +Expertise, mirror on dodge"},
    {"Superior Rune of the Necromancer",  24806, "+Condi Dmg, +Vitality, life siphon"},
    {"Superior Rune of the Privateer",    24782, "+Power, +Swiftness dur, might on kill"},
    {"Superior Rune of the Reaper",       70829, "+Power, +Ferocity, chill on crit"},
    {"Superior Rune of the Renegade",     83502, "+Condi Dmg, +Expertise, summon Razah"},
    {"Superior Rune of the Revenant",     69370, "+All stats, +Boon Duration"},
    {"Superior Rune of the Scourge",      83663, "+Condi Dmg, +Expertise, torment on hit"},
    {"Superior Rune of the Scrapper",     71276, "+Toughness, +Vitality, barrier on hit"},
    {"Superior Rune of the Soulbeast",    83964, "+Precision, +Ferocity, fury on swap"},
    {"Superior Rune of the Spellbreaker", 84749, "+Power, +Precision, boon strip on hit"},
    {"Superior Rune of the Stars",        85713, "+All stats, -Incoming Condi"},
    {"Superior Rune of the Trooper",      24827, "+Toughness, +Boon dur, shouts cleanse"},
    {"Superior Rune of the Wurm",         24791, "+Power, +Toughness, wurm at low HP"},
    {"Superior Rune of the Zephyrite",    88118, "+All stats, swiftness on dodge"},
};
static const int RUNE_COUNT = sizeof(RUNE_LIST) / sizeof(RUNE_LIST[0]);

// Curated sigil list (name + item_id + summary). IDs from GW2 API.
struct SigilEntry { const char* name; uint32_t item_id; const char* bonus; };
static const SigilEntry SIGIL_LIST[] = {
    {"Superior Sigil of Force",           91439, "+5% damage"},
    {"Superior Sigil of Impact",          91405, "+3% stun duration"},
    {"Superior Sigil of Accuracy",        91607, "+7% crit chance"},
    {"Superior Sigil of the Night",       91389, "+10% dmg at night"},
    {"Superior Sigil of Undead Slaying",  91524, "+10% vs undead"},
    {"Superior Sigil of Demon Slaying",   91431, "+10% vs demons"},
    {"Superior Sigil of Air",             91520, "Lightning strike on crit"},
    {"Superior Sigil of Fire",            91559, "AoE fire on crit"},
    {"Superior Sigil of Earth",           91531, "Bleed on crit"},
    {"Superior Sigil of Geomancy",        91552, "Bleed AoE on swap"},
    {"Superior Sigil of Doom",            91480, "Poison on swap"},
    {"Superior Sigil of Torment",         91412, "Torment on crit"},
    {"Superior Sigil of Malice",          91478, "+10% condition duration"},
    {"Superior Sigil of Smoldering",      91488, "+20% burning duration"},
    {"Superior Sigil of Venom",           91532, "+20% poison duration"},
    {"Superior Sigil of Agony",           91534, "+20% torment duration"},
    {"Superior Sigil of Bursting",        91416, "+5% condition damage"},
    {"Superior Sigil of Concentration",   91473, "+10% boon duration"},
    {"Superior Sigil of Transference",    91448, "+10% outgoing heal"},
    {"Superior Sigil of Renewal",         91400, "Heal on kill"},
    {"Superior Sigil of Blood",           91604, "Lifesteal on crit"},
    {"Superior Sigil of Strength",        91561, "Might on crit"},
    {"Superior Sigil of Severance",       91499, "Quickness on cc"},
    {"Superior Sigil of Energy",          91441, "Endurance on swap"},
    {"Superior Sigil of Paralyzation",    91398, "+30% stun duration"},
    {"Superior Sigil of Absorption",      91589, "Steal boon on hit"},
    {"Superior Sigil of Cleansing",       91548, "Condi cleanse on swap"},
    {"Superior Sigil of Draining",        91544, "Steal HP on swap"},
    {"Superior Sigil of Demons",          91388, "+10% vs demons"},
    {"Superior Sigil of Serpent Slaying", 91456, "+10% vs destroyers"},
    // --- Additional sigils ---
    {"Superior Sigil of Battle",          24601, "Might on weapon swap"},
    {"Superior Sigil of Vision",          24600, "Crit after weapon swap"},
    {"Superior Sigil of Hydromancy",      24597, "Chill on weapon swap"},
    {"Superior Sigil of Nullification",   24572, "Remove boon on hit"},
    {"Superior Sigil of Bloodlust",       24575, "+Power stacks on kill"},
    {"Superior Sigil of Corruption",      24578, "+Condi stacks on kill"},
    {"Superior Sigil of Ice",             24555, "Chill on crit"},
    {"Superior Sigil of Chilling",        24630, "Chill on hit"},
    {"Superior Sigil of Leeching",        24599, "Steal HP on kill"},
    {"Superior Sigil of Celerity",        24865, "Quickness on disable"},
    // --- Remaining superior sigils ---
    {"Superior Sigil of Agility",         72092, "Swiftness + quickness on swap"},
    {"Superior Sigil of Benevolence",     24584, "Outgoing heal stacks on kill"},
    {"Superior Sigil of Blight",          67913, "Poison AoE on crit"},
    {"Superior Sigil of Bounty",          81045, "Concentration stacks on kill"},
    {"Superior Sigil of Centaur Slaying", 24645, "+7% vs centaurs"},
    {"Superior Sigil of Cruelty",         67341, "Ferocity stacks on kill"},
    {"Superior Sigil of Debility",        24636, "+20% weakness duration"},
    {"Superior Sigil of Destroyer Slaying",24654,"+7% vs destroyers"},
    {"Superior Sigil of Dreams",          24681, "+7% vs Nightmare Court"},
    {"Superior Sigil of Elemental Slaying",24661,"+7% vs elementals"},
    {"Superior Sigil of Frailty",         24567, "Vuln on flank/behind/defiant"},
    {"Superior Sigil of Frenzy",          82876, "-2s skill cooldown on kill"},
    {"Superior Sigil of Generosity",      38294, "Transfer condi on crit"},
    {"Superior Sigil of Ghost Slaying",   24809, "+7% vs ghosts"},
    {"Superior Sigil of Grawl Slaying",   24648, "+7% vs grawl"},
    {"Superior Sigil of Hobbling",        24627, "+20% cripple duration"},
    {"Superior Sigil of Hologram Slaying",91339, "+17% vs holograms"},
    {"Superior Sigil of Icebrood Slaying",24651, "+7% vs icebrood"},
    {"Superior Sigil of Incapacitation",  67343, "Cripple on flank/defiant"},
    {"Superior Sigil of Justice",         24678, "+7% vs outlaws"},
    {"Superior Sigil of Karka Slaying",   37912, "+7% vs karka"},
    {"Superior Sigil of Life",            24582, "Healing stacks on kill"},
    {"Superior Sigil of Luck",            24591, "75% boon on kill"},
    {"Superior Sigil of Mad Scientists",  24672, "+7% vs Inquest"},
    {"Superior Sigil of Mischief",        68436, "Blinding snowballs on swap"},
    {"Superior Sigil of Momentum",        49457, "Toughness stacks on kill"},
    {"Superior Sigil of Ogre Slaying",    24655, "+7% vs ogres"},
    {"Superior Sigil of Perception",      24580, "Precision stacks on kill"},
    {"Superior Sigil of Peril",           24621, "+20% vuln duration"},
    {"Superior Sigil of Purity",          24571, "Cleanse on flank/defiant"},
    {"Superior Sigil of Rage",            24561, "Quickness on crit"},
    {"Superior Sigil of Rending",         73532, "Vuln on interrupt"},
    {"Superior Sigil of Restoration",     24594, "Heal on interrupt"},
    {"Superior Sigil of Ruthlessness",    71130, "Might on interrupt"},
    {"Superior Sigil of Smothering",      24675, "+7% vs Flame Legion"},
    {"Superior Sigil of Sorrow",          24684, "+7% vs dredge"},
    {"Superior Sigil of Speed",           24589, "Swiftness on kill"},
    {"Superior Sigil of Stamina",         24592, "Endurance refill on kill"},
    {"Superior Sigil of Water",           24551, "AoE heal on crit"},
    {"Superior Sigil of Wrath",           24667, "+7% vs Sons of Svanir"},
    {"Superior Sigil of the Stars",       86170, "All stats stacks on kill"},
};
static const int SIGIL_COUNT = sizeof(SIGIL_LIST) / sizeof(SIGIL_LIST[0]);

// Curated relic list with exotic item IDs from GW2 API
struct RelicEntry { const char* name; uint32_t item_id; const char* bonus; };
static const RelicEntry RELIC_LIST[] = {
    {"Relic of the Eagle",          104241, "+10% dmg >80% HP"},
    {"Relic of Fireworks",          100947, "Strike dmg on weapon skill"},
    {"Relic of the Thief",          100976, "Weapon skill CDR, strike dmg"},
    {"Relic of Cerus",              100074, "Elite skill +dmg, boon/condi"},
    {"Relic of Isgarren",            99997, "+strike/condi dur after evade"},
    {"Relic of the Brawler",        100527, "Strike dmg, protection, resolution"},
    {"Relic of the Claw",           103574, "Strike dmg on disable"},
    {"Relic of the Deadeye",        100924, "Strike dmg, cantrip bonus"},
    {"Relic of the Dragonhunter",   100090, "Trap dmg + condi dur"},
    {"Relic of Mount Balrior",      103872, "Elite skill +strike dmg"},
    {"Relic of Peitha",             100177, "Torment/strike on shadowstep"},
    {"Relic of Lyhr",               100461, "Heal + strike dmg"},
    {"Relic of Fire",               104501, "Fire aura on heal skill"},
    {"Relic of Dagda",              100942, "Elite skill condi transfer"},
    {"Relic of the Fractal",        100153, "Bleed, burn, torment"},
    {"Relic of Akeem",              100432, "Confusion + torment on disable"},
    {"Relic of the Aristocracy",    100849, "Condi dur, weakness, vuln"},
    {"Relic of the Afflicted",      100693, "Poison + bleed"},
    {"Relic of the Nightmare",      100579, "Elite: fear + poison"},
    {"Relic of the Krait",          100230, "Elite: conditions"},
    {"Relic of the Sorcerer",       101863, "Vuln on cripple/chill/immob"},
    {"Relic of the Mirage",         100158, "Torment on evade"},
    {"Relic of Thorns",             104424, "Condition dmg, poison"},
    {"Relic of the Blightbringer",  102199, "Poison + condi"},
    {"Relic of Nourys",             101191, "Boon strip + dmg/heal"},
    {"Relic of the Scourge",        100368, "Barrier + condi dur"},
    {"Relic of the Demon Queen",    101166, "Poison on disable"},
    {"Relic of Mosyn",              101801, "Condition on evade"},
    {"Relic of the Biomancer",      106364, "Heal + bleed/poison"},
    {"Relic of the First Revenant", 105585, "Condi dmg + resistance"},
    {"Relic of Durability",         100562, "Protection/regen/resolution on heal"},
    {"Relic of the Monk",           100031, "Healing + boon"},
    {"Relic of the Flock",          100633, "Heal on healing skill"},
    {"Relic of Castora",            105652, "Heal based on HP"},
    {"Relic of the Water",          100659, "Condi cleanse on heal skill"},
    {"Relic of the Defender",       100934, "Heal on block"},
    {"Relic of Vampirism",          100676, "Lifesteal + heal on kill"},
    {"Relic of Dwayna",             100442, "Regen"},
    {"Relic of the Centaur",        100385, "Stability on heal skill"},
    {"Relic of Mercy",              100429, "Revive speed + heal"},
    {"Relic of Nayos",              101198, "Condi + heal"},
    {"Relic of Karakosa",           101268, "Heal on blast finisher"},
    {"Relic of the Nautical Beast", 106920, "Heal + water field"},
    {"Relic of the Living City",    104938, "Titan Potential on field/disable/evade"},
    {"Relic of Leadership",         100625, "Condi cleanse + boon"},
    {"Relic of the Herald",         100219, "Concentration + boon"},
    {"Relic of the Firebrand",      100453, "Mantra + boon dur"},
    {"Relic of Rivers",             103015, "Alacrity + regen on dodge"},
    {"Relic of the Chronomancer",   100450, "Quickness on well"},
    {"Relic of the Pack",           100752, "Superspeed/might/fury on elite"},
    {"Relic of Altruism",           104256, "Might + fury on heal skill"},
    {"Relic of the Cavalier",       100542, "Aegis/quickness/swiftness on engage"},
    {"Relic of the Twin Generals",  101767, "Might + weakness on heal"},
    {"Relic of Mabon",              100115, "Might + Mabon's Strength"},
    {"Relic of the Midnight King",  101139, "Might + fury on disable"},
    {"Relic of Speed",              100148, "Movement speed + swiftness"},
    {"Relic of the Wayfinder",      101943, "Movement speed + burst speed"},
    {"Relic of Febe",               101116, "Heal skill + swiftness"},
    {"Relic of the Adventurer",     100561, "Endurance on heal skill"},
    {"Relic of Evasion",            100886, "Vigor + evade bonus"},
    {"Relic of Fog",                107030, "Dodge: glancing blows"},
    {"Relic of the Daredevil",      100345, "Dodge rolling bonuses"},
    {"Relic of Geysers",            103763, "Endurance + vigor"},
    {"Relic of Resistance",         100794, "Resistance on heal skill"},
    {"Relic of Antitoxin",          100390, "Condi cleanse"},
    {"Relic of the Trooper",        100411, "Condi cleanse on shout"},
    {"Relic of the Astral Ward",    100388, "Signet + resistance"},
    {"Relic of Reunification",      103984, "Frost/light aura on heal"},
    {"Relic of Surging",            100063, "Shocking aura on elite"},
    {"Relic of the Earth",          100435, "Protection + magnetic aura"},
    {"Relic of the Warrior",        100299, "Weapon swap CDR"},
    {"Relic of the Alliance",       107192, "Signet bonus"},
    {"Relic of Shackles",           106916, "Immobilize"},
    {"Relic of the Necromancer",    100580, "Fear"},
    {"Relic of the Scoundrel",      106355, "Blind/weakness on crit"},
    {"Relic of the Reaper",         100739, "Chill on shout"},
    {"Relic of Atrocity",           102245, "Lifesteal"},
    {"Relic of the Mist Stranger",  106206, "Siphon health"},
    {"Relic of the Ogre",           100311, "Summon rock dog"},
    {"Relic of the Lich",           100238, "Summon jagged horror"},
    {"Relic of the Golemancer",     100403, "Summon golem"},
    {"Relic of the Privateer",      100479, "Summon parrot"},
    {"Relic of the Beehive",        103977, "Elite skill summon"},
    {"Relic of Sorrow",             103424, "Elite: destroy projectiles"},
    {"Relic of the Citadel",        100448, "Elite: stun"},
    {"Relic of the Holosmith",      100908, "Elite skill bonus"},
    {"Relic of the Ice",            100048, "Elite: chill"},
    {"Relic of the Sunless",        100400, "Elite: poison/cripple"},
    {"Relic of the Wizard's Tower", 100557, "Elite: reflect + pull"},
    {"Relic of the Pirate Queen",   106221, "Quickness on disable"},
    {"Relic of the Phenom",         104733, "Cantrip/meditation + protection"},
    {"Relic of the Weaver",         100194, "Stance + strike dmg"},
    {"Relic of Bava Nisos",         104848, "Stance bonus"},
    {"Relic of the Zephyrite",      100893, "Protection + resolution"},
    {"Relic of Bloodstone",         104800, "Bloodstone blast finisher"},
    {"Relic of the Unseen Invasion",100694, "Superspeed + stealth"},
    {"Relic of Mistburn",           104994, "WvW combat bonus"},
    {"Relic of the Forest Dweller", 107124, "Nature bonus"},
    {"Relic of the Founding",       101737, "Barrier on combo field"},
    {"Relic of the Mists Tide",     103901, "Condi on combo finisher"},
    {"Relic of the Stormsinger",    102595, "Movement skill bonus"},
    {"Relic of the Steamshrieker",  104022, "Burn on water combo"},
    {"Relic of Vass",               100775, "Heal + elixir + poison"},
    {"Relic of Zakiros",            101955, "Fury + crit dmg + healing"},
    {"Relic of the Coral Heart",    107061, "Underwater bonus"},
    {"Relic of Agony",              104849, "Interrupt bonus"},
    {"Relic of Galdra",             109264, "Elite: burning projectiles"},
    {"Relic of the Cruel Overseer", 109664, "Condi stacks -> healing burst"},
    {"Relic of the Director",       109351, "Vuln on heal + damage bonus"},
    {"Relic of the Doyen",          109522, "Dmg reduction while resolution"},
    {"Relic of the Sacred Grounds", 109267, "Protection on well/consecration"},
    {"Relic of Watch",              109709, "Immob duration + weaken on immob"},
};
static const int RELIC_COUNT = sizeof(RELIC_LIST) / sizeof(RELIC_LIST[0]);

// Seed the persistent name→ID cache from the curated rune/sigil/relic tables.
// Called once on startup so that even fresh installs can resolve names instantly.
static void SeedItemNameCache() {
    for (int i = 0; i < RUNE_COUNT; i++)
        AlterEgo::GW2API::CacheItemNameId(RUNE_LIST[i].name, RUNE_LIST[i].item_id);
    for (int i = 0; i < SIGIL_COUNT; i++)
        AlterEgo::GW2API::CacheItemNameId(SIGIL_LIST[i].name, SIGIL_LIST[i].item_id);
    for (int i = 0; i < RELIC_COUNT; i++)
        AlterEgo::GW2API::CacheItemNameId(RELIC_LIST[i].name, RELIC_LIST[i].item_id);
}

// Slot display name for gear panel
static const char* GearSlotDisplayName(const std::string& slot) {
    if (slot == "WeaponA1") return "Main Hand A";
    if (slot == "WeaponA2") return "Off Hand A";
    if (slot == "WeaponB1") return "Main Hand B";
    if (slot == "WeaponB2") return "Off Hand B";
    if (slot == "Accessory1") return "Accessory 1";
    if (slot == "Accessory2") return "Accessory 2";
    if (slot == "Ring1") return "Ring 1";
    if (slot == "Ring2") return "Ring 2";
    return slot.c_str();
}

static bool IsArmorSlot(const std::string& s) {
    return s == "Helm" || s == "Shoulders" || s == "Coat" ||
           s == "Gloves" || s == "Leggings" || s == "Boots";
}
static bool IsWeaponSlot(const std::string& s) {
    return s == "WeaponA1" || s == "WeaponA2" || s == "WeaponB1" || s == "WeaponB2";
}
static bool IsMainhandSlot(const std::string& s) {
    return s == "WeaponA1" || s == "WeaponB1";
}
static bool IsOffhandSlot(const std::string& s) {
    return s == "WeaponA2" || s == "WeaponB2";
}
static bool IsTrinketSlot(const std::string& s) {
    return s == "Backpack" || s == "Accessory1" || s == "Accessory2" ||
           s == "Amulet" || s == "Ring1" || s == "Ring2";
}
static bool IsWeaponTwoHanded(const std::string& profession, const std::string& weaponType) {
    if (weaponType.empty()) return false;
    const auto* profWeapons = AlterEgo::GW2API::GetProfessionWeapons(profession);
    if (!profWeapons) return false;
    auto it = profWeapons->find(weaponType);
    return it != profWeapons->end() && it->second.two_handed;
}

// Profession -> armor weight class
static int GetArmorWeight(const std::string& profession) {
    // 0=Heavy, 1=Medium, 2=Light
    if (profession == "Warrior" || profession == "Guardian" || profession == "Revenant") return 0;
    if (profession == "Ranger" || profession == "Thief" || profession == "Engineer") return 1;
    if (profession == "Elementalist" || profession == "Mesmer" || profession == "Necromancer") return 2;
    // Elite specs map to base profession via the profession field in SavedBuild
    return 0; // fallback heavy
}

// Obsidian armor item IDs [weight][slot]: weight 0=Heavy,1=Medium,2=Light, slot 0=Helm..5=Boots
static const uint32_t OBSIDIAN_ARMOR[3][6] = {
    // Heavy: Helmet, Pauldrons, Breastplate, Gauntlets, Cuisses, Greaves
    { 101544, 101551, 101521, 101609, 101568, 101460 },
    // Medium: Mask, Shoulders, Jacket, Gloves, Leggings, Boots
    { 101614, 101645, 101556, 101570, 101579, 101602 },
    // Light: Crown, Mantle, Regalia, Gloves, Pants, Shoes
    { 101516, 101462, 101499, 101536, 101501, 101535 },
};

// Gen1 legendary weapon item IDs by weapon type name
static uint32_t GetLegendaryWeaponId(const std::string& wtype) {
    if (wtype == "Axe") return 30684;           // Frostfang
    if (wtype == "Dagger") return 30687;         // Incinerator
    if (wtype == "Focus") return 30701;          // The Minstrel
    if (wtype == "Greatsword") return 30703;     // Sunrise
    if (wtype == "Hammer") return 30690;         // The Juggernaut
    if (wtype == "Longbow") return 30685;        // Kudzu
    if (wtype == "Mace") return 30692;           // The Moot
    if (wtype == "Pistol") return 30693;         // Quip
    if (wtype == "Rifle") return 30694;          // The Predator
    if (wtype == "Scepter") return 30695;        // Meteorlogicus
    if (wtype == "Shield") return 30696;         // The Flameseeker Prophecies
    if (wtype == "Shortbow") return 30686;       // The Dreamer
    if (wtype == "Staff") return 30698;          // The Bifrost
    if (wtype == "Sword") return 30699;          // Bolt
    if (wtype == "Torch") return 30700;          // Rodgort
    if (wtype == "Warhorn") return 30702;        // Howler
    if (wtype == "Spear") return 30691;          // Kamohoali'i Kotaki
    return 0;
}

// Legendary trinket item IDs by slot
static uint32_t GetTrinketIconId(const std::string& slot) {
    if (slot == "Backpack") return 74155;        // Ad Infinitum
    if (slot == "Accessory1") return 81908;      // Aurora
    if (slot == "Accessory2") return 91048;      // Vision
    if (slot == "Amulet") return 95380;          // Prismatic Champion's Regalia
    if (slot == "Ring1") return 93105;           // Conflux
    if (slot == "Ring2") return 93105;           // Conflux
    if (slot == "Relic") return 101582;          // Legendary Relic
    return 0;
}

// Get the representative item ID for a build gear slot icon
static uint32_t GetGearSlotIconItemId(const AlterEgo::SavedBuild& build, const std::string& slot) {
    if (IsArmorSlot(slot)) {
        int weight = GetArmorWeight(build.profession);
        int idx = -1;
        if (slot == "Helm") idx = 0;
        else if (slot == "Shoulders") idx = 1;
        else if (slot == "Coat") idx = 2;
        else if (slot == "Gloves") idx = 3;
        else if (slot == "Leggings") idx = 4;
        else if (slot == "Boots") idx = 5;
        if (idx >= 0) return OBSIDIAN_ARMOR[weight][idx];
    } else if (IsWeaponSlot(slot)) {
        auto it = build.gear.find(slot);
        if (it != build.gear.end() && !it->second.weapon_type.empty())
            return GetLegendaryWeaponId(it->second.weapon_type);
    } else {
        return GetTrinketIconId(slot);
    }
    return 0;
}

// Open the gear customize dialog for a specific slot
static void OpenGearDialog(const std::string& buildId, const std::string& slot,
                           const AlterEgo::BuildGearSlot& current) {
    g_GearDialogOpen = true;
    g_GearDialogSlot = slot;
    g_GearDialogBuildId = buildId;
    g_GearSelectedStatId = current.stat_id;
    bool isWeaponSlot = (slot.find("Weapon") == 0);
    g_GearSelectorTab = isWeaponSlot ? 2 : 0;
    g_GearEditingSigil2 = false;
    memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
}

// Build a sorted list of stat combos for the selection panel
static std::vector<const AlterEgo::ItemStatInfo*> GetSortedStatCombos(const char* filter) {
    std::vector<const AlterEgo::ItemStatInfo*> result;
    const auto& allStats = AlterEgo::GW2API::GetAllItemStats();
    std::string filterLower;
    if (filter && filter[0]) {
        filterLower = filter;
        for (auto& c : filterLower) c = (char)tolower(c);
    }
    std::set<std::string> seenNames;
    for (const auto& [id, info] : allStats) {
        if (info.name.empty() || info.attributes.empty()) continue;
        // Deduplicate: GW2 API has multiple stat IDs with the same name (different tiers)
        if (!seenNames.insert(info.name).second) continue;
        if (!filterLower.empty()) {
            std::string nameLower = info.name;
            for (auto& c : nameLower) c = (char)tolower(c);
            bool matchesAttr = false;
            for (const auto& a : info.attributes) {
                std::string aLower = FriendlyAttrName(a);
                for (auto& c : aLower) c = (char)tolower(c);
                if (aLower.find(filterLower) != std::string::npos) { matchesAttr = true; break; }
            }
            if (nameLower.find(filterLower) == std::string::npos && !matchesAttr) continue;
        }
        result.push_back(&info);
    }
    std::sort(result.begin(), result.end(),
        [](const AlterEgo::ItemStatInfo* a, const AlterEgo::ItemStatInfo* b) {
            return a->name < b->name;
        });
    return result;
}

// Helper: render a card-based selector list and return the selected name (empty = no change)
// Returns true if a card was clicked (and sets outName)
static bool RenderCardSelector(const char* searchBuf, const char* childId,
                               const char* const* names, const char* const* descs, int count,
                               const std::string& currentSelection) {
    bool changed = false;
    std::string filterLower;
    if (searchBuf && searchBuf[0]) {
        filterLower = searchBuf;
        for (auto& c : filterLower) c = (char)tolower(c);
    }

    ImGui::BeginChild(childId, ImVec2(0, -30), true);
    for (int i = 0; i < count; i++) {
        if (!filterLower.empty()) {
            std::string nameLow = names[i];
            for (auto& c : nameLow) c = (char)tolower(c);
            std::string descLow = descs[i];
            for (auto& c : descLow) c = (char)tolower(c);
            if (nameLow.find(filterLower) == std::string::npos &&
                descLow.find(filterLower) == std::string::npos) continue;
        }

        ImGui::PushID(i);
        bool isSelected = (currentSelection == names[i]);

        ImVec2 cardStart = ImGui::GetCursorScreenPos();
        float cardW = ImGui::GetContentRegionAvail().x;
        float cardH = 32.0f;

        ImU32 bgCol = isSelected ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
        ImGui::GetWindowDrawList()->AddRectFilled(
            cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH), bgCol, 3.0f);
        ImGui::GetWindowDrawList()->AddRect(
            cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH),
            isSelected ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

        ImGui::SetCursorScreenPos(cardStart);
        if (ImGui::InvisibleButton("##sel", ImVec2(cardW, cardH))) {
            changed = true;
        }

        ImVec2 textPos(cardStart.x + 6, cardStart.y + 2);
        ImGui::GetWindowDrawList()->AddText(textPos,
            IM_COL32(200, 180, 255, 255), names[i]);
        textPos.y += 14.0f;
        ImGui::GetWindowDrawList()->AddText(textPos,
            IM_COL32(150, 150, 150, 220), descs[i]);
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x, cardStart.y + cardH + 2));

        ImGui::PopID();
        if (changed) {
            ImGui::EndChild();
            return true; // caller should set the name from names[i]
        }
    }
    ImGui::EndChild();
    return false;
}

// Render the Customize popup dialog
static void RenderGearCustomizeDialog() {
    if (!g_GearDialogOpen) return;

    ImGui::SetNextWindowSize(ImVec2(540, 440), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Customize##GearDialog", &g_GearDialogOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // Find the build being edited
    AlterEgo::SavedBuild* editBuild = nullptr;
    {
        auto& builds = const_cast<std::vector<AlterEgo::SavedBuild>&>(
            AlterEgo::GW2API::GetSavedBuilds());
        for (auto& b : builds) {
            if (b.id == g_GearDialogBuildId) { editBuild = &b; break; }
        }
    }
    if (!editBuild) {
        ImGui::Text("Build not found.");
        ImGui::End();
        return;
    }

    auto& gs = editBuild->gear[g_GearDialogSlot];
    gs.slot = g_GearDialogSlot;
    bool isArmor = IsArmorSlot(g_GearDialogSlot);
    bool isWeapon = IsWeaponSlot(g_GearDialogSlot);
    bool isRelic = (g_GearDialogSlot == "Relic");

    // Two-column layout: left = current item, right = selector
    if (ImGui::BeginTable("##custLayout", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("##custLeft", ImGuiTableColumnFlags_WidthFixed, 220.0f);
        ImGui::TableSetupColumn("##custRight", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        // === LEFT: Current slot info ===
        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", GearSlotDisplayName(g_GearDialogSlot));
        ImGui::Separator();

        // Weapon type area FIRST (clickable to switch to weapon type tab, weapon slots only)
        if (isWeapon) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float h = 30.0f;
            ImU32 bg = (g_GearSelectorTab == 2) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 2) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##weaponTypeArea", ImVec2(w, h))) {
                g_GearSelectorTab = 2;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (!gs.weapon_type.empty()) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Weapon: %s", WeaponDisplayName(gs.weapon_type));
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(220, 200, 120, 255), buf);
            } else {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), "Weapon: (click to select)");
            }
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
        }

        if (isRelic) {
            // Relic display area (clickable to select relic)
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float h = 44.0f;
            ImU32 bg = (g_GearSelectorTab == 0) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 0) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##relicArea", ImVec2(w, h))) {
                g_GearSelectorTab = 0;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (!editBuild->relic_name.empty()) {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(200, 180, 255, 255), editBuild->relic_name.c_str());
                // Show bonus text if we can find it in the list
                for (int i = 0; i < RELIC_COUNT; i++) {
                    if (editBuild->relic_name == RELIC_LIST[i].name) {
                        tp.y += 14.0f;
                        ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(150, 150, 150, 220), RELIC_LIST[i].bonus);
                        break;
                    }
                }
            } else {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), "Relic: (click to select)");
            }
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
        } else {
            // Stat combo (clickable to switch to stats tab)
            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            // Dynamic height: stat name line + one line per attribute + padding
            int numAttrs = 0;
            if (gs.stat_id != 0) {
                const auto* si = AlterEgo::GW2API::GetItemStatInfo(gs.stat_id);
                if (si) numAttrs = (int)si->attributes.size();
            }
            float h = 6.0f + 16.0f + numAttrs * 13.0f;
            if (h < 30.0f) h = 30.0f;
            ImU32 bg = (g_GearSelectorTab == 0) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 0) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##statArea", ImVec2(w, h))) {
                g_GearSelectorTab = 0;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (gs.stat_id != 0 && !gs.stat_name.empty()) {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(255, 210, 80, 255), gs.stat_name.c_str());
                const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(gs.stat_id);
                if (statInfo) {
                    for (const auto& a : statInfo->attributes) {
                        tp.y += 13.0f;
                        char buf[64];
                        snprintf(buf, sizeof(buf), "  +%s", FriendlyAttrName(a));
                        ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(100, 200, 100, 255), buf);
                    }
                }
            } else {
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), "Stat: (click to select)");
            }
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
        }

        // Rune/Sigil area (clickable to switch to rune/sigil tab)
        if (isArmor || isWeapon) {
            bool isTwoHand = isWeapon && editBuild && IsWeaponTwoHanded(editBuild->profession, gs.weapon_type);
            int sigilCount = (isWeapon && isTwoHand) ? 2 : 1;

            for (int sigilIdx = 0; sigilIdx < sigilCount; sigilIdx++) {
                const char* upgradeLabel;
                const std::string* upgradeName;
                if (isArmor) {
                    upgradeLabel = "Rune";
                    upgradeName = &gs.rune;
                } else if (sigilIdx == 0) {
                    upgradeLabel = isTwoHand ? "Sigil 1" : "Sigil";
                    upgradeName = &gs.sigil;
                } else {
                    upgradeLabel = "Sigil 2";
                    upgradeName = &gs.sigil2;
                }

                bool isActiveCard = (g_GearSelectorTab == 1) &&
                    (isArmor || (!g_GearEditingSigil2 && sigilIdx == 0) || (g_GearEditingSigil2 && sigilIdx == 1));

                ImVec2 p = ImGui::GetCursorScreenPos();
                float w = ImGui::GetContentRegionAvail().x;
                float h = 30.0f;
                ImU32 bg = isActiveCard ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
                ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
                ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                    isActiveCard ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

                char btnId[32];
                snprintf(btnId, sizeof(btnId), "##upgradeArea%d", sigilIdx);
                ImGui::SetCursorScreenPos(p);
                if (ImGui::InvisibleButton(btnId, ImVec2(w, h))) {
                    g_GearSelectorTab = 1;
                    g_GearEditingSigil2 = (sigilIdx == 1);
                    memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
                }

                ImVec2 tp(p.x + 6, p.y + 3);
                if (!upgradeName->empty()) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%s: %s", upgradeLabel, upgradeName->c_str());
                    ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(200, 180, 255, 255), buf);
                } else {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%s: (click to select)", upgradeLabel);
                    ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), buf);
                }
                tp.y += 14.0f;
                ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Apply-to-all buttons
        if (IsArmorSlot(g_GearDialogSlot) && gs.stat_id != 0) {
            if (ImGui::Button("Apply Stat to All Armor")) {
                const char* armorSlots[] = {"Helm","Shoulders","Coat","Gloves","Leggings","Boots"};
                for (const char* s : armorSlots) {
                    auto& target = editBuild->gear[s];
                    target.slot = s;
                    target.stat_id = gs.stat_id;
                    target.stat_name = gs.stat_name;
                }
                AlterEgo::GW2API::SaveBuildLibrary();
            }
        }
        if (IsArmorSlot(g_GearDialogSlot) && !gs.rune.empty()) {
            if (ImGui::Button("Apply Rune to All Armor")) {
                const char* armorSlots[] = {"Helm","Shoulders","Coat","Gloves","Leggings","Boots"};
                for (const char* s : armorSlots) {
                    auto& target = editBuild->gear[s];
                    target.slot = s;
                    target.rune = gs.rune;
                    target.rune_id = gs.rune_id;
                }
                editBuild->rune_name = gs.rune;
                editBuild->rune_id = gs.rune_id;
                AlterEgo::GW2API::SaveBuildLibrary();
            }
        }
        if (IsTrinketSlot(g_GearDialogSlot) && gs.stat_id != 0) {
            if (ImGui::Button("Apply Stat to All Trinkets")) {
                const char* trinketSlots[] = {"Backpack","Accessory1","Accessory2","Amulet","Ring1","Ring2"};
                for (const char* s : trinketSlots) {
                    auto& target = editBuild->gear[s];
                    target.slot = s;
                    target.stat_id = gs.stat_id;
                    target.stat_name = gs.stat_name;
                }
                AlterEgo::GW2API::SaveBuildLibrary();
            }
        }

        // === RIGHT: Selector panel ===
        ImGui::TableNextColumn();

        // Search bar (shared between tabs)
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##search", "Search...", g_GearStatSearch, sizeof(g_GearStatSearch));

        if (g_GearSelectorTab == 0 && isRelic) {
            // --- Relic selector ---
            std::string filterLower;
            if (g_GearStatSearch[0]) {
                filterLower = g_GearStatSearch;
                for (auto& c : filterLower) c = (char)tolower(c);
            }

            ImGui::BeginChild("##relicList", ImVec2(0, -30), true);
            for (int i = 0; i < RELIC_COUNT; i++) {
                if (!filterLower.empty()) {
                    std::string nLow = RELIC_LIST[i].name;
                    for (auto& c : nLow) c = (char)tolower(c);
                    std::string bLow = RELIC_LIST[i].bonus;
                    for (auto& c : bLow) c = (char)tolower(c);
                    if (nLow.find(filterLower) == std::string::npos &&
                        bLow.find(filterLower) == std::string::npos) continue;
                }

                ImGui::PushID(i);
                bool isSel = (editBuild->relic_name == RELIC_LIST[i].name);

                ImVec2 cs = ImGui::GetCursorScreenPos();
                float cw = ImGui::GetContentRegionAvail().x;
                float ch = 32.0f;
                ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                    isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                ImGui::SetCursorScreenPos(cs);
                if (ImGui::InvisibleButton("##relsel", ImVec2(cw, ch))) {
                    editBuild->relic_name = RELIC_LIST[i].name;
                    editBuild->relic_id = RELIC_LIST[i].item_id;
                    AlterEgo::GW2API::SaveBuildLibrary();
                }

                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(cs.x+6, cs.y+2), IM_COL32(200, 180, 255, 255), RELIC_LIST[i].name);
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), RELIC_LIST[i].bonus);
                ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                ImGui::PopID();
            }
            ImGui::EndChild();
        } else if (g_GearSelectorTab == 0) {
            // --- Stat combo selector ---
            ImGui::BeginChild("##statList", ImVec2(0, -30), true);
            auto combos = GetSortedStatCombos(g_GearStatSearch);
            for (const auto* stat : combos) {
                ImGui::PushID((int)stat->id);
                bool isSelected = (g_GearSelectedStatId == stat->id);

                ImVec2 cardStart = ImGui::GetCursorScreenPos();
                float cardW = ImGui::GetContentRegionAvail().x;
                float cardH = 14.0f + stat->attributes.size() * 14.0f + 6.0f;

                ImU32 bgCol = isSelected ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH), bgCol, 3.0f);
                ImGui::GetWindowDrawList()->AddRect(
                    cardStart, ImVec2(cardStart.x + cardW, cardStart.y + cardH),
                    isSelected ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                ImGui::SetCursorScreenPos(cardStart);
                if (ImGui::InvisibleButton("##sel", ImVec2(cardW, cardH))) {
                    g_GearSelectedStatId = stat->id;
                    gs.stat_id = stat->id;
                    gs.stat_name = stat->name;
                    AlterEgo::GW2API::SaveBuildLibrary();
                }

                ImVec2 textPos(cardStart.x + 6, cardStart.y + 3);
                ImGui::GetWindowDrawList()->AddText(textPos,
                    IM_COL32(255, 210, 80, 255), stat->name.c_str());
                for (size_t ai = 0; ai < stat->attributes.size(); ai++) {
                    textPos.y += 14.0f;
                    char attrBuf[64];
                    snprintf(attrBuf, sizeof(attrBuf), "+%s", FriendlyAttrName(stat->attributes[ai]));
                    ImGui::GetWindowDrawList()->AddText(textPos,
                        IM_COL32(100, 200, 100, 255), attrBuf);
                }
                ImGui::SetCursorScreenPos(ImVec2(cardStart.x, cardStart.y + cardH + 2));

                ImGui::PopID();
            }
            ImGui::EndChild();
        } else if (g_GearSelectorTab == 1) {
            // --- Rune or Sigil selector ---
            std::string filterLower;
            if (g_GearStatSearch[0]) {
                filterLower = g_GearStatSearch;
                for (auto& c : filterLower) c = (char)tolower(c);
            }

            ImGui::BeginChild("##upgradeList", ImVec2(0, -30), true);

            if (isArmor) {
                for (int i = 0; i < RUNE_COUNT; i++) {
                    if (!filterLower.empty()) {
                        std::string nLow = RUNE_LIST[i].name;
                        for (auto& c : nLow) c = (char)tolower(c);
                        std::string bLow = RUNE_LIST[i].bonus;
                        for (auto& c : bLow) c = (char)tolower(c);
                        if (nLow.find(filterLower) == std::string::npos &&
                            bLow.find(filterLower) == std::string::npos) continue;
                    }

                    ImGui::PushID(i);
                    bool isSel = (gs.rune == RUNE_LIST[i].name);

                    ImVec2 cs = ImGui::GetCursorScreenPos();
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 32.0f;
                    ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                    ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                    ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                        isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                    ImGui::SetCursorScreenPos(cs);
                    if (ImGui::InvisibleButton("##rsel", ImVec2(cw, ch))) {
                        gs.rune = RUNE_LIST[i].name;
                        gs.rune_id = AlterEgo::GW2API::FindItemIdByName(RUNE_LIST[i].name);
                        if (editBuild) {
                            editBuild->rune_id = gs.rune_id;
                        }
                        AlterEgo::GW2API::SaveBuildLibrary();
                    }

                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+2), IM_COL32(200, 180, 255, 255), RUNE_LIST[i].name);
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), RUNE_LIST[i].bonus);
                    ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                    ImGui::PopID();
                }
            } else if (isWeapon) {
                for (int i = 0; i < SIGIL_COUNT; i++) {
                    if (!filterLower.empty()) {
                        std::string nLow = SIGIL_LIST[i].name;
                        for (auto& c : nLow) c = (char)tolower(c);
                        std::string bLow = SIGIL_LIST[i].bonus;
                        for (auto& c : bLow) c = (char)tolower(c);
                        if (nLow.find(filterLower) == std::string::npos &&
                            bLow.find(filterLower) == std::string::npos) continue;
                    }

                    ImGui::PushID(i);
                    const std::string& activeSigil = g_GearEditingSigil2 ? gs.sigil2 : gs.sigil;
                    bool isSel = (activeSigil == SIGIL_LIST[i].name);

                    ImVec2 cs = ImGui::GetCursorScreenPos();
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 32.0f;
                    ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                    ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                    ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                        isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                    ImGui::SetCursorScreenPos(cs);
                    if (ImGui::InvisibleButton("##ssel", ImVec2(cw, ch))) {
                        uint32_t sid = AlterEgo::GW2API::FindItemIdByName(SIGIL_LIST[i].name);
                        if (g_GearEditingSigil2) {
                            gs.sigil2 = SIGIL_LIST[i].name;
                            gs.sigil2_id = sid;
                        } else {
                            gs.sigil = SIGIL_LIST[i].name;
                            gs.sigil_id = sid;
                        }
                        AlterEgo::GW2API::SaveBuildLibrary();
                    }

                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+2), IM_COL32(200, 180, 255, 255), SIGIL_LIST[i].name);
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), SIGIL_LIST[i].bonus);
                    ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
        } else if (g_GearSelectorTab == 2 && isWeapon) {
            // --- Weapon type selector ---
            const auto* profWeapons = AlterEgo::GW2API::GetProfessionWeapons(editBuild->profession);
            if (!profWeapons) {
                AlterEgo::GW2API::FetchProfessionPaletteAsync(editBuild->profession);
                RenderSpinner("Loading weapon data...");
            } else {
                std::string filterLower;
                if (g_GearStatSearch[0]) {
                    filterLower = g_GearStatSearch;
                    for (auto& c : filterLower) c = (char)tolower(c);
                }

                bool isMH = IsMainhandSlot(g_GearDialogSlot);
                bool isOH = IsOffhandSlot(g_GearDialogSlot);

                ImGui::BeginChild("##weaponTypeList", ImVec2(0, -30), true);
                int idx = 0;
                for (const auto& [wname, wd] : *profWeapons) {
                    // Filter by mainhand/offhand/two-hand
                    if (isMH && !wd.mainhand && !wd.two_handed) continue;
                    if (isOH && !wd.offhand) continue;

                    // Search filter
                    if (!filterLower.empty()) {
                        std::string nLow = wname;
                        for (auto& c : nLow) c = (char)tolower(c);
                        if (nLow.find(filterLower) == std::string::npos) continue;
                    }

                    ImGui::PushID(idx++);
                    bool isSel = (gs.weapon_type == wname);

                    // Build description string
                    std::string desc;
                    if (wd.two_handed) desc = "Two-Handed";
                    else if (wd.mainhand && wd.offhand) desc = "Main / Off Hand";
                    else if (wd.mainhand) desc = "Main Hand";
                    else if (wd.offhand) desc = "Off Hand";
                    if (wd.specialization != 0) {
                        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(wd.specialization);
                        if (specInfo) desc += " (" + specInfo->name + ")";
                    }

                    ImVec2 cs = ImGui::GetCursorScreenPos();
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 32.0f;
                    ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                    ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                    ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                        isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                    ImGui::SetCursorScreenPos(cs);
                    if (ImGui::InvisibleButton("##wsel", ImVec2(cw, ch))) {
                        gs.weapon_type = wname;
                        // Fetch weapon skill details (including attunements and dual wield)
                        std::vector<uint32_t> skillIds;
                        for (int si = 0; si < 5; si++) {
                            if (wd.skills[si] != 0) skillIds.push_back(wd.skills[si]);
                        }
                        for (const auto& [att, askills] : wd.attunement_skills) {
                            for (auto sid : askills) { if (sid != 0) skillIds.push_back(sid); }
                        }
                        for (const auto& [dw, sid] : wd.dual_wield) { if (sid != 0) skillIds.push_back(sid); }
                        if (!skillIds.empty())
                            AlterEgo::GW2API::FetchSkillDetailsAsync(skillIds);
                        // Fetch weapon item icon
                        uint32_t weapItemId = GetLegendaryWeaponId(wname);
                        if (weapItemId != 0)
                            AlterEgo::GW2API::FetchItemDetailsAsync({weapItemId});
                        AlterEgo::GW2API::SaveBuildLibrary();
                    }

                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+2), IM_COL32(220, 200, 120, 255), WeaponDisplayName(wname));
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(cs.x+6, cs.y+16), IM_COL32(150, 150, 150, 220), desc.c_str());
                    ImGui::SetCursorScreenPos(ImVec2(cs.x, cs.y + ch + 2));

                    ImGui::PopID();
                }
                ImGui::EndChild();
            }
        }

        // Accept / Cancel buttons
        if (ImGui::Button("Accept", ImVec2(80, 0))) {
            AlterEgo::GW2API::SaveBuildLibrary();
            g_GearDialogOpen = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            g_GearDialogOpen = false;
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

// Save to Library dialog — lets user name the build, pick game mode, and optionally include equipment
static void RenderSaveToLibraryDialog() {
    if (!g_SaveLibDialogOpen) return;

    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Always);
    if (!ImGui::Begin("Save to Library##SaveLibDialog", &g_SaveLibDialogOpen,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Build Name:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##savelib_name", g_SaveLibName, sizeof(g_SaveLibName));

    ImGui::Spacing();
    ImGui::Text("Game Mode:");
    ImGui::SetNextItemWidth(120);
    ImGui::Combo("##savelib_mode", &g_SaveLibMode, GameModeImportNames, IM_ARRAYSIZE(GameModeImportNames));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Include equipment set", &g_SaveLibIncludeEquip);

    if (g_SaveLibIncludeEquip) {
        const auto* ch = AlterEgo::GW2API::GetCharacter(g_SaveLibCharName);
        if (ch) {
            // Find max equipment tab
            int maxTab = 1;
            for (const auto& eq : ch->equipment) {
                if (eq.tab > maxTab) maxTab = eq.tab;
            }

            ImGui::Indent(16);
            for (int t = 1; t <= maxTab; t++) {
                std::string label;
                auto nameIt = ch->equipment_tab_names.find(t);
                if (nameIt != ch->equipment_tab_names.end() && !nameIt->second.empty())
                    label = nameIt->second;
                else
                    label = "Tab " + std::to_string(t);
                if (ch->active_equipment_tab == t) label += " *";

                bool sel = (g_SaveLibEquipTab == t);
                if (ImGui::RadioButton(label.c_str(), sel))
                    g_SaveLibEquipTab = t;
            }
            ImGui::Unindent(16);
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  Character data not available");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Save", ImVec2(80, 0))) {
        std::string name = g_SaveLibName;
        if (name.empty()) name = "Unnamed Build";

        AlterEgo::SavedBuild sb;
        if (DecodeBuildLink(g_SaveLibChatLink, name, GameModeFromIndex(g_SaveLibMode), sb)) {
            // Optionally populate gear from character equipment
            if (g_SaveLibIncludeEquip) {
                const auto* ch = AlterEgo::GW2API::GetCharacter(g_SaveLibCharName);
                if (ch) {
                    // Pre-fetch any item details not yet cached (weapons, upgrades)
                    std::vector<uint32_t> missingIds;
                    for (const auto& eq : ch->equipment) {
                        if (eq.tab != g_SaveLibEquipTab) continue;
                        if (eq.id != 0 && !AlterEgo::GW2API::GetItemInfo(eq.id))
                            missingIds.push_back(eq.id);
                        for (auto uid : eq.upgrades) {
                            if (uid != 0 && !AlterEgo::GW2API::GetItemInfo(uid))
                                missingIds.push_back(uid);
                        }
                    }
                    if (!missingIds.empty())
                        AlterEgo::GW2API::FetchItemDetails(missingIds);

                    std::string sharedRune;
                    for (const auto& eq : ch->equipment) {
                        if (eq.tab != g_SaveLibEquipTab) continue;

                        AlterEgo::BuildGearSlot gs;
                        gs.slot = eq.slot;

                        // Resolve stat name
                        if (eq.stat_id != 0) {
                            gs.stat_id = eq.stat_id;
                            const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(eq.stat_id);
                            if (statInfo) gs.stat_name = statInfo->name;
                        }

                        // Determine if this is an armor or weapon slot for rune/sigil
                        bool isArmor = (eq.slot == "Helm" || eq.slot == "Shoulders" ||
                                        eq.slot == "Coat" || eq.slot == "Gloves" ||
                                        eq.slot == "Leggings" || eq.slot == "Boots");
                        bool isWeapon = (eq.slot.find("Weapon") == 0);

                        // Resolve upgrades (runes for armor, sigils for weapons)
                        for (size_t ui = 0; ui < eq.upgrades.size(); ui++) {
                            if (eq.upgrades[ui] == 0) continue;
                            const auto* upInfo = AlterEgo::GW2API::GetItemInfo(eq.upgrades[ui]);
                            if (!upInfo) continue;
                            std::string upName = upInfo->name;
                            AlterEgo::GW2API::CacheItemNameId(upName, eq.upgrades[ui]);
                            if (isArmor && ui == 0) {
                                gs.rune = upName;
                                gs.rune_id = eq.upgrades[ui];
                                if (sharedRune.empty()) sharedRune = upName;
                            } else if (isWeapon && ui == 0) {
                                gs.sigil = upName;
                                gs.sigil_id = eq.upgrades[ui];
                            } else if (isWeapon && ui == 1) {
                                gs.sigil2 = upName;
                                gs.sigil2_id = eq.upgrades[ui];
                            }
                        }

                        // Resolve weapon type from item details
                        if (isWeapon && eq.id != 0) {
                            const auto* itemInfo = AlterEgo::GW2API::GetItemInfo(eq.id);
                            if (itemInfo && itemInfo->details.contains("type"))
                                gs.weapon_type = itemInfo->details["type"].get<std::string>();
                        }

                        sb.gear[eq.slot] = gs;
                    }
                    sb.rune_name = sharedRune;
                    // Store shared rune ID from first armor piece
                    for (const auto& eq2 : ch->equipment) {
                        if (eq2.tab != g_SaveLibEquipTab) continue;
                        bool isArmor2 = (eq2.slot == "Helm" || eq2.slot == "Shoulders" ||
                                         eq2.slot == "Coat" || eq2.slot == "Gloves" ||
                                         eq2.slot == "Leggings" || eq2.slot == "Boots");
                        if (isArmor2 && !eq2.upgrades.empty() && eq2.upgrades[0] != 0) {
                            sb.rune_id = eq2.upgrades[0];
                            break;
                        }
                    }

                    // Check for relic — resolve name from API, but store exotic relic ID
                    for (const auto& eq : ch->equipment) {
                        if (eq.tab != g_SaveLibEquipTab) continue;
                        if (eq.slot == "Relic" && eq.id != 0) {
                            const auto* relicInfo = AlterEgo::GW2API::GetItemInfo(eq.id);
                            if (relicInfo) {
                                sb.relic_name = relicInfo->name;
                                // Use exotic relic ID from name cache (legendary relic has a shared ID)
                                sb.relic_id = AlterEgo::GW2API::FindItemIdByName(relicInfo->name);
                            }
                            if (sb.relic_id == 0) sb.relic_id = eq.id; // fallback
                            break;
                        }
                    }
                }
            }

            AlterEgo::GW2API::AddSavedBuild(std::move(sb));
            if (APIDefs) APIDefs->GUI_SendAlert("Build saved to library!");
            g_SaveLibDialogOpen = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80, 0))) {
        g_SaveLibDialogOpen = false;
    }

    ImGui::End();
}

// Render a single gear slot with icon for the build gear panel
static void RenderBuildGearSlot(AlterEgo::SavedBuild& build, const char* slot) {
    // Check if off-hand is blocked by a two-handed main hand
    bool disabled = false;
    if (strcmp(slot, "WeaponA2") == 0 || strcmp(slot, "WeaponB2") == 0) {
        const char* mainSlot = (strcmp(slot, "WeaponA2") == 0) ? "WeaponA1" : "WeaponB1";
        auto mhIt = build.gear.find(mainSlot);
        if (mhIt != build.gear.end() && !mhIt->second.weapon_type.empty()) {
            disabled = IsWeaponTwoHanded(build.profession, mhIt->second.weapon_type);
        }
    }

    auto it = build.gear.find(slot);
    bool hasStat = (it != build.gear.end() && it->second.stat_id != 0);
    bool hasWeaponType = (it != build.gear.end() && !it->second.weapon_type.empty());
    bool hasData = hasStat || hasWeaponType;

    ImGui::PushID(slot);

    // Draw icon (same size as equipment panel)
    float iconSz = ICON_SIZE;
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Try to render a real item icon
    static std::set<uint32_t> s_requestedItemIds; // dedup: only fetch once
    uint32_t iconItemId = GetGearSlotIconItemId(build, slot);
    Texture_t* tex = nullptr;
    if (iconItemId != 0) {
        const auto* itemInfo = AlterEgo::GW2API::GetItemInfo(iconItemId);
        if (itemInfo && !itemInfo->icon_url.empty()) {
            AlterEgo::IconManager::RequestIcon(iconItemId, itemInfo->icon_url);
            tex = AlterEgo::IconManager::GetIcon(iconItemId);
        } else if (s_requestedItemIds.find(iconItemId) == s_requestedItemIds.end()) {
            // Fetch item details only once per item ID
            s_requestedItemIds.insert(iconItemId);
            AlterEgo::GW2API::FetchItemDetailsAsync({iconItemId});
        }
    }

    if (tex && tex->Resource) {
        // Render real icon with legendary border (drawn on the icon edge, not outside it,
        // so a flush-left layout doesn't clip the border pixel)
        ImVec4 borderColor = hasData ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.5f, 0.3f, 0.6f, 0.8f);
        ImGui::GetWindowDrawList()->AddRect(
            pos,
            ImVec2(pos.x + iconSz, pos.y + iconSz),
            ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, 1.5f);
        ImGui::Image(tex->Resource, ImVec2(iconSz, iconSz));
    } else {
        // Fallback: colored placeholder with slot initial
        ImU32 bgCol = disabled ? IM_COL32(30, 30, 30, 120) : (hasData ? IM_COL32(50, 45, 30, 200) : IM_COL32(40, 40, 40, 180));
        ImU32 borderCol = disabled ? IM_COL32(50, 50, 50, 120) : (hasData ? IM_COL32(255, 200, 60, 200) : IM_COL32(80, 80, 80, 200));
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, ImVec2(pos.x + iconSz, pos.y + iconSz), bgCol);
        ImGui::GetWindowDrawList()->AddRect(
            pos, ImVec2(pos.x + iconSz, pos.y + iconSz), borderCol, 0.0f, 0, 1.5f);

        const char* initial = slot;
        if (strcmp(slot, "Helm") == 0) initial = "H";
        else if (strcmp(slot, "Shoulders") == 0) initial = "S";
        else if (strcmp(slot, "Coat") == 0) initial = "C";
        else if (strcmp(slot, "Gloves") == 0) initial = "G";
        else if (strcmp(slot, "Leggings") == 0) initial = "L";
        else if (strcmp(slot, "Boots") == 0) initial = "B";
        else if (strcmp(slot, "WeaponA1") == 0 || strcmp(slot, "WeaponB1") == 0) initial = "M";
        else if (strcmp(slot, "WeaponA2") == 0 || strcmp(slot, "WeaponB2") == 0) initial = "O";
        else if (strcmp(slot, "Backpack") == 0) initial = "Bk";
        else if (strcmp(slot, "Accessory1") == 0 || strcmp(slot, "Accessory2") == 0) initial = "A";
        else if (strcmp(slot, "Amulet") == 0) initial = "Am";
        else if (strcmp(slot, "Ring1") == 0 || strcmp(slot, "Ring2") == 0) initial = "R";
        else if (strcmp(slot, "Relic") == 0) initial = "Re";

        ImVec2 textSz = ImGui::CalcTextSize(initial);
        ImU32 textCol = disabled ? IM_COL32(60, 60, 60, 120) : (hasData ? IM_COL32(255, 210, 80, 200) : IM_COL32(100, 100, 100, 180));
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(pos.x + (iconSz - textSz.x) * 0.5f, pos.y + (iconSz - textSz.y) * 0.5f),
            textCol, initial);
        ImGui::Dummy(ImVec2(iconSz, iconSz));
    }

    // Invisible button over icon area (overlaid on top for click handling)
    ImGui::SetCursorScreenPos(pos);
    if (disabled) {
        ImGui::Dummy(ImVec2(iconSz, iconSz));
    } else if (ImGui::InvisibleButton("##icon", ImVec2(iconSz, iconSz))) {
        AlterEgo::BuildGearSlot current;
        if (it != build.gear.end()) current = it->second;
        current.slot = slot;
        OpenGearDialog(build.id, slot, current);
    }
    bool hovered = !disabled && ImGui::IsItemHovered();

    // Name text next to icon — draw directly so lines fit within icon height
    bool isRelicSlot = (strcmp(slot, "Relic") == 0);
    float lineH = ImGui::GetTextLineHeight();
    float textX = pos.x + iconSz + 4.0f;
    float textY = pos.y;
    auto* dl = ImGui::GetWindowDrawList();

    // Line 1: stat/slot name
    char line1[256] = {};
    ImU32 line1Col;
    if (isRelicSlot) {
        if (!build.relic_name.empty()) {
            snprintf(line1, sizeof(line1), "%s", build.relic_name.c_str());
            line1Col = IM_COL32(128, 204, 204, 255);
        } else {
            snprintf(line1, sizeof(line1), "Relic");
            line1Col = IM_COL32(128, 128, 128, 255);
        }
    } else if (hasStat && hasWeaponType) {
        snprintf(line1, sizeof(line1), "%s %s", it->second.stat_name.c_str(), WeaponDisplayName(it->second.weapon_type));
        line1Col = IM_COL32(255, 204, 51, 255);
    } else if (hasStat) {
        snprintf(line1, sizeof(line1), "%s %s", it->second.stat_name.c_str(), GearSlotDisplayName(slot));
        line1Col = IM_COL32(255, 204, 51, 255);
    } else if (hasWeaponType) {
        snprintf(line1, sizeof(line1), "%s %s", WeaponDisplayName(it->second.weapon_type), GearSlotDisplayName(slot));
        line1Col = IM_COL32(204, 191, 128, 255);
    } else if (disabled) {
        snprintf(line1, sizeof(line1), "%s (2H equipped)", GearSlotDisplayName(slot));
        line1Col = IM_COL32(89, 89, 89, 153);
    } else {
        snprintf(line1, sizeof(line1), "%s", GearSlotDisplayName(slot));
        line1Col = IM_COL32(128, 128, 128, 255);
    }
    dl->AddText(ImVec2(textX, textY), line1Col, line1);
    textY += lineH;

    // Line 2: rune or sigil
    if (!isRelicSlot && it != build.gear.end()) {
        if (!it->second.rune.empty()) {
            dl->AddText(ImVec2(textX, textY), IM_COL32(153, 153, 255, 255), it->second.rune.c_str());
            textY += lineH;
        }
        if (!it->second.sigil.empty()) {
            dl->AddText(ImVec2(textX, textY), IM_COL32(153, 153, 255, 255), it->second.sigil.c_str());
            textY += lineH;
        }
        // Line 3: second sigil (two-handed weapons)
        if (!it->second.sigil2.empty()) {
            dl->AddText(ImVec2(textX, textY), IM_COL32(153, 153, 255, 255), it->second.sigil2.c_str());
            textY += lineH;
        }
    }

    // Advance cursor past the icon
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + iconSz + 2.0f));

    // Tooltip
    if (hovered && isRelicSlot && !build.relic_name.empty()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.8f, 1.0f), "%s", build.relic_name.c_str());
        for (int ri = 0; ri < RELIC_COUNT; ri++) {
            if (build.relic_name == RELIC_LIST[ri].name) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", RELIC_LIST[ri].bonus);
                break;
            }
        }
        ImGui::EndTooltip();
    } else if (hovered && hasData) {
        ImGui::BeginTooltip();
        if (hasWeaponType) {
            ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.5f, 1.0f), "%s", WeaponDisplayName(it->second.weapon_type));
        }
        if (hasStat) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s %s",
                it->second.stat_name.c_str(), GearSlotDisplayName(slot));
            const auto* statInfo = AlterEgo::GW2API::GetItemStatInfo(it->second.stat_id);
            if (statInfo) {
                for (const auto& a : statInfo->attributes) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "  +%s", FriendlyAttrName(a));
                }
            }
        }
        ImGui::EndTooltip();
    } else if (hovered) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s (click to customize)", GearSlotDisplayName(slot));
        ImGui::EndTooltip();
    }

    ImGui::PopID();
}

// Render the full gear section for a saved build
static void RenderBuildGearPanel(AlterEgo::SavedBuild& build) {
    RenderSectionHeader("Gear", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));

    if (ImGui::BeginTable("##buildGear", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("##gearLeft", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##gearRight", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        // Left column: Armor
        ImGui::TableNextColumn();
        ImGui::Indent(2.0f);
        RenderSectionHeader("Armor", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
        RenderBuildGearSlot(build, "Helm");
        RenderBuildGearSlot(build, "Shoulders");
        RenderBuildGearSlot(build, "Coat");
        RenderBuildGearSlot(build, "Gloves");
        RenderBuildGearSlot(build, "Leggings");
        RenderBuildGearSlot(build, "Boots");

        ImGui::Spacing();
        RenderSectionHeader("Weapons", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
        RenderBuildGearSlot(build, "WeaponA1");
        RenderBuildGearSlot(build, "WeaponA2");
        RenderBuildGearSlot(build, "WeaponB1");
        RenderBuildGearSlot(build, "WeaponB2");
        ImGui::Unindent(2.0f);

        // Right column: Trinkets + Relic + Rune
        ImGui::TableNextColumn();
        ImGui::Indent(2.0f);
        RenderSectionHeader("Trinkets", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
        RenderBuildGearSlot(build, "Backpack");
        RenderBuildGearSlot(build, "Accessory1");
        RenderBuildGearSlot(build, "Accessory2");
        RenderBuildGearSlot(build, "Amulet");
        RenderBuildGearSlot(build, "Ring1");
        RenderBuildGearSlot(build, "Ring2");
        RenderBuildGearSlot(build, "Relic");
        ImGui::Unindent(2.0f);

        ImGui::EndTable();
    }
}

// Re-resolve item names (rune, relic, sigils, stats) for a saved build from
// the GW2 API cache. If IDs are present but names are missing, queues async
// fetches so the next render attempt can fill them in. Handles the case where
// the GW2 API was unavailable or rate-limited at import time.
static void ResolveBuildItemNames(AlterEgo::SavedBuild& build) {
    bool changed = false;
    std::vector<uint32_t> missingItems;
    std::vector<uint32_t> missingStats;

    auto tryItem = [&](uint32_t id, std::string& name) {
        if (id == 0 || !name.empty()) return;
        const auto* info = AlterEgo::GW2API::GetItemInfo(id);
        if (info && !info->name.empty()) {
            name = info->name;
            changed = true;
        } else {
            missingItems.push_back(id);
        }
    };

    tryItem(build.rune_id, build.rune_name);
    tryItem(build.relic_id, build.relic_name);

    for (auto& [slot, gs] : build.gear) {
        tryItem(gs.sigil_id, gs.sigil);
        tryItem(gs.sigil2_id, gs.sigil2);
        if (gs.rune_id == 0 && build.rune_id != 0) {
            // Propagate shared rune to armor slot if not yet set
            static const char* armorSlots[] = {"Helm","Shoulders","Coat","Gloves","Leggings","Boots"};
            for (const char* s : armorSlots) {
                if (slot == s) { gs.rune_id = build.rune_id; changed = true; break; }
            }
        }
        if (gs.rune_id != 0 && gs.rune.empty() && !build.rune_name.empty()) {
            gs.rune = build.rune_name;
            changed = true;
        }
        if (gs.stat_id != 0 && gs.stat_name.empty()) {
            const auto* si = AlterEgo::GW2API::GetItemStatInfo(gs.stat_id);
            if (si && !si->name.empty()) {
                gs.stat_name = si->name;
                changed = true;
            } else {
                missingStats.push_back(gs.stat_id);
            }
        }
    }

    if (!missingItems.empty())
        AlterEgo::GW2API::FetchItemDetailsAsync(missingItems);
    if (!missingStats.empty())
        AlterEgo::GW2API::FetchItemStatDetailsAsync(missingStats);

    if (changed) AlterEgo::GW2API::SaveBuildLibrary();
}

// Resolve negative trait placeholders in a saved build once spec data is available
static void ResolveBuildTraitPlaceholders(AlterEgo::SavedBuild& build) {
    bool changed = false;
    for (int i = 0; i < 3; i++) {
        if (build.specializations[i].spec_id == 0) continue;
        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(build.specializations[i].spec_id);
        if (!specInfo || specInfo->major_traits.size() < 9) continue;
        for (int tier = 0; tier < 3; tier++) {
            if (build.specializations[i].traits[tier] >= 0) continue;
            int choice = -(build.specializations[i].traits[tier]);
            if (choice >= 1 && choice <= 3) {
                int mIdx = tier * 3 + (choice - 1);
                build.specializations[i].traits[tier] = (int)specInfo->major_traits[mIdx];
                changed = true;
            }
        }
    }
    if (changed) AlterEgo::GW2API::SaveBuildLibrary();
}

// Render the build preview panel for a saved build (reuses spec/trait rendering logic)
static void RenderSavedBuildPreview(const AlterEgo::SavedBuild& build, bool showEditButton = false) {
    // Try to resolve any placeholder traits (negative values from import without spec cache)
    // Safe to cast: we only mutate trait values from negative to positive
    ResolveBuildTraitPlaceholders(const_cast<AlterEgo::SavedBuild&>(build));
    // Re-resolve any item/stat names that weren't available at import time
    // (e.g. API rate-limited or offline). Queues async fetches for missing IDs.
    ResolveBuildItemNames(const_cast<AlterEgo::SavedBuild&>(build));

    ImVec4 profColor = GetProfessionColor(build.profession);
    {
        // Detect elite spec — use its icon and name in the header when present
        const AlterEgo::SpecializationInfo* eliteSpec = nullptr;
        for (int s = 0; s < 3; s++) {
            uint32_t sid = build.specializations[s].spec_id;
            if (!sid) continue;
            const auto* si = AlterEgo::GW2API::GetSpecInfo(sid);
            if (si && si->elite) { eliteSpec = si; break; }
        }

        // Build header card: gold accent bar, name in profession color, sub-line with prof + mode
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        float lineH = ImGui::GetTextLineHeightWithSpacing();
        const float headerIconSz = 44.0f;
        float h = std::max(lineH * 2.0f + 6.0f, headerIconSz + 10.0f);
        // Gradient bg using profession color
        ImU32 left = ImGui::ColorConvertFloat4ToU32(
            ImVec4(profColor.x * 0.24f, profColor.y * 0.24f, profColor.z * 0.24f, 0.55f));
        ImU32 right = IM_COL32(0, 0, 0, 0);
        dl->AddRectFilledMultiColor(
            ImVec2(pos.x + 4, pos.y), ImVec2(pos.x + w, pos.y + h),
            left, right, right, left);
        // Gold left bar
        ImU32 barTop = IM_COL32(227, 196, 122, 255);
        ImU32 barBot = IM_COL32(140, 113, 55, 255);
        dl->AddRectFilledMultiColor(
            ImVec2(pos.x, pos.y + 2),
            ImVec2(pos.x + 3, pos.y + h - 2),
            barTop, barTop, barBot, barBot);
        // Underline
        dl->AddLine(
            ImVec2(pos.x + 4, pos.y + h),
            ImVec2(pos.x + w * 0.55f, pos.y + h),
            ImGui::ColorConvertFloat4ToU32(ImVec4(profColor.x, profColor.y, profColor.z, 0.35f)), 1.0f);

        // Class icon on the left — prefer elite-spec emblem, fall back to profession
        float iconLeft = pos.x + 10.0f;
        float iconTop = pos.y + (h - headerIconSz) * 0.5f;
        Texture_t* iconTex = nullptr;
        if (eliteSpec && !eliteSpec->profession_icon_big_url.empty()) {
            uint32_t specIconId = 2000000u + (uint32_t)(std::hash<std::string>{}(eliteSpec->name) % 100000u);
            iconTex = AlterEgo::IconManager::GetIcon(specIconId);
            if (!iconTex || !iconTex->Resource) {
                AlterEgo::IconManager::RequestIcon(specIconId,
                    eliteSpec->profession_icon_big_url, /*priority=*/true);
            }
        }
        if (!iconTex || !iconTex->Resource) {
            uint32_t profIconId = 9100000u + (uint32_t)(std::hash<std::string>{}(build.profession) % 100000u);
            iconTex = AlterEgo::IconManager::GetIcon(profIconId);
            if (!iconTex || !iconTex->Resource) {
                const auto* prof = AlterEgo::GW2API::GetProfessionInfo(build.profession);
                if (prof && !prof->icon_big_url.empty())
                    AlterEgo::IconManager::RequestIcon(profIconId, prof->icon_big_url, /*priority=*/true);
            }
        }
        if (iconTex && iconTex->Resource) {
            dl->AddImage(iconTex->Resource,
                ImVec2(iconLeft, iconTop),
                ImVec2(iconLeft + headerIconSz, iconTop + headerIconSz),
                ImVec2(0,0), ImVec2(1,1), IM_COL32(255, 255, 255, 255));
        }

        // Title + subtitle to the right of the icon. Use elite spec name as the
        // primary identifier when present; profession name as a subtitle line.
        float textX = iconLeft + headerIconSz + 10.0f;
        float textTop = pos.y + (h - lineH * 2.0f) * 0.5f - 2.0f;

        // Title: build name. Subtitle: elite spec + profession + mode.
        ImGui::SetCursorScreenPos(ImVec2(textX, textTop));
        ImGui::TextColored(profColor, "%s", build.name.c_str());

        ImGui::SetCursorScreenPos(ImVec2(textX, textTop + lineH));
        std::string sub;
        if (eliteSpec) { sub = eliteSpec->name; sub += " "; }
        sub += build.profession;
        sub += "  \xc2\xb7  ";
        sub += GameModeLabel(build.game_mode);
        ImGui::TextColored(ImVec4(0.55f, 0.50f, 0.40f, 1.0f), "%s", sub.c_str());

        // Edit / Share / Delete buttons overlaid in the top-right of the header card
        if (showEditButton) {
            ImVec2 prevCursor = ImGui::GetCursorScreenPos();
            ImGuiStyle& style = ImGui::GetStyle();
            auto btnWidth = [&](const char* t){ return ImGui::CalcTextSize(t).x + style.FramePadding.x * 2.0f; };
            const float gap = 4.0f;
            float wDel   = btnWidth("Delete");
            float wShare = btnWidth("Share");
            float wEdit  = btnWidth("Edit");
            float totalW = wDel + gap + wShare + gap + wEdit;
            float startX = pos.x + w - totalW - 8.0f;
            float btnY   = pos.y + 6.0f;

            ImGui::SetCursorScreenPos(ImVec2(startX, btnY));
            if (ImGui::SmallButton("Share##headerBuild")) ImGui::OpenPopup("##share_menu");

            ImGui::SetCursorScreenPos(ImVec2(startX + wShare + gap, btnY));
            if (ImGui::SmallButton("Edit##headerBuild")) g_LibEditMode = true;

            ImGui::SetCursorScreenPos(ImVec2(startX + wShare + gap + wEdit + gap, btnY));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
            if (ImGui::SmallButton("Delete##headerBuild")) g_LibCtxDeleteIdx = g_LibSelectedIdx;
            ImGui::PopStyleColor();

            // Share menu: the three share actions that used to live in the
            // (now-removed) right-click context menu.
            if (ImGui::BeginPopup("##share_menu")) {
                if (ImGui::Selectable("Copy build-only chat link")) {
                    CopyToClipboard(build.chat_link);
                    if (APIDefs) APIDefs->GUI_SendAlert("Build link copied to clipboard!");
                }
                if (ImGui::Selectable("Copy entire build + equipment code")) {
                    std::string ae2 = ExportBuildToAE2(build);
                    if (!ae2.empty()) {
                        CopyToClipboard(ae2);
                        char info[196];
                        snprintf(info, sizeof(info), "Build code copied to clipboard (%d chars). Paste in GW2 chat to share!", (int)ae2.size());
                        if (APIDefs) APIDefs->GUI_SendAlert(info);
                    } else {
                        if (APIDefs) APIDefs->GUI_SendAlert("Failed to generate build code. Build may be missing a chat link.");
                    }
                }
                if (!g_RelaySending && ImGui::Selectable("Share complete build via one-time code")) {
                    if (build.chat_link.empty()) {
                        if (APIDefs) APIDefs->GUI_SendAlert("Cannot send: build has no chat link.");
                    } else {
                        SendBuildToRelay(build);
                    }
                }
                if (g_RelaySending) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Sending...");
                }
                ImGui::EndPopup();
            }

            ImGui::SetCursorScreenPos(prevCursor);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h + 4));
    }

    // Specializations with trait grid (same logic as RenderBuildPanel)
    const float iconSz = TRAIT_ICON_SIZE;
    const float colW = iconSz * 2.0f;

    for (int i = 0; i < 3; i++) {
        const auto& spec = build.specializations[i];
        if (spec.spec_id == 0) {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "  (empty)");
            continue;
        }

        const auto* specInfo = AlterEgo::GW2API::GetSpecInfo(spec.spec_id);
        std::string specName = specInfo ? specInfo->name : ("Spec " + std::to_string(spec.spec_id));
        bool isElite = specInfo ? specInfo->elite : false;
        ImVec4 specColor = isElite ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

        ImGui::PushID(spec.spec_id);

        if (specInfo && specInfo->major_traits.size() >= 9) {
            ImVec2 specCenter(0, 0);
            ImVec2 minorCenters[3] = {};
            ImVec2 selectedCenters[3] = {};
            bool hasSelected[3] = {false, false, false};

            ImVec2 rowOrigin = ImGui::GetCursorScreenPos();
            float rowW = ImGui::GetContentRegionAvail().x;
            float rowH = iconSz * 3 + ImGui::GetStyle().CellPadding.y * 6;
            ImDrawList* dl = ImGui::GetWindowDrawList();

            // Spec background banner behind the trait grid (panoramic art,
            // native aspect, anchored left, fade to dark on the right).
            if (!specInfo->background_url.empty()) {
                uint32_t bgKey = spec.spec_id + 1000000;
                Texture_t* bgTex = AlterEgo::IconManager::GetIcon(bgKey);
                if (bgTex && bgTex->Resource && bgTex->Height > 0) {
                    // Zoom the banner and anchor it to the row's bottom-left.
                    // 15 specs ship with banners whose top ~66% is transparent
                    // (opaque art lives only in the bottom band). Those need a
                    // much higher zoom so the opaque content fills the row;
                    // everyone else gets the standard 1.4x.
                    auto IsTransparentTopSpec = [](uint32_t id) {
                        switch (id) {
                            case 4: case 7: case 11: case 20: case 22:
                            case 28: case 35: case 36: case 44: case 51:
                            case 54: case 68: case 70: case 71: case 72:
                            case 73: case 74: case 75: case 76: case 77:
                            case 78: case 79: case 80: case 81:
                                return true;
                            default:
                                return false;
                        }
                    };
                    const float zoom = IsTransparentTopSpec(spec.spec_id) ? 2.94f : 1.4f;
                    float drawH = rowH * zoom;
                    float scale = drawH / (float)bgTex->Height;
                    float drawW = (float)bgTex->Width * scale;
                    float bannerLeft = rowOrigin.x;
                    float bannerTop  = rowOrigin.y + rowH - drawH;
                    dl->PushClipRect(rowOrigin,
                        ImVec2(rowOrigin.x + rowW, rowOrigin.y + rowH), true);
                    dl->AddImage(bgTex->Resource,
                        ImVec2(bannerLeft, bannerTop),
                        ImVec2(bannerLeft + drawW, bannerTop + drawH),
                        ImVec2(0, 0), ImVec2(1, 1),
                        IM_COL32(255, 255, 255, 200));
                    // Fade-to-dark anchored to where the trait grid begins,
                    // not the banner. Trait icons start just past column 0.
                    float fadeStart = rowOrigin.x + SPEC_PORTRAIT_SIZE + 20.0f;
                    float fadeEnd   = rowOrigin.x + SPEC_PORTRAIT_SIZE + 80.0f;
                    dl->AddRectFilledMultiColor(
                        ImVec2(fadeStart, rowOrigin.y),
                        ImVec2(fadeEnd,   rowOrigin.y + rowH),
                        IM_COL32(15, 17, 23, 0),   IM_COL32(15, 17, 23, 200),
                        IM_COL32(15, 17, 23, 200), IM_COL32(15, 17, 23, 0));
                    if (fadeEnd < rowOrigin.x + rowW) {
                        dl->AddRectFilled(
                            ImVec2(fadeEnd, rowOrigin.y),
                            ImVec2(rowOrigin.x + rowW, rowOrigin.y + rowH),
                            IM_COL32(15, 17, 23, 200));
                    }
                    dl->PopClipRect();
                } else {
                    AlterEgo::IconManager::RequestIcon(bgKey, specInfo->background_url);
                }
            }

            // Spec name overlay on the banner's left.
            {
                const float nameScale = 1.5f;
                float nameSize = ImGui::GetFontSize() * nameScale;
                ImVec2 nameAnchor(rowOrigin.x + 14.0f,
                                  rowOrigin.y + rowH * 0.5f - nameSize * 0.5f);
                dl->AddText(NULL, nameSize,
                    ImVec2(nameAnchor.x + 1.0f, nameAnchor.y + 1.0f),
                    IM_COL32(0, 0, 0, 220), specName.c_str());
                dl->AddText(NULL, nameSize, nameAnchor,
                    ImGui::ColorConvertFloat4ToU32(specColor), specName.c_str());

                ImVec2 hitSz = ImGui::CalcTextSize(specName.c_str());
                hitSz.x *= nameScale; hitSz.y *= nameScale;
                ImVec2 prevPos = ImGui::GetCursorScreenPos();
                ImGui::SetCursorScreenPos(nameAnchor);
                ImGui::InvisibleButton("##specname", hitSz);
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextColored(specColor, "%s", specName.c_str());
                    const auto* specDesc = AlterEgo::GetSpecDescription(specName);
                    if (specDesc) {
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.9f, 1.0f),
                            "\"%s\"", specDesc->flavor);
                        ImGui::Spacing();
                        ImGui::PushTextWrapPos(350.0f);
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
                            "%s", specDesc->description);
                        ImGui::PopTextWrapPos();
                    }
                    ImGui::EndTooltip();
                }
                ImGui::SetCursorScreenPos(prevPos);
            }

            char outerId[32];
            snprintf(outerId, sizeof(outerId), "##lso_%u", spec.spec_id);
            if (ImGui::BeginTable(outerId, 2,
                    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                    SPEC_PORTRAIT_SIZE + 8.0f);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Dummy(ImVec2(SPEC_PORTRAIT_SIZE, rowH - 2.0f));

                ImGui::TableNextColumn();
                char innerId[32];
                snprintf(innerId, sizeof(innerId), "##ltr_%u", spec.spec_id);
                if (ImGui::BeginTable(innerId, 6,
                        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                    for (int c = 0; c < 6; c++)
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, colW);

                    for (int row = 0; row < 3; row++) {
                        ImGui::TableNextRow();
                        for (int tier = 0; tier < 3; tier++) {
                            ImGui::TableNextColumn();
                            if (row == 1 && tier < (int)specInfo->minor_traits.size()) {
                                minorCenters[tier] = RenderTraitIcon(
                                    specInfo->minor_traits[tier], true, true, iconSz);
                            }

                            ImGui::TableNextColumn();
                            int majorIdx = tier * 3 + row;
                            uint32_t tid = specInfo->major_traits[majorIdx];
                            bool selected;
                            if (spec.traits[tier] < 0) {
                                // Placeholder: negative = -(choice) where 1=top,2=mid,3=bot
                                int choice = -(spec.traits[tier]);
                                selected = (choice >= 1 && choice <= 3 && (choice - 1) == row);
                            } else {
                                selected = ((uint32_t)spec.traits[tier] == tid);
                            }
                            ImVec2 c = RenderTraitIcon(tid, selected, false, iconSz);
                            if (selected) {
                                selectedCenters[tier] = c;
                                hasSelected[tier] = true;
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTable();
            }

            ImU32 lineCol = IM_COL32(180, 230, 255, 180);
            float minorR = MINOR_TRAIT_SIZE * 0.5f;
            float majorR = iconSz * 0.5f;
            (void)specCenter;

            auto InsetLine = [](ImVec2 a, ImVec2 b, float rA, float rB, ImVec2& outA, ImVec2& outB) {
                float dx = b.x - a.x, dy = b.y - a.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len < rA + rB) return false;
                float nx = dx / len, ny = dy / len;
                outA = ImVec2(a.x + nx * rA, a.y + ny * rA);
                outB = ImVec2(b.x - nx * rB, b.y - ny * rB);
                return true;
            };

            // Skip the first connector — the hex spec icon is gone.
            ImVec2 prev;
            float prevR = 0;
            bool havePrev = false;
            for (int tier = 0; tier < 3; tier++) {
                if (minorCenters[tier].x > 0) {
                    if (havePrev) {
                        ImVec2 la, lb;
                        if (InsetLine(prev, minorCenters[tier], prevR, minorR, la, lb))
                            DrawDottedLine(dl, la, lb, lineCol);
                    }
                    prev = minorCenters[tier]; prevR = minorR; havePrev = true;
                }
                if (hasSelected[tier]) {
                    if (havePrev) {
                        ImVec2 la, lb;
                        if (InsetLine(prev, selectedCenters[tier], prevR, majorR, la, lb))
                            DrawDottedLine(dl, la, lb, lineCol);
                    }
                    prev = selectedCenters[tier]; prevR = majorR; havePrev = true;
                }
            }
        }

        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Helper lambda: get weapon skills for a weapon set from gear slots
    // Handles: Elementalist attunements (Fire default), Thief dual wield, standard MH+OH
    auto GetWeaponSkills = [&](const char* mhSlot, const char* ohSlot,
                               std::vector<uint32_t>& outSkills, std::string& outLabel) {
        outSkills.clear();
        outLabel.clear();
        const auto* profWeapons = AlterEgo::GW2API::GetProfessionWeapons(build.profession);
        if (!profWeapons) return;

        auto mhIt = build.gear.find(mhSlot);
        auto ohIt = build.gear.find(ohSlot);
        std::string mhType = (mhIt != build.gear.end()) ? mhIt->second.weapon_type : "";
        std::string ohType = (ohIt != build.gear.end()) ? ohIt->second.weapon_type : "";

        // Mainhand skills (1-3, or 1-5 for two-handed)
        if (!mhType.empty()) {
            auto wit = profWeapons->find(mhType);
            if (wit != profWeapons->end()) {
                const auto& wd = wit->second;
                int count = wd.two_handed ? 5 : 3;

                // Check for attunement skills (Elementalist) — use Fire by default
                if (!wd.attunement_skills.empty()) {
                    auto attIt = wd.attunement_skills.find("Fire");
                    if (attIt == wd.attunement_skills.end())
                        attIt = wd.attunement_skills.begin(); // fallback to first
                    for (int i = 0; i < count; i++) {
                        outSkills.push_back(attIt->second[i]);
                    }
                } else {
                    for (int i = 0; i < count; i++) {
                        outSkills.push_back(wd.skills[i]);
                    }
                }

                // Thief dual wield: replace Weapon_3 if offhand-specific variant exists
                if (!wd.dual_wield.empty() && !ohType.empty() && count >= 3) {
                    auto dwIt = wd.dual_wield.find(ohType);
                    if (dwIt != wd.dual_wield.end()) {
                        outSkills[2] = dwIt->second;
                    }
                }

                outLabel = WeaponDisplayName(mhType);
                if (wd.two_handed) { return; } // two-handed uses all 5 slots
            }
        }

        // Offhand skills (4-5)
        if (!ohType.empty()) {
            auto wit = profWeapons->find(ohType);
            if (wit != profWeapons->end()) {
                const auto& wd = wit->second;
                // Pad to 3 if mainhand didn't fill
                while (outSkills.size() < 3) outSkills.push_back(0);

                // Check for attunement skills (Elementalist offhand)
                if (!wd.attunement_skills.empty()) {
                    auto attIt = wd.attunement_skills.find("Fire");
                    if (attIt == wd.attunement_skills.end())
                        attIt = wd.attunement_skills.begin();
                    outSkills.push_back(attIt->second[3]);
                    outSkills.push_back(attIt->second[4]);
                } else {
                    outSkills.push_back(wd.skills[3]);
                    outSkills.push_back(wd.skills[4]);
                }

                if (!outLabel.empty()) outLabel += " / ";
                outLabel += WeaponDisplayName(ohType);
            }
        }
    };

    // Weapon set A skills
    std::vector<uint32_t> wpnSkillsA, wpnSkillsB;
    std::string wpnLabelA, wpnLabelB;
    GetWeaponSkills("WeaponA1", "WeaponA2", wpnSkillsA, wpnLabelA);
    GetWeaponSkills("WeaponB1", "WeaponB2", wpnSkillsB, wpnLabelB);

    // Fetch skill details for weapon skills
    {
        std::vector<uint32_t> toFetch;
        for (auto id : wpnSkillsA) { if (id && !AlterEgo::GW2API::GetSkillInfo(id)) toFetch.push_back(id); }
        for (auto id : wpnSkillsB) { if (id && !AlterEgo::GW2API::GetSkillInfo(id)) toFetch.push_back(id); }
        if (!toFetch.empty()) AlterEgo::GW2API::FetchSkillDetailsAsync(toFetch);
    }

    float skillSz = ICON_SIZE;

    // Compute heal/utility/elite offset: max weapon skill count + 1 icon gap
    int maxWpnCount = 5; // default for two-handed
    if (!wpnSkillsA.empty() && wpnSkillsA.size() > (size_t)maxWpnCount)
        maxWpnCount = (int)wpnSkillsA.size();
    if (!wpnSkillsB.empty() && wpnSkillsB.size() > (size_t)maxWpnCount)
        maxWpnCount = (int)wpnSkillsB.size();
    float utilOffset = (maxWpnCount + 1) * (skillSz + 4.0f);

    // Skills layout: weapon skills on the left, utility skills on the right
    if (!wpnSkillsA.empty() || !wpnSkillsB.empty()) {
        // Weapon Set A
        if (!wpnSkillsA.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", wpnLabelA.c_str());
            ImGui::SameLine(utilOffset);
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Heal / Utility / Elite");

            ImGui::BeginGroup();
            for (size_t i = 0; i < wpnSkillsA.size(); i++) {
                if (i > 0) ImGui::SameLine();
                RenderSkillIcon(wpnSkillsA[i], skillSz);
            }
            ImGui::EndGroup();

            ImGui::SameLine(utilOffset);
            ImGui::BeginGroup();
            RenderSkillIcon(build.terrestrial_skills.heal, skillSz);
            ImGui::SameLine();
            for (int i = 0; i < 3; i++) {
                RenderSkillIcon(build.terrestrial_skills.utilities[i], skillSz);
                ImGui::SameLine();
            }
            RenderSkillIcon(build.terrestrial_skills.elite, skillSz);
            ImGui::EndGroup();
        } else {
            // No weapon set A, just show utility skills
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Heal / Utility / Elite");
            ImGui::BeginGroup();
            RenderSkillIcon(build.terrestrial_skills.heal, skillSz);
            ImGui::SameLine();
            for (int i = 0; i < 3; i++) {
                RenderSkillIcon(build.terrestrial_skills.utilities[i], skillSz);
                ImGui::SameLine();
            }
            RenderSkillIcon(build.terrestrial_skills.elite, skillSz);
            ImGui::EndGroup();
        }

        // Weapon Set B
        if (!wpnSkillsB.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", wpnLabelB.c_str());
            ImGui::BeginGroup();
            for (size_t i = 0; i < wpnSkillsB.size(); i++) {
                if (i > 0) ImGui::SameLine();
                RenderSkillIcon(wpnSkillsB[i], skillSz);
            }
            ImGui::EndGroup();
        }
    } else {
        // Fallback: no weapon types selected, just show utility skills
        ImGui::Text("Skills");
        ImGui::BeginGroup();
        RenderSkillIcon(build.terrestrial_skills.heal, skillSz);
        ImGui::SameLine();
        for (int i = 0; i < 3; i++) {
            RenderSkillIcon(build.terrestrial_skills.utilities[i], skillSz);
            ImGui::SameLine();
        }
        RenderSkillIcon(build.terrestrial_skills.elite, skillSz);
        ImGui::EndGroup();

        // Legacy weapons from chat link
        if (!build.weapons.empty()) {
            ImGui::Spacing();
            ImGui::Text("Weapons:");
            ImGui::SameLine();
            std::string wpnStr;
            for (size_t i = 0; i < build.weapons.size(); i++) {
                if (i > 0) wpnStr += ", ";
                wpnStr += AlterEgo::ChatLink::WeaponName((uint16_t)build.weapons[i]);
            }
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", wpnStr.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Gear section (clickable slots to customize stats/runes/sigils)
    RenderBuildGearPanel(const_cast<AlterEgo::SavedBuild&>(build));

    if (!build.notes.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Notes: %s", build.notes.c_str());
    }
}

// Render the full Build Library tab
static void RenderBuildLibrary() {
    const auto& builds = AlterEgo::GW2API::GetSavedBuilds();

    // Import toggle button
    if (RenderGoldButton(g_LibShowImport ? "Cancel Import" : "+ Import Build")) {
        g_LibShowImport = !g_LibShowImport;
        g_LibImportBuf[0] = '\0';
        g_LibImportName[0] = '\0';
        g_LibImportMode = 0;
        g_LibImportError.clear();
    }

    // Library backup / restore
    ImGui::SameLine();
    if (RenderChipButton("Export Library", false)) {
        OPENFILENAMEA ofn = {};
        char buf[MAX_PATH] = {};
        // Default filename: alter_ego_builds_YYYYMMDD.json
        {
            std::time_t t = std::time(nullptr);
            std::tm tm = *std::localtime(&t);
            std::snprintf(buf, sizeof(buf), "alter_ego_builds_%04d%02d%02d.json",
                          tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        }
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = "JSON files\0*.json\0All files\0*.*\0";
        ofn.lpstrFile = buf;
        ofn.nMaxFile = sizeof(buf);
        ofn.lpstrTitle = "Export Build Library";
        ofn.lpstrDefExt = "json";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
        if (GetSaveFileNameA(&ofn)) {
            if (AlterEgo::GW2API::ExportBuildLibraryToFile(buf)) {
                if (APIDefs) APIDefs->GUI_SendAlert("Build library exported.");
            } else {
                if (APIDefs) APIDefs->GUI_SendAlert("Export failed: could not write file.");
            }
        }
    }
    ImGui::SameLine();
    if (RenderChipButton("Import from File...", false)) {
        OPENFILENAMEA ofn = {};
        char buf[MAX_PATH] = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = "JSON files\0*.json\0All files\0*.*\0";
        ofn.lpstrFile = buf;
        ofn.nMaxFile = sizeof(buf);
        ofn.lpstrTitle = "Import Build Library";
        ofn.lpstrDefExt = "json";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn)) {
            g_LibImportFilePath = buf;
            ImGui::OpenPopup("Import library");
        }
    }

    // Modal: choose Merge vs Replace for library file import
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                 ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
    if (ImGui::BeginPopupModal("Import library", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Importing: %s", g_LibImportFilePath.c_str());
        ImGui::Spacing();
        ImGui::TextWrapped("Merge keeps your existing builds and adds new ones "
            "(any with matching IDs are skipped). Replace wipes the current "
            "library first.");
        ImGui::Spacing();
        auto doImport = [&](bool replaceAll) {
            int added = 0, skipped = 0;
            std::string err;
            if (AlterEgo::GW2API::ImportBuildLibraryFromFile(
                    g_LibImportFilePath, replaceAll, added, skipped, err)) {
                char msg[256];
                if (replaceAll)
                    std::snprintf(msg, sizeof(msg), "Library replaced: %d builds loaded.", added);
                else
                    std::snprintf(msg, sizeof(msg), "Imported %d new build(s)%s.",
                                  added, skipped > 0 ? (std::string(", skipped ")
                                  + std::to_string(skipped) + " duplicate(s)").c_str() : "");
                if (APIDefs) APIDefs->GUI_SendAlert(msg);
            } else {
                std::string m = "Import failed: " + err;
                if (APIDefs) APIDefs->GUI_SendAlert(m.c_str());
            }
            ImGui::CloseCurrentPopup();
        };
        if (RenderGoldButton("Merge")) doImport(false);
        ImGui::SameLine();
        if (RenderChipButton("Replace All", false)) doImport(true);
        ImGui::SameLine();
        if (RenderChipButton("Cancel", false)) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // Events: Chat addon status
    ImGui::SameLine();
    {
        ImVec4 green(0.35f, 0.82f, 0.35f, 1.0f);
        ImVec4 red  (1.00f, 0.40f, 0.40f, 1.0f);
        (void)RenderChipButton(g_ChatAddonConnected ? "Chat: Connected" : "Chat: Not detected",
                               /*active=*/g_ChatAddonConnected,
                               g_ChatAddonConnected ? green : red);
        if (!g_ChatAddonConnected && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Install 'Events: Chat' from the Nexus addon library");
            ImGui::Text("to import builds shared in GW2 chat.");
            ImGui::EndTooltip();
        }
    }

    // Import panel
    if (g_LibShowImport) {
        ImGui::Separator();
        ImGui::Text("Paste chat link, AE2 build code, or relay code:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##import_link", g_LibImportBuf, sizeof(g_LibImportBuf));

        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("##import_name", g_LibImportName, sizeof(g_LibImportName));
        ImGui::SameLine();
        ImGui::Text("Mode:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::Combo("##import_mode", &g_LibImportMode, GameModeImportNames,
                      IM_ARRAYSIZE(GameModeImportNames));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(for chat links)");

        // Auto-retry once the profession's palette data finishes loading
        bool autoRetry = false;
        if (g_LibImportPending && !g_LibImportPendingProfession.empty() &&
            AlterEgo::GW2API::HasPaletteData(g_LibImportPendingProfession)) {
            g_LibImportPending = false;
            g_LibImportPendingProfession.clear();
            autoRetry = true;
        }

        ImGui::SameLine();
        if (RenderGoldButton("Import") || autoRetry) {
            std::string input(g_LibImportBuf);
            std::string name(g_LibImportName);
            if (input.empty()) {
                g_LibImportError = "Paste a build first.";
            } else {
                // Try shared build format (AE2, relay code, or chat link)
                AlterEgo::SavedBuild sharedBuild;
                std::string sharedError;
                if (ImportSharedBuild(input, sharedBuild, sharedError)) {
                    // If the user entered a name, honour it (AE2/relay imports
                    // fall back to the profession as a placeholder otherwise).
                    if (!name.empty()) sharedBuild.name = name;
                    int newIdx = AlterEgo::GW2API::AddSavedBuild(std::move(sharedBuild));
                    g_LibShowImport = false;
                    g_LibImportBuf[0] = '\0';
                    g_LibImportName[0] = '\0';
                    g_LibImportError.clear();
                    if (newIdx >= 0) g_LibSelectedIdx = newIdx;
                    g_LibDetailsFetched = false;
                    if (APIDefs) APIDefs->GUI_SendAlert("Shared build imported!");
                } else if (!sharedError.empty()) {
                    // Shared format detected but failed. If the failure is just
                    // that the palette is still loading, arm an auto-retry so
                    // the import happens once the data arrives.
                    if (sharedError.rfind("Loading ", 0) == 0) {
                        // Extract profession from "Loading <Prof> skill data..."
                        size_t end = sharedError.find(' ', 8);
                        if (end != std::string::npos) {
                            g_LibImportPendingProfession = sharedError.substr(8, end - 8);
                            g_LibImportPending = true;
                        }
                    }
                    g_LibImportError = sharedError;
                } else {
                    // Not a shared format — try as chat link
                    if (name.empty()) {
                        g_LibImportError = "Enter a build name (required for chat links).";
                    } else {
                        auto linkType = AlterEgo::ChatLink::DetectType(input);
                        if (linkType != AlterEgo::LINK_BUILD) {
                            g_LibImportError = "Not a recognized build format.";
                        } else {
                            AlterEgo::DecodedBuildLink raw;
                            if (!AlterEgo::ChatLink::DecodeBuild(input, raw)) {
                                g_LibImportError = "Failed to decode build link.";
                            } else {
                                std::string prof = ProfessionFromCode(raw.profession);
                                if (prof == "Unknown") {
                                    g_LibImportError = "Unknown profession in link.";
                                } else if (!AlterEgo::GW2API::HasPaletteData(prof)) {
                                    AlterEgo::GW2API::FetchProfessionPaletteAsync(prof);
                                    g_LibImportError = "Loading " + prof + " skill data... try again in a moment.";
                                } else {
                                    AlterEgo::SavedBuild build;
                                    if (DecodeBuildLink(input, name,
                                            GameModeFromIndex(g_LibImportMode), build)) {
                                        int newIdx = AlterEgo::GW2API::AddSavedBuild(std::move(build));
                                        g_LibShowImport = false;
                                        g_LibImportBuf[0] = '\0';
                                        g_LibImportName[0] = '\0';
                                        g_LibImportError.clear();
                                        if (newIdx >= 0) g_LibSelectedIdx = newIdx;
                                        g_LibDetailsFetched = false;
                                        if (APIDefs) APIDefs->GUI_SendAlert("Build imported!");
                                    } else {
                                        g_LibImportError = "Failed to decode build.";
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!g_LibImportError.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", g_LibImportError.c_str());
        }
        ImGui::Separator();
    }

    ImGui::Spacing();

    // Filter + Search
    if (!builds.empty()) {
        ImGui::SetNextItemWidth(80.0f);
        ImGui::Combo("##lib_filter", &g_LibFilterMode, GameModeNames, IM_ARRAYSIZE(GameModeNames));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::InputTextWithHint("##lib_search", "Search...", g_LibSearchBuf, sizeof(g_LibSearchBuf));
    }

    // Left panel: build list, Right panel: preview
    float libAvailW = ImGui::GetContentRegionAvail().x;
    g_LibListWidth = (g_LibListWidth < 120.0f) ? 120.0f : (g_LibListWidth > libAvailW - 200.0f) ? libAvailW - 200.0f : g_LibListWidth;

    ImGui::BeginChild("LibList", ImVec2(g_LibListWidth, 0), true);
    {
        if (builds.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                "No saved builds.\nClick '+ Import Build'\nto add one from a\nchat link.");
        }
        struct LibItemRect { float yMin, yMax; int buildIdx; };
        std::vector<LibItemRect> libItemRects;

        // Group by profession
        std::string lastProf;
        for (int i = 0; i < (int)builds.size(); i++) {
            const auto& b = builds[i];

            // Apply filter
            if (!GameModeMatchesFilter(b.game_mode, g_LibFilterMode)) continue;

            // Apply search
            if (g_LibSearchBuf[0] != '\0') {
                std::string search(g_LibSearchBuf);
                std::string nameLower = b.name;
                std::transform(search.begin(), search.end(), search.begin(), ::tolower);
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                if (nameLower.find(search) == std::string::npos) continue;
            }

            // Profession group header — gold-bar treatment with prof-coloured dot
            if (b.profession != lastProf) {
                if (!lastProf.empty()) ImGui::Dummy(ImVec2(0, 6.0f));
                ImVec4 profColor = GetProfessionColor(b.profession);

                ImVec2 hdrPos = ImGui::GetCursorScreenPos();
                float hdrW = ImGui::GetContentRegionAvail().x;
                float hdrH = ImGui::GetTextLineHeight() + 6.0f;
                ImDrawList* dl = ImGui::GetWindowDrawList();

                // Gold left bar (matches RenderSectionHeader vocabulary)
                dl->AddRectFilled(ImVec2(hdrPos.x, hdrPos.y + 2),
                                  ImVec2(hdrPos.x + 2, hdrPos.y + hdrH - 2),
                                  IM_COL32(232, 196, 122, 230));
                // Profession-coloured dot
                float dotR = 3.5f;
                dl->AddCircleFilled(ImVec2(hdrPos.x + 11.0f, hdrPos.y + hdrH * 0.5f),
                    dotR, ImGui::ColorConvertFloat4ToU32(profColor));
                // Faint trailing rule
                dl->AddLine(ImVec2(hdrPos.x + 18.0f + ImGui::CalcTextSize(b.profession.c_str()).x + 8.0f,
                                   hdrPos.y + hdrH * 0.5f),
                            ImVec2(hdrPos.x + hdrW - 4.0f, hdrPos.y + hdrH * 0.5f),
                            IM_COL32(197, 161, 85, 60), 1.0f);

                // Name (uppercase via Cinzel-style spacing not possible in ImGui — just bold/gold)
                ImGui::Dummy(ImVec2(18.0f, 0)); ImGui::SameLine(0, 0);
                ImGui::TextColored(ImVec4(0.93f, 0.86f, 0.65f, 1.0f), "%s",
                    b.profession.c_str());

                lastProf = b.profession;
            }

            ImGui::PushID(i);
            bool selected = (g_LibSelectedIdx == i);
            ImVec4 bProfColor = GetProfessionColor(b.profession);

            // ===== Compact row card =====
            float rowH = 30.0f;
            ImVec2 cmin = ImGui::GetCursorScreenPos();
            float availW = ImGui::GetContentRegionAvail().x;
            ImVec2 cmax(cmin.x + availW, cmin.y + rowH);

            ImGui::InvisibleButton("##buildrow", ImVec2(availW, rowH));
            bool hovered = ImGui::IsItemHovered();
            bool clicked = ImGui::IsItemClicked();
            // Record rect for drag-and-drop (needs to wrap the InvisibleButton)
            ImVec2 rMin = cmin, rMax = cmax;
            libItemRects.push_back({ rMin.y, rMax.y, i });

            ImDrawList* dl = ImGui::GetWindowDrawList();

            // Background
            if (selected) {
                dl->AddRectFilled(cmin, cmax, IM_COL32(80, 64, 28, 70), 3.0f);
            } else if (hovered) {
                dl->AddRectFilled(cmin, cmax, IM_COL32(255, 255, 255, 18), 3.0f);
            }
            // Selection outline
            if (selected) {
                dl->AddRect(cmin, cmax, IM_COL32(232, 196, 122, 180), 3.0f, 0, 1.0f);
            }

            // Profession-coloured 2px left accent
            ImU32 accentCol = ImGui::ColorConvertFloat4ToU32(
                ImVec4(bProfColor.x, bProfColor.y, bProfColor.z,
                    selected ? 1.0f : (hovered ? 0.8f : 0.55f)));
            dl->AddRectFilled(ImVec2(cmin.x, cmin.y + 2),
                              ImVec2(cmin.x + 2, cmax.y - 2), accentCol);

            // Determine elite spec for this build (if any). Search all three
            // specialization slots — elite isn't always in position 2.
            uint32_t eliteSpecId = 0;
            const AlterEgo::SpecializationInfo* eliteSpec = nullptr;
            for (int s = 0; s < 3; s++) {
                uint32_t sid = b.specializations[s].spec_id;
                if (!sid) continue;
                const auto* si = AlterEgo::GW2API::GetSpecInfo(sid);
                if (si && si->elite) { eliteSpecId = sid; eliteSpec = si; break; }
            }
            // If we have a spec id but no info yet, kick off a fetch so we can
            // tell whether it's elite next frame.
            {
                std::vector<uint32_t> toFetch;
                for (int s = 0; s < 3; s++) {
                    uint32_t sid = b.specializations[s].spec_id;
                    if (sid && !AlterEgo::GW2API::GetSpecInfo(sid)) toFetch.push_back(sid);
                }
                if (!toFetch.empty()) AlterEgo::GW2API::FetchSpecDetailsAsync(toFetch);
            }

            // Icon (18px), priority-loaded. Prefer the elite spec's emblem
            // when present; fall back to the profession icon.
            const float iconSz = 18.0f;
            float iconX = cmin.x + 8.0f;
            float iconY = cmin.y + (rowH - iconSz) * 0.5f;
            Texture_t* iconTex = nullptr;
            if (eliteSpecId && eliteSpec && !eliteSpec->profession_icon_big_url.empty()) {
                uint32_t specIconId = 2000000u + eliteSpecId;
                iconTex = AlterEgo::IconManager::GetIcon(specIconId);
                if (!iconTex || !iconTex->Resource) {
                    AlterEgo::IconManager::RequestIcon(specIconId,
                        eliteSpec->profession_icon_big_url, /*priority=*/true);
                }
            }
            if (!iconTex || !iconTex->Resource) {
                uint32_t profIconId = 9100000u + (uint32_t)(std::hash<std::string>{}(b.profession) % 100000u);
                iconTex = AlterEgo::IconManager::GetIcon(profIconId);
                if (!iconTex || !iconTex->Resource) {
                    const auto* prof = AlterEgo::GW2API::GetProfessionInfo(b.profession);
                    if (prof && !prof->icon_big_url.empty())
                        AlterEgo::IconManager::RequestIcon(profIconId, prof->icon_big_url, /*priority=*/true);
                }
            }
            if (iconTex && iconTex->Resource) {
                dl->AddImage(iconTex->Resource,
                    ImVec2(iconX, iconY), ImVec2(iconX + iconSz, iconY + iconSz),
                    ImVec2(0,0), ImVec2(1,1),
                    selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(220, 220, 220, 220));
            } else {
                // Fallback: profession-coloured tile with a dim ring
                dl->AddRectFilled(ImVec2(iconX, iconY), ImVec2(iconX + iconSz, iconY + iconSz),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(bProfColor.x * 0.25f, bProfColor.y * 0.25f, bProfColor.z * 0.25f, 0.7f)), 2.0f);
                dl->AddRect(ImVec2(iconX, iconY), ImVec2(iconX + iconSz, iconY + iconSz),
                    accentCol, 2.0f, 0, 1.0f);
            }

            // Game-mode chip (right side)
            const char* mode = GameModeLabel(b.game_mode);
            ImU32 chipFg, chipBg, chipBorder;
            switch (b.game_mode) {
                case AlterEgo::GameMode::PvE:
                    chipFg = IM_COL32(160, 220, 170, 255);
                    chipBg = IM_COL32(40, 70, 45, 200);
                    chipBorder = IM_COL32(120, 200, 130, 200);
                    break;
                case AlterEgo::GameMode::WvW:
                    chipFg = IM_COL32(230, 160, 160, 255);
                    chipBg = IM_COL32(70, 30, 30, 200);
                    chipBorder = IM_COL32(200, 110, 110, 200);
                    break;
                case AlterEgo::GameMode::PvP:
                    chipFg = IM_COL32(240, 220, 130, 255);
                    chipBg = IM_COL32(70, 60, 18, 200);
                    chipBorder = IM_COL32(220, 190, 90, 200);
                    break;
                case AlterEgo::GameMode::Raid:
                    chipFg = IM_COL32(232, 196, 122, 255);
                    chipBg = IM_COL32(60, 44, 18, 200);
                    chipBorder = IM_COL32(200, 160, 90, 200);
                    break;
                case AlterEgo::GameMode::Fractal:
                    chipFg = IM_COL32(140, 220, 240, 255);
                    chipBg = IM_COL32(20, 60, 75, 200);
                    chipBorder = IM_COL32(100, 180, 220, 200);
                    break;
                default:
                    chipFg = IM_COL32(190, 180, 160, 230);
                    chipBg = IM_COL32(40, 40, 44, 180);
                    chipBorder = IM_COL32(120, 110, 90, 170);
                    break;
            }
            ImVec2 mSz = ImGui::CalcTextSize(mode);
            float chipPadX = 6.0f, chipPadY = 2.0f;
            float chipW = mSz.x + chipPadX * 2;
            float chipH = mSz.y + chipPadY * 2;
            ImVec2 chipMin(cmax.x - 8.0f - chipW, cmin.y + (rowH - chipH) * 0.5f);
            ImVec2 chipMax(chipMin.x + chipW, chipMin.y + chipH);
            dl->AddRectFilled(chipMin, chipMax, chipBg, 3.0f);
            dl->AddRect(chipMin, chipMax, chipBorder, 3.0f, 0, 1.0f);
            dl->AddText(ImVec2(chipMin.x + chipPadX, chipMin.y + chipPadY), chipFg, mode);

            // Build name (clipped between icon and chip)
            float nameX = iconX + iconSz + 8.0f;
            float nameEndX = chipMin.x - 8.0f;
            ImU32 nameCol = selected ? IM_COL32(245, 220, 150, 255)
                          : hovered  ? IM_COL32(245, 240, 220, 255)
                                     : IM_COL32(225, 220, 205, 255);
            ImVec2 nSz = ImGui::CalcTextSize(b.name.c_str());
            float nameY = cmin.y + (rowH - nSz.y) * 0.5f;
            dl->PushClipRect(ImVec2(nameX, cmin.y), ImVec2(nameEndX, cmax.y), true);
            dl->AddText(ImVec2(nameX, nameY), nameCol, b.name.c_str());
            dl->PopClipRect();

            // Hover tooltip: full name (if clipped) + elite spec (if any)
            if (hovered) {
                bool nameOverflowed = nSz.x > (nameEndX - nameX);
                bool hasSpec = eliteSpec && !eliteSpec->name.empty();
                if (nameOverflowed || hasSpec) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(b.name.c_str());
                    if (hasSpec) {
                        ImGui::TextColored(ImVec4(bProfColor.x, bProfColor.y, bProfColor.z, 1.0f),
                            "%s %s", eliteSpec->name.c_str(), b.profession.c_str());
                    } else {
                        ImGui::TextColored(ImVec4(bProfColor.x, bProfColor.y, bProfColor.z, 1.0f),
                            "%s", b.profession.c_str());
                    }
                    ImGui::EndTooltip();
                }
            }

            if (clicked) {
                g_LibSelectedIdx = i;
                g_LibDetailsFetched = false;
            }

            // Drag source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                g_LibDragIdx = i;
                ImGui::SetDragDropPayload("BUILD_REORDER", &i, sizeof(int));
                ImGui::Text("%s", b.name.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::PopID();
        }

        // Draw insertion line and handle drop
        if (g_LibDragIdx >= 0 && ImGui::GetDragDropPayload() != nullptr) {
            float mouseY = ImGui::GetMousePos().y;
            int insertVisIdx = (int)libItemRects.size();
            float bestLineY = 0;

            for (int vi = 0; vi < (int)libItemRects.size(); vi++) {
                float midY = (libItemRects[vi].yMin + libItemRects[vi].yMax) * 0.5f;
                if (mouseY < midY) {
                    insertVisIdx = vi;
                    bestLineY = libItemRects[vi].yMin;
                    break;
                }
            }
            if (insertVisIdx == (int)libItemRects.size() && !libItemRects.empty())
                bestLineY = libItemRects.back().yMax;

            ImDrawList* dl = ImGui::GetWindowDrawList();
            float xMin = ImGui::GetWindowPos().x + 2;
            float xMax = xMin + ImGui::GetWindowContentRegionMax().x - 4;
            dl->AddLine(ImVec2(xMin, bestLineY), ImVec2(xMax, bestLineY),
                IM_COL32(100, 180, 255, 220), 2.0f);

            if (ImGui::IsMouseReleased(0)) {
                int targetBuildIdx = (insertVisIdx < (int)libItemRects.size())
                    ? libItemRects[insertVisIdx].buildIdx : (int)builds.size();
                if (g_LibDragIdx != targetBuildIdx) {
                    AlterEgo::GW2API::ReorderSavedBuild(g_LibDragIdx, targetBuildIdx);
                    // Fix selection to follow the moved build
                    if (g_LibSelectedIdx == g_LibDragIdx) {
                        int newIdx = (g_LibDragIdx < targetBuildIdx) ? targetBuildIdx - 1 : targetBuildIdx;
                        g_LibSelectedIdx = newIdx;
                    }
                }
                g_LibDragIdx = -1;
            }
        } else if (ImGui::IsMouseReleased(0)) {
            g_LibDragIdx = -1;
        }
    }

    // Deferred delete confirmation from context menu
    if (g_LibCtxDeleteIdx >= 0 && g_LibCtxDeleteIdx < (int)builds.size()) {
        ImGui::OpenPopup("Delete Build?##ctx");
    }
    if (ImGui::BeginPopupModal("Delete Build?##ctx", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        if (g_LibCtxDeleteIdx >= 0 && g_LibCtxDeleteIdx < (int)builds.size()) {
            ImGui::Text("Delete \"%s\"?", builds[g_LibCtxDeleteIdx].name.c_str());
        }
        ImGui::Spacing();
        if (ImGui::Button("Delete", ImVec2(80, 0))) {
            if (g_LibCtxDeleteIdx >= 0 && g_LibCtxDeleteIdx < (int)builds.size()) {
                AlterEgo::GW2API::RemoveSavedBuild(builds[g_LibCtxDeleteIdx].id);
                if (g_LibSelectedIdx == g_LibCtxDeleteIdx) g_LibSelectedIdx = -1;
                else if (g_LibSelectedIdx > g_LibCtxDeleteIdx) g_LibSelectedIdx--;
            }
            g_LibCtxDeleteIdx = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            g_LibCtxDeleteIdx = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();

    // Draggable vertical splitter
    ImGui::SameLine();
    {
        float splitterW = 6.0f;
        float h = ImGui::GetContentRegionAvail().y;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##liblist_splitter", ImVec2(splitterW, h));
        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();
        if (hovered || active)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (active) {
            float delta = ImGui::GetIO().MouseDelta.x;
            if (delta != 0.0f) {
                g_LibListWidth += delta;
                g_LibListWidth = (g_LibListWidth < 120.0f) ? 120.0f : (g_LibListWidth > libAvailW - 200.0f) ? libAvailW - 200.0f : g_LibListWidth;
            }
        }
        if (ImGui::IsItemDeactivated()) {
            SaveSettings();
        }
        ImU32 lineCol = (hovered || active)
            ? IM_COL32(180, 160, 80, 180)
            : IM_COL32(80, 75, 60, 100);
        float cx = pos.x + splitterW * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(cx, pos.y + 4), ImVec2(cx, pos.y + h - 4),
            lineCol, 2.0f);
    }
    ImGui::SameLine();

    // Right panel: preview
    ImGui::BeginChild("LibDetail", ImVec2(0, 0), true);
    if (g_LibSelectedIdx >= 0 && g_LibSelectedIdx < (int)builds.size()) {
        const auto& build = builds[g_LibSelectedIdx];

        // Fetch details on-demand
        if (!g_LibDetailsFetched) {
            FetchDetailsForSavedBuild(build);
            // Ensure palette data for this profession
            if (!AlterEgo::GW2API::HasPaletteData(build.profession)) {
                AlterEgo::GW2API::FetchProfessionPaletteAsync(build.profession);
            }
            // Prefetch all gear-slot icons up front so the preview doesn't
            // sit on letter-badge placeholders waiting for per-slot fetches.
            {
                const char* slots[] = {
                    "Helm","Shoulders","Coat","Gloves","Leggings","Boots",
                    "WeaponA1","WeaponA2","WeaponB1","WeaponB2",
                    "Backpack","Accessory1","Accessory2","Amulet","Ring1","Ring2","Relic"
                };
                std::vector<uint32_t> gearItemIds;
                for (const char* s : slots) {
                    uint32_t id = GetGearSlotIconItemId(build, s);
                    if (id != 0) gearItemIds.push_back(id);
                }
                if (!gearItemIds.empty())
                    AlterEgo::GW2API::FetchItemDetailsAsync(gearItemIds);
            }
            g_LibDetailsFetched = true;
        }

        // Inline rename + notes editing
        if (g_LibEditBuildId != build.id) {
            g_LibEditBuildId = build.id;
            g_LibEditName = build.name;
            g_LibEditNotes = build.notes;
            // Exit edit mode when switching to a different build
            g_LibEditMode = false;
        }

        if (!g_LibEditMode) {
            // Read-only: notes (if any) above the preview. Edit button is
            // drawn inside the preview header's top-right.
            if (!build.notes.empty()) {
                ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "Notes:");
                ImGui::SameLine();
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextColored(ImVec4(0.78f, 0.76f, 0.66f, 1.0f), "%s", build.notes.c_str());
                ImGui::PopTextWrapPos();
            }
        } else {
        // ===== Edit mode =====
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(250.0f);
        if (InputTextString("##edit_name", g_LibEditName,
                ImGuiInputTextFlags_EnterReturnsTrue)) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName.c_str(), g_LibEditNotes.c_str());
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName.c_str(), g_LibEditNotes.c_str());
        }

        // Game-mode chip strip — fix mis-imported modes after the fact
        ImGui::SameLine(0, 12);
        ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "Mode:");
        ImGui::SameLine(0, 6);
        {
            struct ModeChip { AlterEgo::GameMode mode; const char* lbl; };
            const ModeChip modes[] = {
                { AlterEgo::GameMode::PvE,     "PvE" },
                { AlterEgo::GameMode::WvW,     "WvW" },
                { AlterEgo::GameMode::PvP,     "PvP" },
                { AlterEgo::GameMode::Raid,    "Raid" },
                { AlterEgo::GameMode::Fractal, "Fractal" },
            };
            for (int i = 0; i < (int)(sizeof(modes)/sizeof(modes[0])); i++) {
                bool active = (build.game_mode == modes[i].mode);
                if (active) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.40f, 0.18f, 0.80f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.55f, 1.0f));
                }
                ImGui::PushID(i);
                if (ImGui::SmallButton(modes[i].lbl)) {
                    AlterEgo::GW2API::SetSavedBuildGameMode(build.id, modes[i].mode);
                }
                ImGui::PopID();
                if (active) ImGui::PopStyleColor(4);
                if (i < (int)(sizeof(modes)/sizeof(modes[0])) - 1) ImGui::SameLine(0, 3);
            }
        }

        ImGui::Text("Notes:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (InputTextMultilineString("##edit_notes", g_LibEditNotes, ImVec2(0, 60))) {
            // live typing — save handled on deactivate
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName.c_str(), g_LibEditNotes.c_str());
        }

        // Done button to leave edit mode (also persists pending edits)
        if (RenderChipButton("Done##edit", false)) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName.c_str(), g_LibEditNotes.c_str());
            g_LibEditMode = false;
        }
        } // end of edit-mode else

        ImGui::Separator();

        RenderSavedBuildPreview(build, /*showEditButton=*/!g_LibEditMode);
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a build from the list.");
    }
    ImGui::EndChild();

    // Relay code result popup (triggered by SendBuildToRelay)
    if (g_RelaySending && g_RelayResultCode.empty() && g_RelayResultError.empty()) {
        ImGui::OpenPopup("Sending to Mobile...##relay");
    }
    if (!g_RelayResultCode.empty()) {
        ImGui::OpenPopup("Build Sent##relay_ok");
    }
    if (!g_RelayResultError.empty()) {
        ImGui::OpenPopup("Send Failed##relay_err");
    }

    // "Sending..." spinner popup
    if (ImGui::BeginPopupModal("Sending to Mobile...##relay", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Uploading build to relay...");
        if (!g_RelaySending) ImGui::CloseCurrentPopup(); // closes when done
        ImGui::EndPopup();
    }

    // Success popup — show the code
    if (ImGui::BeginPopupModal("Build Sent##relay_ok", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Enter this code in the Alter Ego mobile app,\nor share it with another player.");
        ImGui::Spacing();

        // Large centered code display
        float codeW = ImGui::CalcTextSize("W").x * 8.0f; // generous width for 4 chars with spacing
        float avail = ImGui::GetContentRegionAvail().x;
        if (avail > codeW) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - codeW) * 0.5f);
        ImGui::PushFont(nullptr); // default font, scaled below
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.8f, 0.5f, 1.0f));
        ImGui::SetWindowFontScale(2.0f);
        ImGui::Text("  %s", g_RelayResultCode.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Code expires in 15 minutes. Single use.");

        ImGui::Spacing();
        if (RenderGoldButton("Copy Code", ImVec2(100, 0))) {
            CopyToClipboard(g_RelayResultCode);
            if (APIDefs) APIDefs->GUI_SendAlert("Relay code copied!");
        }
        ImGui::SameLine();
        if (RenderChipButton("Done", false)) {
            g_RelayResultCode.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Error popup
    if (ImGui::BeginPopupModal("Send Failed##relay_err", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", g_RelayResultError.c_str());
        ImGui::Spacing();
        if (RenderChipButton("OK", false)) {
            g_RelayResultError.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

// --- GW2-Themed UI Style ---

static ImGuiStyle g_GW2Style;
static std::vector<ImGuiStyle> g_StyleStack;
static bool g_GW2StyleBuilt = false;

static void PushGW2Theme() {
    g_StyleStack.push_back(ImGui::GetStyle());
    ImGui::GetStyle() = g_GW2Style;
}

static void PopGW2Theme() {
    if (!g_StyleStack.empty()) {
        ImGui::GetStyle() = g_StyleStack.back();
        g_StyleStack.pop_back();
    }
}

static void BuildGW2Theme() {
    // Start from the current (Nexus default) style so we inherit base values
    g_GW2Style = ImGui::GetStyle();
    ImGuiStyle& s = g_GW2Style;

    // Rounding
    s.WindowRounding    = 6.0f;
    s.ChildRounding     = 4.0f;
    s.FrameRounding     = 4.0f;
    s.PopupRounding     = 5.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding      = 3.0f;
    s.TabRounding       = 4.0f;

    // Spacing & padding
    s.WindowPadding     = ImVec2(10, 10);
    s.FramePadding      = ImVec2(6, 4);
    s.ItemSpacing       = ImVec2(8, 5);
    s.ItemInnerSpacing  = ImVec2(6, 4);
    s.ScrollbarSize     = 12.0f;
    s.GrabMinSize       = 8.0f;
    s.WindowBorderSize  = 1.0f;
    s.ChildBorderSize   = 1.0f;
    s.PopupBorderSize   = 1.5f;
    s.FrameBorderSize   = 0.0f;
    s.TabBorderSize     = 0.0f;

    // Colors — dark slate base with warm gold accents
    ImVec4* c = s.Colors;

    // Backgrounds
    c[ImGuiCol_WindowBg]             = ImVec4(0.08f, 0.08f, 0.10f, 0.96f);
    c[ImGuiCol_ChildBg]              = ImVec4(0.07f, 0.07f, 0.09f, 0.80f);
    c[ImGuiCol_PopupBg]              = ImVec4(0.085f, 0.075f, 0.06f, 0.97f);

    // Borders
    c[ImGuiCol_Border]               = ImVec4(0.28f, 0.25f, 0.18f, 0.50f);
    c[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames (input boxes, combos)
    c[ImGuiCol_FrameBg]              = ImVec4(0.14f, 0.13f, 0.11f, 0.80f);
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.22f, 0.20f, 0.14f, 0.80f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.28f, 0.25f, 0.16f, 0.90f);

    // Title bar
    c[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.09f, 0.07f, 1.00f);
    c[ImGuiCol_TitleBgActive]        = ImVec4(0.16f, 0.14f, 0.08f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.08f, 0.07f, 0.05f, 0.75f);

    // Menu bar
    c[ImGuiCol_MenuBarBg]            = ImVec4(0.12f, 0.11f, 0.09f, 1.00f);

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.06f, 0.06f, 0.07f, 0.60f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.30f, 0.27f, 0.18f, 0.80f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.36f, 0.22f, 0.90f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.44f, 0.26f, 1.00f);

    // Checkmark, slider
    c[ImGuiCol_CheckMark]            = ImVec4(0.90f, 0.75f, 0.25f, 1.00f);
    c[ImGuiCol_SliderGrab]           = ImVec4(0.70f, 0.58f, 0.20f, 1.00f);
    c[ImGuiCol_SliderGrabActive]     = ImVec4(0.85f, 0.70f, 0.25f, 1.00f);

    // Buttons — warm gold
    c[ImGuiCol_Button]               = ImVec4(0.22f, 0.20f, 0.12f, 0.80f);
    c[ImGuiCol_ButtonHovered]        = ImVec4(0.35f, 0.30f, 0.14f, 0.90f);
    c[ImGuiCol_ButtonActive]         = ImVec4(0.45f, 0.38f, 0.16f, 1.00f);

    // Headers (selectables, collapsing headers)
    c[ImGuiCol_Header]               = ImVec4(0.18f, 0.16f, 0.10f, 0.70f);
    c[ImGuiCol_HeaderHovered]        = ImVec4(0.28f, 0.24f, 0.12f, 0.80f);
    c[ImGuiCol_HeaderActive]         = ImVec4(0.35f, 0.30f, 0.14f, 0.90f);

    // Separator
    c[ImGuiCol_Separator]            = ImVec4(0.28f, 0.25f, 0.18f, 0.40f);
    c[ImGuiCol_SeparatorHovered]     = ImVec4(0.50f, 0.42f, 0.20f, 0.70f);
    c[ImGuiCol_SeparatorActive]      = ImVec4(0.65f, 0.55f, 0.25f, 1.00f);

    // Resize grip
    c[ImGuiCol_ResizeGrip]           = ImVec4(0.30f, 0.27f, 0.18f, 0.30f);
    c[ImGuiCol_ResizeGripHovered]    = ImVec4(0.50f, 0.44f, 0.26f, 0.60f);
    c[ImGuiCol_ResizeGripActive]     = ImVec4(0.65f, 0.55f, 0.25f, 0.90f);

    // Tabs — gold accent for active
    c[ImGuiCol_Tab]                  = ImVec4(0.14f, 0.13f, 0.10f, 0.86f);
    c[ImGuiCol_TabHovered]           = ImVec4(0.35f, 0.30f, 0.14f, 0.90f);
    c[ImGuiCol_TabActive]            = ImVec4(0.28f, 0.24f, 0.10f, 1.00f);
    c[ImGuiCol_TabUnfocused]         = ImVec4(0.10f, 0.09f, 0.07f, 0.97f);
    c[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.18f, 0.16f, 0.10f, 1.00f);

    // Text
    c[ImGuiCol_Text]                 = ImVec4(0.90f, 0.87f, 0.78f, 1.00f);
    c[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.47f, 0.40f, 1.00f);

    // Modal dim background
    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);

    // Nav highlight
    c[ImGuiCol_NavHighlight]         = ImVec4(0.70f, 0.58f, 0.20f, 1.00f);

    // Table
    c[ImGuiCol_TableHeaderBg]        = ImVec4(0.14f, 0.13f, 0.10f, 1.00f);
    c[ImGuiCol_TableBorderStrong]    = ImVec4(0.28f, 0.25f, 0.18f, 0.60f);
    c[ImGuiCol_TableBorderLight]     = ImVec4(0.22f, 0.20f, 0.15f, 0.40f);
    c[ImGuiCol_TableRowBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_TableRowBgAlt]        = ImVec4(0.10f, 0.10f, 0.08f, 0.30f);

    // Plot (progress bars)
    c[ImGuiCol_PlotHistogram]        = ImVec4(0.65f, 0.55f, 0.15f, 1.00f);
    c[ImGuiCol_PlotHistogramHovered] = ImVec4(0.80f, 0.68f, 0.20f, 1.00f);
}

// --- Addon Lifecycle ---

void AddonLoad(AddonAPI_t* aApi) {
    APIDefs = aApi;
    ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
    ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))APIDefs->ImguiMalloc,
                                 (void(*)(void*, void*))APIDefs->ImguiFree);

    BuildGW2Theme();

    // Initialize subsystems
    AlterEgo::GW2API::Initialize(APIDefs);
    AlterEgo::IconManager::Initialize(APIDefs);

    // Load cached character data and build library from disk
    AlterEgo::GW2API::LoadCharacterData();
    AlterEgo::GW2API::LoadBuildLibrary();
    AlterEgo::GW2API::LoadItemNameCache();
    AlterEgo::GW2API::LoadReferenceCaches();
    SeedItemNameCache();
    LoadLoginTimestamps();
    AlterEgo::GW2API::FetchAllItemStatsAsync();
    LoadCharSortConfig();
    RebuildCharDisplayOrder();

    // Subscribe to MumbleLink identity for current character indicator
    APIDefs->Events_Subscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

    // Subscribe to Events: Chat for build link detection
    APIDefs->Events_Subscribe(EV_CHAT_MESSAGE, OnEvChatMessage);

    // Subscribe to Events: Alerts for real-time skin unlock + achievement completion
    APIDefs->Events_Subscribe(EV_ALERT_UNLOCKED_SKIN, OnEvAlertSkinUnlocked);
    APIDefs->Events_Subscribe(EV_ALERT_ACHIEVEMENT_COMPLETED, OnEvAlertAchievementCompleted);

    // Subscribe to H&S events
    APIDefs->Events_Subscribe(EV_HOARD_PONG, AlterEgo::GW2API::OnHoardPong);
    APIDefs->Events_Subscribe(EV_HOARD_PONG, OnHoardPongForAch);
    APIDefs->Events_Subscribe(EV_HOARD_DATA_UPDATED, AlterEgo::GW2API::OnHoardDataUpdated);
    APIDefs->Events_Subscribe(EV_AE_CHAR_LIST_RESP, AlterEgo::GW2API::OnCharListResponse);
    APIDefs->Events_Subscribe(EV_AE_CHAR_DATA_RESP, AlterEgo::GW2API::OnCharDataResponse);
    APIDefs->Events_Subscribe(EV_AE_SKIN_UNLOCK_RESP, AlterEgo::GW2API::OnSkinUnlocksResponse);
    APIDefs->Events_Subscribe(EV_AE_ITEM_LOC_RESP, AlterEgo::GW2API::OnItemLocationResponse);
    APIDefs->Events_Subscribe(EV_AE_CLEARS_ACH_RESPONSE, OnClearsAchResponse);
    APIDefs->Events_Subscribe(EV_AE_VAULT_RESPONSE, OnVaultResponse);
    APIDefs->Events_Subscribe(EV_AE_VAULT_SEASON_RESP, OnVaultSeasonResponse);
    APIDefs->Events_Subscribe(EV_AE_ACH_PROGRESS_RESPONSE, OnAchProgressResponse);
    APIDefs->Events_Subscribe(EV_AE_ACCOUNTS_RESP, AlterEgo::GW2API::OnAccountsResponse);
    APIDefs->Events_Subscribe(EV_HOARD_ACCOUNTS_CHANGED, AlterEgo::GW2API::OnAccountsChanged);

    // Register render functions
    APIDefs->GUI_Register(RT_Render, AddonRender);
    APIDefs->GUI_Register(RT_OptionsRender, AddonOptions);

    // Register keybind
    APIDefs->InputBinds_RegisterWithString("KB_ALTER_EGO_TOGGLE", ProcessKeybind, "CTRL+SHIFT+E");
    APIDefs->InputBinds_RegisterWithString("KB_ALTER_EGO_TRACKER", ProcessKeybind, "(null)");

    // Load icon textures from embedded PNG data
    APIDefs->Textures_LoadFromMemory(TEX_ICON, (void*)ICON_NORMAL, ICON_NORMAL_size, nullptr);
    APIDefs->Textures_LoadFromMemory(TEX_ICON_HOVER, (void*)ICON_HOVER, ICON_HOVER_size, nullptr);

    // Load settings, session, and cached data
    LoadSettings();
    LoadSession();
    LoadClearsCache();
    LoadVaultCache();
    LoadAchTrackerState();
    LoadAchGroupCache();
    LoadAchDefCache();
    LoadAchNameIndex();
    LoadAchWaypoints();
    LoadHeroChallenges();

    // Register quick access shortcut
    if (g_ShowQAIcon) {
        APIDefs->QuickAccess_Add(QA_ID, TEX_ICON, TEX_ICON_HOVER, "KB_ALTER_EGO_TOGGLE", "Alter Ego");
    }

    // Register close-on-escape
    APIDefs->GUI_RegisterCloseOnEscape("Alter Ego", &g_WindowVisible);

    // Start fetching dye colors and profession icons (public endpoints, no auth needed)
    AlterEgo::GW2API::FetchDyeColorsAsync();
    static const char* allProfs[] = {
        "Guardian", "Warrior", "Engineer", "Ranger", "Thief",
        "Elementalist", "Mesmer", "Necromancer", "Revenant"
    };
    for (const char* p : allProfs)
        AlterEgo::GW2API::FetchProfessionInfoAsync(p);

    // Ping H&S to check availability
    AlterEgo::GW2API::PingHoard();

    // Fetch pinned + saved category achievement definitions (public API, no H&S needed)
    // Progress is queried separately when H&S becomes available (OnHoardPongForAch)
    if (!g_AchPinned.empty() || g_AchSelectedCatId > 0) {
        uint32_t savedCat = g_AchSelectedCatId;
        std::thread([savedCat]() {
            FetchPinnedAchDefs();
            if (savedCat > 0) FetchAchCategoryDefs(savedCat);
        }).detach();
    }

    // Initialize Skinventory subsystems
    {
        std::string dataDir = AlterEgo::GW2API::GetDataDirectory();
        std::string skinDataDir = dataDir + "\\Skinventory";
        try { std::filesystem::create_directories(skinDataDir); } catch (...) {}
        std::string wikiCacheDir = skinDataDir + "\\wiki_cache";

        Skinventory::SkinCache::SetDataDirectory(skinDataDir);
        Skinventory::Commerce::SetDataDirectory(skinDataDir);
        Skinventory::OwnedSkins::SetDataDirectory(skinDataDir);
        Skinventory::OwnedSkins::Initialize(APIDefs);
        Skinventory::WikiImage::Initialize(APIDefs, wikiCacheDir);
        // Prefetch raid rotation icons used by the Clears tab.
        Skinventory::WikiImage::DownloadCurrencyIcons("Emboldened,Call_of_the_Mists");

        if (Skinventory::SkinCache::LoadFromDisk()) {
            Skinventory::SkinCache::UpdateCacheAsync();
        } else {
            Skinventory::SkinCache::FetchAllSkinsAsync();
        }

        if (!Skinventory::Commerce::LoadItemMap()) {
            Skinventory::Commerce::BuildItemMapAsync();
        }
        Skinventory::Commerce::LoadPriceCache();

        // Ping H&S so Skinventory's OwnedSkins receives the pong
        // (must be after Initialize which subscribes to EV_HOARD_PONG)
        Skinventory::OwnedSkins::PingHoardAndSeek();

        g_SkinInitialized = true;
    }

    APIDefs->Log(LOGL_INFO, "AlterEgo", "Addon loaded successfully");
}

void AddonUnload() {
    // Unsubscribe MumbleLink identity
    APIDefs->Events_Unsubscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

    // Unsubscribe chat events
    APIDefs->Events_Unsubscribe(EV_CHAT_MESSAGE, OnEvChatMessage);

    // Unsubscribe Events: Alerts
    APIDefs->Events_Unsubscribe(EV_ALERT_UNLOCKED_SKIN, OnEvAlertSkinUnlocked);
    APIDefs->Events_Unsubscribe(EV_ALERT_ACHIEVEMENT_COMPLETED, OnEvAlertAchievementCompleted);

    // Unsubscribe H&S events
    APIDefs->Events_Unsubscribe(EV_HOARD_PONG, AlterEgo::GW2API::OnHoardPong);
    APIDefs->Events_Unsubscribe(EV_HOARD_PONG, OnHoardPongForAch);
    APIDefs->Events_Unsubscribe(EV_HOARD_DATA_UPDATED, AlterEgo::GW2API::OnHoardDataUpdated);
    APIDefs->Events_Unsubscribe(EV_AE_CHAR_LIST_RESP, AlterEgo::GW2API::OnCharListResponse);
    APIDefs->Events_Unsubscribe(EV_AE_CHAR_DATA_RESP, AlterEgo::GW2API::OnCharDataResponse);
    APIDefs->Events_Unsubscribe(EV_AE_SKIN_UNLOCK_RESP, AlterEgo::GW2API::OnSkinUnlocksResponse);
    APIDefs->Events_Unsubscribe(EV_AE_ITEM_LOC_RESP, AlterEgo::GW2API::OnItemLocationResponse);
    APIDefs->Events_Unsubscribe(EV_AE_CLEARS_ACH_RESPONSE, OnClearsAchResponse);
    APIDefs->Events_Unsubscribe(EV_AE_VAULT_RESPONSE, OnVaultResponse);
    APIDefs->Events_Unsubscribe(EV_AE_VAULT_SEASON_RESP, OnVaultSeasonResponse);
    APIDefs->Events_Unsubscribe(EV_AE_ACH_PROGRESS_RESPONSE, OnAchProgressResponse);
    APIDefs->Events_Unsubscribe(EV_AE_ACCOUNTS_RESP, AlterEgo::GW2API::OnAccountsResponse);
    APIDefs->Events_Unsubscribe(EV_HOARD_ACCOUNTS_CHANGED, AlterEgo::GW2API::OnAccountsChanged);

    // Shutdown Skinventory subsystems
    Skinventory::WikiImage::Shutdown();
    Skinventory::OwnedSkins::Shutdown();

    AlterEgo::IconManager::Shutdown();
    AlterEgo::GW2API::Shutdown();

    APIDefs->GUI_DeregisterCloseOnEscape("Alter Ego");
    APIDefs->QuickAccess_Remove(QA_ID);
    APIDefs->GUI_Deregister(AddonOptions);
    APIDefs->GUI_Deregister(AddonRender);

    SaveSession();
    SaveSettings();
    SaveAchTrackerState();
    if (g_LoginTimestampsDirty) SaveLoginTimestamps();
    if (g_AchProgressDirty && !g_AchCachedAccount.empty()) {
        SaveAchProgress(g_AchCachedAccount);
    }
    AlterEgo::GW2API::SaveItemNameCache();
    APIDefs = nullptr;
}

static void SaveLoginTimestamps() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/login_times.json";
    nlohmann::json j;
    for (const auto& [name, ts] : g_LoginTimestamps)
        j[name] = ts;
    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
    g_LoginTimestampsDirty = false;
}

static void LoadLoginTimestamps() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/login_times.json";
    std::ifstream file(path);
    if (!file.is_open()) return;
    try {
        auto j = nlohmann::json::parse(file);
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.value().is_number())
                g_LoginTimestamps[it.key()] = it.value().get<int64_t>();
        }
    } catch (...) {}
}

// Sanitize an account name to a filename-safe slug (account names contain '.').
static std::string SanitizeForFilename(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (isalnum((unsigned char)c) || c == '-' || c == '_') out += c;
        else out += '_';
    }
    return out;
}

// Persist g_AchProgress for the given account to disk. Per-account so that
// switching accounts doesn't clobber the previous account's cache.
static void SaveAchProgress(const std::string& account) {
    if (account.empty()) return;
    std::string dir = AlterEgo::GW2API::GetDataDirectory() + "/cache";
    std::filesystem::create_directories(dir);
    std::string path = dir + "/ach_progress_" + SanitizeForFilename(account) + ".json";

    // Snapshot under the lock, then build/serialize JSON outside the lock
    // so the render thread isn't blocked while we touch a large map.
    std::vector<std::pair<uint32_t, AchProgress>> snapshot;
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        snapshot.reserve(g_AchProgress.size());
        for (const auto& [id, p] : g_AchProgress) snapshot.emplace_back(id, p);
        g_AchProgressDirty = false;
        g_AchLastSave = std::chrono::steady_clock::now();
    }

    nlohmann::json j = nlohmann::json::object();
    j["saved_at"] = (int64_t)std::time(nullptr);
    nlohmann::json entries = nlohmann::json::object();
    for (const auto& [id, p] : snapshot) {
        nlohmann::json e;
        e["c"] = p.current;
        e["m"] = p.max;
        e["d"] = p.done;
        e["r"] = p.repeated;
        e["u"] = p.unlocked;
        if (!p.completed_bits.empty()) {
            nlohmann::json bits = nlohmann::json::array();
            for (uint32_t b : p.completed_bits) bits.push_back(b);
            e["b"] = bits;
        }
        entries[std::to_string(id)] = e;
    }
    j["entries"] = entries;

    std::ofstream file(path);
    if (file.is_open()) file << j.dump();
}

// Load achievement progress for an account from disk. Bumps the progress gen
// so dependent UI caches (rail counts, popout) refresh.
static void LoadAchProgress(const std::string& account) {
    if (account.empty()) return;
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/cache/ach_progress_" +
                       SanitizeForFilename(account) + ".json";
    std::ifstream file(path);
    if (!file.is_open()) {
        g_AchCachedAccount = account;
        return;
    }
    try {
        auto j = nlohmann::json::parse(file);
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        g_AchProgress.clear();
        if (j.contains("entries") && j["entries"].is_object()) {
            for (auto it = j["entries"].begin(); it != j["entries"].end(); ++it) {
                AchProgress p;
                p.id = (uint32_t)std::stoul(it.key());
                const auto& e = it.value();
                p.current = e.value("c", 0);
                p.max = e.value("m", 0);
                p.done = e.value("d", false);
                p.repeated = e.value("r", 0);
                p.unlocked = e.value("u", true);
                if (e.contains("b") && e["b"].is_array()) {
                    for (const auto& b : e["b"]) p.completed_bits.insert(b.get<uint32_t>());
                }
                g_AchProgress[p.id] = std::move(p);
            }
        }
        g_AchProgressGen++;
        g_AchCachedAccount = account;
    } catch (...) {
        g_AchCachedAccount = account;
    }
}

void OnMumbleIdentityUpdated(void* eventArgs) {
    if (!eventArgs) return;
    const MumbleIdentity* id = (const MumbleIdentity*)eventArgs;
    char buf[20] = {};
    memcpy(buf, id->Name, 19);
    std::string newName(buf);
    // Validate: GW2 character names contain only letters, spaces, hyphens, accented chars
    for (unsigned char c : newName) {
        if (c == ' ' || c == '-' || isalpha(c) || c >= 0x80) continue;
        return; // invalid — likely garbage data from uninitialized MumbleLink
    }
    if (newName.empty()) return;
    if (newName != g_CurrentCharName) {
        g_LoginTimestamps[newName] = (int64_t)std::time(nullptr);
        g_LoginTimestampsDirty = true;
        g_CurrentCharName = newName;
        // Resolve which account this character belongs to
        AlterEgo::GW2API::SetCurrentAccountFromCharacter(newName);
    }
}

void ProcessKeybind(const char* aIdentifier, bool aIsRelease) {
    if (aIsRelease) return;

    if (strcmp(aIdentifier, "KB_ALTER_EGO_TOGGLE") == 0) {
        g_WindowVisible = !g_WindowVisible;
        if (APIDefs) {
            APIDefs->Log(LOGL_INFO, "AlterEgo",
                g_WindowVisible ? "Window shown" : "Window hidden");
        }
    }
    else if (strcmp(aIdentifier, "KB_ALTER_EGO_TRACKER") == 0) {
        g_AchPopoutVisible = !g_AchPopoutVisible;
        SaveAchTrackerState();
    }
}


// =========================================================================
// Skinventory UI
// =========================================================================

static ImVec4 GetSkinRarityColor(const std::string& rarity) {
    if (rarity == "Junk")       return ImVec4(0.67f, 0.67f, 0.67f, 1.0f);
    if (rarity == "Basic")      return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    if (rarity == "Fine")       return ImVec4(0.38f, 0.58f, 1.0f, 1.0f);
    if (rarity == "Masterwork") return ImVec4(0.12f, 0.72f, 0.12f, 1.0f);
    if (rarity == "Rare")       return ImVec4(0.98f, 0.82f, 0.0f, 1.0f);
    if (rarity == "Exotic")     return ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
    if (rarity == "Ascended")   return ImVec4(0.98f, 0.35f, 0.56f, 1.0f);
    if (rarity == "Legendary")  return ImVec4(0.63f, 0.21f, 0.93f, 1.0f);
    return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
}

static const ImVec4 COIN_GOLD   = ImVec4(0.85f, 0.75f, 0.10f, 1.0f);
static const ImVec4 COIN_SILVER = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
static const ImVec4 COIN_COPPER = ImVec4(0.72f, 0.45f, 0.20f, 1.0f);

static void DrawCoinIcon(ImVec4 color, float radius) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float y_center = pos.y + ImGui::GetTextLineHeight() * 0.5f;
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(pos.x + radius, y_center), radius,
        ImGui::GetColorU32(color));
    ImGui::GetWindowDrawList()->AddCircle(
        ImVec2(pos.x + radius, y_center), radius,
        ImGui::GetColorU32(ImVec4(color.x * 0.5f, color.y * 0.5f, color.z * 0.5f, 1.0f)));
    ImGui::Dummy(ImVec2(radius * 2.0f + 1.0f, ImGui::GetTextLineHeight()));
}

static void RenderCoins(int copper) {
    if (copper <= 0) {
        ImGui::TextColored(COIN_COPPER, "0");
        ImGui::SameLine(0, 2);
        DrawCoinIcon(COIN_COPPER, 5.0f);
        return;
    }
    int gold = copper / 10000;
    int silver = (copper % 10000) / 100;
    int cop = copper % 100;

    bool needSame = false;
    if (gold > 0) {
        ImGui::TextColored(COIN_GOLD, "%d", gold);
        ImGui::SameLine(0, 1);
        DrawCoinIcon(COIN_GOLD, 5.0f);
        needSame = true;
    }
    if (silver > 0 || gold > 0) {
        if (needSame) ImGui::SameLine(0, 2);
        ImGui::TextColored(COIN_SILVER, "%d", silver);
        ImGui::SameLine(0, 1);
        DrawCoinIcon(COIN_SILVER, 5.0f);
        needSame = true;
    }
    if (needSame) ImGui::SameLine(0, 2);
    ImGui::TextColored(COIN_COPPER, "%d", cop);
    ImGui::SameLine(0, 1);
    DrawCoinIcon(COIN_COPPER, 5.0f);
}

// Cached owned/total counts per skin category — recomputed when the
// OwnedSkins generation changes (i.e. after a Refresh).
static std::pair<int,int> SkinCategoryCounts(const std::string& type,
                                             const std::string& sub,
                                             const std::string& wc) {
    static std::unordered_map<std::string, std::pair<int,int>> s_cache;
    static uint64_t s_gen = (uint64_t)-1;
    uint64_t gen = Skinventory::OwnedSkins::GetGeneration();
    if (gen != s_gen) { s_cache.clear(); s_gen = gen; }

    std::string key = type + "|" + sub + "|" + wc;
    auto it = s_cache.find(key);
    if (it != s_cache.end()) return it->second;

    auto skins = Skinventory::SkinCache::GetSkinsByCategory(type, sub, wc);
    int owned = 0;
    for (uint32_t id : skins) {
        if (Skinventory::OwnedSkins::IsOwned(id)) owned++;
    }
    auto result = std::make_pair(owned, (int)skins.size());
    s_cache[key] = result;
    return result;
}

// Single nav row with optional owned/total count and active gold accent.
static bool RenderSkinNavRow(const char* label, bool active,
                             int owned, int total,
                             ImVec4 dotColor, bool indent) {
    ImGui::PushID(label);
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    float rowH = ImGui::GetTextLineHeight() + 4.0f;

    if (indent) ImGui::Indent(8.0f);

    // Active gold-bar background fill behind text
    if (active) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 bgMin(startPos.x, startPos.y);
        ImVec2 bgMax(startPos.x + availW, startPos.y + rowH);
        dl->AddRectFilled(bgMin, bgMax, IM_COL32(80, 64, 28, 60), 2.0f);
        dl->AddRectFilled(ImVec2(bgMin.x, bgMin.y + 2),
                          ImVec2(bgMin.x + 2, bgMax.y - 2),
                          IM_COL32(232, 196, 122, 230));
    }

    // Small coloured dot as a category accent
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float cx = p.x + 4.0f;
        float cy = p.y + ImGui::GetTextLineHeight() * 0.5f;
        dl->AddCircleFilled(ImVec2(cx, cy), 3.5f, ImGui::GetColorU32(dotColor));
        ImGui::Dummy(ImVec2(12.0f, ImGui::GetTextLineHeight()));
        ImGui::SameLine(0, 2);
    }

    // Tag the count to the label so Selectable owns the full row width
    char countBuf[32] = {0};
    if (total > 0) {
        if (owned >= 0) snprintf(countBuf, sizeof(countBuf), "  %d/%d", owned, total);
        else            snprintf(countBuf, sizeof(countBuf), "  (%d)", total);
    }

    // Compute right-aligned count and use Selectable for hit-testing
    ImVec4 textCol = active ? ImVec4(0.95f, 0.85f, 0.55f, 1.0f)
                            : ImVec4(0.85f, 0.83f, 0.75f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, textCol);
    bool clicked = ImGui::Selectable(label, false, 0, ImVec2(0, 0));
    ImGui::PopStyleColor();

    // Right-align count (drawn on top of selectable)
    if (countBuf[0]) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cSize = ImGui::CalcTextSize(countBuf);
        float cx = startPos.x + availW - cSize.x - 6.0f;
        float cy = startPos.y + (rowH - cSize.y) * 0.5f - 2.0f;
        ImU32 countCol = (owned >= 0 && total > 0 && owned == total)
            ? IM_COL32(120, 200, 120, 220)
            : active ? IM_COL32(232, 196, 122, 220) : IM_COL32(140, 132, 110, 220);
        dl->AddText(ImVec2(cx, cy), countCol, countBuf);
    }

    if (indent) ImGui::Unindent(8.0f);
    ImGui::PopID();
    return clicked;
}

static void RenderSkinCategoryNav() {
    ImGui::BeginChild("CategoryNav", ImVec2(180, 0), true);

    bool hasOwner = Skinventory::OwnedSkins::HasData();

    // Compute aggregate count for a top-level type by summing its sub-categories
    auto typeTotals = [&](const std::string& type) -> std::pair<int,int> {
        int owned = 0, total = 0;
        if (type == "Armor") {
            for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
                if (wc == "Clothing") continue;
                for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(wc)) {
                    auto p = SkinCategoryCounts("Armor", slot, wc);
                    owned += p.first; total += p.second;
                }
            }
        } else if (type == "Weapon") {
            for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
                auto p = SkinCategoryCounts("Weapon", wt, "");
                owned += p.first; total += p.second;
            }
        } else if (type == "Back") {
            auto p = SkinCategoryCounts("Back", "", "");
            owned += p.first; total += p.second;
        }
        return {owned, total};
    };

    ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "CATEGORY");
    ImGui::Separator();

    {
        auto p = typeTotals("Armor");
        if (RenderSkinNavRow("Armor", g_SkinSelectedType == "Armor",
                             hasOwner ? p.first : -1, p.second,
                             ImVec4(0.45f, 0.65f, 0.85f, 1.0f), false)) {
            g_SkinSelectedType = "Armor";
            g_SkinSelectedWeightClass = "Heavy";
            g_SkinSelectedSubtype = "Helm";
            g_SkinSelectedId = 0;
        }
    }
    {
        auto p = typeTotals("Weapon");
        if (RenderSkinNavRow("Weapons", g_SkinSelectedType == "Weapon",
                             hasOwner ? p.first : -1, p.second,
                             ImVec4(0.85f, 0.45f, 0.35f, 1.0f), false)) {
            g_SkinSelectedType = "Weapon";
            g_SkinSelectedWeightClass = "";
            g_SkinSelectedSubtype = "Axe";
            g_SkinSelectedId = 0;
        }
    }
    {
        auto p = typeTotals("Back");
        if (RenderSkinNavRow("Back", g_SkinSelectedType == "Back",
                             hasOwner ? p.first : -1, p.second,
                             ImVec4(0.65f, 0.50f, 0.80f, 1.0f), false)) {
            g_SkinSelectedType = "Back";
            g_SkinSelectedWeightClass = "";
            g_SkinSelectedSubtype = "";
            g_SkinSelectedId = 0;
        }
    }

    ImGui::Separator();

    if (g_SkinSelectedType == "Armor") {
        ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "WEIGHT CLASS");
        auto wcColor = [](const std::string& wc) -> ImVec4 {
            if (wc == "Heavy")  return ImVec4(0.65f, 0.65f, 0.70f, 1.0f);
            if (wc == "Medium") return ImVec4(0.65f, 0.50f, 0.30f, 1.0f);
            if (wc == "Light")  return ImVec4(0.55f, 0.45f, 0.70f, 1.0f);
            return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        };
        for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
            if (wc == "Clothing") continue;
            int owned = 0, total = 0;
            for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(wc)) {
                auto p = SkinCategoryCounts("Armor", slot, wc);
                owned += p.first; total += p.second;
            }
            if (RenderSkinNavRow(wc.c_str(), g_SkinSelectedWeightClass == wc,
                                 hasOwner ? owned : -1, total,
                                 wcColor(wc), false)) {
                g_SkinSelectedWeightClass = wc;
                g_SkinSelectedId = 0;
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "SLOT");
        for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(g_SkinSelectedWeightClass)) {
            auto p = SkinCategoryCounts("Armor", slot, g_SkinSelectedWeightClass);
            if (RenderSkinNavRow(slot.c_str(), g_SkinSelectedSubtype == slot,
                                 hasOwner ? p.first : -1, p.second,
                                 ImVec4(0.45f, 0.65f, 0.85f, 1.0f), true)) {
                g_SkinSelectedSubtype = slot;
                g_SkinSelectedId = 0;
            }
        }
    } else if (g_SkinSelectedType == "Weapon") {
        ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "WEAPON TYPE");
        for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
            auto p = SkinCategoryCounts("Weapon", wt, "");
            if (RenderSkinNavRow(wt.c_str(), g_SkinSelectedSubtype == wt,
                                 hasOwner ? p.first : -1, p.second,
                                 ImVec4(0.85f, 0.45f, 0.35f, 1.0f), false)) {
                g_SkinSelectedSubtype = wt;
                g_SkinSelectedId = 0;
            }
        }
    }

    ImGui::EndChild();
}

static void RenderSkinList() {
    ImGui::BeginChild("SkinList", ImVec2(300, 0), true);

    // Search input + view toggle on the same row
    {
        const char* opts[] = { "List", "Grid" };
        float btnW = 44.0f;
        float gap = 2.0f;
        float toggleW = btnW * 2 + gap;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float inputW = ImGui::GetContentRegionAvail().x - toggleW - spacing;
        if (inputW < 60.0f) inputW = 60.0f;

        ImGui::SetNextItemWidth(inputW);
        ImGui::InputTextWithHint("##skinfilter", "Filter skins...", g_SkinSearchFilter, sizeof(g_SkinSearchFilter));

        ImGui::SameLine();
        for (int i = 0; i < 2; i++) {
            bool active = (g_SkinViewMode == i);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.40f, 0.18f, 0.80f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.55f, 1.0f));
            }
            if (ImGui::Button(opts[i], ImVec2(btnW, 0))) {
                g_SkinViewMode = i;
                SaveSettings();
            }
            if (active) ImGui::PopStyleColor(4);
            if (i == 0) ImGui::SameLine(0, gap);
        }
    }

    {
        // Chip strip: All / Owned / Unowned (filters only take effect once owned data is loaded)
        bool hasOwnerForFilter = Skinventory::OwnedSkins::HasData();
        if (!hasOwnerForFilter) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.55f);
        struct Chip { const char* lbl; bool sOwned; bool sUnowned; };
        const Chip chips[] = {
            { "All",     true,  true  },
            { "Owned",   true,  false },
            { "Unowned", false, true  },
        };
        for (int i = 0; i < 3; i++) {
            bool active = (g_SkinShowOwned == chips[i].sOwned) &&
                          (g_SkinShowUnowned == chips[i].sUnowned);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.40f, 0.18f, 0.80f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.55f, 1.0f));
            }
            if (ImGui::SmallButton(chips[i].lbl)) {
                g_SkinShowOwned = chips[i].sOwned;
                g_SkinShowUnowned = chips[i].sUnowned;
            }
            if (active) ImGui::PopStyleColor(4);
            if (i < 2) ImGui::SameLine(0, 4);
        }
        if (!hasOwnerForFilter) ImGui::PopStyleVar();
    }

    ImGui::Separator();

    // Cached display list — only rebuild when inputs change
    struct SkinDisplayEntry { uint32_t id; bool owned; };
    static std::vector<SkinDisplayEntry> s_displaySkins;
    static int s_ownedCount = 0;
    static int s_totalCount = 0;
    static std::string s_cacheType;
    static std::string s_cacheSubtype;
    static std::string s_cacheWeight;
    static std::string s_cacheFilter;
    static bool s_cacheShowOwned = true;
    static bool s_cacheShowUnowned = true;
    static bool s_cacheHasOwnerData = false;
    static uint64_t s_cacheOwnedGen = 0;

    std::string filterLower(g_SkinSearchFilter);
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

    bool hasOwnerData = Skinventory::OwnedSkins::HasData();
    uint64_t ownedGen = Skinventory::OwnedSkins::GetGeneration();

    bool needRebuild = (s_cacheType != g_SkinSelectedType) ||
                       (s_cacheSubtype != g_SkinSelectedSubtype) ||
                       (s_cacheWeight != g_SkinSelectedWeightClass) ||
                       (s_cacheFilter != filterLower) ||
                       (s_cacheShowOwned != g_SkinShowOwned) ||
                       (s_cacheShowUnowned != g_SkinShowUnowned) ||
                       (s_cacheHasOwnerData != hasOwnerData) ||
                       (s_cacheOwnedGen != ownedGen);

    if (needRebuild) {
        s_cacheType = g_SkinSelectedType;
        s_cacheSubtype = g_SkinSelectedSubtype;
        s_cacheWeight = g_SkinSelectedWeightClass;
        s_cacheFilter = filterLower;
        s_cacheShowOwned = g_SkinShowOwned;
        s_cacheShowUnowned = g_SkinShowUnowned;
        s_cacheHasOwnerData = hasOwnerData;
        s_cacheOwnedGen = ownedGen;

        s_displaySkins.clear();
        s_ownedCount = 0;
        s_totalCount = 0;

        std::vector<uint32_t> skins;
        if (g_SkinSelectedType == "Armor") {
            skins = Skinventory::SkinCache::GetSkinsByCategory("Armor", g_SkinSelectedSubtype, g_SkinSelectedWeightClass);
        } else if (g_SkinSelectedType == "Weapon") {
            skins = Skinventory::SkinCache::GetSkinsByCategory("Weapon", g_SkinSelectedSubtype, "");
        } else if (g_SkinSelectedType == "Back") {
            skins = Skinventory::SkinCache::GetSkinsByCategory("Back", "", "");
        }

        s_displaySkins.reserve(skins.size());
        for (uint32_t skinId : skins) {
            auto skinOpt = Skinventory::SkinCache::GetSkin(skinId);
            if (!skinOpt || skinOpt->name.empty()) continue;

            s_totalCount++;
            bool owned = Skinventory::OwnedSkins::IsOwned(skinId);
            if (owned) s_ownedCount++;

            if (hasOwnerData) {
                if (owned && !g_SkinShowOwned) continue;
                if (!owned && !g_SkinShowUnowned) continue;
            }

            if (!filterLower.empty()) {
                std::string nameLower = skinOpt->name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                if (nameLower.find(filterLower) == std::string::npos) continue;
            }

            s_displaySkins.push_back({skinId, owned});
        }
    }

    int ownedCount = s_ownedCount;
    int totalCount = s_totalCount;
    const auto& displaySkins = s_displaySkins;

    bool showStatus = hasOwnerData;
    float statusHeight = showStatus ? (ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y) : 0;
    float scrollHeight = ImGui::GetContentRegionAvail().y - statusHeight;

    ImGui::BeginChild("SkinListScroll", ImVec2(0, scrollHeight));
    if (g_SkinScrollToSkin) {
        for (int i = 0; i < (int)displaySkins.size(); i++) {
            if (displaySkins[i].id == g_SkinSelectedId) {
                float itemHeight = ImGui::GetTextLineHeightWithSpacing();
                float targetY = i * itemHeight;
                float windowHeight = ImGui::GetWindowHeight();
                ImGui::SetScrollY(targetY - windowHeight * 0.5f + itemHeight * 0.5f);
                g_SkinScrollToSkin = false;
                break;
            }
        }
    }
    if (g_SkinViewMode == 1) {
        // ===== Grid view =====
        const float tileSize = 36.0f;
        const float tileGap  = 3.0f;
        float availW = ImGui::GetContentRegionAvail().x;
        int cols = std::max(1, (int)((availW + tileGap) / (tileSize + tileGap)));
        int rows = ((int)displaySkins.size() + cols - 1) / cols;

        // Scroll-to-selected
        if (g_SkinScrollToSkin) {
            for (int i = 0; i < (int)displaySkins.size(); i++) {
                if (displaySkins[i].id == g_SkinSelectedId) {
                    float rowH = tileSize + tileGap;
                    int r = i / cols;
                    float targetY = r * rowH;
                    float winH = ImGui::GetWindowHeight();
                    ImGui::SetScrollY(targetY - winH * 0.5f + rowH * 0.5f);
                    g_SkinScrollToSkin = false;
                    break;
                }
            }
        }

        ImGuiListClipper clipper;
        clipper.Begin(rows, tileSize + tileGap);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        while (clipper.Step()) {
            for (int r = clipper.DisplayStart; r < clipper.DisplayEnd; r++) {
                for (int c = 0; c < cols; c++) {
                    int idx = r * cols + c;
                    if (idx >= (int)displaySkins.size()) break;
                    const auto& entry = displaySkins[idx];
                    auto skinOpt = Skinventory::SkinCache::GetSkin(entry.id);
                    if (!skinOpt) continue;
                    const auto& skin = *skinOpt;

                    if (c > 0) ImGui::SameLine(0, tileGap);

                    bool selected = (g_SkinSelectedId == entry.id);
                    bool dim = hasOwnerData && !entry.owned;

                    uint32_t displayId = entry.id + 5000000;
                    Texture_t* tex = AlterEgo::IconManager::GetIcon(displayId);
                    if (!tex || !tex->Resource) {
                        if (!skin.icon_url.empty())
                            AlterEgo::IconManager::RequestIcon(displayId, skin.icon_url, /*priority=*/true);
                    }

                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImVec2 tmin = pos;
                    ImVec2 tmax(pos.x + tileSize, pos.y + tileSize);

                    // Background fill (rarity-tinted placeholder)
                    ImVec4 rc = GetSkinRarityColor(skin.rarity);
                    ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(ImVec4(rc.x * 0.18f, rc.y * 0.18f, rc.z * 0.18f, 1.0f));
                    dl->AddRectFilled(tmin, tmax, bgCol, 2.0f);

                    if (tex && tex->Resource) {
                        ImU32 tint = dim
                            ? IM_COL32(160, 160, 160, 110)
                            : IM_COL32(255, 255, 255, 255);
                        dl->AddImage(tex->Resource, tmin, tmax,
                            ImVec2(0,0), ImVec2(1,1), tint);
                    }

                    // Rarity border
                    ImU32 borderCol = ImGui::ColorConvertFloat4ToU32(
                        dim ? ImVec4(rc.x * 0.55f, rc.y * 0.55f, rc.z * 0.55f, 0.85f) : rc);
                    dl->AddRect(tmin, tmax, borderCol, 2.0f, 0, 1.5f);

                    // Selection ring (gold)
                    if (selected) {
                        dl->AddRect(ImVec2(tmin.x - 1, tmin.y - 1), ImVec2(tmax.x + 1, tmax.y + 1),
                            IM_COL32(232, 196, 122, 255), 2.5f, 0, 2.0f);
                    }

                    // Owned check (small green corner mark)
                    if (hasOwnerData && entry.owned) {
                        float cx = tmax.x - 6.0f;
                        float cy = tmin.y + 6.0f;
                        dl->AddCircleFilled(ImVec2(cx, cy), 4.0f, IM_COL32(0, 0, 0, 200));
                        dl->AddCircle(ImVec2(cx, cy), 4.0f, IM_COL32(111, 204, 122, 255), 12, 1.2f);
                        dl->AddLine(ImVec2(cx - 2, cy + 0), ImVec2(cx - 0.5f, cy + 1.5f),
                            IM_COL32(160, 230, 170, 255), 1.5f);
                        dl->AddLine(ImVec2(cx - 0.5f, cy + 1.5f), ImVec2(cx + 2.5f, cy - 2),
                            IM_COL32(160, 230, 170, 255), 1.5f);
                    }

                    ImGui::PushID((int)entry.id);
                    ImGui::InvisibleButton("##tile", ImVec2(tileSize, tileSize));
                    bool hovered = ImGui::IsItemHovered();
                    if (hovered) {
                        dl->AddRect(tmin, tmax, IM_COL32(232, 196, 122, 180), 2.0f, 0, 1.5f);
                        ImGui::BeginTooltip();
                        ImGui::TextColored(rc, "%s", skin.name.c_str());
                        if (hasOwnerData) {
                            ImGui::TextColored(entry.owned
                                ? ImVec4(0.35f, 0.82f, 0.35f, 1.0f)
                                : ImVec4(0.55f, 0.55f, 0.55f, 1.0f),
                                entry.owned ? "Owned" : "Not owned");
                        }
                        ImGui::EndTooltip();
                    }
                    if (ImGui::IsItemClicked()) {
                        g_SkinSelectedId = entry.id;
                        Skinventory::WikiImage::RequestImage(entry.id, skin.name, skin.weight_class);
                        Skinventory::Commerce::FetchPriceForSkin(entry.id);
                    }
                    ImGui::PopID();
                }
                ImGui::Dummy(ImVec2(0, 0)); // newline
            }
        }
    } else {
        // ===== Compact list view =====
        const float rowH   = 22.0f;
        const float iconSz = 16.0f;

        // Scroll-to-selected for list mode
        if (g_SkinScrollToSkin) {
            for (int i = 0; i < (int)displaySkins.size(); i++) {
                if (displaySkins[i].id == g_SkinSelectedId) {
                    float winH = ImGui::GetWindowHeight();
                    ImGui::SetScrollY(i * rowH - winH * 0.5f + rowH * 0.5f);
                    g_SkinScrollToSkin = false;
                    break;
                }
            }
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiListClipper clipper;
        clipper.Begin((int)displaySkins.size(), rowH);
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                const auto& entry = displaySkins[row];
                auto skinOpt = Skinventory::SkinCache::GetSkin(entry.id);
                if (!skinOpt) { ImGui::Dummy(ImVec2(0, rowH)); continue; }
                const auto& skin = *skinOpt;

                ImVec2 cmin = ImGui::GetCursorScreenPos();
                float availW = ImGui::GetContentRegionAvail().x;
                ImVec2 cmax(cmin.x + availW, cmin.y + rowH);

                bool selected = (g_SkinSelectedId == entry.id);
                bool dim = hasOwnerData && !entry.owned;
                ImVec4 rc = GetSkinRarityColor(skin.rarity);

                // Hit-test full row
                ImGui::PushID((int)entry.id);
                ImGui::InvisibleButton("##lrow", ImVec2(availW, rowH));
                bool hovered = ImGui::IsItemHovered();
                bool clicked = ImGui::IsItemClicked();
                ImGui::PopID();

                // Background
                ImU32 bgCol;
                if (selected)      bgCol = IM_COL32(80, 64, 28, 90);
                else if (hovered)  bgCol = IM_COL32(60, 56, 40, 60);
                else if (row & 1)  bgCol = IM_COL32(255, 255, 255, 10);
                else               bgCol = IM_COL32(0, 0, 0, 0);
                if ((bgCol & 0xFF000000) != 0)
                    dl->AddRectFilled(cmin, cmax, bgCol, 2.0f);

                // Selection / hover border
                if (selected)
                    dl->AddRect(cmin, cmax, IM_COL32(232, 196, 122, 220), 2.0f, 0, 1.0f);
                else if (hovered)
                    dl->AddRect(cmin, cmax, IM_COL32(197, 161, 85, 110), 2.0f, 0, 1.0f);

                // Rarity left accent
                ImU32 accent = ImGui::ColorConvertFloat4ToU32(
                    dim ? ImVec4(rc.x * 0.55f, rc.y * 0.55f, rc.z * 0.55f, 0.85f) : rc);
                dl->AddRectFilled(ImVec2(cmin.x, cmin.y + 2),
                                  ImVec2(cmin.x + 3, cmax.y - 2), accent);

                // Icon (lazy, priority-loaded)
                float iconX = cmin.x + 8.0f;
                float iconY = cmin.y + (rowH - iconSz) * 0.5f;
                uint32_t displayId = entry.id + 5000000;
                Texture_t* tex = AlterEgo::IconManager::GetIcon(displayId);
                if (!tex || !tex->Resource) {
                    if (!skin.icon_url.empty())
                        AlterEgo::IconManager::RequestIcon(displayId, skin.icon_url, /*priority=*/true);
                }
                ImVec2 imin(iconX, iconY);
                ImVec2 imax(iconX + iconSz, iconY + iconSz);
                ImU32 iconBg = ImGui::ColorConvertFloat4ToU32(
                    ImVec4(rc.x * 0.18f, rc.y * 0.18f, rc.z * 0.18f, 1.0f));
                dl->AddRectFilled(imin, imax, iconBg, 1.5f);
                if (tex && tex->Resource) {
                    ImU32 tint = dim ? IM_COL32(170, 170, 170, 130) : IM_COL32(255, 255, 255, 255);
                    dl->AddImage(tex->Resource, imin, imax, ImVec2(0,0), ImVec2(1,1), tint);
                }

                // Name (clipped between icon + status)
                float textX = imax.x + 6.0f;
                float textEndX = cmax.x - (hasOwnerData ? 22.0f : 6.0f);
                ImU32 nameCol = dim
                    ? ImGui::ColorConvertFloat4ToU32(ImVec4(rc.x * 0.65f, rc.y * 0.65f, rc.z * 0.65f, 0.85f))
                    : ImGui::ColorConvertFloat4ToU32(rc);
                ImVec2 nameSz = ImGui::CalcTextSize(skin.name.c_str());
                float nameY = cmin.y + (rowH - nameSz.y) * 0.5f;
                dl->PushClipRect(ImVec2(textX, cmin.y), ImVec2(textEndX, cmax.y), true);
                dl->AddText(ImVec2(textX, nameY), nameCol, skin.name.c_str());
                dl->PopClipRect();

                // Status dot
                if (hasOwnerData) {
                    float dotX = cmax.x - 12.0f;
                    float dotY = cmin.y + rowH * 0.5f;
                    if (entry.owned) {
                        dl->AddCircleFilled(ImVec2(dotX, dotY), 4.0f, IM_COL32(50, 110, 60, 220));
                        dl->AddCircle(ImVec2(dotX, dotY), 4.0f, IM_COL32(111, 204, 122, 255), 14, 1.2f);
                        dl->AddLine(ImVec2(dotX - 2, dotY + 0), ImVec2(dotX - 0.5f, dotY + 1.5f),
                            IM_COL32(180, 240, 190, 255), 1.5f);
                        dl->AddLine(ImVec2(dotX - 0.5f, dotY + 1.5f), ImVec2(dotX + 2.5f, dotY - 2),
                            IM_COL32(180, 240, 190, 255), 1.5f);
                    } else {
                        dl->AddCircle(ImVec2(dotX, dotY), 3.5f, IM_COL32(120, 110, 90, 200), 14, 1.2f);
                    }
                }

                // Hover tooltip (full name when clipped)
                if (hovered && nameSz.x > (textEndX - textX)) {
                    ImGui::BeginTooltip();
                    ImGui::TextColored(rc, "%s", skin.name.c_str());
                    ImGui::EndTooltip();
                }

                if (clicked) {
                    g_SkinSelectedId = entry.id;
                    Skinventory::WikiImage::RequestImage(entry.id, skin.name, skin.weight_class);
                    Skinventory::Commerce::FetchPriceForSkin(entry.id);
                }
            }
        }
    }
    ImGui::EndChild();

    if (showStatus && totalCount > 0) {
        ImGui::Separator();
        float fraction = (float)ownedCount / (float)totalCount;
        char overlay[64];
        snprintf(overlay, sizeof(overlay), "%d / %d owned", ownedCount, totalCount);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.25f, 0.65f, 0.25f, 1.0f));
        ImGui::ProgressBar(fraction, ImVec2(-1, 0), overlay);
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

static void RenderSkinDetailPanel() {
    ImGui::BeginChild("SkinDetailPanel", ImVec2(0, 0), true);

    if (g_SkinSelectedId == 0) {
        ImGui::TextWrapped("Select a skin to view details.");
        ImGui::EndChild();
        return;
    }

    auto skinOpt = Skinventory::SkinCache::GetSkin(g_SkinSelectedId);
    if (!skinOpt) {
        ImGui::Text("Skin not found.");
        ImGui::EndChild();
        return;
    }
    const auto& skin = *skinOpt;

    auto sectionHeader = [](const char* label) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.75f, 0.9f, 1.0f), "%s", label);
        ImGui::Separator();
    };

    // ===== Rarity-coloured banner card =====
    {
        ImVec4 rc = GetSkinRarityColor(skin.rarity);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float pad = 10.0f;
        ImVec2 cmin = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;

        // Pre-measure so we can fit the wrapped name
        ImGui::PushTextWrapPos(cmin.x + w - pad * 2);
        ImVec2 nameSz = ImGui::CalcTextSize(skin.name.c_str(), nullptr, false, w - pad * 2 - 8.0f);
        ImGui::PopTextWrapPos();
        float nameH = nameSz.y;
        float subH = ImGui::GetTextLineHeight();
        float h = pad + nameH + 4.0f + subH + pad;

        ImVec2 cmax(cmin.x + w, cmin.y + h);

        // Background gradient tinted by rarity
        ImU32 bgL = ImGui::ColorConvertFloat4ToU32(ImVec4(rc.x * 0.22f, rc.y * 0.22f, rc.z * 0.22f, 0.95f));
        ImU32 bgR = IM_COL32(20, 20, 24, 250);
        dl->AddRectFilledMultiColor(cmin, cmax, bgL, bgR, bgR, bgL);
        dl->AddRect(cmin, cmax, IM_COL32(0, 0, 0, 200), 4.0f, 0, 1.0f);

        // Left rarity bar
        dl->AddRectFilled(ImVec2(cmin.x, cmin.y + 3),
                          ImVec2(cmin.x + 3, cmax.y - 3),
                          ImGui::ColorConvertFloat4ToU32(rc));

        // Reserve the card area in ImGui layout and render the name + subtitle on top
        ImGui::Dummy(ImVec2(w, h));
        ImVec2 textStart(cmin.x + pad + 4.0f, cmin.y + pad);

        // Name (rarity-coloured, wrapped, larger via no font change but bold-ish via colour)
        ImGui::SetCursorScreenPos(textStart);
        ImGui::PushStyleColor(ImGuiCol_Text, rc);
        ImGui::PushTextWrapPos(textStart.x + w - pad * 2);
        ImGui::TextUnformatted(skin.name.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();

        // Subtitle: rarity · type · weight   [owned chip]
        ImGui::SetCursorScreenPos(ImVec2(textStart.x, textStart.y + nameH + 4.0f));

        std::string subtitle = skin.rarity;
        if (!skin.type.empty())        { subtitle += "  ·  "; subtitle += skin.type; }
        if (!skin.subtype.empty())     { subtitle += "  "; subtitle += skin.subtype; }
        if (!skin.weight_class.empty()){ subtitle += "  ·  "; subtitle += skin.weight_class; }
        ImGui::TextColored(ImVec4(rc.x * 0.85f, rc.y * 0.85f, rc.z * 0.85f, 0.95f),
            "%s", subtitle.c_str());

        // Owned/not-owned chip, right-aligned inside the banner
        if (Skinventory::OwnedSkins::HasData()) {
            bool owned = Skinventory::OwnedSkins::IsOwned(g_SkinSelectedId);
            const char* chipText = owned ? "OWNED" : "NOT OWNED";
            ImVec2 chipSz = ImGui::CalcTextSize(chipText);
            float chipPadX = 8.0f, chipPadY = 3.0f;
            ImVec2 chipMin(cmax.x - pad - chipSz.x - chipPadX * 2,
                           cmin.y + pad - 2.0f);
            ImVec2 chipMax(chipMin.x + chipSz.x + chipPadX * 2,
                           chipMin.y + chipSz.y + chipPadY * 2);
            ImU32 chipBg, chipBorder, chipFg;
            if (owned) {
                chipBg = IM_COL32(40, 78, 44, 220);
                chipBorder = IM_COL32(120, 200, 130, 230);
                chipFg = IM_COL32(180, 240, 190, 255);
            } else {
                chipBg = IM_COL32(60, 30, 30, 200);
                chipBorder = IM_COL32(180, 90, 90, 200);
                chipFg = IM_COL32(220, 150, 150, 255);
            }
            dl->AddRectFilled(chipMin, chipMax, chipBg, 3.0f);
            dl->AddRect(chipMin, chipMax, chipBorder, 3.0f, 0, 1.2f);
            dl->AddText(ImVec2(chipMin.x + chipPadX, chipMin.y + chipPadY), chipFg, chipText);
        }
    }
    ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.45f, 1.0f), "ID: %u", skin.id);

    // Pricing (unowned only)
    bool isOwned = Skinventory::OwnedSkins::HasData() &&
                   Skinventory::OwnedSkins::IsOwned(g_SkinSelectedId);
    if (!isOwned) {
        int vendorPrice = Skinventory::Commerce::GetVendorPrice(g_SkinSelectedId);
        const auto* price = Skinventory::Commerce::GetPrice(g_SkinSelectedId);
        bool hasPrice = (vendorPrice > 0) ||
                        (price && price->tradeable && (price->sell_price > 0 || price->buy_price > 0)) ||
                        (!price && vendorPrice == 0);

        if (hasPrice) {
            sectionHeader("Pricing");

            if (vendorPrice > 0) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Vendor:");
                ImGui::SameLine();
                RenderCoins(vendorPrice);
            }

            if (price) {
                if (price->tradeable) {
                    if (price->sell_price > 0) {
                        ImGui::Text("TP Buy:");
                        ImGui::SameLine();
                        RenderCoins(price->sell_price);
                    }
                    if (price->buy_price > 0) {
                        ImGui::Text("TP Sell:");
                        ImGui::SameLine();
                        RenderCoins(price->buy_price);
                    }
                }
            } else if (vendorPrice == 0) {
                if (Skinventory::Commerce::IsItemMapReady()) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Price: click to load");
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Item map loading...");
                }
            }
        }
    }

    // Acquisition
    {
        auto wikiData = Skinventory::WikiImage::GetWikiData(g_SkinSelectedId);
        bool hasAny = !wikiData.acquisition.empty() || !wikiData.collection.empty() ||
                      !wikiData.set_name.empty() || !wikiData.vendor_name.empty();
        if (hasAny) {
            sectionHeader("Acquisition");
            if (!wikiData.acquisition.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Source:");
                ImGui::SameLine();
                ImGui::TextWrapped("%s", wikiData.acquisition.c_str());
            }
            if (!wikiData.collection.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Collection:");
                ImGui::SameLine();
                ImGui::TextWrapped("%s", wikiData.collection.c_str());
            }
            if (!wikiData.set_name.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Set:");
                ImGui::SameLine();
                ImGui::TextWrapped("%s", wikiData.set_name.c_str());
            }
            if (!wikiData.vendor_name.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Vendor:");
                ImGui::SameLine();
                if (!wikiData.vendor_location.empty()) {
                    ImGui::TextWrapped("%s (%s)", wikiData.vendor_name.c_str(),
                                       wikiData.vendor_location.c_str());
                } else {
                    ImGui::TextWrapped("%s", wikiData.vendor_name.c_str());
                }
            }
            if (!wikiData.vendor_cost.empty()) {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Cost:");
                ImGui::SameLine();
                std::string costStr = wikiData.vendor_cost;
                std::string keysStr = wikiData.vendor_cost_keys;
                std::vector<std::string> iconKeys;
                {
                    std::istringstream kss(keysStr);
                    std::string kk;
                    while (std::getline(kss, kk, ',')) {
                        if (!kk.empty()) iconKeys.push_back(kk);
                    }
                }
                std::vector<std::string> costParts;
                {
                    size_t p = 0;
                    while (true) {
                        size_t sep = costStr.find(" + ", p);
                        if (sep == std::string::npos) {
                            costParts.push_back(costStr.substr(p));
                            break;
                        }
                        costParts.push_back(costStr.substr(p, sep - p));
                        p = sep + 3;
                    }
                }
                for (size_t ci = 0; ci < costParts.size(); ci++) {
                    if (ci > 0) {
                        ImGui::SameLine(0, 2);
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "+");
                        ImGui::SameLine(0, 2);
                    }
                    if (ci < iconKeys.size()) {
                        Texture_t* cIcon = Skinventory::WikiImage::GetCurrencyIcon(iconKeys[ci]);
                        if (cIcon && cIcon->Resource) {
                            float iconH = ImGui::GetTextLineHeight();
                            float iconW = iconH * ((float)cIcon->Width / (float)cIcon->Height);
                            ImGui::Image(cIcon->Resource, ImVec2(iconW, iconH));
                            ImGui::SameLine(0, 3);
                        }
                    }
                    ImGui::TextColored(ImVec4(0.9f, 0.7f, 1.0f, 1.0f), "%s", costParts[ci].c_str());
                }
            }
            if (!wikiData.vendor_waypoint.empty()) {
                static float s_copiedTimer = 0.0f;
                static uint32_t s_copiedSkinId = 0;

                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.6f, 1.0f), "Waypoint:");
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
                if (ImGui::SmallButton(wikiData.vendor_waypoint.c_str())) {
                    if (OpenClipboard(NULL)) {
                        EmptyClipboard();
                        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,
                            wikiData.vendor_waypoint.size() + 1);
                        if (hMem) {
                            char* pMem = (char*)GlobalLock(hMem);
                            memcpy(pMem, wikiData.vendor_waypoint.c_str(),
                                   wikiData.vendor_waypoint.size() + 1);
                            GlobalUnlock(hMem);
                            SetClipboardData(CF_TEXT, hMem);
                        }
                        CloseClipboard();
                    }
                    s_copiedTimer = 2.0f;
                    s_copiedSkinId = g_SkinSelectedId;
                }
                ImGui::PopStyleColor();

                if (s_copiedTimer > 0.0f && s_copiedSkinId == g_SkinSelectedId) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, s_copiedTimer / 2.0f),
                                       "Copied!");
                    s_copiedTimer -= ImGui::GetIO().DeltaTime;
                }
            }
        }
    }

    // Preview
    sectionHeader("Preview");

    Texture_t* wikiTex = Skinventory::WikiImage::GetImage(g_SkinSelectedId);
    if (wikiTex && wikiTex->Resource) {
        float panelWidth = ImGui::GetContentRegionAvail().x;
        float maxHeight = 400.0f;
        float imgW = (float)wikiTex->Width;
        float imgH = (float)wikiTex->Height;

        float scale = panelWidth / imgW;
        if (imgH * scale > maxHeight) {
            scale = maxHeight / imgH;
        }

        float displayW = imgW * scale;
        float displayH = imgH * scale;

        ImGui::Image(wikiTex->Resource, ImVec2(displayW, displayH));
    } else if (Skinventory::WikiImage::IsLoading(g_SkinSelectedId)) {
        RenderSpinner("Loading wiki image...", ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
    }

    // Action buttons
    ImGui::Separator();
    if (ImGui::Button("Open Wiki Page")) {
        std::string wikiUrl = "https://wiki.guildwars2.com/wiki/" + skin.name;
        std::replace(wikiUrl.begin(), wikiUrl.end(), ' ', '_');
        ShellExecuteA(NULL, "open", wikiUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    ImGui::EndChild();
}

static void RenderSkinShoppingList() {
    if (!Skinventory::OwnedSkins::HasData()) {
        ImGui::TextWrapped("Hoard & Seek data not yet loaded. Ensure Hoard & Seek is installed and has fetched account data.");
        return;
    }

    if (!Skinventory::Commerce::IsItemMapReady()) {
        std::string fetchMsg = Skinventory::Commerce::GetFetchStatus();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", fetchMsg.c_str());
        ImGui::TextWrapped("Building the skin-to-item map from the GW2 API. This is a one-time operation and will be cached for future sessions.");
        return;
    }

    static bool s_needsFetch = true;
    static int shopTypeFilter = 0;
    static int shopSourceFilter = 0;

    // ===== Compact toolbar: Refresh + Category chips + Source chips =====
    if (ImGui::Button("Refresh Prices") && !Skinventory::Commerce::IsFetching()) {
        g_SkinShopListDirty = true;
        s_needsFetch = true;
    }

    auto chipButton = [](const char* lbl, bool active) -> bool {
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.40f, 0.18f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.55f, 1.0f));
        }
        bool clicked = ImGui::SmallButton(lbl);
        if (active) ImGui::PopStyleColor(4);
        return clicked;
    };

    ImGui::SameLine(0, 16);
    ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "Category:");
    ImGui::SameLine();
    const char* catLabels[]    = { "All##cat",   "Armor##cat",  "Weapons##cat" };
    const char* catDisplay[]   = { "All",        "Armor",       "Weapons" };
    for (int i = 0; i < 3; i++) {
        if (chipButton(catLabels[i], shopTypeFilter == i)) {
            if (shopTypeFilter != i) { shopTypeFilter = i; g_SkinShopListDirty = true; }
        }
        if (i < 2) ImGui::SameLine(0, 4);
        (void)catDisplay;
    }

    ImGui::SameLine(0, 16);
    ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f), "Source:");
    ImGui::SameLine();
    const char* srcLabels[] = { "All##src", "TP##src", "Vendor##src" };
    for (int i = 0; i < 3; i++) {
        if (chipButton(srcLabels[i], shopSourceFilter == i)) {
            if (shopSourceFilter != i) { shopSourceFilter = i; g_SkinShopListDirty = true; }
        }
        if (i < 2) ImGui::SameLine(0, 4);
    }

    // Thin progress bar when fetching (replaces verbose status text)
    if (Skinventory::Commerce::IsFetching()) {
        std::string fetchMsg = Skinventory::Commerce::GetFetchStatus();
        int curN = 0, totN = 0;
        // Parse "...N/M" out of the status message
        for (size_t i = 0; i < fetchMsg.size(); i++) {
            if (isdigit((unsigned char)fetchMsg[i])) {
                int a = 0, b = 0; int consumed = 0;
                if (sscanf(fetchMsg.c_str() + i, "%d/%d%n", &a, &b, &consumed) == 2 && b > 0) {
                    curN = a; totN = b;
                    break;
                }
                while (i < fetchMsg.size() && isdigit((unsigned char)fetchMsg[i])) i++;
            }
        }
        float frac = (totN > 0) ? std::min(1.0f, (float)curN / (float)totN) : 0.0f;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.95f, 0.76f, 0.30f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.13f, 1.0f));
        ImGui::ProgressBar(frac, ImVec2(-1, 3.0f), "");
        ImGui::PopStyleColor(2);
        ImGui::TextColored(ImVec4(0.85f, 0.74f, 0.40f, 1.0f), "%s", fetchMsg.c_str());
    }

    ImGui::Separator();

    static bool s_wasFetching = false;
    bool isFetching = Skinventory::Commerce::IsFetching();
    if (s_wasFetching && !isFetching) {
        g_SkinShopListDirty = true;
        s_needsFetch = false;
    }
    s_wasFetching = isFetching;

    // Amortized shopping list rebuild: collect IDs once, then process a batch per frame
    static std::vector<uint32_t> s_shopQueue;
    static bool s_shopBuilding = false;
    static const int SHOP_BATCH_SIZE = 50;

    if (g_SkinShopListDirty && !isFetching) {
        g_SkinShopListDirty = false;
        g_SkinShopList.clear();
        s_shopQueue.clear();

        auto collectSkins = [&](const std::string& type, const std::string& subtype,
                                 const std::string& weight) {
            auto skins = Skinventory::SkinCache::GetSkinsByCategory(type, subtype, weight);
            for (uint32_t id : skins) {
                if (!Skinventory::OwnedSkins::IsOwned(id)) {
                    s_shopQueue.push_back(id);
                }
            }
        };

        if (shopTypeFilter == 0 || shopTypeFilter == 1) {
            for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
                for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(wc)) {
                    collectSkins("Armor", slot, wc);
                }
            }
        }
        if (shopTypeFilter == 0 || shopTypeFilter == 2) {
            for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
                collectSkins("Weapon", wt, "");
            }
        }

        if (s_needsFetch) {
            Skinventory::Commerce::FetchPricesForSkins(s_shopQueue);
        }

        s_shopBuilding = !s_shopQueue.empty();
    }

    // Process a batch of queued skins per frame
    if (s_shopBuilding && !s_shopQueue.empty()) {
        int count = std::min(SHOP_BATCH_SIZE, (int)s_shopQueue.size());
        for (int i = 0; i < count; i++) {
            uint32_t id = s_shopQueue.back();
            s_shopQueue.pop_back();

            int vendorPrice = Skinventory::Commerce::GetVendorPrice(id);
            const auto* p = Skinventory::Commerce::GetPrice(id);
            int tpPrice = (p && p->tradeable && p->sell_price > 0) ? p->sell_price : 0;

            bool useVendor = false;
            bool useTP = false;

            if (vendorPrice > 0 && tpPrice > 0) {
                if (vendorPrice <= tpPrice) useVendor = true;
                else useTP = true;
            } else if (vendorPrice > 0) {
                useVendor = true;
            } else if (tpPrice > 0) {
                useTP = true;
            }

            if (useVendor && shopSourceFilter == 1) { useVendor = false; useTP = (tpPrice > 0); }
            if (useTP && shopSourceFilter == 2) { useTP = false; useVendor = (vendorPrice > 0); }

            if (useVendor) {
                g_SkinShopList.push_back({id, vendorPrice, 1});
            } else if (useTP) {
                g_SkinShopList.push_back({id, tpPrice, 0});
            }
        }

        if (s_shopQueue.empty()) {
            s_shopBuilding = false;
            std::sort(g_SkinShopList.begin(), g_SkinShopList.end(),
                [](const auto& a, const auto& b) { return a.price < b.price; });
        }
    }

    int tpCount = 0, vendorCount = 0;
    for (const auto& e : g_SkinShopList) {
        if (e.source == 0) tpCount++;
        else vendorCount++;
    }

    ImGui::Text("Cheapest unowned skins: %d TP, %d Vendor (%zu total)",
                tpCount, vendorCount, g_SkinShopList.size());
    ImGui::Separator();

    float reserveHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f;
    ImVec2 tableSize(0.0f, -reserveHeight);
    if (ImGui::BeginTable("ShoppingList", 4,
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Sortable,
        tableSize)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80, 1);
        ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed, 55, 2);
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 120, 4);
        ImGui::TableHeadersRow();

        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
            if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];
                bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending);
                int col = spec.ColumnUserID;

                std::sort(g_SkinShopList.begin(), g_SkinShopList.end(),
                    [col, ascending](const auto& a, const auto& b) {
                        if (col == 4) {
                            return ascending ? a.price < b.price : a.price > b.price;
                        }
                        if (col == 2) {
                            return ascending ? a.source < b.source : a.source > b.source;
                        }
                        auto sa = Skinventory::SkinCache::GetSkin(a.skinId);
                        auto sb = Skinventory::SkinCache::GetSkin(b.skinId);
                        if (!sa || !sb) return false;

                        int cmp = 0;
                        if (col == 0) {
                            cmp = sa->name.compare(sb->name);
                        } else if (col == 1) {
                            cmp = sa->subtype.compare(sb->subtype);
                        }
                        return ascending ? cmp < 0 : cmp > 0;
                    });
                sortSpecs->SpecsDirty = false;
            }
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)g_SkinShopList.size());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                const auto& entry = g_SkinShopList[row];
                auto skinOpt = Skinventory::SkinCache::GetSkin(entry.skinId);
                if (!skinOpt) continue;
                const auto& skin = *skinOpt;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // Rarity-coloured left accent bar in the Name cell
                {
                    ImVec4 rc = GetSkinRarityColor(skin.rarity);
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    float h = ImGui::GetTextLineHeightWithSpacing();
                    dl->AddRectFilled(ImVec2(p.x, p.y + 2),
                                      ImVec2(p.x + 2, p.y + h - 2),
                                      ImGui::ColorConvertFloat4ToU32(rc));
                    ImGui::Dummy(ImVec2(4.0f, 0.0f));
                    ImGui::SameLine(0, 0);
                }

                std::string selectLabel = skin.name + "##shop" + std::to_string(entry.skinId);
                ImGui::PushStyleColor(ImGuiCol_Text, GetSkinRarityColor(skin.rarity));
                if (ImGui::Selectable(selectLabel.c_str(), g_SkinSelectedId == entry.skinId,
                    ImGuiSelectableFlags_SpanAllColumns)) {
                    g_SkinSelectedId = entry.skinId;
                    Skinventory::WikiImage::RequestImage(entry.skinId, skin.name, skin.weight_class);
                    Skinventory::Commerce::FetchPriceForSkin(entry.skinId);
                    g_SkinSelectedType = skin.type;
                    if (skin.type == "Armor") {
                        g_SkinSelectedWeightClass = skin.weight_class;
                    }
                    g_SkinSelectedSubtype = skin.subtype;
                    g_SkinSwitchToBrowser = true;
                    g_SkinScrollToSkin = true;
                }
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();
                ImGui::Text("%s", skin.subtype.c_str());

                ImGui::TableNextColumn();
                if (entry.source == 1) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Vendor");
                } else {
                    ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.3f, 1.0f), "TP");
                }

                ImGui::TableNextColumn();
                RenderCoins(entry.price);
            }
        }

        ImGui::EndTable();
    }

    if (!g_SkinShopList.empty()) {
        int tpTotal = 0, vendorTotal = 0;
        for (const auto& e : g_SkinShopList) {
            if (e.source == 0) tpTotal += e.price;
            else vendorTotal += e.price;
        }
        ImGui::Separator();
        ImGui::Text("TP total:");
        ImGui::SameLine();
        RenderCoins(tpTotal);
        if (vendorTotal > 0) {
            ImGui::SameLine();
            ImGui::Text("  Vendor total:");
            ImGui::SameLine();
            RenderCoins(vendorTotal);
        }
    }
}

// Helper: draw a gradient-backed section header with colored accent underline
// Render a scrollable chip strip (used for equipment & build tab selectors).
// Returns true if the user clicked a different chip (selectedIdx is updated in place).
static bool RenderChipStrip(const std::vector<std::string>& labels,
                            const std::vector<bool>& activeMarkers,
                            int& selectedIdx,
                            float& scrollOffset,
                            const char* idScope) {
    int n = (int)labels.size();
    if (n <= 0) return false;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float chipMinW = 90.0f;
    const float chipH = 30.0f;
    const float gap = 5.0f;
    const float arrowW = 22.0f;
    const float arrowGap = 4.0f;

    float availW = ImGui::GetContentRegionAvail().x - 6.0f;
    float idealW = chipMinW * n + gap * (n - 1);
    bool needsScroll = idealW > availW;

    float chipW = chipMinW;
    float stripW = availW;
    if (!needsScroll) {
        chipW = (availW - gap * (n - 1)) / (float)n;
    } else {
        stripW = availW - (arrowW + arrowGap) * 2.0f;
    }

    ImVec2 stripOrigin = ImGui::GetCursorScreenPos();
    if (needsScroll) stripOrigin.x += arrowW + arrowGap;

    bool changed = false;

    if (needsScroll) {
        float maxScroll = (chipW * n + gap * (n - 1)) - stripW;
        if (maxScroll < 0) maxScroll = 0;
        if (scrollOffset < 0) scrollOffset = 0;
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        bool canLeft = scrollOffset > 0.5f;
        bool canRight = scrollOffset < maxScroll - 0.5f;

        auto DrawArrow = [&](float ax, bool right, bool enabled) -> bool {
            ImVec2 amin(ax, stripOrigin.y);
            ImVec2 amax(ax + arrowW, stripOrigin.y + chipH);
            ImGui::PushID(right ? "##chipR" : "##chipL");
            ImGui::SetCursorScreenPos(amin);
            bool clicked = ImGui::InvisibleButton("##chipArrow",
                ImVec2(arrowW, chipH)) && enabled;
            bool hov = ImGui::IsItemHovered() && enabled;
            ImGui::PopID();

            ImU32 bg = !enabled ? IM_COL32(20, 18, 14, 200)
                      : hov ? IM_COL32(50, 42, 24, 255)
                            : IM_COL32(34, 28, 18, 255);
            dl->AddRectFilled(amin, amax, bg, 4.0f);
            ImU32 border = !enabled ? IM_COL32(60, 50, 30, 80)
                          : IM_COL32(140, 115, 60, 200);
            dl->AddRect(amin, amax, border, 4.0f, 0, 1.0f);

            ImU32 chev = !enabled ? IM_COL32(110, 95, 60, 110)
                        : IM_COL32(227, 196, 122, 255);
            float cx = amin.x + arrowW * 0.5f;
            float cy = amin.y + chipH * 0.5f;
            float s = 6.0f;
            if (right) {
                dl->AddTriangleFilled(
                    ImVec2(cx - s * 0.4f, cy - s),
                    ImVec2(cx - s * 0.4f, cy + s),
                    ImVec2(cx + s * 0.6f, cy), chev);
            } else {
                dl->AddTriangleFilled(
                    ImVec2(cx + s * 0.4f, cy - s),
                    ImVec2(cx + s * 0.4f, cy + s),
                    ImVec2(cx - s * 0.6f, cy), chev);
            }
            return clicked;
        };

        if (DrawArrow(stripOrigin.x - arrowW - arrowGap, false, canLeft)) {
            scrollOffset -= chipW + gap;
            if (scrollOffset < 0) scrollOffset = 0;
        }
        if (DrawArrow(stripOrigin.x + stripW + arrowGap, true, canRight)) {
            scrollOffset += chipW + gap;
            if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        }
    } else {
        scrollOffset = 0;
    }

    if (needsScroll) {
        dl->PushClipRect(stripOrigin,
            ImVec2(stripOrigin.x + stripW, stripOrigin.y + chipH), true);
    }

    ImVec2 origin = ImVec2(stripOrigin.x - scrollOffset, stripOrigin.y);

    ImGui::PushID(idScope);
    for (int i = 0; i < n; i++) {
        bool active = (selectedIdx == i);
        bool isActiveInGame = (i < (int)activeMarkers.size()) && activeMarkers[i];

        ImVec2 cMin(origin.x + (chipW + gap) * i, origin.y);
        ImVec2 cMax(cMin.x + chipW, cMin.y + chipH);

        ImGui::PushID(i);
        ImGui::SetCursorScreenPos(cMin);
        if (ImGui::InvisibleButton("##chip", ImVec2(chipW, chipH))) {
            if (selectedIdx != i) {
                selectedIdx = i;
                changed = true;
            }
        }
        bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();

        if (active) {
            ImU32 bgTop = IM_COL32(42, 36, 24, 255);
            ImU32 bgBot = IM_COL32(28, 24, 16, 255);
            dl->AddRectFilledMultiColor(cMin, cMax, bgTop, bgTop, bgBot, bgBot);
        } else {
            ImU32 bg = hovered ? IM_COL32(37, 35, 44, 255) : IM_COL32(29, 28, 36, 255);
            dl->AddRectFilled(cMin, cMax, bg, 4.0f);
        }
        ImU32 border = active ? IM_COL32(197, 161, 85, 255)
                      : (hovered ? IM_COL32(110, 95, 55, 180)
                                 : IM_COL32(70, 60, 40, 110));
        dl->AddRect(cMin, cMax, border, 4.0f, 0, active ? 1.5f : 1.0f);
        if (active) {
            dl->AddRectFilled(
                ImVec2(cMin.x, cMax.y - 3),
                ImVec2(cMax.x, cMax.y + 4),
                IM_COL32(197, 161, 85, 40), 3.0f);
        }

        const std::string& label = labels[i];
        ImVec2 ts = ImGui::CalcTextSize(label.c_str());
        // Truncate if needed
        std::string drawLabel = label;
        if (ts.x > chipW - 12.0f) {
            // Find truncated length
            const char* ell = "\xe2\x80\xa6";
            float ellW = ImGui::CalcTextSize(ell).x;
            int lo = 0, hi = (int)label.size();
            while (lo < hi) {
                int mid = (lo + hi + 1) / 2;
                if (ImGui::CalcTextSize(label.substr(0, mid).c_str()).x + ellW <= chipW - 12.0f) lo = mid;
                else hi = mid - 1;
            }
            drawLabel = label.substr(0, lo) + ell;
            ts = ImGui::CalcTextSize(drawLabel.c_str());
        }
        ImU32 textCol = active ? IM_COL32(240, 226, 190, 255)
                               : IM_COL32(170, 162, 140, 255);
        ImVec2 textPos(cMin.x + chipW * 0.5f - ts.x * 0.5f,
                       cMin.y + chipH * 0.5f - ts.y * 0.5f);
        dl->AddText(textPos, textCol, drawLabel.c_str());

        if (isActiveInGame) {
            dl->AddCircleFilled(
                ImVec2(cMax.x - 6, cMin.y + 6),
                2.5f, IM_COL32(227, 196, 122, 255));
        }
    }
    ImGui::PopID();

    if (needsScroll) dl->PopClipRect();

    ImGui::SetCursorScreenPos(ImVec2(stripOrigin.x - (needsScroll ? (arrowW + arrowGap) : 0),
                                     stripOrigin.y + chipH + 4.0f));
    return changed;
}

void RenderSectionHeader(const char* label, ImVec4 color, const char* suffix) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float h = ImGui::GetTextLineHeightWithSpacing() + 6.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Gradient background
    ImU32 left = ImGui::ColorConvertFloat4ToU32(
        ImVec4(color.x * 0.22f, color.y * 0.22f, color.z * 0.22f, 0.55f));
    ImU32 right = IM_COL32(0, 0, 0, 0);
    dl->AddRectFilledMultiColor(
        ImVec2(pos.x + 4.0f, pos.y),
        ImVec2(pos.x + w, pos.y + h),
        left, right, right, left);

    // Left gold accent bar (vertical)
    ImU32 barTop = ImGui::ColorConvertFloat4ToU32(ImVec4(
        std::min(1.0f, color.x * 1.30f),
        std::min(1.0f, color.y * 1.30f),
        std::min(1.0f, color.z * 1.30f), 1.0f));
    ImU32 barBot = ImGui::ColorConvertFloat4ToU32(ImVec4(
        color.x * 0.70f, color.y * 0.70f, color.z * 0.70f, 1.0f));
    dl->AddRectFilledMultiColor(
        ImVec2(pos.x, pos.y + 2.0f),
        ImVec2(pos.x + 3.0f, pos.y + h - 2.0f),
        barTop, barTop, barBot, barBot);

    // Accent underline
    dl->AddLine(ImVec2(pos.x + 4.0f, pos.y + h), ImVec2(pos.x + w * 0.55f, pos.y + h),
        ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, 0.35f)), 1.0f);

    ImGui::SetCursorScreenPos(ImVec2(pos.x + 10.0f, pos.y + 3.0f));
    ImGui::TextColored(color, "%s", label);
    if (suffix) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.50f, 0.47f, 0.40f, 1.0f), "%s", suffix);
    }
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h + 3.0f));
}

// Gold-trimmed button — primary-action vocabulary matching RenderSectionHeader.
// size = ImVec2(0,0) auto-sizes to label; pass explicit size for fixed-width rows.
// Returns true on click.
bool RenderGoldButton(const char* label, ImVec2 size) {
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.14f, 0.06f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.22f, 0.10f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.36f, 0.28f, 0.12f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Border,        ImVec4(0.70f, 0.58f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.95f, 0.85f, 0.55f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
    return clicked;
}

// Small chip-style button — secondary action vocabulary.
// active: highlighted (gold tint) when true.
// accentColor: optional accent (e.g. profession colour or status colour). Pass ImVec4(0,0,0,0) for default gold.
// Returns true on click.
bool RenderChipButton(const char* label, bool active, ImVec4 accentColor) {
    bool hasAccent = (accentColor.w > 0.01f);
    ImVec4 base    = hasAccent ? ImVec4(accentColor.x * 0.30f, accentColor.y * 0.30f, accentColor.z * 0.30f, 0.55f)
                               : ImVec4(0.15f, 0.13f, 0.09f, 0.55f);
    ImVec4 hover   = hasAccent ? ImVec4(accentColor.x * 0.45f, accentColor.y * 0.45f, accentColor.z * 0.45f, 0.75f)
                               : ImVec4(0.25f, 0.20f, 0.10f, 0.75f);
    ImVec4 actVar  = hasAccent ? ImVec4(accentColor.x * 0.55f, accentColor.y * 0.55f, accentColor.z * 0.55f, 0.90f)
                               : ImVec4(0.50f, 0.40f, 0.18f, 0.80f);
    ImVec4 text    = active    ? ImVec4(0.95f, 0.85f, 0.55f, 1.00f)
                               : ImVec4(0.78f, 0.76f, 0.66f, 1.00f);

    ImGui::PushStyleColor(ImGuiCol_Button,        active ? actVar : base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  actVar);
    ImGui::PushStyleColor(ImGuiCol_Text,          text);
    bool clicked = ImGui::SmallButton(label);
    ImGui::PopStyleColor(4);
    return clicked;
}

// Gold-bordered combo box. Wraps ImGui::BeginCombo with themed styling for both
// the closed combo button and the open popup.
//
// `id`:      ImGui id (e.g. "##AccountSelect").
// `current`: the label shown when collapsed.
// `width`:   <= 0 means caller already used SetNextItemWidth; > 0 sets the width here.
//
// Usage:
//   if (RenderThemedCombo("##acct", currentLabel.c_str(), 180.0f)) {
//       if (ImGui::Selectable("...", isSel)) {...}
//       ImGui::EndCombo();
//   }
//
// Returns true when the combo is open (same contract as ImGui::BeginCombo).
// Caller is responsible for ImGui::EndCombo() when the function returns true.
bool RenderThemedCombo(const char* id, const char* current, float width) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.10f, 0.09f, 0.06f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.16f, 0.13f, 0.08f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.20f, 0.16f, 0.09f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Border,         ImVec4(0.70f, 0.58f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.95f, 0.85f, 0.55f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg,        ImVec4(0.08f, 0.07f, 0.05f, 0.98f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
    if (width > 0.0f) ImGui::SetNextItemWidth(width);
    bool open = ImGui::BeginCombo(id, current);
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(6);
    return open;
}

// Centered themed empty-state / first-run card.
// iconTex:        optional Nexus texture. Pass nullptr to skip the icon row.
// headline:       bold title text (gold).
// body:           multi-line body text (wraps inside the card).
// primaryLabel /  primary action button (gold). Pass nullptr to omit.
//   primaryCB
// secondaryLabel/ secondary action button (chip). Pass nullptr to omit.
//   secondaryCB
void RenderEmptyCard(Texture_t* iconTex,
                     const char* headline,
                     const char* body,
                     const char* primaryLabel, std::function<void()> primaryCB,
                     const char* secondaryLabel, std::function<void()> secondaryCB) {
    const float cardW = 420.0f;
    const float cardH = 260.0f;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float offsetX = (avail.x - cardW) * 0.5f;
    float offsetY = (avail.y - cardH) * 0.5f;
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;

    ImGui::Dummy(ImVec2(0, offsetY));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    ImVec2 cardOrigin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(cardOrigin, ImVec2(cardOrigin.x + cardW, cardOrigin.y + cardH),
                      IM_COL32(20, 17, 11, 235), 6.0f);
    dl->AddRect(cardOrigin, ImVec2(cardOrigin.x + cardW, cardOrigin.y + cardH),
                IM_COL32(178, 148, 51, 255), 6.0f, 0, 1.5f);

    ImGui::BeginChild("##empty_card_inner", ImVec2(cardW, cardH), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Dummy(ImVec2(0, 12));

    if (iconTex && iconTex->Resource) {
        float iconSize = 64.0f;
        ImGui::SetCursorPosX((cardW - iconSize) * 0.5f);
        ImGui::Image(iconTex->Resource, ImVec2(iconSize, iconSize));
        ImGui::Dummy(ImVec2(0, 8));
    }

    if (headline && *headline) {
        ImVec2 sz = ImGui::CalcTextSize(headline);
        ImGui::SetCursorPosX((cardW - sz.x) * 0.5f);
        ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.55f, 1.0f), "%s", headline);
        ImGui::Dummy(ImVec2(0, 6));
    }

    if (body && *body) {
        float bodyW = cardW - 40.0f;
        ImGui::SetCursorPosX((cardW - bodyW) * 0.5f);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + bodyW);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78f, 0.76f, 0.66f, 1.0f));
        ImGui::TextWrapped("%s", body);
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();
        ImGui::Dummy(ImVec2(0, 14));
    }

    bool hasPrimary   = primaryLabel   && *primaryLabel;
    bool hasSecondary = secondaryLabel && *secondaryLabel;
    if (hasPrimary || hasSecondary) {
        float pW = hasPrimary   ? ImGui::CalcTextSize(primaryLabel).x   + 24.0f : 0;
        float sW = hasSecondary ? ImGui::CalcTextSize(secondaryLabel).x + 16.0f : 0;
        float gap = (hasPrimary && hasSecondary) ? 10.0f : 0.0f;
        float totalW = pW + sW + gap;
        ImGui::SetCursorPosX((cardW - totalW) * 0.5f);

        if (hasPrimary) {
            if (RenderGoldButton(primaryLabel, ImVec2(pW, 0))) {
                if (primaryCB) primaryCB();
            }
            if (hasSecondary) ImGui::SameLine(0, gap);
        }
        if (hasSecondary) {
            if (RenderChipButton(secondaryLabel, false)) {
                if (secondaryCB) secondaryCB();
            }
        }
    }

    ImGui::EndChild();
}

// Small inline loading spinner: rotating partial arc + label.
// Used in place of plain "Loading..." text to signal active work.
void RenderSpinner(const char* label, ImVec4 color) {
    float lineH = ImGui::GetTextLineHeight();
    float radius = lineH * 0.40f;
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 center(cursor.x + radius + 2.0f, cursor.y + lineH * 0.5f);
    float t = (float)ImGui::GetTime();
    float start = t * 6.0f;
    float end = start + IM_PI * 1.5f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PathClear();
    dl->PathArcTo(center, radius, start, end, 32);
    dl->PathStroke(ImGui::ColorConvertFloat4ToU32(color), false, 2.0f);
    ImGui::Dummy(ImVec2(radius * 2.0f + 6.0f, lineH));
    if (label && *label) {
        ImGui::SameLine(0, 6);
        ImGui::TextColored(color, "%s", label);
    }
}

// =========================================================================
// Achievement Tracker — UI
// =========================================================================

// Find achievement ID by name match (reverse lookup on g_AchNameIndex)
// Tries: exact match, then parentName + ": " + name
static uint32_t FindAchIdByName(const std::string& name, const std::string& parentName = "") {
    // Exact match
    for (const auto& [id, n] : g_AchNameIndex) {
        if (n == name) return id;
    }
    // Try parentName + ": " + name (common meta-achievement pattern)
    if (!parentName.empty()) {
        std::string qualified = parentName + ": " + name;
        for (const auto& [id, n] : g_AchNameIndex) {
            if (n == qualified) return id;
        }
    }
    return 0;
}

// Navigate to a specific achievement: find its category, select it, fetch defs, expand it
static void NavigateToAchievement(uint32_t achId) {
    // Find which category contains this achievement
    auto catIt = g_AchIdToCategory.find(achId);
    if (catIt != g_AchIdToCategory.end()) {
        uint32_t catId = catIt->second;
        if (g_AchSelectedCatId != catId) {
            g_AchSelectedCatId = catId;
            // Find which group owns this category
            for (const auto& group : g_AchGroups) {
                for (uint32_t gCatId : group.categories) {
                    if (gCatId == catId) {
                        g_AchSelectedGroupId = group.id;
                        break;
                    }
                }
            }
            FetchAchCategoryDefs(catId);
        }
    }
    // Mark this achievement as expanded and set scroll target
    g_AchExpandedInList.insert(achId);
    g_AchNavigateToId = achId;
}

static bool IsAchPinned(uint32_t id) {
    return std::find(g_AchPinned.begin(), g_AchPinned.end(), id) != g_AchPinned.end();
}

static void ToggleAchPin(uint32_t id) {
    auto it = std::find(g_AchPinned.begin(), g_AchPinned.end(), id);
    if (it != g_AchPinned.end()) {
        g_AchPinned.erase(it);
    } else {
        if (g_AchPinned.size() < 20) g_AchPinned.push_back(id);
    }
    SaveAchTrackerState();
}

static void RenderAchProgressBar(float fraction, const ImVec4& color, float height = 4.0f) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
        ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.15f, 0.8f)));
    if (fraction > 0.0f) {
        dl->AddRectFilled(pos, ImVec2(pos.x + width * fraction, pos.y + height),
            ImGui::GetColorU32(color));
    }
    ImGui::Dummy(ImVec2(width, height));
}

static void RenderAchEntry(uint32_t achId, bool showPinButton) {
    auto defIt = g_AchDefs.find(achId);
    auto progIt = g_AchProgress.find(achId);

    // If no definition yet, show ID and loading indicator
    if (defIt == g_AchDefs.end()) {
        // Check name index for at least a name
        auto nameIt = g_AchNameIndex.find(achId);
        if (nameIt != g_AchNameIndex.end()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s (loading...)", nameIt->second.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Achievement %u (loading...)", achId);
        }
        return;
    }

    const AchDef& def = defIt->second;
    bool done = progIt != g_AchProgress.end() && progIt->second.done;
    int current = (progIt != g_AchProgress.end()) ? progIt->second.current : 0;
    int max = def.max_count > 0 ? def.max_count : ((progIt != g_AchProgress.end()) ? progIt->second.max : 0);

    ImGui::PushID((int)achId);

    bool hasBits = !def.bits.empty() && !done;
    bool expanded = g_AchExpandedInList.count(achId) > 0;
    bool pinned = IsAchPinned(achId);
    bool inProgress = !done && current > 0 && max > 0;

    // ===== Compact row =====
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float rowH = 30.0f;
    float availRowW = ImGui::GetContentRegionAvail().x;
    ImVec2 cmin = ImGui::GetCursorScreenPos();
    ImVec2 cmax(cmin.x + availRowW, cmin.y + rowH);

    // State colour
    ImU32 stateCol;
    if (done)            stateCol = IM_COL32(111, 204, 122, 255);
    else if (pinned)     stateCol = IM_COL32(232, 196, 122, 255);
    else if (inProgress) stateCol = IM_COL32(244, 196, 122, 230);
    else                 stateCol = IM_COL32(120, 110, 90, 200);

    // Background
    ImU32 bg = pinned ? IM_COL32(80, 64, 28, 65)
             : done   ? IM_COL32(28, 38, 28, 60)
                      : IM_COL32(255, 255, 255, 14);
    dl->AddRectFilled(cmin, cmax, bg, 3.0f);
    dl->AddRect(cmin, cmax, IM_COL32(0, 0, 0, 130), 3.0f, 0, 1.0f);

    // Left state bar
    dl->AddRectFilled(ImVec2(cmin.x, cmin.y + 3),
                      ImVec2(cmin.x + 3, cmax.y - 3), stateCol);

    // Main row hit area (excluding the pin button + WP button on the right)
    float pinReserve = showPinButton ? 22.0f : 0.0f;
    float wpReserve = 0.0f;
    bool hasAchWp = false;
    std::string achWaypoint;
    {
        auto wpIt = g_AchWaypoints.find(achId);
        if (wpIt != g_AchWaypoints.end()) {
            auto wpBitIt = wpIt->second.find(-1);
            if (wpBitIt != wpIt->second.end()) {
                hasAchWp = true;
                achWaypoint = wpBitIt->second;
                wpReserve = 26.0f;
            }
        }
    }
    float rightReserve = pinReserve + wpReserve + 8.0f;

    ImGui::InvisibleButton("##achrow", ImVec2(availRowW - rightReserve, rowH));
    bool rowHovered = ImGui::IsItemHovered();
    bool rowClicked = ImGui::IsItemClicked();

    if (rowHovered)
        dl->AddRect(cmin, cmax, IM_COL32(197, 161, 85, 130), 3.0f, 0, 1.0f);

    // Right-click context (open wiki)
    if (ImGui::BeginPopupContextItem("##ach_ctx")) {
        if (ImGui::MenuItem("Open in Wiki")) {
            std::string wikiUrl = "https://wiki.guildwars2.com/wiki/" + def.name;
            std::replace(wikiUrl.begin(), wikiUrl.end(), ' ', '_');
            ShellExecuteA(NULL, "open", wikiUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
        ImGui::EndPopup();
    }

    if (rowClicked && hasBits) {
        if (expanded) g_AchExpandedInList.erase(achId);
        else g_AchExpandedInList.insert(achId);
    }

    // Title + AP pill + progress badge — drawn on top of the row
    float padX = 10.0f;
    float midY = cmin.y + rowH * 0.5f;

    // Right-side elements (progress badge + AP pill, right-to-left)
    float rightX = cmax.x - rightReserve;

    if (def.total_ap > 0) {
        char ap[16]; snprintf(ap, sizeof(ap), "%d AP", def.total_ap);
        ImVec2 sz = ImGui::CalcTextSize(ap);
        float pillW = sz.x + 10.0f;
        float pillH = 16.0f;
        ImVec2 pMin(rightX - pillW, midY - pillH * 0.5f);
        ImVec2 pMax(rightX, pMin.y + pillH);
        ImU32 pBg, pBorder, pText;
        if (done) {
            pBg = IM_COL32(40, 60, 40, 180); pBorder = IM_COL32(120, 200, 130, 200);
            pText = IM_COL32(160, 220, 170, 255);
        } else {
            pBg = IM_COL32(54, 38, 14, 200); pBorder = IM_COL32(200, 160, 90, 200);
            pText = IM_COL32(232, 196, 122, 255);
        }
        dl->AddRectFilled(pMin, pMax, pBg, 3.0f);
        dl->AddRect(pMin, pMax, pBorder, 3.0f, 0, 1.0f);
        dl->AddText(ImVec2(pMin.x + 5.0f, pMin.y + (pillH - sz.y) * 0.5f), pText, ap);
        rightX = pMin.x - 6.0f;
    }

    if (max > 1) {
        char progBuf[24];
        snprintf(progBuf, sizeof(progBuf), "%d / %d", done ? max : current, max);
        ImVec2 sz = ImGui::CalcTextSize(progBuf);
        float bdW = sz.x + 10.0f;
        float bdH = 16.0f;
        ImVec2 bMin(rightX - bdW, midY - bdH * 0.5f);
        ImVec2 bMax(rightX, bMin.y + bdH);
        ImU32 bBg, bBorder, bText;
        if (done) {
            bBg = IM_COL32(40, 60, 40, 180); bBorder = IM_COL32(120, 200, 130, 200);
            bText = IM_COL32(160, 220, 170, 255);
        } else {
            bBg = IM_COL32(0, 0, 0, 110); bBorder = IM_COL32(160, 145, 110, 170);
            bText = IM_COL32(220, 210, 180, 240);
        }
        dl->AddRectFilled(bMin, bMax, bBg, 3.0f);
        dl->AddRect(bMin, bMax, bBorder, 3.0f, 0, 1.0f);
        dl->AddText(ImVec2(bMin.x + 5.0f, bMin.y + (bdH - sz.y) * 0.5f), bText, progBuf);
        rightX = bMin.x - 6.0f;
    }

    // Title (clipped between left bar and remaining right space)
    float titleX = cmin.x + padX + 4.0f;
    float titleEndX = rightX - 8.0f;
    ImU32 titleCol;
    if (done)        titleCol = IM_COL32(150, 200, 150, 255);
    else if (pinned) titleCol = IM_COL32(245, 220, 150, 255);
    else             titleCol = IM_COL32(230, 224, 208, 255);
    ImVec2 tSz = ImGui::CalcTextSize(def.name.c_str());
    dl->PushClipRect(ImVec2(titleX, cmin.y), ImVec2(titleEndX, cmax.y), true);
    dl->AddText(ImVec2(titleX, midY - tSz.y * 0.5f - 1.0f), titleCol, def.name.c_str());
    dl->PopClipRect();

    // 2px progress strip at the bottom of the row
    if (max > 0) {
        float frac = done ? 1.0f : (float)current / (float)max;
        float stripY = cmax.y - 4.0f;
        ImVec2 sMin(cmin.x + 4.0f, stripY);
        ImVec2 sMax(cmax.x - 4.0f, stripY + 2.0f);
        dl->AddRectFilled(sMin, sMax, IM_COL32(255, 255, 255, 18), 1.0f);
        float fillEnd = sMin.x + (sMax.x - sMin.x) * frac;
        if (fillEnd > sMin.x)
            dl->AddRectFilled(sMin, ImVec2(fillEnd, sMax.y), stateCol, 1.0f);
    }

    // Hover tooltip with requirement text (so the row stays one line)
    if (rowHovered && !def.requirement.empty()) {
        std::string cleanReq = StripGW2Markup(def.requirement);
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(def.name.c_str());
        ImGui::Separator();
        ImGui::PushTextWrapPos(360.0f);
        ImGui::TextWrapped("%s", cleanReq.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    // Position the right-side buttons (WP, Pin)
    float btnX = cmax.x - 4.0f;
    if (showPinButton) {
        float pinW = 22.0f, pinH = 20.0f;
        btnX -= pinW;
        ImGui::SetCursorScreenPos(ImVec2(btnX, midY - pinH * 0.5f));
        ImU32 pinBg = pinned ? IM_COL32(80, 64, 28, 200) : IM_COL32(0, 0, 0, 130);
        ImU32 pinBorder = pinned ? IM_COL32(232, 196, 122, 230) : IM_COL32(140, 130, 110, 170);
        ImU32 pinFg = pinned ? IM_COL32(245, 220, 150, 255) : IM_COL32(170, 158, 130, 230);
        ImVec2 bMin = ImGui::GetCursorScreenPos();
        ImVec2 bMax(bMin.x + pinW, bMin.y + pinH);
        dl->AddRectFilled(bMin, bMax, pinBg, 3.0f);
        dl->AddRect(bMin, bMax, pinBorder, 3.0f, 0, 1.0f);
        const char* star = pinned ? "*" : "*";
        ImVec2 sz = ImGui::CalcTextSize(star);
        dl->AddText(ImVec2(bMin.x + (pinW - sz.x) * 0.5f, bMin.y + (pinH - sz.y) * 0.5f),
            pinFg, star);
        if (ImGui::InvisibleButton("##pinbtn", ImVec2(pinW, pinH))) {
            ToggleAchPin(achId);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(pinned ? "Unpin from tracker" : "Pin to tracker");
        }
        btnX -= 4.0f;
    }

    if (hasAchWp) {
        float wpW = 22.0f, wpH = 20.0f;
        btnX -= wpW;
        ImGui::SetCursorScreenPos(ImVec2(btnX, midY - wpH * 0.5f));
        ImVec2 bMin = ImGui::GetCursorScreenPos();
        ImVec2 bMax(bMin.x + wpW, bMin.y + wpH);
        dl->AddRectFilled(bMin, bMax, IM_COL32(0, 0, 0, 130), 3.0f);
        dl->AddRect(bMin, bMax, IM_COL32(140, 130, 110, 170), 3.0f, 0, 1.0f);
        const char* wp = "WP";
        ImVec2 sz = ImGui::CalcTextSize(wp);
        dl->AddText(ImVec2(bMin.x + (wpW - sz.x) * 0.5f, bMin.y + (wpH - sz.y) * 0.5f),
            IM_COL32(180, 168, 130, 230), wp);
        if (ImGui::InvisibleButton("##wpbtn", ImVec2(wpW, wpH))) {
            ImGui::SetClipboardText(achWaypoint.c_str());
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Copy waypoint: %s", achWaypoint.c_str());
        }
    }

    // Restore cursor to below the row
    ImGui::SetCursorScreenPos(ImVec2(cmin.x, cmax.y + 2.0f));

    // Expanded bits (sub-objectives) — shown when clicked
    if (hasBits && expanded) {
        auto wpIt = g_AchWaypoints.find(achId);
        ImGui::Indent(16.0f);
        for (size_t i = 0; i < def.bits.size(); i++) {
            bool bitDone = (progIt != g_AchProgress.end()) &&
                progIt->second.completed_bits.count((uint32_t)i) > 0;
            const AchBitDef& bit = def.bits[i];

            // Resolve label and detect type: Skin, sub-Achievement, or plain Text
            std::string label;
            uint32_t skinId = 0;
            uint32_t subAchId = 0;
            std::optional<Skinventory::SkinInfo> skinInfo;
            if (bit.type == "Skin" && bit.item_id > 0) {
                skinId = bit.item_id;
                skinInfo = Skinventory::SkinCache::GetSkin(skinId);
                if (skinInfo.has_value()) {
                    label = skinInfo->name;
                } else if (!bit.text.empty()) {
                    label = bit.text;
                } else {
                    label = "Skin #" + std::to_string(skinId);
                }
            } else if (bit.type == "Text" && !bit.text.empty()) {
                label = bit.text;
                // Check if this text matches an achievement name (sub-achievement in meta)
                subAchId = FindAchIdByName(bit.text, def.name);
            } else {
                label = bit.text.empty() ? ("Step " + std::to_string(i + 1)) : bit.text;
            }

            // Check for per-bit waypoint
            std::string bitWaypoint;
            if (wpIt != g_AchWaypoints.end()) {
                auto wpBitIt = wpIt->second.find((int)i);
                if (wpBitIt != wpIt->second.end()) {
                    bitWaypoint = wpBitIt->second;
                }
            }

            ImGui::PushID((int)i);
            if (bitDone) {
                if (subAchId > 0) {
                    // Completed sub-achievement — still clickable to navigate
                    std::string selLabel = "[x] " + label + "###subach";
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.82f, 0.35f, 0.7f));
                    if (ImGui::Selectable(selLabel.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                        NavigateToAchievement(subAchId);
                    }
                    ImGui::PopStyleColor();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Click to navigate to achievement");
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 0.7f), "[x] %s", label.c_str());
                }
            } else if (skinId > 0 && skinInfo.has_value()) {
                // Clickable skin entry → navigate to Skinventory
                std::string selLabel = "[ ] " + label + "###skinbit";
                if (ImGui::Selectable(selLabel.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                    g_SkinSelectedId = skinId;
                    g_SkinSelectedType = skinInfo->type;
                    if (skinInfo->type == "Armor") {
                        g_SkinSelectedWeightClass = skinInfo->weight_class;
                    }
                    g_SkinSelectedSubtype = skinInfo->subtype;
                    g_SkinSwitchToBrowser = true;
                    g_SkinScrollToSkin = true;
                    g_SwitchToSkinventory = true;
                    Skinventory::WikiImage::RequestImage(skinId, skinInfo->name, skinInfo->weight_class);
                    Skinventory::Commerce::FetchPriceForSkin(skinId);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Click to view in Skinventory");
                }
            } else if (subAchId > 0) {
                // Clickable sub-achievement → navigate to it
                std::string selLabel = "[ ] " + label + "###subach";
                if (ImGui::Selectable(selLabel.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                    NavigateToAchievement(subAchId);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Click to navigate to achievement");
                }
            } else {
                ImGui::Text("[ ] %s", label.c_str());
            }
            // Waypoint copy button for this bit
            if (!bitWaypoint.empty()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("WP")) {
                    ImGui::SetClipboardText(bitWaypoint.c_str());
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Copy waypoint: %s", bitWaypoint.c_str());
                }
            }
            ImGui::PopID();
        }
        ImGui::Unindent(16.0f);
    }

    // Scroll-to target: if this achievement is the navigation target, scroll here
    if (g_AchNavigateToId == achId) {
        ImGui::SetScrollHereY(0.3f);
        g_AchNavigateToId = 0;
    }

    ImGui::PopID();
    ImGui::Spacing();
}

static void RenderAchievements() {
    // On first open, trigger fetch if not cached
    if (!g_AchGroupsFetched && !g_AchGroupsFetching) {
        FetchAchGroups();
    }
    if (!g_AchNameIndexReady && !g_AchNameIndexFetching) {
        FetchAchNameIndex();
    }
    if (!g_AchWaypointsReady && !g_AchWaypointsFetching) {
        FetchAchWaypoints();
    }
    if (g_AchGroupsFetched && !g_AchActiveEventFetched && !g_AchActiveEventFetching) {
        FetchActiveSpecialEvent();
    }

    // Hydrate cache for the current account on first opportunity (post-H&S handshake).
    {
        const std::string& curAcct = AlterEgo::GW2API::GetCurrentAccountName();
        if (!curAcct.empty() && g_AchCachedAccount != curAcct) {
            LoadAchProgress(curAcct);
        }
    }

    // Periodic flush: write dirty cache every 30s instead of on every change.
    if (g_AchProgressDirty && !g_AchCachedAccount.empty()) {
        auto now = std::chrono::steady_clock::now();
        if (now - g_AchLastSave > std::chrono::seconds(30)) {
            SaveAchProgress(g_AchCachedAccount);
        }
    }

    // Deferred requery on account change
    if (g_AchNeedRequery && !g_AchProgressFetching) {
        g_AchNeedRequery = false;
        // Persist the outgoing account's progress before swapping.
        if (g_AchProgressDirty && !g_AchCachedAccount.empty()) {
            SaveAchProgress(g_AchCachedAccount);
        }
        std::vector<uint32_t> pinnedCopy;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            g_AchProgress.clear();        // discard stale progress from previous account
            g_AchOptimisticTime.clear();
            g_AchProgressGen++;
            pinnedCopy = g_AchPinned;
        }
        // Hydrate the new account from disk (instant rail counts; background refresh
        // below catches any updates since the cache was written).
        const std::string& newAcct = AlterEgo::GW2API::GetCurrentAccountName();
        if (!newAcct.empty()) LoadAchProgress(newAcct);
        if (!pinnedCopy.empty()) {
            SendAchProgressQuery(pinnedCopy);
        }
        if (g_AchSelectedCatId > 0) {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            auto it = g_AchCategories.find(g_AchSelectedCatId);
            if (it != g_AchCategories.end()) {
                SendAchProgressQuery(it->second.achievements);
            }
        }
    }

    // Consume deferred progress query from bg thread (FetchAchCategoryDefs)
    {
        uint32_t pendingCat = g_AchCatProgressPending.exchange(0);
        if (pendingCat > 0 && !g_AchProgressFetching) {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            auto it = g_AchCategories.find(pendingCat);
            if (it != g_AchCategories.end()) {
                SendAchProgressQuery(it->second.achievements);
            }
        }
    }

    // Consume deferred PENDING retry for achievement progress
    if (g_AchRetryPending.exchange(false) && !g_AchProgressFetching) {
        std::vector<uint32_t> retryIds;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            retryIds = g_AchPinned;
            if (g_AchSelectedCatId > 0) {
                auto it = g_AchCategories.find(g_AchSelectedCatId);
                if (it != g_AchCategories.end()) {
                    retryIds.insert(retryIds.end(), it->second.achievements.begin(),
                                    it->second.achievements.end());
                }
            }
        }
        if (!retryIds.empty()) {
            SendAchProgressQuery(retryIds);
        }
    }

    // Toolbar: search + popout toggle + refresh
    float searchWidth = ImGui::GetContentRegionAvail().x - 180.0f;
    if (searchWidth < 100.0f) searchWidth = 100.0f;
    ImGui::SetNextItemWidth(searchWidth);
    ImGui::InputTextWithHint("##AchSearch", "Search achievements...", g_AchSearchBuf, sizeof(g_AchSearchBuf));
    ImGui::SameLine();
    if (ImGui::Button(g_AchPopoutVisible ? "Hide Popout Tracker" : "Show Popout Tracker")) {
        g_AchPopoutVisible = !g_AchPopoutVisible;
        SaveAchTrackerState();
    }
    ImGui::SameLine();
    if (g_AchProgressFetching) {
        RenderSpinner("", ImVec4(0.95f, 0.85f, 0.55f, 1.0f));
    } else if (ImGui::Button("Refresh")) {
        g_AchGroupsFetched = false;
        g_AchActiveEventFetched = false;
        FetchAchGroups();
        if (g_AchSelectedCatId > 0) FetchAchCategoryDefs(g_AchSelectedCatId);
        // Also refresh pinned achievement progress
        std::vector<uint32_t> pinnedCopy;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            pinnedCopy = g_AchPinned;
        }
        if (!pinnedCopy.empty()) {
            SendAchProgressQuery(pinnedCopy);
        }
    }

    // Status message
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        if (!g_AchStatusMsg.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", g_AchStatusMsg.c_str());
        }
    }

    if (!g_AchGroupsFetched) return;

    // Throttled prefetch: rail counts need progress for every achievement, but
    // firing all ~40 H&S batches at once will trip GW2 API rate limits and put
    // H&S into a bad state. Build a queue once per account, then drain it one
    // batch per ~750ms while idle.
    static std::vector<uint32_t> s_progressQueue;
    static std::string s_prefetchedAccount;
    {
        std::string currentAccount = GetEffectiveAccountName();
        if (s_prefetchedAccount != currentAccount) {
            s_prefetchedAccount = currentAccount;
            s_progressQueue.clear();
            // Build the queue so pinned achievements end up at the back —
            // the drain consumes from the back, so pinned go first.
            {
                std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                std::unordered_set<uint32_t> pinnedSet(g_AchPinned.begin(), g_AchPinned.end());
                for (const auto& [catId, cat] : g_AchCategories) {
                    if (g_AchHiddenCatIds.count(catId)) continue;
                    for (uint32_t aid : cat.achievements) {
                        if (pinnedSet.count(aid)) continue;
                        s_progressQueue.push_back(aid);
                    }
                }
                // Append pinned last → drained first.
                for (uint32_t aid : g_AchPinned) s_progressQueue.push_back(aid);
            }
        }
    }
    if (!s_progressQueue.empty() && !g_AchProgressFetching) {
        static auto s_lastBatchTime = std::chrono::steady_clock::time_point{};
        auto now = std::chrono::steady_clock::now();
        if (s_lastBatchTime == std::chrono::steady_clock::time_point{} ||
            (now - s_lastBatchTime) >= std::chrono::milliseconds(750)) {
            s_lastBatchTime = now;
            constexpr size_t BATCH = 200;
            size_t take = std::min(BATCH, s_progressQueue.size());
            std::vector<uint32_t> batch(s_progressQueue.end() - take, s_progressQueue.end());
            s_progressQueue.erase(s_progressQueue.end() - take, s_progressQueue.end());
            SendAchProgressQuery(batch);
        }
    }

    // Search mode — cached results to avoid iterating 8000+ names every frame
    bool searchActive = g_AchSearchBuf[0] != '\0';
    if (searchActive) {
        std::string query(g_AchSearchBuf);
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);

        if (!g_AchNameIndexReady) {
            RenderSpinner("Loading search index...");
            return;
        }

        // Cache search results — only re-search when query changes
        static std::string s_achSearchCache;
        static std::vector<uint32_t> s_achSearchResults;
        static bool s_achSearchTruncated = false;

        if (s_achSearchCache != query) {
            s_achSearchCache = query;
            s_achSearchResults.clear();
            s_achSearchTruncated = false;

            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            for (const auto& [id, name] : g_AchNameIndex) {
                std::string lowerName = name;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                if (lowerName.find(query) != std::string::npos) {
                    // Skip achievements from hidden groups (e.g. Character Adventure Guide)
                    auto catIt = g_AchIdToCategory.find(id);
                    if (catIt != g_AchIdToCategory.end() && g_AchHiddenCatIds.count(catIt->second)) continue;
                    s_achSearchResults.push_back(id);
                    if (s_achSearchResults.size() >= 50) {
                        s_achSearchTruncated = true;
                        break;
                    }
                }
            }
        }

        ImGui::BeginChild("AchSearchResults", ImVec2(0, 0), true);
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        for (uint32_t id : s_achSearchResults) {
            // Show category hint
            auto catIt = g_AchIdToCategory.find(id);
            if (catIt != g_AchIdToCategory.end()) {
                auto catDefIt = g_AchCategories.find(catIt->second);
                if (catDefIt != g_AchCategories.end()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s]", catDefIt->second.name.c_str());
                    ImGui::SameLine();
                }
            }

            // If we have the full def, render it
            if (g_AchDefs.count(id)) {
                RenderAchEntry(id, true);
            } else {
                auto nameIt = g_AchNameIndex.find(id);
                ImGui::Text("%s", nameIt != g_AchNameIndex.end() ? nameIt->second.c_str() : "?");
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30.0f);
                ImGui::PushID((int)id);
                bool pinned = IsAchPinned(id);
                if (pinned) {
                    if (ImGui::SmallButton("Unpin")) ToggleAchPin(id);
                } else {
                    if (ImGui::SmallButton("Pin")) ToggleAchPin(id);
                }
                ImGui::PopID();
                ImGui::Spacing();
            }
        }
        if (s_achSearchTruncated) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "... and more (refine your search)");
        }
        if (s_achSearchResults.empty()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No results");
        }
        ImGui::EndChild();
        return;
    }

    // Main layout: left tree + splitter + right list
    float availWidth = ImGui::GetContentRegionAvail().x;
    float availHeight = ImGui::GetContentRegionAvail().y;
    g_AchTreeWidth = (g_AchTreeWidth < 120.0f) ? 120.0f : (g_AchTreeWidth > availWidth - 200.0f) ? availWidth - 200.0f : g_AchTreeWidth;

    // Left panel: Group/Category tree
    ImGui::BeginChild("AchTree", ImVec2(g_AchTreeWidth, availHeight), true);

    if (g_AchRestoreScroll) {
        ImGui::SetScrollY(g_AchTreeScrollY);
    }
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);

        // Compute (done, total, hasAny) for a category — cached per progress generation
        struct CatStats { int done; int total; bool hasAny; };
        static std::unordered_map<uint32_t, CatStats> s_catStats;
        static uint64_t s_catStatsGen = (uint64_t)-1;
        if (s_catStatsGen != g_AchProgressGen) {
            s_catStats.clear();
            s_catStatsGen = g_AchProgressGen;
        }
        auto getCatStats = [&](uint32_t catId) -> CatStats {
            auto it = s_catStats.find(catId);
            if (it != s_catStats.end()) return it->second;
            CatStats st{0, 0, false};
            auto catIt = g_AchCategories.find(catId);
            if (catIt != g_AchCategories.end()) {
                st.total = (int)catIt->second.achievements.size();
                for (uint32_t aid : catIt->second.achievements) {
                    auto pit = g_AchProgress.find(aid);
                    if (pit != g_AchProgress.end()) {
                        st.hasAny = true;
                        if (pit->second.done) st.done++;
                    }
                }
            }
            s_catStats[catId] = st;
            return st;
        };

        // Render a category row with gold-bar selection, owned/total badge, dim if no progress
        auto renderCatRow = [&](const AchCategoryDef* cat, const std::string& groupId,
                                bool isSelected) -> bool {
            CatStats st = getCatStats(cat->id);
            bool complete = (st.done == st.total && st.total > 0 && st.hasAny);

            ImGui::PushID((int)cat->id);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 startPos = ImGui::GetCursorScreenPos();
            float availW = ImGui::GetContentRegionAvail().x;
            float rowH = ImGui::GetTextLineHeight() + 4.0f;

            // Active background + gold left bar
            if (isSelected) {
                ImVec2 bgMin = startPos;
                ImVec2 bgMax(startPos.x + availW, startPos.y + rowH);
                dl->AddRectFilled(bgMin, bgMax, IM_COL32(80, 64, 28, 70), 2.0f);
                dl->AddRectFilled(ImVec2(bgMin.x, bgMin.y + 2),
                                  ImVec2(bgMin.x + 2, bgMax.y - 2),
                                  IM_COL32(232, 196, 122, 230));
            }

            ImVec4 textCol = isSelected ? ImVec4(0.95f, 0.85f, 0.55f, 1.0f)
                                        : ImVec4(0.85f, 0.83f, 0.75f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, textCol);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0.04f));
            bool clicked = ImGui::Selectable(cat->name.c_str(), isSelected);
            ImGui::PopStyleColor(3);

            // Count badge — only shown once progress data exists for at least one
            // achievement in the category (otherwise lazy-load means "0" would lie)
            if (st.total > 0 && st.hasAny) {
                char buf[24];
                snprintf(buf, sizeof(buf), "%d/%d", st.done, st.total);
                ImVec2 cSz = ImGui::CalcTextSize(buf);
                float cx = startPos.x + availW - cSz.x - 6.0f;
                float cy = startPos.y + (rowH - cSz.y) * 0.5f - 1.0f;
                ImU32 countCol = complete ? IM_COL32(120, 200, 130, 230)
                                : isSelected ? IM_COL32(232, 196, 122, 230)
                                             : IM_COL32(140, 132, 110, 220);
                dl->AddText(ImVec2(cx, cy), countCol, buf);
            }

            if (clicked && g_AchSelectedCatId != cat->id) {
                g_AchSelectedCatId = cat->id;
                g_AchSelectedGroupId = groupId;
                FetchAchCategoryDefs(cat->id);
            }
            ImGui::PopID();
            return true;
        };

        // Active event — shown at top of tree when a bonus event or festival is active
        if (!g_AchActiveMainCatIds.empty()) {
            static const std::string kEventGroupId = "__active_event__";
            bool forceOpenEvent = g_AchRestoreScroll && (g_AchSelectedGroupId == kEventGroupId);
            if (forceOpenEvent) ImGui::SetNextItemOpen(true);
            else if (!g_AchRestoreScroll) ImGui::SetNextItemOpen(true, ImGuiCond_Once);

            std::string eventLabel = g_AchActiveEventName.empty() ? "Current Event" : g_AchActiveEventName;
            if (ImGui::TreeNode(eventLabel.c_str())) {
                for (uint32_t catId : g_AchActiveMainCatIds) {
                    auto it = g_AchCategories.find(catId);
                    if (it == g_AchCategories.end() || it->second.name.empty()) continue;
                    renderCatRow(&it->second, kEventGroupId, g_AchSelectedCatId == catId);
                }
                ImGui::TreePop();
            }
        }

        // Build set of active daily cat IDs for quick lookup
        std::unordered_set<uint32_t> activeDailySet(g_AchActiveDailyCatIds.begin(), g_AchActiveDailyCatIds.end());

        for (const auto& group : g_AchGroups) {
            if (group.name.empty()) continue;
            if (group.name == "Character Adventure Guide") continue;

            bool isDaily = (group.name == "Daily");

            // Sort categories by order, skipping hidden categories — and aggregate group stats
            std::vector<const AchCategoryDef*> sortedCats;
            int grpDone = 0, grpTotal = 0;
            bool grpHasAny = false;
            for (uint32_t catId : group.categories) {
                if (g_AchHiddenCatIds.count(catId)) continue;
                auto it = g_AchCategories.find(catId);
                if (it != g_AchCategories.end() && !it->second.name.empty()) {
                    sortedCats.push_back(&it->second);
                    CatStats st = getCatStats(catId);
                    grpDone += st.done; grpTotal += st.total;
                    if (st.hasAny) grpHasAny = true;
                }
            }
            bool grpComplete = (grpDone == grpTotal && grpTotal > 0 && grpHasAny);
            (void)grpComplete;

            std::sort(sortedCats.begin(), sortedCats.end(),
                [](const AchCategoryDef* a, const AchCategoryDef* b) { return a->order < b->order; });

            // Compose group header with count badge
            std::string groupHeader = group.name;
            if (grpTotal > 0) {
                char buf[32];
                if (grpHasAny) snprintf(buf, sizeof(buf), "  (%d/%d)", grpDone, grpTotal);
                else           snprintf(buf, sizeof(buf), "  (%d)", grpTotal);
                groupHeader += buf;
            }

            // Auto-open the saved group. Use the group's stable id as the
            // TreeNode identity so changing counts in the label don't make
            // ImGui treat it as a new node and forget the open state.
            bool forceOpen = g_AchRestoreScroll && (group.id == g_AchSelectedGroupId);
            if (forceOpen) ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx(group.id.c_str(),
                ImGuiTreeNodeFlags_SpanAvailWidth, "%s", groupHeader.c_str())) {
                // For Daily group: show active event dailies first
                if (isDaily && !g_AchActiveDailyCatIds.empty()) {
                    for (uint32_t catId : g_AchActiveDailyCatIds) {
                        auto it = g_AchCategories.find(catId);
                        if (it == g_AchCategories.end() || it->second.name.empty()) continue;
                        renderCatRow(&it->second, group.id, g_AchSelectedCatId == catId);
                    }
                }

                for (const auto* cat : sortedCats) {
                    if (isDaily && activeDailySet.count(cat->id)) continue;
                    renderCatRow(cat, group.id, g_AchSelectedCatId == cat->id);
                }
                ImGui::TreePop();
            }
        }
    }
    g_AchTreeScrollY = ImGui::GetScrollY();
    ImGui::EndChild();

    // Draggable vertical splitter
    ImGui::SameLine();
    {
        float splitterW = 6.0f;
        float h = availHeight;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##achtree_splitter", ImVec2(splitterW, h));
        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();
        if (hovered || active)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (active) {
            float delta = ImGui::GetIO().MouseDelta.x;
            if (delta != 0.0f) {
                g_AchTreeWidth += delta;
                g_AchTreeWidth = (g_AchTreeWidth < 120.0f) ? 120.0f : (g_AchTreeWidth > availWidth - 200.0f) ? availWidth - 200.0f : g_AchTreeWidth;
            }
        }
        if (ImGui::IsItemDeactivated()) {
            SaveAchTrackerState();
        }
        ImU32 lineCol = (hovered || active)
            ? IM_COL32(180, 160, 80, 180)
            : IM_COL32(80, 75, 60, 100);
        float cx = pos.x + splitterW * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(cx, pos.y), ImVec2(cx, pos.y + h), lineCol, 2.0f);
    }

    ImGui::SameLine();

    // Right panel: split into category list (top) + pinned section (bottom)
    ImGui::BeginChild("##AchRight", ImVec2(0, availHeight), false);
    {
        float rightWidth = ImGui::GetContentRegionAvail().x;
        bool hasPinned = !g_AchPinned.empty();
        float splitterH = hasPinned ? 6.0f : 0.0f;

        // Clamp pinned height
        if (hasPinned) {
            float minPinned = 60.0f;
            float maxPinned = availHeight - 100.0f;
            g_AchPinnedHeight = (g_AchPinnedHeight < minPinned) ? minPinned : (g_AchPinnedHeight > maxPinned) ? maxPinned : g_AchPinnedHeight;
        }

        float innerAvail = ImGui::GetContentRegionAvail().y;
        float spacing = ImGui::GetStyle().ItemSpacing.y;
        float topHeight = hasPinned ? (innerAvail - g_AchPinnedHeight - splitterH - 2 * spacing) : innerAvail;

        // Top: Fixed category heading + scrollable list
        float catHeaderH = 0.0f;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            if (g_AchSelectedCatId > 0) {
                auto catIt = g_AchCategories.find(g_AchSelectedCatId);
                if (catIt != g_AchCategories.end()) {
                    float startY = ImGui::GetCursorPosY();
                    ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "%s", catIt->second.name.c_str());

                    // Hide Complete chip — focuses the list on remaining work
                    {
                        bool active = g_AchHideCompletedCats;
                        if (active) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.40f, 0.18f, 0.80f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.58f, 0.46f, 0.22f, 0.90f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.55f, 1.0f));
                        }
                        ImGui::SameLine(ImGui::GetContentRegionMax().x - 110.0f);
                        if (ImGui::SmallButton("Hide Complete##list")) {
                            g_AchHideCompletedCats = !g_AchHideCompletedCats;
                            SaveAchTrackerState();
                        }
                        if (active) ImGui::PopStyleColor(4);
                    }

                    if (!catIt->second.description.empty()) {
                        std::string cleanDesc = StripGW2Markup(catIt->second.description);
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", cleanDesc.c_str());
                    }
                    ImGui::Separator();
                    catHeaderH = ImGui::GetCursorPosY() - startY;
                }
            }
        }

        ImGui::BeginChild("AchList", ImVec2(rightWidth, topHeight - catHeaderH), true);
        if (g_AchRestoreScroll) {
            ImGui::SetScrollY(g_AchListScrollY);
            g_AchRestoreScroll = false;
        }
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);

            if (g_AchSelectedCatId > 0) {
                auto catIt = g_AchCategories.find(g_AchSelectedCatId);
                if (catIt != g_AchCategories.end()) {
                    if (g_AchCatFetching) {
                        RenderSpinner("Loading...", ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
                    }

                    for (uint32_t achId : catIt->second.achievements) {
                        if (g_AchHideCompletedCats) {
                            auto pit = g_AchProgress.find(achId);
                            if (pit != g_AchProgress.end() && pit->second.done) continue;
                        }
                        RenderAchEntry(achId, true);
                    }
                }
            } else {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Select a category from the left panel");
            }
        }
        g_AchListScrollY = ImGui::GetScrollY();
        ImGui::EndChild();

        // Horizontal splitter + Pinned section
        if (hasPinned) {
            // Draggable horizontal splitter
            {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton("##achpinned_splitter", ImVec2(rightWidth, splitterH));
                bool hovered = ImGui::IsItemHovered();
                bool active = ImGui::IsItemActive();
                if (hovered || active)
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                if (active) {
                    float delta = ImGui::GetIO().MouseDelta.y;
                    if (delta != 0.0f) {
                        g_AchPinnedHeight -= delta;
                        float minPinned = 60.0f;
                        float maxPinned = availHeight - 100.0f;
                        g_AchPinnedHeight = (g_AchPinnedHeight < minPinned) ? minPinned : (g_AchPinnedHeight > maxPinned) ? maxPinned : g_AchPinnedHeight;
                    }
                }
                if (ImGui::IsItemDeactivated()) {
                    SaveAchTrackerState();
                }
                ImU32 lineCol = (hovered || active)
                    ? IM_COL32(180, 160, 80, 180)
                    : IM_COL32(80, 75, 60, 100);
                float cy = pos.y + splitterH * 0.5f;
                ImGui::GetWindowDrawList()->AddLine(
                    ImVec2(pos.x, cy), ImVec2(pos.x + rightWidth, cy), lineCol, 2.0f);
            }

            // Bottom: Fixed pinned heading + scrollable pinned list
            float pinnedHeaderH = 0.0f;
            {
                float startY = ImGui::GetCursorPosY();
                ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "Pinned (%d)", (int)g_AchPinned.size());
                ImGui::Separator();
                pinnedHeaderH = ImGui::GetCursorPosY() - startY;
            }
            ImGui::BeginChild("AchPinned", ImVec2(rightWidth, g_AchPinnedHeight - pinnedHeaderH), true);
            {
                std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                int pinnedCount = (int)g_AchPinned.size();
                int moveFrom = -1, moveTo = -1;
                for (int idx = 0; idx < pinnedCount; idx++) {
                    uint32_t id = g_AchPinned[idx];
                    ImGui::PushID(idx + 90000);

                    // Up/down arrow buttons (drawn triangles)
                    float btnSize = ImGui::GetFrameHeight();
                    ImVec4 dimCol(0.5f, 0.5f, 0.5f, 0.3f);
                    ImVec4 normalCol(0.7f, 0.65f, 0.4f, 1.0f);

                    // Up button
                    bool canUp = idx > 0;
                    if (!canUp) ImGui::PushStyleColor(ImGuiCol_Text, dimCol);
                    if (ImGui::InvisibleButton("##up", ImVec2(btnSize, btnSize)) && canUp) {
                        moveFrom = idx; moveTo = idx - 1;
                    }
                    {
                        ImVec2 rmin = ImGui::GetItemRectMin();
                        ImVec2 rmax = ImGui::GetItemRectMax();
                        float cx = (rmin.x + rmax.x) * 0.5f;
                        float cy = (rmin.y + rmax.y) * 0.5f;
                        float hs = btnSize * 0.25f;
                        ImU32 col = canUp ? (ImGui::IsItemHovered() ? IM_COL32(255, 230, 120, 255) : IM_COL32(180, 165, 100, 255))
                                          : IM_COL32(128, 128, 128, 80);
                        ImGui::GetWindowDrawList()->AddTriangleFilled(
                            ImVec2(cx, cy - hs), ImVec2(cx - hs, cy + hs), ImVec2(cx + hs, cy + hs), col);
                    }
                    if (!canUp) ImGui::PopStyleColor();

                    ImGui::SameLine(0, 1);

                    // Down button
                    bool canDown = idx < pinnedCount - 1;
                    if (!canDown) ImGui::PushStyleColor(ImGuiCol_Text, dimCol);
                    if (ImGui::InvisibleButton("##dn", ImVec2(btnSize, btnSize)) && canDown) {
                        moveFrom = idx; moveTo = idx + 1;
                    }
                    {
                        ImVec2 rmin = ImGui::GetItemRectMin();
                        ImVec2 rmax = ImGui::GetItemRectMax();
                        float cx = (rmin.x + rmax.x) * 0.5f;
                        float cy = (rmin.y + rmax.y) * 0.5f;
                        float hs = btnSize * 0.25f;
                        ImU32 col = canDown ? (ImGui::IsItemHovered() ? IM_COL32(255, 230, 120, 255) : IM_COL32(180, 165, 100, 255))
                                            : IM_COL32(128, 128, 128, 80);
                        ImGui::GetWindowDrawList()->AddTriangleFilled(
                            ImVec2(cx, cy + hs), ImVec2(cx - hs, cy - hs), ImVec2(cx + hs, cy - hs), col);
                    }
                    if (!canDown) ImGui::PopStyleColor();

                    ImGui::SameLine(0, 4);

                    RenderAchEntry(id, true);

                    ImGui::PopID();
                }
                // Apply move after loop
                if (moveFrom >= 0 && moveTo >= 0 && moveFrom != moveTo) {
                    std::swap(g_AchPinned[moveFrom], g_AchPinned[moveTo]);
                    SaveAchTrackerState();
                }
            }
            ImGui::EndChild();
        }
    }
    ImGui::EndChild(); // ##AchRight
}

// =========================================================================
// Achievement Tracker — Popout Window
// =========================================================================

// Cached display data for the popout — rebuilt only when inputs change
struct PopoutBitDisplay {
    std::string label;
    uint32_t skinId = 0;
    uint32_t subAchId = 0;
    std::string skinType;
    std::string skinWeightClass;
    std::string skinSubtype;
    std::string skinName;
    std::string waypoint;
    bool done = false;
};

struct PopoutAchDisplay {
    uint32_t achId = 0;
    std::string name;
    std::string loadingName;
    bool hasDef = false;
    bool done = false;
    bool hasBits = false;
    int current = 0;
    int max = 0;
    std::vector<PopoutBitDisplay> bits;
};

static std::vector<PopoutAchDisplay> s_popoutCache;
static std::vector<uint32_t> s_popoutLastPinned;
static uint64_t s_popoutLastProgressGen = 0;
static bool s_popoutCacheDirty = true;


static void InvalidatePopoutCache() { s_popoutCacheDirty = true; }

static void RebuildPopoutCache() {
    // Called under g_AchMutex or after snapshot
    s_popoutCache.clear();
    s_popoutCache.reserve(g_AchPinned.size());

    for (uint32_t achId : g_AchPinned) {
        PopoutAchDisplay disp;
        disp.achId = achId;

        auto defIt = g_AchDefs.find(achId);
        auto progIt = g_AchProgress.find(achId);

        disp.done = (progIt != g_AchProgress.end()) && progIt->second.done;
        disp.current = (progIt != g_AchProgress.end()) ? progIt->second.current : 0;

        if (defIt != g_AchDefs.end()) {
            const AchDef& def = defIt->second;
            disp.hasDef = true;
            disp.name = def.name;
            disp.max = def.max_count > 0 ? def.max_count : ((progIt != g_AchProgress.end()) ? progIt->second.max : 0);
            disp.hasBits = !def.bits.empty() && !disp.done;

            if (disp.hasBits) {
                auto wpIt = g_AchWaypoints.find(achId);
                disp.bits.reserve(def.bits.size());
                for (size_t i = 0; i < def.bits.size(); i++) {
                    PopoutBitDisplay bd;
                    bd.done = (progIt != g_AchProgress.end()) &&
                        progIt->second.completed_bits.count((uint32_t)i) > 0;

                    const AchBitDef& bit = def.bits[i];
                    if (bit.type == "Skin" && bit.item_id > 0) {
                        bd.skinId = bit.item_id;
                        auto skinInfo = Skinventory::SkinCache::GetSkin(bd.skinId);
                        if (skinInfo.has_value()) {
                            bd.label = skinInfo->name;
                            bd.skinType = skinInfo->type;
                            bd.skinWeightClass = skinInfo->weight_class;
                            bd.skinSubtype = skinInfo->subtype;
                            bd.skinName = skinInfo->name;
                        } else {
                            bd.label = !bit.text.empty() ? bit.text : "Skin #" + std::to_string(bd.skinId);
                        }
                    } else if (bit.type == "Text" && !bit.text.empty()) {
                        bd.label = bit.text;
                        bd.subAchId = FindAchIdByName(bit.text, def.name);
                    } else {
                        bd.label = bit.text.empty() ? ("Step " + std::to_string(i + 1)) : bit.text;
                    }

                    if (wpIt != g_AchWaypoints.end()) {
                        auto wpBitIt = wpIt->second.find((int)i);
                        if (wpBitIt != wpIt->second.end()) {
                            bd.waypoint = wpBitIt->second;
                        }
                    }
                    disp.bits.push_back(std::move(bd));
                }
            }
        } else {
            disp.hasDef = false;
            auto nameIt = g_AchNameIndex.find(achId);
            disp.loadingName = (nameIt != g_AchNameIndex.end()) ? nameIt->second : "";
        }

        s_popoutCache.push_back(std::move(disp));
    }

    s_popoutLastPinned = g_AchPinned;
    s_popoutLastProgressGen = g_AchProgressGen;
    s_popoutCacheDirty = false;
}

static void RenderAchPopout() {
    // Pinned-progress 10-minute auto-refresh is handled globally in
    // AddonRender so it fires regardless of popout / tab visibility.

    // Refresh pinned progress whenever the popout becomes visible
    // (button-pressed, or first time after addon load). Resets when hidden
    // so it'll fire again next time the user reopens it.
    static bool s_popoutWasVisible = false;
    if (!g_AchPopoutVisible) {
        s_popoutWasVisible = false;
        return;
    }
    if (g_CurrentCharName.empty()) return; // not yet logged in (character select screen)

    if (!s_popoutWasVisible) {
        s_popoutWasVisible = true;
        std::vector<uint32_t> pinnedCopy;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            pinnedCopy = g_AchPinned;
        }
        if (!pinnedCopy.empty() && !g_AchProgressFetching) {
            g_LastAchProgressQuery = std::chrono::steady_clock::now();
            SendAchProgressQuery(pinnedCopy);
        }
    }

    // Consume retry flag here too — popout may be open without the main
    // Achievements tab being visible. Previously this only fired from
    // RenderAchievements, so a PENDING response would never re-query.
    if (g_AchRetryPending.exchange(false) && !g_AchProgressFetching) {
        std::vector<uint32_t> pinnedCopy;
        {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            pinnedCopy = g_AchPinned;
        }
        if (!pinnedCopy.empty()) {
            SendAchProgressQuery(pinnedCopy);
        }
    }

    // Rebuild cache if dirty (brief lock, then release)
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        if (s_popoutCacheDirty || g_AchPinned != s_popoutLastPinned ||
            g_AchProgressGen != s_popoutLastProgressGen) {
            RebuildPopoutCache();
        }
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(250, 100), ImVec2(500, 800));
    if (ImGui::Begin("Achievement Tracker", &g_AchPopoutVisible,
                     ImGuiWindowFlags_NoCollapse)) {

        // Toolbar
        ImGui::Checkbox("Show completed steps", &g_AchShowCompletedSteps);
        ImGui::SameLine();
        if (g_AchProgressFetching) {
            RenderSpinner("", ImVec4(0.95f, 0.85f, 0.55f, 1.0f));
        } else if (RenderChipButton("Refresh##pop", false)) {
            std::vector<uint32_t> pinnedCopy;
            {
                std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                pinnedCopy = g_AchPinned;
            }
            if (!pinnedCopy.empty()) {
                SendAchProgressQuery(pinnedCopy);
            }
        }
        ImGui::Separator();

        if (s_popoutCache.empty()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No pinned achievements");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Pin achievements from the Achievements tab");
        }

        // Render from cache — no lock needed. Tighten vertical gap between rows.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
            ImVec2(ImGui::GetStyle().ItemSpacing.x, 2.0f));
        for (int idx = 0; idx < (int)s_popoutCache.size(); idx++) {
            const auto& disp = s_popoutCache[idx];
            ImGui::PushID(idx + 80000);
            ImGui::BeginGroup();

            if (disp.hasDef) {
                // ===== Compact row (matches main list) =====
                ImDrawList* dl = ImGui::GetWindowDrawList();
                float rowH = 28.0f;
                float availRowW = ImGui::GetContentRegionAvail().x;
                ImVec2 cmin = ImGui::GetCursorScreenPos();
                ImVec2 cmax(cmin.x + availRowW, cmin.y + rowH);

                bool inProgress = !disp.done && disp.current > 0 && disp.max > 0;
                ImU32 stateCol = disp.done ? IM_COL32(111, 204, 122, 255)
                              : inProgress ? IM_COL32(244, 196, 122, 230)
                                           : IM_COL32(232, 196, 122, 230);

                ImU32 bgCol = disp.done ? IM_COL32(28, 38, 28, 60) : IM_COL32(80, 64, 28, 50);
                dl->AddRectFilled(cmin, cmax, bgCol, 3.0f);
                dl->AddRect(cmin, cmax, IM_COL32(0, 0, 0, 130), 3.0f, 0, 1.0f);
                dl->AddRectFilled(ImVec2(cmin.x, cmin.y + 3),
                                  ImVec2(cmin.x + 3, cmax.y - 3), stateCol);

                bool expanded = g_AchExpandedInPopout[disp.achId];

                // Reserve right side for unpin button
                const float unpinW = 18.0f;
                float rowHitW = availRowW - unpinW - 6.0f;

                ImGui::InvisibleButton("##poprow", ImVec2(rowHitW, rowH));
                bool rowHovered = ImGui::IsItemHovered();
                bool rowClicked = ImGui::IsItemClicked();
                if (rowHovered)
                    dl->AddRect(cmin, cmax, IM_COL32(197, 161, 85, 130), 3.0f, 0, 1.0f);
                if (rowClicked && disp.hasBits) {
                    g_AchExpandedInPopout[disp.achId] = !expanded;
                    SaveAchTrackerState();
                }

                float padX = 10.0f;
                float midY = cmin.y + rowH * 0.5f;

                // Progress badge on the right (before unpin)
                float rightX = cmax.x - unpinW - 6.0f;
                if (disp.max > 1) {
                    char progBuf[24];
                    snprintf(progBuf, sizeof(progBuf), "%d / %d",
                        disp.done ? disp.max : disp.current, disp.max);
                    ImVec2 sz = ImGui::CalcTextSize(progBuf);
                    float bdW = sz.x + 10.0f;
                    float bdH = 16.0f;
                    ImVec2 bMin(rightX - bdW, midY - bdH * 0.5f);
                    ImVec2 bMax(rightX, bMin.y + bdH);
                    ImU32 bBg, bBorder, bText;
                    if (disp.done) {
                        bBg = IM_COL32(40, 60, 40, 180); bBorder = IM_COL32(120, 200, 130, 200);
                        bText = IM_COL32(160, 220, 170, 255);
                    } else {
                        bBg = IM_COL32(0, 0, 0, 110); bBorder = IM_COL32(160, 145, 110, 170);
                        bText = IM_COL32(220, 210, 180, 240);
                    }
                    dl->AddRectFilled(bMin, bMax, bBg, 3.0f);
                    dl->AddRect(bMin, bMax, bBorder, 3.0f, 0, 1.0f);
                    dl->AddText(ImVec2(bMin.x + 5.0f, bMin.y + (bdH - sz.y) * 0.5f), bText, progBuf);
                    rightX = bMin.x - 6.0f;
                }

                // Title (clipped)
                float titleX = cmin.x + padX + 4.0f;
                float titleEndX = rightX - 6.0f;
                ImU32 titleCol = disp.done
                    ? IM_COL32(150, 200, 150, 255)
                    : IM_COL32(245, 220, 150, 255);
                ImVec2 tSz = ImGui::CalcTextSize(disp.name.c_str());
                dl->PushClipRect(ImVec2(titleX, cmin.y), ImVec2(titleEndX, cmax.y), true);
                dl->AddText(ImVec2(titleX, midY - tSz.y * 0.5f - 1.0f), titleCol, disp.name.c_str());
                dl->PopClipRect();

                // 2px progress strip at bottom
                if (disp.max > 0) {
                    float frac = disp.done ? 1.0f : (float)disp.current / (float)disp.max;
                    float stripY = cmax.y - 4.0f;
                    ImVec2 sMin(cmin.x + 4.0f, stripY);
                    ImVec2 sMax(cmax.x - 4.0f, stripY + 2.0f);
                    dl->AddRectFilled(sMin, sMax, IM_COL32(255, 255, 255, 18), 1.0f);
                    float fillEnd = sMin.x + (sMax.x - sMin.x) * frac;
                    if (fillEnd > sMin.x)
                        dl->AddRectFilled(sMin, ImVec2(fillEnd, sMax.y), stateCol, 1.0f);
                }

                // Unpin button (small ×)
                {
                    float btnH = 18.0f;
                    ImGui::SetCursorScreenPos(ImVec2(cmax.x - unpinW - 4.0f, midY - btnH * 0.5f));
                    ImVec2 bMin = ImGui::GetCursorScreenPos();
                    ImVec2 bMax(bMin.x + unpinW, bMin.y + btnH);
                    dl->AddRectFilled(bMin, bMax, IM_COL32(0, 0, 0, 130), 3.0f);
                    dl->AddRect(bMin, bMax, IM_COL32(140, 130, 110, 170), 3.0f, 0, 1.0f);
                    const char* x = "x";
                    ImVec2 sz = ImGui::CalcTextSize(x);
                    dl->AddText(ImVec2(bMin.x + (unpinW - sz.x) * 0.5f, bMin.y + (btnH - sz.y) * 0.5f),
                        IM_COL32(180, 168, 130, 230), x);
                    if (ImGui::InvisibleButton("##popunpin", ImVec2(unpinW, btnH))) {
                        ToggleAchPin(disp.achId);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Unpin from tracker");
                    }
                }

                ImGui::SetCursorScreenPos(ImVec2(cmin.x, cmax.y + 2.0f));

                if (disp.hasBits && g_AchExpandedInPopout[disp.achId]) {
                    for (size_t i = 0; i < disp.bits.size(); i++) {
                        const auto& bd = disp.bits[i];
                        if (!g_AchShowCompletedSteps && bd.done) continue;

                        ImGui::PushID((int)i);
                        if (bd.done) {
                            if (bd.subAchId > 0) {
                                std::string selLabel = "    [x] " + bd.label + "###popsubach";
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.82f, 0.35f, 0.6f));
                                if (ImGui::Selectable(selLabel.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                                    NavigateToAchievement(bd.subAchId);
                                }
                                ImGui::PopStyleColor();
                            } else {
                                ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 0.6f), "    [x] %s", bd.label.c_str());
                            }
                        } else if (bd.skinId > 0 && !bd.skinName.empty()) {
                            std::string selLabel = "    [ ] " + bd.label + "###popskinbit";
                            if (ImGui::Selectable(selLabel.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                                g_SkinSelectedId = bd.skinId;
                                g_SkinSelectedType = bd.skinType;
                                if (bd.skinType == "Armor") {
                                    g_SkinSelectedWeightClass = bd.skinWeightClass;
                                }
                                g_SkinSelectedSubtype = bd.skinSubtype;
                                g_SkinSwitchToBrowser = true;
                                g_SkinScrollToSkin = true;
                                g_SwitchToSkinventory = true;
                                Skinventory::WikiImage::RequestImage(bd.skinId, bd.skinName, bd.skinWeightClass);
                                Skinventory::Commerce::FetchPriceForSkin(bd.skinId);
                            }
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Click to view in Skinventory");
                            }
                        } else if (bd.subAchId > 0) {
                            std::string selLabel = "    [ ] " + bd.label + "###popsubach";
                            if (ImGui::Selectable(selLabel.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                                NavigateToAchievement(bd.subAchId);
                            }
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Click to navigate to achievement");
                            }
                        } else {
                            if (!bd.waypoint.empty()) {
                                std::string selLabel = "    [ ] " + bd.label + "###popbit";
                                if (ImGui::Selectable(selLabel.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                                    ImGui::SetClipboardText(bd.waypoint.c_str());
                                }
                                if (ImGui::IsItemHovered()) {
                                    ImGui::SetTooltip("Click to copy: %s", bd.waypoint.c_str());
                                }
                            } else {
                                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "    [ ] %s", bd.label.c_str());
                                if (ImGui::IsItemHovered()) {
                                    ImGui::SetTooltip("Click to copy name");
                                    if (ImGui::IsItemClicked()) {
                                        ImGui::SetClipboardText(bd.label.c_str());
                                    }
                                }
                            }
                        }
                        ImGui::PopID();
                    }
                }
            } else {
                if (!disp.loadingName.empty()) {
                    ImGui::Text("%s (loading...)", disp.loadingName.c_str());
                } else {
                    ImGui::Text("Achievement %u (loading...)", disp.achId);
                }
            }
            ImGui::EndGroup();

            ImGui::PopID();
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();
}

static void RenderSkinventory() {
    // Keep OwnedSkins account in sync with dropdown selection
    Skinventory::OwnedSkins::SetAccountName(GetEffectiveAccountName());

    auto cacheStatus = Skinventory::SkinCache::GetStatus();

    if (cacheStatus == Skinventory::CacheStatus::Loading ||
        cacheStatus == Skinventory::CacheStatus::Empty) {
        std::string msg = Skinventory::SkinCache::GetStatusMessage();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", msg.c_str());
        return;
    }

    if (cacheStatus == Skinventory::CacheStatus::Error) {
        std::string msg = Skinventory::SkinCache::GetStatusMessage();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", msg.c_str());
        return;
    }

    // Status bar + manual refresh
    {
        static auto s_lastRefreshTime = std::chrono::steady_clock::time_point{};
        static bool s_refreshPending = false;

        // Consume H&S data-updated flag (e.g. user refreshed in H&S)
        if (Skinventory::OwnedSkins::ConsumeDataUpdatedFlag()) {
            g_SkinRefreshOwned = true;
        }

        bool isQuerying = Skinventory::OwnedSkins::IsQuerying();
        bool canRefresh = Skinventory::OwnedSkins::IsHoardAndSeekAvailable() && !isQuerying;

        // Auto-refresh once per (account, session) when we land on the tab with no data
        {
            static std::string s_autoRefreshAccount;
            std::string currentAccount = GetEffectiveAccountName();
            if (canRefresh
                && !Skinventory::OwnedSkins::HasData()
                && s_autoRefreshAccount != currentAccount) {
                s_autoRefreshAccount = currentAccount;
                g_SkinRefreshOwned = true;
            }
        }

        if (!canRefresh) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        if (ImGui::SmallButton("Refresh Owned") && canRefresh) {
            g_SkinRefreshOwned = true;
        }
        if (!canRefresh) ImGui::PopStyleVar();

        // Kick off query when requested — use API proxy (single request, 64KB buffer)
        if (g_SkinRefreshOwned && canRefresh) {
            g_SkinRefreshOwned = false;
            s_refreshPending = true;

            // Update account for the query
            Skinventory::OwnedSkins::SetAccountName(GetEffectiveAccountName());

            Skinventory::OwnedSkins::RequestOwnedSkinsViaApi();
        }

        // Detect when query finishes
        if (s_refreshPending && !isQuerying) {
            s_refreshPending = false;
            s_lastRefreshTime = std::chrono::steady_clock::now();
        }

        // Status text
        ImGui::SameLine();
        if (isQuerying) {
            std::string hsMsg = Skinventory::OwnedSkins::GetStatusMessage();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", hsMsg.c_str());
        } else if (Skinventory::OwnedSkins::HasData()) {
            size_t owned = Skinventory::OwnedSkins::GetOwnedCount();
            if (s_lastRefreshTime != std::chrono::steady_clock::time_point{}) {
                auto elapsed = std::chrono::steady_clock::now() - s_lastRefreshTime;
                int secs = (int)std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
                if (secs < 60)
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%zu owned  |  Refreshed %ds ago", owned, secs);
                else
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%zu owned  |  Refreshed %dm ago", owned, secs / 60);
            } else {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%zu owned (cached)", owned);
            }
        } else if (!Skinventory::OwnedSkins::IsHoardAndSeekAvailable()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Waiting for Hoard & Seek...");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Click Refresh Owned to query skins");
        }

        if (cacheStatus == Skinventory::CacheStatus::Updating) {
            ImGui::SameLine();
            std::string updateMsg = Skinventory::SkinCache::GetStatusMessage();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "| %s", updateMsg.c_str());
        }
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("##skin_tabs")) {
        ImGuiTabItemFlags browserFlags = 0;
        if (g_SkinSwitchToBrowser) {
            browserFlags = ImGuiTabItemFlags_SetSelected;
            g_SkinSwitchToBrowser = false;
        }
        if (ImGui::BeginTabItem("Browser", nullptr, browserFlags)) {
            g_SkinActiveTab = 0;
            RenderSkinCategoryNav();
            ImGui::SameLine();
            RenderSkinList();
            ImGui::SameLine();
            RenderSkinDetailPanel();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Shopping List")) {
            g_SkinActiveTab = 1;
            RenderSkinShoppingList();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

// --- Main Render ---

void AddonRender() {
    // Process icon download queue every frame
    AlterEgo::IconManager::Tick();
    Skinventory::WikiImage::Tick();
    Skinventory::OwnedSkins::Tick();

    // Persist login timestamps when dirty (crash-safe)
    if (g_LoginTimestampsDirty) SaveLoginTimestamps();

    // Pinned-achievement priority refresh — runs every frame regardless of
    // tab/popout visibility. Pinned achievements matter to the user most, so
    // they get a 10-minute refresh cycle independent of UI state.
    {
        static auto s_lastPinnedRefresh = std::chrono::steady_clock::time_point{};
        auto hStatus = AlterEgo::GW2API::GetHoardStatus();
        bool hReady = (hStatus == AlterEgo::HoardStatus::Available ||
                       hStatus == AlterEgo::HoardStatus::Ready);
        if (hReady && !g_AchProgressFetching) {
            auto now = std::chrono::steady_clock::now();
            bool firstRun = (s_lastPinnedRefresh == std::chrono::steady_clock::time_point{});
            bool expired  = !firstRun &&
                (now - s_lastPinnedRefresh) >= std::chrono::minutes(10);
            if (firstRun || expired) {
                std::vector<uint32_t> pinnedCopy;
                {
                    std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
                    pinnedCopy = g_AchPinned;
                }
                if (!pinnedCopy.empty()) {
                    s_lastPinnedRefresh = now;
                    g_LastAchProgressQuery = now;
                    SendAchProgressQuery(pinnedCopy);
                }
            }
        }
    }

    ThemeGuard themeGuard;

    // Render gear customize dialog (separate window, always checked)
    RenderGearCustomizeDialog();
    RenderSaveToLibraryDialog();

    // Render chat build detection toast (always visible, even when main window is hidden)
    RenderBuildToast();

    // Achievement tracker popout (independent of main window)
    RenderAchPopout();

    if (!g_WindowVisible) { return; }

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX));
    if (!ImGui::Begin("Alter Ego", &g_WindowVisible, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    // H&S status
    auto hoardStatus = AlterEgo::GW2API::GetHoardStatus();
    auto fetchStatus = AlterEgo::GW2API::GetFetchStatus();
    bool scanning = (fetchStatus == AlterEgo::FetchStatus::InProgress);
    bool hoardReady = (hoardStatus == AlterEgo::HoardStatus::Available ||
                       hoardStatus == AlterEgo::HoardStatus::Ready);

    // H&S connection warnings (shown globally, above tab bar)
    if (hoardStatus == AlterEgo::HoardStatus::Unknown) {
        RenderSpinner("Checking for Hoard & Seek...");
        static auto lastPing = std::chrono::steady_clock::time_point{};
        auto now = std::chrono::steady_clock::now();
        if (now - lastPing > std::chrono::seconds(5)) {
            lastPing = now;
            AlterEgo::GW2API::PingHoard();
        }
    } else if (hoardStatus == AlterEgo::HoardStatus::Unavailable) {
        RenderEmptyCard(
            APIDefs ? APIDefs->Textures_Get(TEX_ICON) : nullptr,
            "Hoard & Seek is required",
            "Alter Ego reads your characters, equipment, and inventory through the Hoard & Seek companion addon. Install it from the Nexus addon library to get started.",
            "Retry Detection", []() { AlterEgo::GW2API::PingHoard(); },
            "Copy Library URL", []() {
                CopyToClipboard(std::string("https://raidcore.gg/Nexus"));
                if (APIDefs) APIDefs->GUI_SendAlert("Nexus library URL copied!");
            }
        );
        return;
    } else if (hoardStatus == AlterEgo::HoardStatus::PermPending) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Approve Alter Ego in H&S permission popup.");
        static auto lastPermRetry = std::chrono::steady_clock::time_point{};
        auto permNow = std::chrono::steady_clock::now();
        if (permNow - lastPermRetry > std::chrono::seconds(3)) {
            lastPermRetry = permNow;
            AlterEgo::GW2API::RequestCharacterList();
        }
    } else if (hoardStatus == AlterEgo::HoardStatus::PermDenied) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "H&S permission denied. Enable in H&S settings.");
    }

    // Auto-query accounts when H&S becomes available (multi-account)
    {
        static bool s_accountsQuerySent = false;
        if (hoardReady && !s_accountsQuerySent) {
            s_accountsQuerySent = true;
            AlterEgo::GW2API::QueryAccounts();
        }
    }

    // Account selector (only when multi-account) — rendered on the same row as
    // the tab bar (right-aligned) to save vertical space.
    static bool g_ForceCharactersTab = false;
    float tabRowY = ImGui::GetCursorPosY();

    // Top-level tab bar: Characters | Build Library
    if (ImGui::BeginTabBar("##main_tabs")) {
        ImGuiTabItemFlags charTabFlags = 0;
        if (g_ForceCharactersTab) {
            charTabFlags = ImGuiTabItemFlags_SetSelected;
            g_ForceCharactersTab = false;
        }
        if (ImGui::BeginTabItem("Characters", nullptr, charTabFlags)) {
            g_MainTab = 0;

            // Re-trigger detail fetch when character data finishes loading
            {
                static AlterEgo::FetchStatus lastFetchStatus = AlterEgo::FetchStatus::Idle;
                if (fetchStatus == AlterEgo::FetchStatus::Success && lastFetchStatus != AlterEgo::FetchStatus::Success) {
                    g_DetailsFetched = false;
                }
                lastFetchStatus = fetchStatus;
            }

            const auto& characters = AlterEgo::GW2API::GetCharacters();
            // Also check pending names from a list-only fetch (new user, no full data yet)
            const auto& pendingNames = AlterEgo::GW2API::GetPendingCharNames();

            // Auto-fetch character list when H&S first becomes ready and we have no data
            // Wait for accounts data so we get characters from ALL accounts
            if (!g_AutoCharListRequested && hoardReady && !scanning &&
                characters.empty() && pendingNames.empty() &&
                AlterEgo::GW2API::HasAccountsData()) {
                g_AutoCharListRequested = true;
                AlterEgo::GW2API::RequestCharacterList();
            }

            if (!characters.empty()) {
                // Rebuild display order when character count or current account changes
                static std::string s_lastCurrentAcct;
                const std::string& curAcct = AlterEgo::GW2API::GetCurrentAccountName();
                if (characters.size() != g_LastCharCount || curAcct != s_lastCurrentAcct) {
                    s_lastCurrentAcct = curAcct;
                    RebuildCharDisplayOrder();
                }

                // Auto-select first character if none selected, or restore session
                if (g_SelectedCharIdx < 0 || g_SelectedCharIdx >= (int)g_CharDisplayOrder.size()) {
                    g_SelectedCharIdx = 0;
                    // Restore session: find saved character by name
                    if (!g_SessionSelectedCharName.empty()) {
                        for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                            int ri = g_CharDisplayOrder[di];
                            if (ri >= 0 && ri < (int)characters.size() &&
                                characters[ri].name == g_SessionSelectedCharName) {
                                g_SelectedCharIdx = di;
                                break;
                            }
                        }
                        g_SessionSelectedCharName.clear();
                        // Keep session-restored g_SelectedBuildTab and g_SelectedEquipTab
                    } else {
                        g_SelectedBuildTab = -1;
                    }
                    g_DetailsFetched = false;
                }

                // Character list (left) + Detail panel (right)
                // Clamp width to reasonable bounds
                float availW = ImGui::GetContentRegionAvail().x;
                g_CharListWidth = (g_CharListWidth < 120.0f) ? 120.0f : (g_CharListWidth > availW - 200.0f) ? availW - 200.0f : g_CharListWidth;

                ImGui::BeginChild("CharList", ImVec2(g_CharListWidth, 0), true);

                // Refresh Characters button + status
                {
                    bool refreshDisabled = scanning || !hoardReady;
                    if (refreshDisabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                    if (ImGui::Button("Refresh", ImVec2(-1, 0)) && !refreshDisabled) {
                        g_RefreshListFetching = true;
                        AlterEgo::GW2API::RequestCharacterList();
                    }
                    if (refreshDisabled) ImGui::PopStyleVar();

                    // Compact status line
                    if (fetchStatus == AlterEgo::FetchStatus::InProgress) {
                        g_FetchDoneTime = std::chrono::steady_clock::time_point{};
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s",
                            AlterEgo::GW2API::GetFetchStatusMessage().c_str());
                    } else if (fetchStatus == AlterEgo::FetchStatus::Error ||
                               fetchStatus == AlterEgo::FetchStatus::Success) {
                        if (g_FetchDoneTime == std::chrono::steady_clock::time_point{})
                            g_FetchDoneTime = std::chrono::steady_clock::now();
                        float elapsed = std::chrono::duration<float>(
                            std::chrono::steady_clock::now() - g_FetchDoneTime).count();
                        if (elapsed < 5.0f) {
                            float alpha = (elapsed < 4.0f) ? 1.0f : (5.0f - elapsed);
                            ImVec4 col = (fetchStatus == AlterEgo::FetchStatus::Error)
                                ? ImVec4(1.0f, 0.3f, 0.3f, alpha)
                                : ImVec4(0.35f, 0.82f, 0.35f, alpha);
                            ImGui::TextColored(col, "%s",
                                AlterEgo::GW2API::GetFetchStatusMessage().c_str());
                        }
                    }
                    if (!scanning) {
                        time_t last = AlterEgo::GW2API::GetLastUpdated();
                        if (last > 0) {
                            time_t now_t = std::time(nullptr);
                            int el = (int)difftime(now_t, last);
                            std::string ago;
                            if (el < 60) ago = "just now";
                            else if (el < 3600) ago = std::to_string(el / 60) + "m ago";
                            else if (el < 86400) ago = std::to_string(el / 3600) + "h ago";
                            else ago = std::to_string(el / 86400) + "d ago";
                            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Updated %s", ago.c_str());
                        } else if (hoardReady && !AlterEgo::GW2API::HasCharacterData()) {
                            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Click Refresh to load");
                        }
                    }
                    ImGui::Separator();
                }

                // Sort mode selector + direction toggle (inside the list child)
                {
                    float arrowBtnWidth = (g_CharSortMode != Sort_Custom) ? 24.0f : 0.0f;
                    float avail = ImGui::GetContentRegionAvail().x;
                    float comboWidth = avail - arrowBtnWidth - (arrowBtnWidth > 0 ? 4.0f : 0.0f);
                    ImGui::SetNextItemWidth(comboWidth);
                    const char* sortLabels[] = { "Custom", "Name", "Class", "Level", "Age", "Birthday" };
                    if (ImGui::Combo("##sort", &g_CharSortMode, sortLabels, IM_ARRAYSIZE(sortLabels))) {
                        g_CharSortAscending = true; // reset direction on mode change
                        std::string selName;
                        if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                            int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                            if (ri >= 0 && ri < (int)characters.size()) selName = characters[ri].name;
                        }
                        RebuildCharDisplayOrder();
                        g_SelectedCharIdx = 0;
                        for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                            if (characters[g_CharDisplayOrder[di]].name == selName) {
                                g_SelectedCharIdx = di;
                                break;
                            }
                        }
                        SaveCharSortConfig();
                    }
                    if (g_CharSortMode != Sort_Custom) {
                        ImGui::SameLine();
                        if (ImGui::Button("##sortdir", ImVec2(arrowBtnWidth, 0))) {
                            g_CharSortAscending = !g_CharSortAscending;
                            std::string selName;
                            if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                                int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                                if (ri >= 0 && ri < (int)characters.size()) selName = characters[ri].name;
                            }
                            RebuildCharDisplayOrder();
                            g_SelectedCharIdx = 0;
                            for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                                if (characters[g_CharDisplayOrder[di]].name == selName) {
                                    g_SelectedCharIdx = di;
                                    break;
                                }
                            }
                            SaveCharSortConfig();
                        }
                        // Draw triangle arrow on the button
                        ImVec2 btnMin = ImGui::GetItemRectMin();
                        ImVec2 btnMax = ImGui::GetItemRectMax();
                        float cx = (btnMin.x + btnMax.x) * 0.5f;
                        float cy = (btnMin.y + btnMax.y) * 0.5f;
                        float sz = 5.0f;
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        ImU32 col = IM_COL32(220, 220, 220, 255);
                        if (g_CharSortAscending) {
                            // Up arrow
                            dl->AddTriangleFilled(
                                ImVec2(cx, cy - sz), ImVec2(cx - sz, cy + sz), ImVec2(cx + sz, cy + sz), col);
                        } else {
                            // Down arrow
                            dl->AddTriangleFilled(
                                ImVec2(cx - sz, cy - sz), ImVec2(cx + sz, cy - sz), ImVec2(cx, cy + sz), col);
                        }
                    }
                    ImGui::Separator();
                }

                // Character search bar
                {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::InputTextWithHint("##charsearch", "Search...", g_CharSearchBuf, sizeof(g_CharSearchBuf));
                }

                // Collect item rects for insertion-line drag-and-drop
                struct CharItemRect { float yMin, yMax; int di; std::string acctName; };
                std::vector<CharItemRect> itemRects;
                itemRects.reserve(g_CharDisplayOrder.size());

                // Cache which character is currently being fetched
                std::string fetchingCharName = AlterEgo::GW2API::GetCurrentFetchCharName();

                // Prepare lowercase search string
                std::string charSearchLower;
                if (g_CharSearchBuf[0] != '\0') {
                    charSearchLower = g_CharSearchBuf;
                    std::transform(charSearchLower.begin(), charSearchLower.end(),
                                   charSearchLower.begin(), ::tolower);
                }

                // Build account display name lookup for headers
                bool showAcctHeaders = AlterEgo::GW2API::IsMultiAccount();
                std::unordered_map<std::string, std::string> acctDisplayNames;
                if (showAcctHeaders) {
                    for (const auto& acct : AlterEgo::GW2API::GetAccounts())
                        acctDisplayNames[acct.name] = acct.display_name;
                }
                std::string lastAcctHeader;

                for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                    int realIdx = g_CharDisplayOrder[di];
                    if (realIdx < 0 || realIdx >= (int)characters.size()) continue;
                    const auto& ch = characters[realIdx];

                    // Apply account filter
                    if (!g_SelectedAccountFilter.empty() && ch.account_name != g_SelectedAccountFilter)
                        continue;

                    // Apply search filter
                    if (!charSearchLower.empty()) {
                        std::string nameLower = ch.name;
                        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        std::string profLower = ch.profession;
                        std::transform(profLower.begin(), profLower.end(), profLower.begin(), ::tolower);
                        if (nameLower.find(charSearchLower) == std::string::npos &&
                            profLower.find(charSearchLower) == std::string::npos)
                            continue;
                    }

                    // Account group header
                    if (showAcctHeaders && ch.account_name != lastAcctHeader) {
                        lastAcctHeader = ch.account_name;
                        auto it = acctDisplayNames.find(ch.account_name);
                        const char* label = it != acctDisplayNames.end() ? it->second.c_str() : ch.account_name.c_str();
                        if (di > 0) ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.5f, 1.0f), "%s", label);
                        ImGui::Separator();
                    }

                    ImGui::PushID(di);

                    bool selected = (g_SelectedCharIdx == di);
                    bool isLoggedIn = (!g_CurrentCharName.empty() && ch.name == g_CurrentCharName);
                    ImVec4 profColor = GetProfessionColor(ch.profession);

                    // Count extra lines for dynamic row height
                    int extraLines = 0;
                    if (!g_CompactCharList) {
                        if (g_ShowCraftingIcons && !ch.crafting.empty()) extraLines++;
                        if (g_ShowAge && !ch.created.empty()) extraLines++;
                        if (g_ShowPlaytime && ch.age > 0) extraLines++;
                        if (g_ShowLastLogin && g_LoginTimestamps.count(ch.name)) extraLines++;
                        if (g_BirthdayMode != 2) {
                            int bdays = DaysUntilBirthday(ch.created);
                            bool showBday = (g_BirthdayMode == 0 && bdays >= 0) ||
                                            (g_BirthdayMode == 1 && bdays >= 0 && bdays <= 7);
                            if (showBday) extraLines++;
                        }
                    }
                    float lineH = ImGui::GetTextLineHeightWithSpacing();
                    float rowHeight = g_CompactCharList ? 18.0f
                        : (28.0f + extraLines * lineH);

                    // Profession-tinted selection highlight
                    ImVec4 selCol(profColor.x * 0.35f, profColor.y * 0.35f, profColor.z * 0.35f, 0.65f);
                    ImVec4 hovCol(profColor.x * 0.25f, profColor.y * 0.25f, profColor.z * 0.25f, 0.50f);
                    ImVec4 actCol(profColor.x * 0.40f, profColor.y * 0.40f, profColor.z * 0.40f, 0.75f);
                    ImGui::PushStyleColor(ImGuiCol_Header, selCol);
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hovCol);
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, actCol);

                    // Character entry
                    if (ImGui::Selectable("##char", selected, 0, ImVec2(0, rowHeight))) {
                        g_SelectedCharIdx = di;
                        g_DetailsFetched = false;
                        g_SelectedEquipTab = 0;
                        g_SelectedBuildTab = -1;
                        // Refresh portrait detection for this character
                        s_portraitPathCache.erase(ch.name);
                        s_portraitMissing.erase(ch.name);
                    }

                    // Draw profession-colored left accent bar (non-compact only)
                    if (!g_CompactCharList) {
                        ImVec2 rMin = ImGui::GetItemRectMin();
                        ImVec2 rMax = ImGui::GetItemRectMax();
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        float barW = 3.0f;
                        ImU32 barCol = ImGui::ColorConvertFloat4ToU32(
                            ImVec4(profColor.x, profColor.y, profColor.z,
                                   selected ? 1.0f : (ImGui::IsItemHovered() ? 0.7f : 0.35f)));
                        dl->AddRectFilled(
                            ImVec2(rMin.x, rMin.y + 1), ImVec2(rMin.x + barW, rMax.y - 1),
                            barCol, 1.0f);
                        // Subtle glow on selected items
                        if (selected) {
                            ImU32 glowCol = ImGui::ColorConvertFloat4ToU32(
                                ImVec4(profColor.x, profColor.y, profColor.z, 0.08f));
                            dl->AddRectFilledMultiColor(
                                ImVec2(rMin.x + barW, rMin.y),
                                ImVec2(rMin.x + barW + 30.0f, rMax.y),
                                glowCol, IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), glowCol);
                        }
                    }

                    // Record item rect for insertion line calculation
                    ImVec2 rMin = ImGui::GetItemRectMin();
                    ImVec2 rMax = ImGui::GetItemRectMax();

                    itemRects.push_back({ rMin.y, rMax.y, di, ch.account_name });

                    // Drag source for Custom sort mode
                    if (g_CharSortMode == Sort_Custom) {
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            g_CharDragIdx = di;
                            ImGui::SetDragDropPayload("CHAR_REORDER", &di, sizeof(int));
                            ImGui::TextColored(profColor, "%s", ch.name.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }

                    // Draw profession icon + character info on top of selectable
                    ImGui::SameLine(4);
                    ImGui::BeginGroup();
                    if (!g_CompactCharList) {
                        const float iconSz = 20.0f;
                        uint32_t profIconId = GetProfessionIconId(ch.profession);
                        const char* profIconUrl = GetProfessionIconUrl(ch.profession);
                        ImVec2 cpos = ImGui::GetCursorPos();
                        float yOff = (extraLines > 0) ? 2.0f : (rowHeight - iconSz) * 0.5f;
                        if (profIconId && profIconUrl) {
                            auto* tex = AlterEgo::IconManager::GetIcon(profIconId);
                            if (tex && tex->Resource) {
                                ImGui::SetCursorPos(ImVec2(cpos.x, cpos.y + yOff));
                                ImGui::Image(tex->Resource, ImVec2(iconSz, iconSz));
                            } else {
                                AlterEgo::IconManager::RequestIcon(profIconId, profIconUrl);
                                ImGui::SetCursorPos(ImVec2(cpos.x, cpos.y + yOff));
                                ImGui::Dummy(ImVec2(iconSz, iconSz));
                            }
                        } else {
                            ImGui::SetCursorPos(ImVec2(cpos.x, cpos.y + yOff));
                            ImGui::Dummy(ImVec2(iconSz, iconSz));
                        }
                        ImGui::SameLine(0, 4);
                        float textYOff = (extraLines > 0) ? 2.0f : (rowHeight - ImGui::GetTextLineHeight()) * 0.5f;
                        ImGui::SetCursorPosY(cpos.y + textYOff);
                    }
                    // First line: name + level + green dot
                    ImGui::TextColored(profColor, "%s", ch.name.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(selected ? ImVec4(0.75f, 0.75f, 0.75f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Lv%d", ch.level);
                    if (isLoggedIn) {
                        ImGui::SameLine();
                        float dotY = ImGui::GetCursorScreenPos().y - 3.0f + ImGui::GetTextLineHeight() * 0.5f;
                        float dotX = ImGui::GetCursorScreenPos().x;
                        ImGui::GetWindowDrawList()->AddCircleFilled(
                            ImVec2(dotX + 3.0f, dotY), 3.0f,
                            IM_COL32(60, 200, 80, 255));
                        ImGui::Dummy(ImVec2(8, 0));
                    }
                    // Spinning refresh icon for the character currently being fetched
                    if (!fetchingCharName.empty() && ch.name == fetchingCharName) {
                        ImGui::SameLine();
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        float iconR = 5.0f;
                        ImVec2 scrPos = ImGui::GetCursorScreenPos();
                        float cx = scrPos.x + iconR + 1.0f;
                        float cy = scrPos.y + ImGui::GetTextLineHeight() * 0.5f;
                        float angle = (float)ImGui::GetTime() * 3.0f; // spin speed
                        ImU32 col = IM_COL32(255, 220, 60, 220);
                        // Draw two arcs with arrowheads
                        for (int arc = 0; arc < 2; arc++) {
                            float arcStart = angle + arc * 3.14159f;
                            int segments = 8;
                            float arcSpan = 2.4f; // ~137 degrees per arc
                            for (int s = 0; s < segments; s++) {
                                float a0 = arcStart + (arcSpan * s / segments);
                                float a1 = arcStart + (arcSpan * (s + 1) / segments);
                                dl->AddLine(
                                    ImVec2(cx + cosf(a0) * iconR, cy + sinf(a0) * iconR),
                                    ImVec2(cx + cosf(a1) * iconR, cy + sinf(a1) * iconR),
                                    col, 1.5f);
                            }
                            // Arrowhead at end of arc
                            float aEnd = arcStart + arcSpan;
                            float tx = cosf(aEnd); // tangent direction (perpendicular to radius)
                            float ty = sinf(aEnd);
                            ImVec2 tip(cx + tx * iconR, cy + ty * iconR);
                            // Arrow points along the arc direction (tangent)
                            float tanX = -ty; // tangent = perpendicular to radial
                            float tanY = tx;
                            float arrSz = 3.0f;
                            // Two sides of the arrowhead
                            ImVec2 p1(tip.x - tanX * arrSz + tx * arrSz * 0.5f,
                                      tip.y - tanY * arrSz + ty * arrSz * 0.5f);
                            ImVec2 p2(tip.x - tanX * arrSz - tx * arrSz * 0.5f,
                                      tip.y - tanY * arrSz - ty * arrSz * 0.5f);
                            dl->AddTriangleFilled(tip, p1, p2, col);
                        }
                        ImGui::Dummy(ImVec2(iconR * 2.0f + 2.0f, 0));
                    }
                    // Extra lines (each on its own line with prefix)
                    if (!g_CompactCharList) {
                        ImGui::Indent(6.0f);
                        ImVec4 dimCol = selected ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
                        ImVec4 labelCol = selected ? ImVec4(0.7f, 0.7f, 0.7f, 1.0f) : ImVec4(0.45f, 0.45f, 0.45f, 1.0f);

                        // 1. Crafting icons (no prefix, inactive dimmed)
                        if (g_ShowCraftingIcons && !ch.crafting.empty()) {
                            const float craftIconSz = 14.0f;
                            for (size_t ci = 0; ci < ch.crafting.size(); ci++) {
                                if (ci > 0) ImGui::SameLine(0, 2);
                                const std::string& disc = ch.crafting[ci];
                                bool isActive = (ci < ch.crafting_active.size()) ? ch.crafting_active[ci] : true;
                                float iconAlpha = isActive ? 1.0f : 0.35f;
                                uint32_t cIconId = GetCraftingIconId(disc);
                                const char* cIconUrl = GetCraftingIconUrl(disc);
                                bool rendered = false;
                                if (cIconId && cIconUrl) {
                                    auto* tex = AlterEgo::IconManager::GetIcon(cIconId);
                                    if (tex && tex->Resource) {
                                        ImGui::Image(tex->Resource, ImVec2(craftIconSz, craftIconSz),
                                            ImVec2(0, 0), ImVec2(1, 1),
                                            ImVec4(1, 1, 1, iconAlpha));
                                        rendered = true;
                                    } else {
                                        AlterEgo::IconManager::RequestIcon(cIconId, cIconUrl);
                                    }
                                }
                                if (!rendered) ImGui::Dummy(ImVec2(craftIconSz, craftIconSz));
                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    int lvl = (ci < ch.crafting_levels.size()) ? ch.crafting_levels[ci] : 0;
                                    ImGui::Text("%s %d%s", disc.c_str(), lvl,
                                        isActive ? "" : " (inactive)");
                                    ImGui::EndTooltip();
                                }
                            }
                        }

                        // 2. Age: (character creation age)
                        if (g_ShowAge && !ch.created.empty()) {
                            struct tm ctm = {};
                            if (sscanf(ch.created.c_str(), "%d-%d-%dT%d:%d:%d",
                                    &ctm.tm_year, &ctm.tm_mon, &ctm.tm_mday,
                                    &ctm.tm_hour, &ctm.tm_min, &ctm.tm_sec) == 6) {
                                ctm.tm_year -= 1900;
                                ctm.tm_mon -= 1;
                                time_t created_t = mktime(&ctm);
                                time_t now_t = std::time(nullptr);
                                int totalDays = (int)(difftime(now_t, created_t) / 86400.0);
                                ImGui::TextColored(labelCol, "Age:");
                                ImGui::SameLine();
                                if (totalDays >= 365)
                                    ImGui::TextColored(dimCol, "%dy %dd", totalDays / 365, totalDays % 365);
                                else
                                    ImGui::TextColored(dimCol, "%dd", totalDays);
                            }
                        }

                        // 3. Next Birthday: (right after Age)
                        if (g_BirthdayMode != 2) {
                            int bdays = DaysUntilBirthday(ch.created);
                            bool showBday = (g_BirthdayMode == 0 && bdays >= 0) ||
                                            (g_BirthdayMode == 1 && bdays >= 0 && bdays <= 7);
                            if (showBday) {
                                ImGui::TextColored(labelCol, "Next Birthday:");
                                ImGui::SameLine();
                                if (bdays == 0)
                                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Today!");
                                else
                                    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "%dd", bdays);
                            }
                        }

                        // 4. Playtime:
                        if (g_ShowPlaytime && ch.age > 0) {
                            int hours = ch.age / 3600;
                            ImGui::TextColored(labelCol, "Playtime:");
                            ImGui::SameLine();
                            if (hours >= 24)
                                ImGui::TextColored(dimCol, "%dd %dh", hours / 24, hours % 24);
                            else
                                ImGui::TextColored(dimCol, "%dh", hours);
                        }

                        // 5. Last Login (from local MumbleLink tracking):
                        if (g_ShowLastLogin) {
                            auto tsIt = g_LoginTimestamps.find(ch.name);
                            if (tsIt != g_LoginTimestamps.end()) {
                                time_t now_t = std::time(nullptr);
                                int elapsed = (int)difftime(now_t, (time_t)tsIt->second);
                                std::string ago;
                                if (elapsed < 60) ago = "Just Now";
                                else if (elapsed < 3600) ago = std::to_string(elapsed / 60) + "m ago";
                                else if (elapsed < 86400) ago = std::to_string(elapsed / 3600) + "h ago";
                                else ago = std::to_string(elapsed / 86400) + "d ago";
                                ImGui::TextColored(labelCol, "Last Login:");
                                ImGui::SameLine();
                                ImGui::TextColored(dimCol, "%s", ago.c_str());
                            }
                        }
                        ImGui::Unindent(6.0f);
                    }
                    ImGui::EndGroup();

                    ImGui::PopStyleColor(3);

                    ImGui::PopID();
                }

                // Custom sort: draw insertion line and handle drop
                if (g_CharSortMode == Sort_Custom && g_CharDragIdx >= 0 &&
                    ImGui::GetDragDropPayload() != nullptr) {
                    // Find source account for constraining drops
                    std::string srcAcct;
                    {
                        int srcReal = (g_CharDragIdx >= 0 && g_CharDragIdx < (int)g_CharDisplayOrder.size())
                            ? g_CharDisplayOrder[g_CharDragIdx] : -1;
                        if (srcReal >= 0 && srcReal < (int)characters.size())
                            srcAcct = characters[srcReal].account_name;
                    }

                    // Find the range of itemRects indices belonging to the same account
                    int acctFirst = -1, acctLast = -1;
                    for (int i = 0; i < (int)itemRects.size(); i++) {
                        if (itemRects[i].acctName == srcAcct) {
                            if (acctFirst < 0) acctFirst = i;
                            acctLast = i;
                        }
                    }

                    float mouseY = ImGui::GetMousePos().y;
                    int insertIdx = -1;
                    float bestLineY = 0;

                    if (acctFirst >= 0) {
                        // Clamp insertion to within the account group
                        insertIdx = acctLast + 1; // default: end of account group
                        for (int i = acctFirst; i <= acctLast; i++) {
                            float midY = (itemRects[i].yMin + itemRects[i].yMax) * 0.5f;
                            if (mouseY < midY) {
                                insertIdx = i;
                                bestLineY = itemRects[i].yMin;
                                break;
                            }
                        }
                        if (insertIdx == acctLast + 1)
                            bestLineY = itemRects[acctLast].yMax;
                    }

                    // Draw insertion line
                    if (insertIdx >= 0) {
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        float xMin = ImGui::GetWindowPos().x + 2;
                        float xMax = xMin + ImGui::GetWindowContentRegionMax().x - 4;
                        dl->AddLine(ImVec2(xMin, bestLineY), ImVec2(xMax, bestLineY),
                            IM_COL32(100, 180, 255, 220), 2.0f);
                    }

                    // Handle drop
                    if (ImGui::IsMouseReleased(0)) {
                        if (insertIdx >= 0) {
                            // Map itemRects indices back to display-order indices
                            int srcDi = g_CharDragIdx;
                            int dstDi = (insertIdx < (int)itemRects.size()) ? itemRects[insertIdx].di
                                : (itemRects[acctLast].di + 1);

                            if (srcDi != dstDi && srcDi >= 0 && srcDi < (int)g_CharDisplayOrder.size()) {
                                int movedIdx = g_CharDisplayOrder[srcDi];
                                g_CharDisplayOrder.erase(g_CharDisplayOrder.begin() + srcDi);
                                int finalPos = (srcDi < dstDi) ? dstDi - 1 : dstDi;
                                if (finalPos > (int)g_CharDisplayOrder.size())
                                    finalPos = (int)g_CharDisplayOrder.size();
                                g_CharDisplayOrder.insert(g_CharDisplayOrder.begin() + finalPos, movedIdx);
                                // Update custom order
                                g_CustomCharOrder.clear();
                                for (int idx : g_CharDisplayOrder)
                                    g_CustomCharOrder.push_back(characters[idx].name);
                                // Fix selection
                                if (g_SelectedCharIdx == srcDi)
                                    g_SelectedCharIdx = finalPos;
                                else if (srcDi < finalPos && g_SelectedCharIdx > srcDi && g_SelectedCharIdx <= finalPos)
                                    g_SelectedCharIdx--;
                                else if (srcDi > finalPos && g_SelectedCharIdx >= finalPos && g_SelectedCharIdx < srcDi)
                                    g_SelectedCharIdx++;
                                SaveCharSortConfig();
                            }
                        }
                        g_CharDragIdx = -1;
                    }
                } else if (ImGui::IsMouseReleased(0)) {
                    g_CharDragIdx = -1;
                }

                ImGui::EndChild();

                // Draggable vertical splitter
                ImGui::SameLine();
                {
                    float splitterW = 6.0f;
                    float h = ImGui::GetContentRegionAvail().y;
                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImGui::InvisibleButton("##charlist_splitter", ImVec2(splitterW, h));
                    bool hovered = ImGui::IsItemHovered();
                    bool active = ImGui::IsItemActive();
                    if (hovered || active)
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    if (active) {
                        float delta = ImGui::GetIO().MouseDelta.x;
                        if (delta != 0.0f) {
                            g_CharListWidth += delta;
                            g_CharListWidth = (g_CharListWidth < 120.0f) ? 120.0f : (g_CharListWidth > availW - 200.0f) ? availW - 200.0f : g_CharListWidth;
                        }
                    }
                    if (ImGui::IsItemDeactivated()) {
                        SaveSettings();
                    }
                    // Visual: draw a subtle line
                    ImU32 lineCol = (hovered || active)
                        ? IM_COL32(180, 160, 80, 180)
                        : IM_COL32(80, 75, 60, 100);
                    float cx = pos.x + splitterW * 0.5f;
                    ImGui::GetWindowDrawList()->AddLine(
                        ImVec2(cx, pos.y + 4), ImVec2(cx, pos.y + h - 4),
                        lineCol, 2.0f);
                }
                ImGui::SameLine();

                // Detail panel — resolve through display order
                int selRealIdx = -1;
                if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size())
                    selRealIdx = g_CharDisplayOrder[g_SelectedCharIdx];

                ImGui::BeginChild("CharDetail", ImVec2(0, 0), true);
                if (selRealIdx >= 0 && selRealIdx < (int)characters.size()) {
                    const auto& ch = characters[selRealIdx];

                    // Fetch details on-demand when character is selected
                    if (!g_DetailsFetched) {
                        FetchDetailsForCharacter(ch);
                        g_DetailsFetched = true;
                    }

                    // Character header — avatar + name + meta + right-aligned profession emblem
                    {
                        ImVec4 profColor = GetProfessionColor(ch.profession);
                        const float avatarSize = 46.0f;
                        const float emblemSize = 46.0f;
                        float startY = ImGui::GetCursorPosY();
                        float startX = ImGui::GetCursorPosX();
                        float availW = ImGui::GetContentRegionAvail().x;

                        // Avatar (left)
                        ImGui::BeginGroup();
                        RenderCharacterAvatar(ch, avatarSize);
                        ImGui::EndGroup();
                        ImGui::SameLine(0, 10);

                        // Name + meta (middle) — clip width so it doesn't overlap the emblem
                        float middleW = availW - avatarSize - 10.0f - emblemSize - 8.0f;
                        ImGui::BeginGroup();
                        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + middleW);
                        ImGui::SetCursorPosY(startY + 2.0f);
                        ImGui::TextColored(profColor, "%s", ch.name.c_str());

                        char meta[160];
                        std::string raceUp = ch.race;
                        std::string profUp = ch.profession;
                        for (auto& c : raceUp) c = (char)toupper((unsigned char)c);
                        for (auto& c : profUp) c = (char)toupper((unsigned char)c);
                        snprintf(meta, sizeof(meta), "LV %d  \xe2\x80\xa2  %s  \xe2\x80\xa2  %s",
                            ch.level, raceUp.c_str(), profUp.c_str());
                        ImGui::TextColored(ImVec4(0.55f, 0.50f, 0.40f, 1.0f), "%s", meta);
                        ImGui::PopTextWrapPos();
                        ImGui::EndGroup();

                        // Profession emblem (right) — fetched from GW2 wiki via IconManager
                        ImGui::SameLine();
                        ImGui::SetCursorPosX(startX + availW - emblemSize);
                        ImGui::SetCursorPosY(startY);
                        RenderProfessionEmblem(ch.profession, emblemSize);

                        // Make sure cursor sits below the row
                        float endY = ImGui::GetCursorPosY();
                        float minY = startY + avatarSize + 4.0f;
                        if (endY < minY) ImGui::SetCursorPosY(minY);
                        else ImGui::SetCursorPosY(endY + 2.0f);
                    }

                    // Secondary info row: age / birthday / last login / crafting — all dim, single horizontal flow.
                    // Crafting professions render as small icons + level so the row fits all 9 disciplines without overflow.
                    {
                        int bdays = DaysUntilBirthday(ch.created);
                        int age = CharacterAgeYears(ch.created);
                        bool birthdayToday = (bdays == 0 && age >= 0);

                        if (birthdayToday) {
                            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
                                "\xe2\x9c\xa6 Happy Birthday! Turning %d today!", age + 1);
                        } else {
                            const ImVec4 dim(0.50f, 0.47f, 0.40f, 1.0f);
                            const ImVec4 dot(0.40f, 0.37f, 0.30f, 1.0f);
                            bool first = true;
                            float iconSize = ImGui::GetTextLineHeight();
                            auto Sep = [&]() {
                                ImGui::SameLine(0, 6);
                                ImGui::TextColored(dot, "\xc2\xb7");
                                ImGui::SameLine(0, 6);
                            };
                            // Render an item-icon + value pair. Falls back to a
                            // text label if the icon hasn't loaded yet.
                            auto IconValue = [&](uint32_t iconId, const char* url,
                                                 const char* tooltip, const char* fmt,
                                                 auto value) {
                                Texture_t* tex = AlterEgo::IconManager::GetIcon(iconId);
                                if ((!tex || !tex->Resource) && url) {
                                    AlterEgo::IconManager::RequestIcon(iconId, url);
                                }
                                if (tex && tex->Resource) {
                                    ImGui::Image(tex->Resource, ImVec2(iconSize, iconSize));
                                } else {
                                    ImGui::TextColored(dim, "%s", tooltip);
                                }
                                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
                                ImGui::SameLine(0, 4);
                                ImGui::TextColored(dim, fmt, value);
                            };
                            if (age >= 0) {
                                IconValue(43766,
                                    "https://render.guildwars2.com/file/1932B731E2F70F2F1E3D453A4B7C26B24CF647C0/603246.png",
                                    "Age", "%dy", age);
                                first = false;
                            }
                            if (bdays > 0) {
                                if (!first) Sep(); else first = false;
                                IconValue(98501,
                                    "https://render.guildwars2.com/file/5C759EC1C95F3BE53C167A7D9F0D27BF3AE56277/625611.png",
                                    "Birthday", "%dd away", bdays);
                            }
                            auto tsIt = g_LoginTimestamps.find(ch.name);
                            if (tsIt != g_LoginTimestamps.end()) {
                                time_t now_t = std::time(nullptr);
                                int elapsed = (int)difftime(now_t, (time_t)tsIt->second);
                                std::string ago;
                                if (elapsed < 60) ago = "Just now";
                                else if (elapsed < 3600) ago = std::to_string(elapsed / 60) + "m ago";
                                else if (elapsed < 86400) ago = std::to_string(elapsed / 3600) + "h ago";
                                else ago = std::to_string(elapsed / 86400) + "d ago";
                                if (!first) Sep(); else first = false;
                                IconValue(68326,
                                    "https://render.guildwars2.com/file/01E7C8B0EB04F6E0CB25AA0403C96E0355B63A39/924581.png",
                                    "Last login", "%s", ago.c_str());
                            }
                            if (!ch.crafting.empty()) {
                                if (!first) Sep(); else first = false;
                                float iconSize = ImGui::GetTextLineHeight();
                                for (size_t ci = 0; ci < ch.crafting.size(); ci++) {
                                    if (ci > 0) ImGui::SameLine(0, 8);
                                    const std::string& disc = ch.crafting[ci];
                                    uint32_t iconId = GetCraftingIconId(disc);
                                    const char* iconUrl = GetCraftingIconUrl(disc);
                                    Texture_t* tex = AlterEgo::IconManager::GetIcon(iconId);
                                    if ((!tex || !tex->Resource) && iconUrl) {
                                        AlterEgo::IconManager::RequestIcon(iconId, iconUrl);
                                    }
                                    if (tex && tex->Resource) {
                                        ImGui::Image(tex->Resource, ImVec2(iconSize, iconSize));
                                    } else {
                                        ImGui::TextColored(dim, "%.3s", disc.c_str());
                                    }
                                    if (ImGui::IsItemHovered()) {
                                        ImGui::SetTooltip("%s", disc.c_str());
                                    }
                                    ImGui::SameLine(0, 3);
                                    int lvl = (ci < ch.crafting_levels.size()) ? ch.crafting_levels[ci] : 0;
                                    ImGui::TextColored(dim, "%d", lvl);
                                }
                            }
                        }
                    }

                    ImGui::Spacing();
                    ImGui::Separator();

                    // Tab bar: Equipment | Build
                    if (ImGui::BeginTabBar("##detail_tabs")) {
                        if (ImGui::BeginTabItem("Equipment")) {
                            g_SelectedTab = 0;
                            ImGui::BeginChild("EquipScroll", ImVec2(0, 0), false);
                            RenderEquipmentPanel(ch);
                            ImGui::EndChild();
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Build")) {
                            g_SelectedTab = 1;
                            ImGui::BeginChild("BuildScroll", ImVec2(0, 0), false);
                            RenderBuildPanel(ch);
                            ImGui::EndChild();
                            ImGui::EndTabItem();
                        }
                        // Hero Challenges tab hidden — GW2 API /heropoints returns [] for all characters (upstream bug)
                        // if (ImGui::BeginTabItem("Hero Challenges")) {
                        //     g_SelectedTab = 2;
                        //     ImGui::BeginChild("HeroChallengeScroll", ImVec2(0, 0), false);
                        //     RenderHeroChallengesPanel(ch);
                        //     ImGui::EndChild();
                        //     ImGui::EndTabItem();
                        // }
                        ImGui::EndTabBar();
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a character from the list.");
                }
                ImGui::EndChild();
            } else {
                // Empty state: no cached character data yet (new user)
                // Use same width as normal character list panel
                float availW = ImGui::GetContentRegionAvail().x;
                g_CharListWidth = (g_CharListWidth < 120.0f) ? 120.0f : (g_CharListWidth > availW - 200.0f) ? availW - 200.0f : g_CharListWidth;

                ImGui::BeginChild("CharListEmpty", ImVec2(g_CharListWidth, 0), true);

                // Refresh button (opens the standard refresh popup when names are available)
                {
                    bool refreshDisabled = scanning || !hoardReady;
                    if (refreshDisabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                    if (ImGui::Button("Refresh", ImVec2(-1, 0)) && !refreshDisabled) {
                        g_RefreshListFetching = true;
                        AlterEgo::GW2API::RequestCharacterList();
                    }
                    if (refreshDisabled) ImGui::PopStyleVar();
                }

                // Status messages
                if (fetchStatus == AlterEgo::FetchStatus::InProgress) {
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s",
                        AlterEgo::GW2API::GetFetchStatusMessage().c_str());
                } else if (fetchStatus == AlterEgo::FetchStatus::Error) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s",
                        AlterEgo::GW2API::GetFetchStatusMessage().c_str());
                } else if (!pendingNames.empty()) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                        "Click Refresh to fetch full data.");
                } else if (hoardReady) {
                    RenderSpinner("Loading character list...");
                } else if (hoardStatus == AlterEgo::HoardStatus::Unknown) {
                    RenderSpinner("Waiting for Hoard & Seek...");
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                        "Hoard & Seek required for character data.");
                }

                // Show pending character names grouped by account
                if (!pendingNames.empty() && AlterEgo::GW2API::IsMultiAccount()) {
                    ImGui::Separator();
                    const auto& accounts = AlterEgo::GW2API::GetAccounts();
                    // Sort so current account appears first
                    const std::string& currentAcct = AlterEgo::GW2API::GetCurrentAccountName();
                    std::vector<int> acctOrder(accounts.size());
                    std::iota(acctOrder.begin(), acctOrder.end(), 0);
                    std::stable_partition(acctOrder.begin(), acctOrder.end(),
                        [&](int i) { return accounts[i].name == currentAcct; });
                    for (int ai : acctOrder) {
                        const auto& acct = accounts[ai];
                        if (!g_SelectedAccountFilter.empty() && acct.name != g_SelectedAccountFilter)
                            continue;
                        // Collect chars for this account
                        bool hasChars = false;
                        for (size_t i = 0; i < pendingNames.size(); i++) {
                            if (AlterEgo::GW2API::GetAccountForCharacter(pendingNames[i]) == acct.name) {
                                if (!hasChars) {
                                    hasChars = true;
                                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.5f, 1.0f), "%s", acct.display_name.c_str());
                                }
                                ImGui::PushID((int)i);
                                ImGui::Selectable(pendingNames[i].c_str(), false, 0, ImVec2(0, 20.0f));
                                ImGui::PopID();
                            }
                        }
                    }
                } else if (!pendingNames.empty()) {
                    ImGui::Separator();
                    for (size_t i = 0; i < pendingNames.size(); i++) {
                        ImGui::PushID((int)i);
                        ImGui::Selectable(pendingNames[i].c_str(), false, 0, ImVec2(0, 20.0f));
                        ImGui::PopID();
                    }
                }

                ImGui::EndChild();

                // Detail panel placeholder (right side)
                ImGui::SameLine();
                ImGui::BeginChild("CharDetailEmpty", ImVec2(0, 0), true);
                if (!pendingNames.empty()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                        "Click Refresh and select characters to fetch their data.");
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                        "Waiting for character list...");
                }
                ImGui::EndChild();
            }

            // When the character list fetch completes, populate the refresh popup
            if (g_RefreshListFetching &&
                AlterEgo::GW2API::GetFetchStatus() != AlterEgo::FetchStatus::InProgress) {
                g_RefreshListFetching = false;
                const auto& names = AlterEgo::GW2API::GetPendingCharNames();
                if (!names.empty()) {
                    // Filter by selected account if applicable
                    if (!g_SelectedAccountFilter.empty()) {
                        g_RefreshNames.clear();
                        for (const auto& n : names) {
                            if (AlterEgo::GW2API::GetAccountForCharacter(n) == g_SelectedAccountFilter)
                                g_RefreshNames.push_back(n);
                        }
                    } else {
                        g_RefreshNames = names;
                    }
                    g_RefreshSelection.assign(g_RefreshNames.size(), false);
                    const auto& chars = AlterEgo::GW2API::GetCharacters();
                    std::string currentName;
                    if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                        int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                        if (ri >= 0 && ri < (int)chars.size()) currentName = chars[ri].name;
                    }
                    for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                        if (g_RefreshNames[i] == currentName)
                            g_RefreshSelection[i] = true;
                    }
                    g_RefreshPopupOpen = true;
                    ImGui::OpenPopup("Select Characters to Refresh");
                }
            }

            // Character refresh selection popup
            if (ImGui::BeginPopup("Select Characters to Refresh")) {
                ImGui::Text("Select characters to refresh:");
                ImGui::Spacing();

                if (ImGui::SmallButton("All")) {
                    for (size_t i = 0; i < g_RefreshSelection.size(); i++)
                        g_RefreshSelection[i] = true;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("None")) {
                    for (size_t i = 0; i < g_RefreshSelection.size(); i++)
                        g_RefreshSelection[i] = false;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Lv80")) {
                    const auto& cachedChars = AlterEgo::GW2API::GetCharacters();
                    for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                        g_RefreshSelection[i] = false;
                        for (const auto& cc : cachedChars) {
                            if (cc.name == g_RefreshNames[i] && cc.level == 80) {
                                g_RefreshSelection[i] = true;
                                break;
                            }
                        }
                    }
                }

                ImGui::Separator();

                ImGui::BeginChild("##charCheckList", ImVec2(280, 300), true);
                const auto& cachedChars = AlterEgo::GW2API::GetCharacters();
                for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                    ImGui::PushID((int)i);
                    bool checked = g_RefreshSelection[i];

                    const AlterEgo::Character* cached = nullptr;
                    for (const auto& cc : cachedChars) {
                        if (cc.name == g_RefreshNames[i]) { cached = &cc; break; }
                    }

                    if (ImGui::Checkbox("##cb", &checked))
                        g_RefreshSelection[i] = checked;
                    ImGui::SameLine();
                    if (cached) {
                        ImGui::TextColored(GetProfessionColor(cached->profession), "%s", g_RefreshNames[i].c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Lv%d", cached->level);
                    } else {
                        ImGui::Text("%s", g_RefreshNames[i].c_str());
                    }
                    ImGui::PopID();
                }
                ImGui::EndChild();

                int selCount = 0;
                for (bool b : g_RefreshSelection) { if (b) selCount++; }

                ImGui::Spacing();
                char btnLabel[64];
                snprintf(btnLabel, sizeof(btnLabel), "Refresh Selected (%d)", selCount);
                bool canRefresh = (selCount > 0);
                if (!canRefresh) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                if (ImGui::Button(btnLabel) && canRefresh) {
                    std::vector<std::string> selected;
                    const auto& chars = AlterEgo::GW2API::GetCharacters();
                    std::string currentName;
                    if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                        int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                        if (ri >= 0 && ri < (int)chars.size()) currentName = chars[ri].name;
                    }

                    for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                        if (g_RefreshSelection[i] && g_RefreshNames[i] == currentName) {
                            selected.push_back(g_RefreshNames[i]);
                            break;
                        }
                    }
                    for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                        if (g_RefreshSelection[i] && g_RefreshNames[i] != currentName)
                            selected.push_back(g_RefreshNames[i]);
                    }

                    AlterEgo::GW2API::RequestCharacterRefreshSelected(selected);
                    g_DetailsFetched = false;
                    g_RefreshPopupOpen = false;
                    ImGui::CloseCurrentPopup();
                }
                if (!canRefresh) ImGui::PopStyleVar();
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    g_RefreshPopupOpen = false;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Build Library")) {
            g_MainTab = 1;
            RenderBuildLibrary();
            ImGui::EndTabItem();
        }

        // Disable account-scoped tabs when viewing "All Accounts"
        bool acctTabsDisabled = AlterEgo::GW2API::IsMultiAccount() && g_SelectedAccountFilter.empty();
        const char* acctTabTooltip = "Select a specific account to use this tab.";

        ImGuiTabItemFlags skinTabFlags = 0;
        if (g_SwitchToSkinventory && !acctTabsDisabled) {
            skinTabFlags = ImGuiTabItemFlags_SetSelected;
            g_SwitchToSkinventory = false;
        }
        if (acctTabsDisabled) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
            ImGui::TabItemButton("Skinventory", ImGuiTabItemFlags_NoReorder);
            ImGui::PopStyleVar();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", acctTabTooltip);
        } else if (g_SkinInitialized && ImGui::BeginTabItem("Skinventory", nullptr, skinTabFlags)) {
            g_MainTab = 2;
            RenderSkinventory();
            ImGui::EndTabItem();
        }

        if (acctTabsDisabled) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
            ImGui::TabItemButton("Vault & Clears", ImGuiTabItemFlags_NoReorder);
            ImGui::PopStyleVar();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", acctTabTooltip);
        } else if (ImGui::BeginTabItem("Vault & Clears")) {
            g_MainTab = 3;
            RenderClears();
            ImGui::EndTabItem();
        }

        if (acctTabsDisabled) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
            ImGui::TabItemButton("Achievements", ImGuiTabItemFlags_NoReorder);
            ImGui::PopStyleVar();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", acctTabTooltip);
        } else if (ImGui::BeginTabItem("Achievements")) {
            g_MainTab = 4;
            RenderAchievements();
            ImGui::EndTabItem();
        }

        // Right-aligned account selector on the same row as the tab strip.
        // Drawn after tab items so it overlays the empty right-hand portion of
        // the tab bar row (which has no clickable items there).
        if (AlterEgo::GW2API::IsMultiAccount()) {
            const auto& accounts = AlterEgo::GW2API::GetAccounts();
            std::string currentLabel = "All Accounts";
            for (const auto& acct : accounts) {
                if (acct.name == g_SelectedAccountFilter) {
                    currentLabel = acct.display_name;
                    break;
                }
            }
            const float comboWidth = 180.0f;
            float afterY = ImGui::GetCursorPosY();
            ImGui::SetCursorPos(ImVec2(
                ImGui::GetContentRegionMax().x - comboWidth - ImGui::GetStyle().ItemSpacing.x,
                tabRowY - 3.0f));
            if (RenderThemedCombo("##AccountSelect", currentLabel.c_str(), comboWidth)) {
                std::string prevFilter = g_SelectedAccountFilter;
                if (ImGui::Selectable("All Accounts", g_SelectedAccountFilter.empty())) {
                    g_SelectedAccountFilter.clear();
                    if (g_MainTab >= 2) g_ForceCharactersTab = true;
                }
                for (const auto& acct : accounts) {
                    bool selected = (g_SelectedAccountFilter == acct.name);
                    if (ImGui::Selectable(acct.display_name.c_str(), selected)) {
                        g_SelectedAccountFilter = acct.name;
                    }
                }
                if (g_SelectedAccountFilter != prevFilter && !g_SelectedAccountFilter.empty()) {
                    Skinventory::OwnedSkins::SetAccountName(g_SelectedAccountFilter);
                    g_SkinRefreshOwned = true;
                    g_ClearsNeedRequery = true;
                    g_VaultNeedRequery = true;
                    g_AchNeedRequery = true;
                }
                ImGui::EndCombo();
            }
            ImGui::SetCursorPosY(afterY);
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

// --- Options/Settings Render ---

void AddonOptions() {
    ThemeGuard themeGuard;
    // Header with links
    ImGui::Text("Alter Ego");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
    ImGui::SameLine();
    if (RenderChipButton("Homepage", false)) {
        ShellExecuteA(NULL, "open", "https://pie.rocks.cc/projects/alter-ego/", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
    ImGui::SameLine();
    if (RenderChipButton("Buy me a coffee!", false)) {
        ShellExecuteA(NULL, "open", "https://ko-fi.com/pieorcake", NULL, NULL, SW_SHOWNORMAL);
    }

    // H&S connection status
    RenderSectionHeader("Data Source: Hoard & Seek", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
    auto hoardStatus = AlterEgo::GW2API::GetHoardStatus();
    switch (hoardStatus) {
        case AlterEgo::HoardStatus::Unknown:
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status: Checking...");
            break;
        case AlterEgo::HoardStatus::Unavailable:
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Status: Not detected");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Hoard & Seek is required. Install it from the Nexus addon library.");
            break;
        case AlterEgo::HoardStatus::Available:
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Status: Connected");
            break;
        case AlterEgo::HoardStatus::PermPending:
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Status: Permission pending");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Accept the Alter Ego permission popup in Hoard & Seek.");
            break;
        case AlterEgo::HoardStatus::PermDenied:
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Status: Permission denied");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Re-enable in H&S settings under Permissions.");
            if (RenderGoldButton("Retry Connection")) {
                AlterEgo::GW2API::PingHoard();
            }
            break;
        case AlterEgo::HoardStatus::Ready:
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Status: Ready");
            break;
    }

    ImGui::Spacing();
    RenderSectionHeader("UI Settings", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
    if (ImGui::Checkbox("Show Quick Access icon", &g_ShowQAIcon)) {
        if (g_ShowQAIcon) {
            APIDefs->QuickAccess_Add(QA_ID, TEX_ICON, TEX_ICON_HOVER, "KB_ALTER_EGO_TOGGLE", "Alter Ego");
        } else {
            APIDefs->QuickAccess_Remove(QA_ID);
        }
        SaveSettings();
    }
    ImGui::Spacing();
    RenderSectionHeader("Chat Build Detection", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Requires 'Events: Chat' addon from the Nexus library.");
    if (ImGui::Checkbox("Detect build links in chat", &g_ChatBuildDetection)) {
        SaveSettings();
    }
    if (g_ChatBuildDetection) {
        ImGui::Indent(16.0f);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
            "Drag the toast notification to reposition it.");
        if (RenderChipButton("Reset Toast Position", false)) {
            g_ToastPosX = -1.0f;
            g_ToastPosY = 100.0f;
            g_ToastPosInitialized = false;
            SaveSettings();
        }
        ImGui::Unindent(16.0f);
    }
    ImGui::Spacing();
    RenderSectionHeader("Character List", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
    bool expanded = !g_CompactCharList;
    if (ImGui::Checkbox("Expanded mode", &expanded)) {
        g_CompactCharList = !expanded;
        SaveSettings();
    }
    ImGui::TextColored(ImVec4(0.55f, 0.53f, 0.45f, 1.0f),
        "Columns shown in expanded mode:");
    ImGui::Indent(16.0f);
    if (g_CompactCharList) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.45f);
    }
    if (ImGui::Checkbox("Crafting Profession Icons", &g_ShowCraftingIcons)) {
        SaveSettings();
    }
    if (ImGui::Checkbox("Age", &g_ShowAge)) {
        SaveSettings();
    }
    if (ImGui::Checkbox("Playtime", &g_ShowPlaytime)) {
        SaveSettings();
    }
    if (ImGui::Checkbox("Last Login", &g_ShowLastLogin)) {
        SaveSettings();
    }
    ImGui::Text("Next Birthday");
    ImGui::SameLine();
    if (ImGui::RadioButton("Always", g_BirthdayMode == 0)) {
        g_BirthdayMode = 0;
        SaveSettings();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("A week out", g_BirthdayMode == 1)) {
        g_BirthdayMode = 1;
        SaveSettings();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Never", g_BirthdayMode == 2)) {
        g_BirthdayMode = 2;
        SaveSettings();
    }
    if (g_CompactCharList) {
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
    }
    ImGui::Unindent(16.0f);
}

// --- Export: GetAddonDef ---

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef() {
    AddonDef.Signature = 0xA17E3E90;  // Unique ID for Alter Ego
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "Alter Ego";
    AddonDef.Version.Major = V_MAJOR;
    AddonDef.Version.Minor = V_MINOR;
    AddonDef.Version.Build = V_BUILD;
    AddonDef.Version.Revision = V_REVISION;
    AddonDef.Author = "PieOrCake.7635";
    AddonDef.Description = "Manage characters, builds, skins, clears, achievements. Requires Hoard & Seek addon for account data retrieval. Optional - Events:Chat and Events:Alerts";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = AF_None;
    AddonDef.Provider = UP_GitHub;
    AddonDef.UpdateLink = "https://github.com/PieOrCake/alter_ego";

    return &AddonDef;
}
