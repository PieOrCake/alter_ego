#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <set>
#include <mutex>
#include <thread>
#include <fstream>
#include <filesystem>
#include <chrono>

#include "nexus/Nexus.h"
#include "imgui.h"
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

// Version constants
#define V_MAJOR 0
#define V_MINOR 9
#define V_BUILD 3
#define V_REVISION 0

// Quick Access icon identifiers
#define QA_ID "QA_ALTER_EGO"
#define TEX_ICON "TEX_ALTER_EGO_ICON"
#define TEX_ICON_HOVER "TEX_ALTER_EGO_ICON_HOVER"

// --- Normal icon ---
static const unsigned char ICON_NORMAL[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a,
    0xf4, 0x00, 0x00, 0x09, 0xdc, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xed, 0x56, 0x69, 0x8c, 0x1b,
    0xe5, 0x19, 0x7e, 0xe6, 0xf0, 0xcc, 0x78, 0x6c, 0x8f, 0xd7, 0xf6, 0xc6, 0xde, 0xcb, 0x7b, 0x78,
    0x37, 0xbb, 0x21, 0xc9, 0x66, 0x13, 0x9a, 0x04, 0x9a, 0x08, 0x12, 0x8a, 0x20, 0xb4, 0x3f, 0x08,
    0x2a, 0xbd, 0x54, 0x51, 0x89, 0x4a, 0xf4, 0x07, 0x95, 0x68, 0x7f, 0xb4, 0x52, 0x25, 0x04, 0x7f,
    0x4a, 0xa9, 0x2a, 0x95, 0x42, 0x4f, 0xf5, 0x12, 0x55, 0x1b, 0xa4, 0x14, 0x44, 0x2f, 0x09, 0x11,
    0x71, 0x25, 0x85, 0xb0, 0x09, 0x39, 0x76, 0x43, 0x9c, 0xcd, 0xee, 0x66, 0x2f, 0x7b, 0xd7, 0x5e,
    0x7b, 0x7d, 0x8c, 0xaf, 0xb1, 0xc7, 0xf6, 0xdc, 0xd5, 0x0c, 0x02, 0xe5, 0x28, 0xc9, 0xaf, 0xfe,
    0xeb, 0x2b, 0x59, 0xfe, 0x66, 0xfc, 0xf9, 0x7b, 0x9f, 0xef, 0x7d, 0xdf, 0xe7, 0x79, 0x5f, 0xe0,
    0xff, 0x76, 0x9d, 0xbd, 0xfc, 0xe2, 0xf3, 0xfd, 0xf8, 0x1f, 0xd8, 0xa1, 0x03, 0x7b, 0xc9, 0xbf,
    0xfe, 0xf1, 0xb9, 0xae, 0xeb, 0xdf, 0x13, 0x57, 0x3f, 0xbc, 0x76, 0xf4, 0xf7, 0xcf, 0x78, 0x04,
    0xe1, 0x3b, 0xf9, 0xcc, 0xfa, 0x8e, 0x40, 0xe7, 0x26, 0x2a, 0x3a, 0xba, 0xad, 0x51, 0xc8, 0xa4,
    0xfc, 0x55, 0x31, 0xb7, 0xa5, 0xd9, 0x6c, 0x7a, 0x85, 0x8e, 0xd0, 0xe5, 0xad, 0xb7, 0xdf, 0xb9,
    0x38, 0x3c, 0xb2, 0x55, 0xb9, 0xfe, 0xa0, 0xb2, 0x98, 0x27, 0x2f, 0x5f, 0x9a, 0x0e, 0xd4, 0xf2,
    0x99, 0x5e, 0x86, 0x65, 0x98, 0x60, 0x24, 0x9a, 0x1a, 0xde, 0xb2, 0xbd, 0x14, 0x0c, 0x85, 0x8d,
    0x5f, 0x3d, 0xf7, 0x14, 0x11, 0xee, 0x08, 0xfc, 0xc4, 0xed, 0xe6, 0x0f, 0x37, 0xa4, 0xca, 0x97,
    0xbf, 0xfe, 0xf8, 0x93, 0x33, 0x37, 0x00, 0x18, 0xea, 0xf3, 0x13, 0xbf, 0xfe, 0xd9, 0x0b, 0xa5,
    0xce, 0xae, 0xee, 0xc0, 0xd4, 0x07, 0x93, 0x6f, 0xf6, 0x0d, 0xf4, 0x8f, 0x55, 0x45, 0x71, 0x80,
    0xe7, 0x3d, 0x84, 0x8b, 0x65, 0xa0, 0xeb, 0x3a, 0xe4, 0x7a, 0x1d, 0x14, 0x45, 0xa9, 0x14, 0x49,
    0x9e, 0x26, 0x68, 0xfa, 0x48, 0xb1, 0xa9, 0xbf, 0x34, 0xda, 0xdf, 0x3b, 0xde, 0x96, 0xeb, 0xdf,
    0x35, 0x74, 0xf5, 0x7e, 0x82, 0x20, 0x22, 0xa6, 0x69, 0x91, 0x84, 0x65, 0x02, 0x04, 0x69, 0x91,
    0x24, 0xa1, 0xd5, 0x2a, 0x95, 0x8b, 0xfe, 0x60, 0x48, 0x6f, 0xd6, 0xa5, 0x7d, 0x04, 0x45, 0xc1,
    0xed, 0xf1, 0x3d, 0xf5, 0xd0, 0x23, 0x8f, 0x3f, 0xfb, 0xb1, 0x5f, 0xfa, 0xe3, 0x45, 0x34, 0xd4,
    0x45, 0xd7, 0x2a, 0x25, 0xaf, 0xaf, 0xc3, 0x8f, 0x6a, 0x4d, 0x3a, 0xd4, 0xa5, 0x5b, 0x60, 0xdd,
    0x3c, 0x92, 0x2b, 0xcb, 0xe8, 0x08, 0x75, 0x82, 0x65, 0x59, 0x28, 0xad, 0x36, 0x4c, 0xcb, 0x64,
    0x78, 0xaf, 0xf7, 0xa0, 0x5c, 0x29, 0x1d, 0x14, 0x7c, 0x81, 0x1f, 0x18, 0x6a, 0x6b, 0xd8, 0x30,
    0x74, 0xba, 0xdd, 0x56, 0x40, 0x53, 0x14, 0x6a, 0xd5, 0x1a, 0x74, 0x5d, 0x03, 0xcb, 0xd0, 0x84,
    0x47, 0xe8, 0x60, 0x34, 0xcd, 0xd8, 0x3b, 0x1b, 0x8f, 0xc3, 0x27, 0x08, 0xe8, 0xe9, 0xef, 0x07,
    0xef, 0xf1, 0x85, 0xae, 0x8e, 0x1c, 0xf9, 0xf1, 0xe2, 0x64, 0x7c, 0x41, 0xf3, 0xf8, 0x84, 0xb8,
    0x54, 0xab, 0x61, 0xc7, 0x8e, 0x71, 0x18, 0x9a, 0x0a, 0x7b, 0x5d, 0x97, 0x9b, 0x38, 0x3f, 0xf5,
    0x21, 0x08, 0x10, 0x68, 0x6b, 0x3a, 0x5a, 0xad, 0x16, 0x9a, 0xb2, 0x0c, 0x59, 0x6e, 0x21, 0xbb,
    0x9e, 0x1e, 0x33, 0x41, 0xd0, 0xba, 0x61, 0xc2, 0xc5, 0xb0, 0xb0, 0xbf, 0x6b, 0x52, 0x0d, 0xf9,
    0x62, 0x09, 0xeb, 0xd9, 0x22, 0x0c, 0xc3, 0x44, 0x5b, 0x51, 0x90, 0xcd, 0x17, 0x01, 0x92, 0x44,
    0xbb, 0xa5, 0x20, 0x97, 0x5d, 0x9f, 0xfa, 0xaf, 0x00, 0xa6, 0x4e, 0xbf, 0xf5, 0xa3, 0x68, 0x6c,
    0x64, 0xb7, 0xcf, 0x1f, 0x80, 0x69, 0x5a, 0xa8, 0x55, 0x44, 0x9c, 0x39, 0x73, 0x0e, 0x99, 0x8d,
    0x02, 0xb6, 0x6e, 0x1e, 0x80, 0x24, 0x55, 0xd1, 0x90, 0x65, 0xac, 0xa5, 0x33, 0x20, 0x49, 0x02,
    0xaa, 0xa2, 0x40, 0x14, 0x45, 0xb0, 0x6e, 0x37, 0x84, 0x40, 0x10, 0xf5, 0x86, 0x8c, 0xb5, 0x54,
    0x1a, 0x04, 0x49, 0xa1, 0x52, 0xad, 0x81, 0x64, 0x58, 0x14, 0x8b, 0x22, 0x3e, 0x8c, 0xcf, 0x40,
    0x10, 0x3c, 0xf0, 0xf9, 0x7c, 0xa0, 0x49, 0x20, 0x1c, 0x89, 0xbc, 0xf8, 0xfa, 0x3f, 0x8e, 0xdc,
    0x7b, 0x4d, 0x0a, 0x5e, 0xf9, 0xcb, 0xef, 0xb8, 0x86, 0x54, 0xff, 0xb6, 0xd2, 0x6a, 0xa1, 0x54,
    0x2a, 0xc1, 0xc5, 0x30, 0x58, 0x4e, 0xac, 0xc1, 0xd4, 0x34, 0x30, 0x1c, 0x87, 0xb6, 0x6a, 0x40,
    0x33, 0x64, 0xb4, 0x65, 0x19, 0xa1, 0xce, 0x4d, 0x48, 0x26, 0x56, 0x51, 0xab, 0x49, 0x28, 0x88,
    0x25, 0x34, 0x1b, 0x0d, 0x48, 0x75, 0x19, 0xb9, 0x5c, 0x0e, 0x8d, 0x6a, 0x05, 0x4a, 0xbb, 0x8d,
    0xc1, 0xa1, 0x11, 0xe8, 0x5a, 0x1b, 0x27, 0x26, 0xcf, 0xa2, 0x58, 0x10, 0xa1, 0xb4, 0x15, 0x70,
    0xac, 0x1b, 0x20, 0x29, 0xf4, 0xb4, 0x54, 0x2e, 0xda, 0x1f, 0x7d, 0x02, 0xc0, 0xf1, 0x4f, 0x00,
    0xf4, 0x44, 0xfb, 0xfa, 0x72, 0xeb, 0xa9, 0x40, 0xb5, 0x54, 0x42, 0x43, 0x6e, 0x82, 0xa4, 0x48,
    0x4c, 0x4c, 0xec, 0x00, 0x26, 0x26, 0x50, 0x16, 0x45, 0x3b, 0xd4, 0x10, 0xcb, 0x55, 0xa7, 0x66,
    0x49, 0x9a, 0x46, 0x6c, 0x30, 0x8a, 0xb5, 0xb5, 0x14, 0x16, 0xd6, 0x72, 0x48, 0x25, 0x93, 0xd0,
    0x54, 0x15, 0x52, 0xa5, 0x0c, 0x5d, 0x51, 0xb0, 0x63, 0xf7, 0x5e, 0xa7, 0xb2, 0xdf, 0x3d, 0x71,
    0x02, 0x9a, 0xa2, 0xe0, 0xfe, 0x03, 0x77, 0xc0, 0xb2, 0x2c, 0x27, 0x2d, 0x91, 0xae, 0x30, 0xc4,
    0x42, 0x1e, 0x3e, 0x0f, 0x17, 0xbb, 0x26, 0x05, 0xcf, 0xff, 0xf8, 0x99, 0x15, 0x58, 0x98, 0xec,
    0x8b, 0x8d, 0xa0, 0x5c, 0x95, 0x90, 0xcd, 0x64, 0x31, 0x7b, 0xe9, 0x12, 0x96, 0x17, 0xae, 0x20,
    0xb5, 0xb6, 0x86, 0x85, 0xe5, 0x55, 0xc4, 0xe7, 0x13, 0x88, 0xcf, 0x2d, 0x23, 0xbb, 0x91, 0x43,
    0xb5, 0x52, 0x01, 0x2f, 0x74, 0x40, 0xd1, 0x4c, 0xac, 0xa7, 0xd3, 0x70, 0xd1, 0x04, 0x46, 0x46,
    0x47, 0x31, 0xba, 0x7d, 0x1c, 0x1c, 0xc7, 0x61, 0x7e, 0x6e, 0x1e, 0x89, 0x54, 0x16, 0xba, 0xaa,
    0x42, 0x2c, 0x96, 0x20, 0xcb, 0x4d, 0xc8, 0x4d, 0x19, 0x52, 0x4d, 0x42, 0x70, 0x53, 0x04, 0x86,
    0x45, 0xfc, 0xed, 0x1a, 0x00, 0xff, 0x7c, 0xe7, 0xac, 0x35, 0x36, 0x3e, 0x71, 0xaa, 0x27, 0xda,
    0x8f, 0x07, 0x1f, 0x7e, 0xd8, 0x39, 0x0c, 0x94, 0x0b, 0x45, 0xa9, 0x89, 0x53, 0xd3, 0x97, 0x91,
    0xc8, 0x16, 0x21, 0x2b, 0x2a, 0xdc, 0x82, 0x17, 0x39, 0xb1, 0x8a, 0xc4, 0x6a, 0x1a, 0xed, 0x66,
    0x13, 0x16, 0x2c, 0xc4, 0xe7, 0x16, 0xc1, 0xf0, 0x3e, 0x74, 0xf7, 0xf5, 0x63, 0x7e, 0x7e, 0x01,
    0x1b, 0xd9, 0x2c, 0xce, 0x9c, 0xbb, 0x80, 0x56, 0x4b, 0x81, 0xdc, 0x6a, 0x23, 0xb5, 0x51, 0xc0,
    0xc2, 0x72, 0x02, 0xdd, 0x5d, 0xdd, 0xd8, 0x7f, 0xe0, 0x00, 0xb6, 0xef, 0xdc, 0x89, 0x91, 0x6d,
    0xdb, 0x4b, 0x37, 0x14, 0xa1, 0xae, 0x6b, 0x87, 0x0c, 0xc3, 0x00, 0x49, 0x92, 0xf0, 0xb8, 0x59,
    0x90, 0x84, 0x85, 0xc4, 0x4a, 0x02, 0x07, 0xf7, 0x7d, 0xc6, 0x79, 0xe7, 0x17, 0xbc, 0x60, 0x0c,
    0x15, 0x24, 0x41, 0x20, 0x2f, 0x56, 0x90, 0x4c, 0x6f, 0x80, 0x20, 0x08, 0xa4, 0x73, 0x65, 0xf0,
    0x5e, 0x01, 0x89, 0xc5, 0x05, 0x9c, 0x9f, 0x8e, 0xe3, 0xec, 0xd4, 0x05, 0x34, 0x5a, 0x6d, 0xa8,
    0x86, 0x09, 0xdd, 0xb4, 0xec, 0xe2, 0x47, 0x4b, 0xd1, 0xa1, 0x29, 0x6d, 0x50, 0x34, 0xed, 0x7c,
    0xd4, 0x56, 0xf3, 0xee, 0x1b, 0x74, 0x20, 0xb5, 0xbc, 0xc4, 0x64, 0xd2, 0x69, 0xd0, 0x14, 0x81,
    0x54, 0x2a, 0x85, 0x52, 0xbe, 0x88, 0x2d, 0x43, 0xfd, 0x10, 0x8b, 0x22, 0x04, 0xce, 0x05, 0x59,
    0x51, 0x90, 0x6e, 0x29, 0xd8, 0xb3, 0x7d, 0x14, 0xf1, 0x2b, 0x09, 0xe7, 0xf6, 0x36, 0xcd, 0x6c,
    0x7a, 0xae, 0x2d, 0x2f, 0x39, 0xec, 0x18, 0xbb, 0x6d, 0x0c, 0x95, 0x62, 0x01, 0x2c, 0x4d, 0x83,
    0x80, 0x0e, 0x17, 0x49, 0xa0, 0xc3, 0xe7, 0x85, 0x97, 0xe3, 0x50, 0xc8, 0x17, 0xb1, 0x70, 0xe9,
    0x22, 0xe4, 0x4a, 0x09, 0x9a, 0x81, 0xd6, 0x0d, 0x11, 0x90, 0xa5, 0x5a, 0xf2, 0xbd, 0xe3, 0xc7,
    0x61, 0x11, 0x24, 0xe6, 0xe6, 0x16, 0xc1, 0x7a, 0xbd, 0x18, 0xde, 0x1c, 0xc3, 0x8e, 0x5d, 0x3b,
    0x11, 0xe9, 0xe9, 0x81, 0x65, 0x9a, 0xce, 0x81, 0x8b, 0x2b, 0xab, 0x18, 0xe9, 0x09, 0xa0, 0xd3,
    0xcb, 0x3b, 0x11, 0xb0, 0xb9, 0x9f, 0xcb, 0xe7, 0xc1, 0xb9, 0xdd, 0x18, 0x8b, 0x0d, 0x3a, 0xb4,
    0x6d, 0x6b, 0x1a, 0x5c, 0x34, 0x05, 0xbf, 0xdf, 0x87, 0x0e, 0xbf, 0x80, 0x48, 0x64, 0x13, 0xca,
    0xb5, 0x3a, 0xe6, 0xe7, 0xae, 0xe0, 0xe4, 0xfb, 0xa7, 0x91, 0xcd, 0xa4, 0x17, 0xae, 0x91, 0xe2,
    0x0b, 0x1f, 0xbc, 0x1d, 0x59, 0x4f, 0x2e, 0x27, 0x15, 0xdd, 0x74, 0x7b, 0x78, 0x37, 0x28, 0xd6,
    0x03, 0xaf, 0xd7, 0x83, 0x4b, 0xe7, 0x4e, 0xa3, 0x5c, 0xad, 0xe3, 0xb5, 0x37, 0xfe, 0x0d, 0x4d,
    0x37, 0xa0, 0xa8, 0x2a, 0xb6, 0x0c, 0x75, 0x83, 0xe7, 0x58, 0xe4, 0xf2, 0x65, 0xe4, 0x2b, 0x0d,
    0x80, 0x24, 0x30, 0xd2, 0x13, 0xc2, 0xee, 0x3d, 0xb7, 0xe3, 0xdc, 0x99, 0x73, 0x58, 0xda, 0x28,
    0xc3, 0xe7, 0x66, 0x41, 0x11, 0x04, 0x28, 0x8a, 0xc6, 0xde, 0x5d, 0x5b, 0x31, 0x14, 0x1b, 0x44,
    0xef, 0xd0, 0x08, 0x72, 0xa9, 0x24, 0xdc, 0x5e, 0x1f, 0x18, 0x9a, 0x5c, 0x4d, 0xac, 0x66, 0x47,
    0x9f, 0x7c, 0xf6, 0x05, 0xcd, 0x89, 0x40, 0xa3, 0x5a, 0x7e, 0x54, 0xaa, 0x56, 0xdd, 0xbd, 0x83,
    0x31, 0x0c, 0xdf, 0x36, 0x81, 0x70, 0x24, 0x8c, 0x4c, 0x72, 0x09, 0x33, 0xf1, 0x19, 0xbc, 0xf7,
    0xfe, 0x07, 0x08, 0x05, 0x04, 0x50, 0xb6, 0xf8, 0xd8, 0x35, 0xc2, 0xf0, 0x20, 0x49, 0x1a, 0xba,
    0x61, 0x80, 0x65, 0x28, 0x08, 0x6e, 0x17, 0xea, 0x8d, 0x26, 0x96, 0x97, 0x93, 0x30, 0x2d, 0x0b,
    0x25, 0xa9, 0x65, 0xf7, 0x01, 0x28, 0xba, 0x89, 0xae, 0x48, 0x08, 0x17, 0x67, 0x17, 0xb0, 0xba,
    0xb2, 0x02, 0x86, 0xb4, 0xb0, 0xe7, 0xae, 0x83, 0xe8, 0xea, 0x8d, 0xa2, 0x98, 0xdf, 0x18, 0xbc,
    0x73, 0xdf, 0x9e, 0xcf, 0x7d, 0x92, 0x02, 0x82, 0xa2, 0xbf, 0x49, 0x33, 0x2c, 0x2a, 0xc5, 0x22,
    0x4c, 0x43, 0x43, 0x7a, 0x65, 0x01, 0x53, 0xa7, 0x26, 0xb1, 0xb8, 0x9a, 0x75, 0x44, 0xa8, 0x26,
    0x35, 0x1c, 0x9d, 0x9f, 0x18, 0x1d, 0x44, 0x5f, 0x38, 0x84, 0xe5, 0xb5, 0x0c, 0x5a, 0xaa, 0x0e,
    0xa9, 0xa5, 0x41, 0x6a, 0x6a, 0x68, 0xb4, 0x55, 0x2c, 0x25, 0x52, 0xe0, 0x39, 0x0e, 0xb1, 0xee,
    0x4e, 0x58, 0x16, 0xc0, 0x31, 0x2e, 0x6c, 0x14, 0xca, 0x0e, 0x55, 0xe7, 0x16, 0x92, 0x58, 0x9c,
    0xb9, 0x88, 0xb6, 0x5c, 0x87, 0xdc, 0xa8, 0x83, 0x20, 0x5d, 0xd0, 0x35, 0xfd, 0x11, 0xa7, 0x08,
    0xff, 0xfc, 0xdb, 0xe7, 0x42, 0x93, 0x27, 0xde, 0x19, 0xae, 0xcb, 0x0a, 0x60, 0xa8, 0x08, 0x75,
    0x86, 0x1c, 0x20, 0x95, 0x9a, 0xed, 0x94, 0x86, 0xcb, 0x45, 0xa2, 0xd5, 0x56, 0xc0, 0xba, 0x29,
    0x8c, 0x8f, 0xc5, 0x70, 0xfc, 0xe4, 0x79, 0x7c, 0xc4, 0x16, 0x0a, 0xba, 0x69, 0x82, 0x05, 0xe9,
    0xec, 0x33, 0x2c, 0x13, 0x1b, 0x62, 0x0d, 0xdb, 0x06, 0xbb, 0x21, 0x4a, 0x32, 0x8a, 0xd5, 0x06,
    0x28, 0x92, 0x02, 0x45, 0x91, 0x20, 0x08, 0x12, 0xf5, 0x7a, 0x03, 0x52, 0x59, 0xc4, 0xc9, 0xb7,
    0xdf, 0x02, 0xef, 0xf5, 0xa1, 0x5a, 0x2e, 0xef, 0x77, 0x22, 0xf0, 0xe8, 0xe3, 0xdf, 0x2f, 0xcd,
    0x4c, 0x7f, 0xd8, 0x37, 0x3b, 0x73, 0xf9, 0xe9, 0xbe, 0xd1, 0xad, 0x8e, 0x96, 0x5b, 0x96, 0x89,
    0x50, 0x30, 0x00, 0xaf, 0x9b, 0x01, 0xec, 0x7a, 0xb7, 0x4c, 0xe8, 0x9a, 0x81, 0x13, 0x93, 0xd3,
    0xd0, 0x2c, 0x0b, 0xba, 0x09, 0xe8, 0x86, 0xe5, 0x30, 0xc0, 0x2e, 0x44, 0x3b, 0x35, 0x76, 0x30,
    0x0d, 0x8b, 0xc0, 0x62, 0x2a, 0x87, 0xb2, 0x24, 0x7f, 0x54, 0xe1, 0x24, 0x01, 0x3f, 0xcf, 0xa2,
    0x37, 0x12, 0x04, 0xef, 0xe1, 0x1d, 0x6d, 0x09, 0xf5, 0xf4, 0x6b, 0xb3, 0x33, 0xf3, 0x0f, 0x2e,
    0xce, 0x5d, 0xb9, 0xe7, 0x93, 0x14, 0x1c, 0x7d, 0xfb, 0x4c, 0xfe, 0x8e, 0x3b, 0x76, 0x1d, 0x65,
    0x29, 0x28, 0xb9, 0x8d, 0x0d, 0x10, 0x94, 0xcb, 0x39, 0x40, 0x33, 0x0c, 0x68, 0x9a, 0xe6, 0x14,
    0x94, 0x9b, 0xa1, 0xc0, 0x30, 0xb4, 0x3d, 0x0f, 0x80, 0x75, 0xd9, 0xec, 0xb5, 0x40, 0x10, 0x00,
    0x4d, 0x91, 0xa0, 0x48, 0x12, 0x8c, 0x8b, 0x02, 0x49, 0x11, 0x60, 0x19, 0x17, 0x18, 0xfb, 0xd6,
    0xf6, 0x0e, 0xd3, 0x74, 0x6a, 0x87, 0xe3, 0x58, 0xb8, 0x5c, 0x0c, 0x24, 0xa9, 0x0e, 0xbf, 0x87,
    0x59, 0x1a, 0x8c, 0x76, 0x1e, 0x3b, 0x72, 0xec, 0xfd, 0xb5, 0x6b, 0x68, 0x98, 0x5b, 0x5d, 0x4d,
    0x87, 0x23, 0x91, 0x85, 0x4d, 0xe1, 0x30, 0x8a, 0x1b, 0x59, 0x14, 0xc4, 0x32, 0xda, 0x8a, 0xea,
    0x6c, 0xf0, 0xf3, 0x2e, 0x27, 0x1a, 0xaa, 0x6e, 0x80, 0x26, 0x49, 0x87, 0x62, 0x2e, 0x9a, 0x84,
    0x05, 0x38, 0x9d, 0xd3, 0x7e, 0xb6, 0x41, 0xd8, 0x82, 0x65, 0x7b, 0xf6, 0x7b, 0x58, 0xf8, 0x38,
    0xda, 0xd1, 0x14, 0x45, 0xd5, 0x9c, 0xfe, 0x52, 0x2e, 0x95, 0xc1, 0xbb, 0x19, 0x84, 0xbb, 0x22,
    0x47, 0x7e, 0xfe, 0xd2, 0xeb, 0xc6, 0x0d, 0x3a, 0xf0, 0xcb, 0x57, 0xde, 0xd4, 0x36, 0x8f, 0x6d,
    0x4b, 0xd8, 0x6b, 0xc3, 0x02, 0x2a, 0x55, 0x09, 0x2e, 0x82, 0x40, 0xc8, 0xcb, 0x22, 0xe8, 0xe3,
    0x51, 0x6b, 0xb6, 0x61, 0x9a, 0x00, 0x45, 0x00, 0x0c, 0x4d, 0xc1, 0xbe, 0xbe, 0xad, 0x8a, 0x36,
    0x08, 0x92, 0x20, 0xc1, 0xba, 0x28, 0x27, 0x1a, 0x8a, 0x6a, 0x38, 0xb7, 0x0f, 0xfb, 0x3d, 0xce,
    0x7f, 0x6d, 0x98, 0x92, 0x24, 0xa3, 0xd9, 0x6c, 0x81, 0xe5, 0x3c, 0xe8, 0x8f, 0x8d, 0x2d, 0x5d,
    0x3d, 0x0f, 0xd0, 0x57, 0x3f, 0xa4, 0x56, 0xae, 0x78, 0x93, 0xc9, 0x04, 0x66, 0x67, 0xe6, 0xc0,
    0x90, 0x04, 0x3a, 0x83, 0x1e, 0x7b, 0xb2, 0x41, 0xaa, 0x50, 0x83, 0x05, 0x12, 0x7e, 0x0f, 0x0d,
    0x53, 0xd7, 0xe1, 0xc0, 0xb7, 0x2c, 0x98, 0xa6, 0xe9, 0x00, 0xa0, 0x69, 0x1b, 0x00, 0x0d, 0x58,
    0x3a, 0x68, 0x86, 0x40, 0x45, 0x56, 0xc0, 0xb3, 0x34, 0x36, 0xf9, 0x79, 0x68, 0xaa, 0x8e, 0x46,
    0x5b, 0x43, 0x7a, 0x3d, 0x8b, 0xf9, 0xf8, 0x05, 0x0c, 0xc6, 0x86, 0x3a, 0x3e, 0x15, 0xc0, 0xeb,
    0xaf, 0xbe, 0xfc, 0x98, 0x6a, 0xe2, 0x89, 0x80, 0x97, 0x3d, 0xc4, 0x08, 0xec, 0x16, 0x4d, 0xd7,
    0xe9, 0x74, 0x49, 0x82, 0x8b, 0x63, 0xb1, 0xb9, 0x7b, 0x13, 0x4a, 0xa5, 0x0a, 0xea, 0xcd, 0x36,
    0x9a, 0x8a, 0x06, 0xd5, 0xae, 0x44, 0x9b, 0xc2, 0x84, 0x9d, 0x77, 0xda, 0x49, 0x83, 0x9d, 0x0e,
    0x3b, 0xec, 0x3e, 0xaf, 0x1b, 0x85, 0x72, 0xdd, 0xa1, 0x62, 0x50, 0x70, 0x5b, 0x81, 0x0e, 0x4f,
    0x59, 0x6e, 0x29, 0x6f, 0xc4, 0xcf, 0x4d, 0x9d, 0x97, 0xcb, 0xe5, 0xa3, 0x9f, 0x3a, 0x15, 0x5f,
    0x6d, 0x8f, 0x1e, 0xbe, 0xd7, 0x5f, 0xa9, 0x49, 0xb3, 0xaa, 0x49, 0xf4, 0xde, 0x77, 0xcf, 0x67,
    0x61, 0x28, 0x6d, 0x64, 0xd2, 0xeb, 0x90, 0x9b, 0x6d, 0x18, 0x96, 0x05, 0x9a, 0x65, 0x91, 0x17,
    0xab, 0x08, 0x87, 0xfc, 0x70, 0xd1, 0x34, 0x5a, 0x4d, 0xbb, 0xe5, 0x2a, 0xe0, 0x18, 0x1a, 0x7d,
    0x7d, 0x5d, 0x28, 0xd7, 0x64, 0xa4, 0x33, 0x39, 0xf4, 0x46, 0x02, 0x5f, 0x7b, 0xe8, 0x1b, 0xdf,
    0x7a, 0xf5, 0x0b, 0x0f, 0x7e, 0xe5, 0x23, 0xc4, 0xd7, 0x19, 0x81, 0x9b, 0xd8, 0x97, 0xee, 0xdb,
    0xf7, 0xaf, 0x89, 0x9d, 0xe3, 0x87, 0xed, 0x26, 0x23, 0x6e, 0x64, 0x51, 0x11, 0x45, 0x70, 0x5e,
    0x1f, 0xfa, 0x87, 0x47, 0x41, 0xbb, 0x68, 0xe4, 0xb2, 0x59, 0xa7, 0x4b, 0x6a, 0x9a, 0x0e, 0x31,
    0x9f, 0x77, 0x66, 0x07, 0x43, 0xd5, 0x10, 0x09, 0x07, 0xc1, 0x77, 0x84, 0x70, 0xf6, 0xec, 0xb4,
    0x3c, 0x30, 0xd0, 0xdd, 0xf5, 0xd3, 0x3f, 0xbc, 0xda, 0xf8, 0x34, 0x1f, 0xf4, 0xcd, 0x00, 0x8c,
    0x0d, 0x0f, 0xfc, 0x26, 0x36, 0x32, 0x72, 0xd8, 0x96, 0x5e, 0x8f, 0xe0, 0x87, 0xd7, 0x1f, 0x44,
    0x6c, 0xcb, 0x76, 0x70, 0x3c, 0x07, 0xcb, 0x30, 0xc0, 0x71, 0x3c, 0x02, 0xc1, 0x20, 0x34, 0x5d,
    0x47, 0xcf, 0x40, 0x0c, 0xe1, 0xde, 0x28, 0x32, 0xc9, 0x65, 0x27, 0x1d, 0x3e, 0xc1, 0x8b, 0xbe,
    0x9e, 0xf0, 0x2f, 0x6e, 0xe6, 0xfc, 0x96, 0x00, 0x82, 0x5d, 0xe1, 0xf7, 0xbc, 0x82, 0xaf, 0xac,
    0xa9, 0x6a, 0xb0, 0xa3, 0x33, 0x82, 0x70, 0x57, 0x0f, 0x7c, 0x82, 0x0f, 0xba, 0xda, 0x82, 0x45,
    0x02, 0x5e, 0x0f, 0xef, 0x4c, 0x43, 0x76, 0xd3, 0x61, 0x18, 0x1f, 0x38, 0xf7, 0x66, 0x67, 0x7c,
    0x97, 0xeb, 0x35, 0x7b, 0x9e, 0x50, 0x87, 0x06, 0xa2, 0xcf, 0xe3, 0x16, 0x46, 0xdc, 0x6a, 0xc3,
    0xb1, 0xbf, 0xff, 0x69, 0x92, 0x61, 0xb8, 0xfd, 0x42, 0x47, 0x00, 0x2c, 0xeb, 0x82, 0xae, 0xb4,
    0x9c, 0x39, 0x51, 0xcc, 0x17, 0x50, 0xad, 0x56, 0xc0, 0x72, 0x2c, 0xfc, 0xfe, 0x00, 0x82, 0x9d,
    0x9d, 0xe0, 0x3c, 0x5e, 0xe8, 0x16, 0x50, 0xab, 0x54, 0x20, 0x57, 0xcb, 0xab, 0x5f, 0x7d, 0xec,
    0x7b, 0x43, 0xb7, 0x3a, 0x9f, 0xbe, 0xd9, 0x8f, 0x77, 0xed, 0xde, 0x46, 0xf2, 0x1e, 0x21, 0x1a,
    0x08, 0x75, 0xc2, 0xd4, 0x14, 0x14, 0x32, 0xeb, 0x88, 0x4f, 0x4f, 0x63, 0x79, 0x71, 0x09, 0x86,
    0x69, 0x21, 0x99, 0x29, 0x38, 0x9a, 0x60, 0xe8, 0x06, 0x78, 0xce, 0x85, 0xfd, 0x77, 0xef, 0xc7,
    0xce, 0xdb, 0x77, 0x39, 0x93, 0x33, 0x4d, 0x92, 0x36, 0x43, 0x6f, 0x69, 0x37, 0x8d, 0xc0, 0xe9,
    0x77, 0x8f, 0x3d, 0xed, 0x13, 0x84, 0x1f, 0x2a, 0xcd, 0x06, 0x66, 0xa6, 0xce, 0xe2, 0xcc, 0xa9,
    0xd3, 0xa8, 0x56, 0x24, 0xb4, 0x14, 0x05, 0x26, 0xe3, 0xc6, 0xde, 0x5d, 0xe3, 0xd8, 0xc8, 0x15,
    0x30, 0x33, 0x7b, 0x05, 0x82, 0x9b, 0x71, 0x04, 0xac, 0x27, 0x1c, 0xc4, 0xbd, 0x0f, 0x3c, 0x80,
    0xd1, 0xf1, 0x9d, 0xed, 0x5a, 0xbd, 0xb1, 0xf5, 0xbe, 0xcf, 0x7f, 0x31, 0x79, 0x33, 0x1f, 0xff,
    0x01, 0xa2, 0xd8, 0xaf, 0xae, 0x47, 0x3e, 0xd0, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
    0x44, 0xae, 0x42, 0x60, 0x82,
};
static const unsigned int ICON_NORMAL_size = 2581;

// --- Hover icon ---
static const unsigned char ICON_HOVER[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a,
    0xf4, 0x00, 0x00, 0x0a, 0x23, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xed, 0x96, 0x59, 0x70, 0x5c,
    0x77, 0x95, 0xc6, 0x7f, 0x77, 0xeb, 0xbe, 0xb7, 0x17, 0xf5, 0x26, 0xb7, 0x76, 0xc9, 0xd6, 0x62,
    0x59, 0xb2, 0x22, 0x1b, 0x25, 0x24, 0x24, 0xb6, 0x13, 0x07, 0x92, 0x38, 0x24, 0x99, 0x49, 0x05,
    0x8a, 0x02, 0x66, 0xa8, 0x14, 0xa1, 0x28, 0x96, 0x29, 0x02, 0x45, 0xd5, 0x54, 0x51, 0x53, 0x15,
    0x5e, 0x86, 0xa5, 0xf2, 0x30, 0x33, 0x14, 0x54, 0xf1, 0x30, 0x0b, 0x05, 0x84, 0x21, 0x21, 0x81,
    0x81, 0xc4, 0x54, 0xbc, 0x84, 0x38, 0x9b, 0x6c, 0xe3, 0xdd, 0x96, 0x2d, 0x45, 0x52, 0xcb, 0xda,
    0xa5, 0x56, 0xab, 0xd5, 0x8b, 0x7a, 0xbd, 0xdd, 0x7d, 0xef, 0xed, 0x3b, 0xd5, 0x9d, 0x49, 0xca,
    0x8e, 0x27, 0xce, 0x13, 0x6f, 0x9c, 0xa7, 0xff, 0xbf, 0xef, 0xed, 0x73, 0xce, 0x3d, 0xdf, 0xf9,
    0xbe, 0x73, 0xe0, 0xaf, 0xf6, 0x3e, 0x8b, 0x9c, 0x3b, 0xd2, 0xc9, 0x5f, 0xc0, 0xbe, 0xf0, 0xd8,
    0x03, 0xe2, 0xf4, 0xe9, 0x43, 0xcd, 0xef, 0xff, 0x5d, 0xb8, 0xf6, 0x12, 0x9b, 0x3c, 0xf9, 0xbd,
    0x86, 0x60, 0xe8, 0x9b, 0xcb, 0xb3, 0x33, 0xc3, 0xe1, 0xd6, 0x76, 0xc9, 0xdf, 0xb5, 0x2b, 0x5f,
    0xde, 0x5c, 0xf4, 0x6d, 0x44, 0x17, 0x77, 0xe4, 0x72, 0x39, 0x4f, 0xb0, 0xb1, 0x65, 0xbc, 0xa9,
    0x67, 0x24, 0x22, 0x08, 0x42, 0xf9, 0xfd, 0x8e, 0x6c, 0xdb, 0x16, 0x2d, 0x3d, 0x19, 0x48, 0x2c,
    0xcf, 0xb6, 0xa9, 0x9a, 0xea, 0xf0, 0x75, 0xf4, 0x2d, 0x81, 0x96, 0x14, 0x04, 0xc1, 0x7a, 0xfd,
    0xd0, 0x2f, 0x85, 0xf6, 0xc6, 0xf0, 0xd3, 0x1e, 0x8f, 0xf7, 0xd1, 0xcd, 0x54, 0xfc, 0x33, 0x03,
    0x7b, 0x3f, 0x75, 0xe5, 0x86, 0x04, 0x06, 0x7b, 0x43, 0xc2, 0x9b, 0x87, 0x5f, 0x49, 0x36, 0x76,
    0x6d, 0x0d, 0xbc, 0x75, 0xe8, 0xe0, 0xd1, 0xbe, 0x81, 0xfe, 0xfe, 0x8d, 0x68, 0xb4, 0xcb, 0xeb,
    0xf5, 0x09, 0x0e, 0x4d, 0xc5, 0x30, 0x2a, 0x64, 0xd3, 0x69, 0x64, 0x59, 0xae, 0xc8, 0xa2, 0x74,
    0x52, 0x50, 0x94, 0x67, 0x56, 0x73, 0xc6, 0xaf, 0x3e, 0xd2, 0xdf, 0x73, 0x4b, 0x31, 0x9b, 0xfe,
    0x96, 0x69, 0x94, 0x1e, 0x10, 0x04, 0xb1, 0xc9, 0xaa, 0x56, 0x45, 0xb1, 0x5a, 0x05, 0x51, 0xb4,
    0x45, 0x51, 0x34, 0x92, 0xf1, 0xf8, 0xa5, 0x50, 0x53, 0xb3, 0x99, 0xdb, 0x4c, 0xdd, 0x25, 0x4a,
    0x32, 0x6e, 0x5f, 0xe0, 0xa9, 0xf6, 0x9d, 0x7b, 0x7f, 0xf0, 0x6e, 0x5c, 0xf9, 0xdd, 0xc3, 0xf6,
    0x96, 0x2e, 0x39, 0x11, 0x5f, 0xf3, 0x04, 0xb6, 0x84, 0xd8, 0x48, 0xa6, 0x0e, 0x74, 0x55, 0x6c,
    0x34, 0xb7, 0x97, 0x89, 0xcb, 0x63, 0x6c, 0x69, 0x69, 0x45, 0xd3, 0x34, 0xf4, 0x7c, 0x11, 0xcb,
    0xb6, 0x1c, 0x5e, 0x9f, 0x7f, 0x7f, 0x36, 0xbe, 0xb6, 0x3f, 0x18, 0x08, 0x7f, 0xc7, 0x2c, 0xe5,
    0x7b, 0x4c, 0xd3, 0x90, 0x0b, 0x05, 0x1d, 0x45, 0x96, 0x49, 0x26, 0x92, 0x18, 0x95, 0x32, 0x9a,
    0xaa, 0x08, 0x0d, 0xc1, 0x2d, 0x8e, 0x72, 0xc5, 0xbc, 0xfd, 0xd4, 0xe8, 0x28, 0x81, 0x50, 0x90,
    0xee, 0xfe, 0x7e, 0xbc, 0x0d, 0x81, 0xd0, 0xb5, 0x95, 0x13, 0xdf, 0x3d, 0xbc, 0x38, 0x7a, 0xc1,
    0xf0, 0x05, 0x82, 0x63, 0x35, 0x07, 0x7b, 0xf7, 0xee, 0xc1, 0xac, 0x94, 0x48, 0x25, 0x93, 0xa4,
    0xb3, 0x39, 0xfe, 0xf4, 0xea, 0x1b, 0x08, 0x08, 0x14, 0xca, 0x06, 0xf9, 0x7c, 0x81, 0x5c, 0x36,
    0x4b, 0x36, 0x5b, 0x60, 0xee, 0xea, 0x4c, 0xbf, 0x85, 0x28, 0x57, 0x4c, 0x0b, 0xa7, 0xaa, 0x61,
    0x98, 0x55, 0x92, 0xc9, 0x04, 0x4b, 0xab, 0x6b, 0x5c, 0x9d, 0x5b, 0xc5, 0x34, 0xab, 0x14, 0x8b,
    0x3a, 0xf3, 0x4b, 0xab, 0x20, 0x4a, 0x14, 0xf3, 0x3a, 0x8b, 0x73, 0x33, 0xe7, 0xfe, 0xdf, 0x04,
    0x6c, 0x3b, 0xf3, 0xfd, 0x96, 0x81, 0x5d, 0xb7, 0x05, 0x1a, 0xc3, 0x54, 0xad, 0x2a, 0xc9, 0x78,
    0x94, 0xc3, 0x87, 0x5f, 0x61, 0x76, 0x7e, 0x99, 0xdb, 0x77, 0x0f, 0x90, 0x4a, 0x25, 0xc8, 0x64,
    0xb3, 0x4c, 0x45, 0x66, 0x91, 0x24, 0x81, 0x92, 0x5e, 0x24, 0x1a, 0x8d, 0xe2, 0xf2, 0xb8, 0x09,
    0x85, 0x9b, 0x49, 0x67, 0xb2, 0x4c, 0x4d, 0x47, 0x10, 0x24, 0x99, 0xf8, 0x46, 0x12, 0x51, 0xd5,
    0x58, 0x5d, 0x8d, 0xf2, 0xc6, 0xe8, 0x09, 0x82, 0xa1, 0x06, 0x02, 0x81, 0x00, 0x8a, 0x08, 0x1d,
    0x9d, 0x9d, 0x3f, 0xdb, 0x58, 0xba, 0xfc, 0x89, 0xeb, 0x20, 0x98, 0x19, 0x3b, 0xa1, 0x56, 0x36,
    0xd3, 0xff, 0xa0, 0xe7, 0x0b, 0xac, 0xad, 0xad, 0xe1, 0x54, 0x55, 0xc6, 0xc6, 0xa7, 0xb0, 0x2a,
    0x65, 0x54, 0x97, 0x9b, 0x62, 0xd9, 0xa4, 0x62, 0x66, 0x28, 0x64, 0xb3, 0xb4, 0xb4, 0xb6, 0x31,
    0x71, 0xe5, 0x6d, 0x12, 0xc9, 0x14, 0x2b, 0xd1, 0x18, 0xb9, 0x4c, 0x86, 0x64, 0x3a, 0xc3, 0xe2,
    0xc2, 0x22, 0x9b, 0x89, 0x38, 0x7a, 0xa1, 0xc0, 0xc0, 0xce, 0x5d, 0x18, 0x95, 0x22, 0x2f, 0x1c,
    0x3c, 0xc2, 0xea, 0x4a, 0x14, 0xbd, 0xa8, 0xe3, 0xd2, 0x3c, 0x20, 0xca, 0x74, 0x17, 0x4a, 0xea,
    0xf6, 0xfe, 0xed, 0x4f, 0x02, 0xc7, 0xde, 0x4b, 0xa0, 0xbb, 0xaf, 0xb7, 0x7d, 0xf1, 0x6a, 0x24,
    0xb0, 0xb1, 0xb6, 0xc6, 0x66, 0x36, 0x87, 0x24, 0x8b, 0xec, 0xdb, 0xb7, 0x17, 0xf6, 0xed, 0x63,
    0x3d, 0x1a, 0x65, 0x6e, 0x26, 0x42, 0x74, 0x3d, 0x51, 0xef, 0x59, 0x49, 0x51, 0x18, 0x1a, 0xdc,
    0xce, 0xe4, 0xd4, 0x34, 0xe7, 0xa7, 0x16, 0x98, 0x9e, 0x98, 0xa0, 0x5c, 0x2a, 0x91, 0x8a, 0xc7,
    0xa8, 0xe8, 0x3a, 0x7b, 0xef, 0x3b, 0x50, 0xef, 0xec, 0xff, 0x79, 0xe1, 0x85, 0xfa, 0xfd, 0xef,
    0x1f, 0x7b, 0x10, 0xdb, 0xae, 0xb2, 0xb4, 0xb2, 0x46, 0xe7, 0xd6, 0x0e, 0xa2, 0xcb, 0xcb, 0x04,
    0x1a, 0xdc, 0xdd, 0xd7, 0x41, 0xf0, 0x8d, 0x2f, 0x3d, 0x3e, 0x8b, 0xcd, 0xf1, 0xde, 0x5b, 0x76,
    0xb1, 0xbe, 0x91, 0x62, 0x6e, 0x76, 0x9e, 0x53, 0xc7, 0x8f, 0x73, 0xf9, 0xfc, 0x39, 0xa6, 0x27,
    0xa7, 0x38, 0x7f, 0x79, 0x92, 0xd1, 0xb3, 0x57, 0x18, 0x3d, 0x3d, 0xc6, 0xdc, 0xc2, 0x22, 0x1b,
    0xf1, 0x38, 0x0d, 0xc1, 0x2d, 0xe8, 0xe5, 0x2a, 0x33, 0x91, 0x08, 0x4e, 0x45, 0x60, 0x78, 0x64,
    0x84, 0x91, 0x3b, 0xf7, 0xe0, 0x72, 0xb9, 0x38, 0x7b, 0xfa, 0x0c, 0xe3, 0xd3, 0x73, 0x18, 0xa5,
    0x12, 0xd1, 0xd5, 0x35, 0x32, 0xd9, 0x1c, 0xd9, 0x5c, 0x96, 0x54, 0x22, 0x49, 0x53, 0x5b, 0x07,
    0xa6, 0x2d, 0xfc, 0xee, 0x06, 0x1a, 0xda, 0xb6, 0xfd, 0x34, 0xf0, 0x1d, 0xd0, 0x39, 0xf3, 0xda,
    0x51, 0xde, 0xbe, 0x72, 0x85, 0x54, 0xae, 0xc0, 0xeb, 0x6f, 0x9c, 0x24, 0x57, 0xd4, 0xa9, 0x18,
    0x26, 0xad, 0xad, 0x61, 0x72, 0xe9, 0x4d, 0x86, 0x7a, 0x3b, 0x51, 0x14, 0x85, 0xa3, 0x7f, 0x1e,
    0xe3, 0x96, 0xde, 0x76, 0xbe, 0xfe, 0xd5, 0x27, 0xe8, 0xeb, 0x1f, 0xe4, 0xb9, 0x5f, 0x3f, 0xcb,
    0xc0, 0x8e, 0x7e, 0xfe, 0xfd, 0x3f, 0x7f, 0x49, 0x2e, 0x5f, 0x44, 0x71, 0xc8, 0xb4, 0x84, 0x43,
    0xd8, 0x96, 0xc1, 0xc7, 0xee, 0xf8, 0x28, 0x0f, 0x3c, 0xfc, 0x10, 0x1d, 0xdd, 0x7d, 0xe0, 0x74,
    0x7c, 0x43, 0x10, 0x7c, 0x3f, 0xbd, 0xae, 0x09, 0xa1, 0x72, 0x00, 0x2c, 0x40, 0xa2, 0xc1, 0xa3,
    0x21, 0x09, 0x55, 0xc6, 0x2f, 0x8f, 0xf3, 0xe9, 0x47, 0x3e, 0x8e, 0x24, 0x8a, 0x84, 0x82, 0x3e,
    0x54, 0xb3, 0x84, 0x28, 0x08, 0x2c, 0x45, 0xe3, 0x4c, 0xcc, 0x2c, 0x20, 0x08, 0x22, 0x91, 0xc5,
    0x18, 0x5e, 0x7f, 0x90, 0xf1, 0x8b, 0xe7, 0x79, 0xf5, 0xb5, 0x51, 0x8e, 0x1c, 0x7b, 0x9d, 0xcd,
    0x7c, 0x91, 0x92, 0x69, 0x61, 0x58, 0x55, 0x24, 0x11, 0xf2, 0xba, 0x41, 0x59, 0x2f, 0x22, 0x2b,
    0x0e, 0x50, 0x14, 0xaa, 0x85, 0xdc, 0xdd, 0x37, 0xe8, 0xc0, 0xea, 0xd4, 0x25, 0xc7, 0xec, 0x4c,
    0x04, 0x45, 0x16, 0x99, 0x9e, 0x9e, 0x26, 0xb6, 0xb4, 0xc2, 0x6d, 0x3b, 0xfb, 0x89, 0xae, 0x44,
    0x09, 0xba, 0x9d, 0x64, 0x75, 0x9d, 0x48, 0x5e, 0xe7, 0xfe, 0x3b, 0x3f, 0xc2, 0xe8, 0xb9, 0x71,
    0xaa, 0xd8, 0x98, 0xa6, 0x55, 0xa7, 0xe7, 0xd4, 0xd8, 0xa5, 0x3a, 0x3b, 0x46, 0x3e, 0x7a, 0x2b,
    0xf1, 0xd5, 0x65, 0x5c, 0x0e, 0x85, 0x12, 0xe0, 0x94, 0x44, 0x1a, 0x03, 0x7e, 0x7c, 0x6e, 0x37,
    0xcb, 0x4b, 0xab, 0x9c, 0x3f, 0xfe, 0x26, 0xbd, 0xf1, 0x35, 0x2a, 0x26, 0xfa, 0x0d, 0x34, 0xcc,
    0xa6, 0x92, 0xf3, 0xbf, 0x7f, 0xfe, 0x79, 0x6c, 0x41, 0xe4, 0xcc, 0xe9, 0x0b, 0x68, 0x3e, 0x3f,
    0xc3, 0xbb, 0x87, 0xd8, 0xbb, 0xff, 0x6e, 0x3a, 0xbb, 0xb7, 0xd5, 0xa9, 0xe9, 0x90, 0x04, 0x2e,
    0x5c, 0x99, 0x64, 0xb8, 0x3b, 0x4c, 0xab, 0xdf, 0x8b, 0x20, 0x0a, 0x18, 0xa6, 0xc5, 0xe2, 0xd2,
    0x12, 0x2e, 0x8f, 0x87, 0x5b, 0x87, 0x06, 0x98, 0x9d, 0x5f, 0xa1, 0x50, 0xae, 0xe0, 0x70, 0xc8,
    0x84, 0x42, 0x01, 0xb6, 0x34, 0x06, 0xe9, 0xec, 0x6c, 0x63, 0x3d, 0x91, 0xe6, 0xec, 0x99, 0x73,
    0xbc, 0xf8, 0xd2, 0xcb, 0xcc, 0xcd, 0x46, 0xa6, 0xaf, 0xeb, 0x01, 0xdb, 0xce, 0x35, 0xc5, 0xa6,
    0xc6, 0xe6, 0x75, 0xc3, 0xd2, 0x1a, 0xbc, 0x1e, 0x64, 0xad, 0x01, 0x9f, 0xbf, 0x81, 0x13, 0xaf,
    0xbc, 0x4c, 0x6c, 0x23, 0xcd, 0x7f, 0x3d, 0xf3, 0xbb, 0x7a, 0x0f, 0xe8, 0xa5, 0x12, 0xb7, 0xed,
    0xdc, 0x86, 0xd7, 0xad, 0xb1, 0xb8, 0x14, 0x63, 0x29, 0x9e, 0x01, 0x51, 0x60, 0xb8, 0xbb, 0x85,
    0xfb, 0xee, 0xbf, 0x97, 0x57, 0x0e, 0x1f, 0xe5, 0xd2, 0xc2, 0x3a, 0x01, 0xb7, 0x86, 0x2c, 0x0a,
    0x48, 0x92, 0x83, 0x03, 0xfb, 0x6f, 0x67, 0xe7, 0xd0, 0x20, 0x3d, 0x3b, 0x77, 0xb1, 0x30, 0x3d,
    0x81, 0xc7, 0x1f, 0x40, 0x55, 0xc4, 0x85, 0xf1, 0xb7, 0xe7, 0xb6, 0x7f, 0xea, 0x89, 0x6f, 0x1a,
    0xf5, 0x0a, 0x58, 0xa9, 0xd8, 0x17, 0x53, 0x89, 0x0d, 0x6d, 0xdb, 0xe0, 0x10, 0xa1, 0xad, 0x23,
    0xf8, 0x9a, 0x3a, 0x98, 0x9d, 0xb8, 0xc4, 0x89, 0xb7, 0x4e, 0xf2, 0x87, 0x97, 0x0e, 0xd1, 0x12,
    0x0e, 0x22, 0xd7, 0xc4, 0xc7, 0x34, 0x11, 0x55, 0x2f, 0xa2, 0xa8, 0x50, 0x31, 0x4d, 0x34, 0xa7,
    0x4c, 0xd0, 0xe3, 0x60, 0x73, 0x33, 0xc7, 0xd8, 0xd8, 0x04, 0x96, 0x6d, 0xb3, 0x96, 0xcc, 0xbf,
    0xa3, 0x7a, 0x95, 0x2a, 0x5b, 0x3b, 0x9b, 0x79, 0xf3, 0xd4, 0x05, 0xde, 0xbe, 0x72, 0x19, 0xa7,
    0x54, 0xe5, 0xce, 0x87, 0x3f, 0x4d, 0x57, 0x4f, 0x1f, 0xab, 0x4b, 0x0b, 0x5b, 0x1f, 0x7c, 0xe4,
    0xfe, 0x8f, 0xbf, 0x07, 0x81, 0x20, 0x2b, 0x4f, 0x28, 0x4e, 0x8d, 0xc4, 0xca, 0xea, 0x3b, 0x53,
    0x31, 0x72, 0x9e, 0x63, 0x7f, 0x3c, 0xc8, 0xc5, 0xc9, 0x39, 0x0a, 0x25, 0x93, 0x64, 0x6a, 0xb3,
    0xae, 0xf3, 0xfb, 0x46, 0x06, 0xe9, 0xeb, 0x68, 0xe6, 0xf2, 0xd4, 0x2c, 0x85, 0x92, 0x41, 0xaa,
    0x50, 0x26, 0x95, 0xab, 0xb0, 0x59, 0x2c, 0x71, 0x69, 0x7c, 0x1a, 0xaf, 0xcb, 0xcd, 0xd0, 0xb6,
    0x56, 0x6a, 0xb3, 0xc8, 0xad, 0x3a, 0x98, 0x5f, 0x59, 0x47, 0xaf, 0x58, 0x9c, 0x3e, 0x3f, 0xc1,
    0xc5, 0x13, 0x6f, 0x41, 0x31, 0x4d, 0x76, 0x33, 0x8d, 0x20, 0x39, 0x30, 0xca, 0xc6, 0x17, 0xea,
    0x4d, 0x78, 0xe9, 0xf8, 0xa1, 0xd0, 0xc1, 0xdf, 0xfe, 0xa6, 0x27, 0x9d, 0xd1, 0xc1, 0x2c, 0xf1,
    0xc4, 0xb7, 0xff, 0x91, 0xf8, 0xca, 0x2a, 0xf1, 0x64, 0x06, 0x45, 0x56, 0x70, 0x38, 0x54, 0x0a,
    0x45, 0x9d, 0x06, 0x55, 0x62, 0xcf, 0xc8, 0x10, 0xbf, 0x79, 0xf1, 0x4f, 0x98, 0xb5, 0x4a, 0x48,
    0x32, 0x15, 0xcb, 0x42, 0x53, 0xa4, 0xfa, 0x7b, 0xa6, 0x6d, 0xb1, 0x10, 0x4d, 0xf0, 0xb1, 0x81,
    0x6d, 0x44, 0x53, 0x19, 0x56, 0x37, 0x32, 0xc8, 0xa2, 0x8c, 0x2c, 0x8b, 0x88, 0x82, 0x44, 0x3a,
    0xbd, 0xc9, 0xe6, 0x7a, 0x94, 0x3f, 0x3c, 0xf7, 0x6b, 0xbc, 0xbe, 0x00, 0x1b, 0xb1, 0xf5, 0x3d,
    0xf5, 0x0a, 0xec, 0xde, 0xfb, 0x50, 0xf2, 0xc4, 0xb1, 0x37, 0xda, 0x4f, 0x9d, 0xfc, 0xf3, 0x77,
    0xfb, 0x46, 0xee, 0x00, 0x49, 0xc6, 0xb6, 0x2d, 0x5a, 0x9a, 0xc2, 0xf8, 0x3d, 0x2a, 0x50, 0xa5,
    0x6a, 0x5b, 0x18, 0x65, 0x8b, 0x17, 0x0e, 0xbe, 0x46, 0xc5, 0xb6, 0xa9, 0x58, 0xd4, 0x07, 0x8f,
    0x88, 0x88, 0x20, 0xbc, 0x03, 0x4d, 0x8d, 0xbe, 0xa6, 0x2d, 0x70, 0x71, 0x7a, 0x81, 0xf5, 0x54,
    0xb6, 0x5e, 0xc9, 0xda, 0xcc, 0x08, 0x79, 0x5d, 0xf4, 0x74, 0x36, 0xe1, 0xf5, 0x79, 0x41, 0x76,
    0xd2, 0xb2, 0xad, 0xdf, 0x38, 0x7d, 0xf2, 0xec, 0xdf, 0x5e, 0x3c, 0x73, 0xee, 0xde, 0xf7, 0x20,
    0xf8, 0x97, 0xe7, 0x8e, 0xac, 0x3f, 0x78, 0xe0, 0x9e, 0x67, 0x35, 0x99, 0xf2, 0xda, 0xfc, 0x3c,
    0x82, 0xec, 0xac, 0x3b, 0x28, 0x9b, 0x26, 0x95, 0x4a, 0x05, 0x59, 0x10, 0x70, 0xab, 0x12, 0xaa,
    0xaa, 0x20, 0x4b, 0x32, 0x2e, 0xa7, 0x52, 0x6b, 0x5d, 0x04, 0x01, 0x14, 0x59, 0x42, 0x96, 0x24,
    0x54, 0x87, 0x8c, 0x28, 0x8b, 0x68, 0xaa, 0x13, 0x55, 0x96, 0xea, 0xdd, 0x5d, 0x63, 0x4e, 0xad,
    0x77, 0x5c, 0x6e, 0x17, 0x4e, 0x87, 0x4a, 0x32, 0x95, 0xa6, 0xd1, 0xa7, 0xce, 0x0c, 0xf4, 0xb5,
    0x1e, 0xfa, 0xe1, 0x2f, 0x5e, 0x5a, 0xbc, 0x8e, 0x86, 0x8b, 0x93, 0x93, 0xcb, 0xed, 0x1d, 0x9d,
    0xd3, 0x2d, 0x1d, 0x1d, 0xac, 0xce, 0xcf, 0xb1, 0x1c, 0x5d, 0xa7, 0xa8, 0x97, 0x90, 0x80, 0x46,
    0xaf, 0xb3, 0x5e, 0x8d, 0x92, 0x61, 0xa2, 0x48, 0x22, 0x0e, 0x45, 0xc6, 0xa9, 0x48, 0xd8, 0xff,
    0x17, 0xc4, 0xa9, 0xc8, 0xf5, 0x24, 0x6a, 0x82, 0x55, 0xf3, 0x18, 0x6a, 0xd0, 0x08, 0xb8, 0x15,
    0x1c, 0xb2, 0x80, 0x5e, 0x2a, 0x93, 0xc9, 0xe4, 0x88, 0xc5, 0xd6, 0xf1, 0xba, 0x9d, 0xb4, 0x77,
    0x75, 0x3e, 0xf3, 0xad, 0xa7, 0x7f, 0x6e, 0xdd, 0xa0, 0x03, 0xdf, 0xfe, 0xd1, 0x7f, 0x1b, 0x4d,
    0x3d, 0xb7, 0xce, 0xd5, 0xce, 0xa6, 0x4d, 0x7d, 0xa4, 0x3a, 0x04, 0x81, 0x66, 0xbf, 0x46, 0x53,
    0xc0, 0x4b, 0x22, 0x57, 0xc4, 0xb2, 0x40, 0x16, 0x40, 0x55, 0x64, 0x6a, 0x9f, 0x2f, 0x8a, 0x02,
    0xd5, 0x9a, 0x13, 0x41, 0x44, 0x73, 0xd4, 0x7a, 0x41, 0x42, 0x2f, 0x99, 0xf5, 0xaf, 0xef, 0x08,
    0x35, 0xd4, 0xff, 0x5b, 0x4b, 0x33, 0x99, 0xca, 0x90, 0xcb, 0xe5, 0x71, 0xb9, 0x7d, 0xf4, 0x0e,
    0xdd, 0x3a, 0x73, 0xed, 0x3e, 0x20, 0x5f, 0x7b, 0xc9, 0x2e, 0x8f, 0x79, 0x26, 0x26, 0xae, 0x70,
    0xea, 0xe4, 0x69, 0x54, 0x49, 0xa4, 0xb5, 0xc9, 0x57, 0xdb, 0x6c, 0x98, 0x5e, 0x4e, 0x60, 0x23,
    0xd2, 0xe8, 0xd3, 0xb0, 0x0c, 0x83, 0x1a, 0xe2, 0xd8, 0x36, 0x55, 0xcb, 0xc2, 0x46, 0xc1, 0xa1,
    0x48, 0x68, 0x4e, 0x07, 0xd8, 0x15, 0x14, 0x55, 0x21, 0x9e, 0xd5, 0xf1, 0x6a, 0x0a, 0xed, 0x21,
    0x2f, 0xe5, 0xb2, 0xc1, 0x66, 0xa1, 0x4c, 0x64, 0x66, 0x8e, 0x33, 0xa3, 0xaf, 0x33, 0x38, 0xb4,
    0xd3, 0xff, 0x81, 0x09, 0xfc, 0xfc, 0xc7, 0xff, 0xfa, 0xe5, 0x52, 0x95, 0x27, 0xc3, 0x3e, 0xed,
    0x80, 0x1a, 0xd4, 0x76, 0x94, 0x0d, 0x43, 0x8e, 0xac, 0xa5, 0x70, 0xb8, 0x5d, 0xec, 0xde, 0xda,
    0xc6, 0x5a, 0x2c, 0x4e, 0x3a, 0x57, 0x20, 0xa7, 0x57, 0x28, 0x19, 0xef, 0x54, 0x51, 0x10, 0x6b,
    0xb8, 0x2b, 0x75, 0x18, 0x6a, 0x70, 0xd4, 0xa4, 0x3c, 0xe0, 0x77, 0xb3, 0x1c, 0x4b, 0xd7, 0xa9,
    0xd8, 0x14, 0x74, 0xdb, 0xe1, 0xc6, 0x86, 0x54, 0xb6, 0xa0, 0x1f, 0x19, 0x3d, 0xfa, 0xea, 0xd9,
    0x6c, 0x2c, 0xf6, 0xec, 0x07, 0x6e, 0xc5, 0xd7, 0xda, 0x53, 0x5f, 0xf9, 0xac, 0x2f, 0x9e, 0x4c,
    0x4e, 0x94, 0x2c, 0xb1, 0xed, 0xef, 0x3e, 0xf3, 0x10, 0xa6, 0x5e, 0x60, 0x36, 0x72, 0x95, 0x6c,
    0xae, 0x88, 0x69, 0x57, 0x51, 0x34, 0x8d, 0xa5, 0x68, 0x82, 0x8e, 0xe6, 0x10, 0x0e, 0x45, 0xa1,
    0x90, 0xcf, 0x91, 0xc9, 0xe9, 0xb8, 0x9d, 0x0a, 0xbd, 0x7d, 0x5b, 0x59, 0x4f, 0x64, 0x88, 0xcc,
    0x2e, 0xd2, 0xd3, 0x19, 0xfe, 0xdc, 0xd7, 0xfe, 0xe9, 0x7b, 0xbf, 0x6d, 0x6c, 0xe9, 0xad, 0xa1,
    0x75, 0x83, 0xc9, 0x7c, 0x80, 0x7d, 0xff, 0x3f, 0x9e, 0xcf, 0x3c, 0xf9, 0xf9, 0x87, 0xcf, 0xdd,
    0x77, 0xcf, 0x5d, 0x6d, 0xbd, 0x3b, 0x76, 0xb0, 0x36, 0x3f, 0x47, 0x5b, 0x7b, 0x1b, 0xdb, 0xfd,
    0x01, 0xfa, 0x87, 0x47, 0x50, 0x1c, 0x0a, 0x8b, 0x73, 0x73, 0x84, 0x82, 0xfe, 0x3a, 0x53, 0xa2,
    0x4b, 0xcb, 0x4c, 0x4f, 0x4d, 0x62, 0x96, 0x2a, 0x78, 0x54, 0x07, 0x2d, 0xbb, 0x87, 0x89, 0xad,
    0x27, 0x0a, 0x8d, 0xe1, 0x2d, 0x2f, 0x7f, 0x50, 0xf0, 0x9b, 0x26, 0x50, 0xb3, 0x91, 0xe1, 0x1d,
    0x3f, 0x1d, 0x1a, 0xde, 0xf5, 0xa8, 0x24, 0x29, 0x34, 0x84, 0x1a, 0xf1, 0x35, 0x36, 0x33, 0x74,
    0xdb, 0x9d, 0x08, 0x6e, 0x17, 0x54, 0x4d, 0x5c, 0x2e, 0x2f, 0xc1, 0xe6, 0x26, 0xaa, 0x86, 0x41,
    0xf7, 0x8e, 0x5b, 0x68, 0xef, 0xe9, 0x63, 0x76, 0x62, 0xac, 0x0e, 0x47, 0x20, 0xe4, 0xa3, 0xaf,
    0xbb, 0xfd, 0xc7, 0x5f, 0x7b, 0xea, 0x27, 0xf9, 0x9b, 0xc5, 0x90, 0x6f, 0xf6, 0xb0, 0xb9, 0xab,
    0xe3, 0x4d, 0x7f, 0x28, 0x90, 0x2a, 0x97, 0x4a, 0xc1, 0x2d, 0xad, 0x9d, 0xb4, 0x74, 0x6d, 0x03,
    0x67, 0x00, 0x28, 0xd4, 0xf9, 0xe3, 0xaf, 0x89, 0x8b, 0x22, 0x20, 0xca, 0x0a, 0x1e, 0xa7, 0x9f,
    0x41, 0xcf, 0xee, 0xfa, 0xfa, 0x9e, 0x4d, 0x27, 0x91, 0x44, 0xbb, 0xb2, 0x73, 0xc7, 0xf6, 0x7f,
    0xe3, 0x43, 0x4c, 0xf8, 0xb0, 0x17, 0x12, 0x0b, 0xe7, 0x8f, 0xab, 0xaa, 0x6b, 0x8f, 0x7b, 0x4b,
    0x18, 0x44, 0x27, 0x54, 0xf3, 0xe4, 0xa3, 0x51, 0xa2, 0x4b, 0x2b, 0x6c, 0x6c, 0xc4, 0xd1, 0xdc,
    0x1a, 0xa1, 0x50, 0x98, 0xe6, 0xd6, 0x56, 0x9c, 0x0d, 0x3e, 0xaa, 0x36, 0x24, 0xe2, 0x71, 0xb2,
    0x1b, 0xeb, 0x0b, 0x7d, 0xb7, 0x7f, 0x72, 0xdb, 0x87, 0xf9, 0x97, 0x6f, 0xf6, 0xf0, 0xd1, 0x4f,
    0xdc, 0x21, 0x7a, 0x7d, 0xc1, 0x0e, 0x87, 0xbf, 0x95, 0xda, 0xaa, 0x96, 0x5f, 0xb9, 0xca, 0xe8,
    0x6b, 0xc7, 0x18, 0xbb, 0x70, 0x09, 0xb3, 0x6a, 0x33, 0x31, 0xbb, 0x5c, 0xd7, 0x04, 0xd3, 0x30,
    0xf1, 0xba, 0x9c, 0xfc, 0xcd, 0x63, 0x8f, 0x70, 0xf7, 0xbd, 0xfb, 0x09, 0xb7, 0xb6, 0xe1, 0x90,
    0xa4, 0x9a, 0x4e, 0x7d, 0xa8, 0xdd, 0xb4, 0x02, 0x76, 0x39, 0xf1, 0x5d, 0x1c, 0xc1, 0x7f, 0xc6,
    0xce, 0x70, 0xf1, 0xd8, 0x11, 0x0e, 0xff, 0xf1, 0x65, 0x12, 0xf1, 0x14, 0x79, 0x5d, 0xc7, 0x52,
    0xdd, 0x1c, 0xb8, 0x67, 0x0f, 0xf3, 0x8b, 0xcb, 0x9c, 0x3c, 0x75, 0x8e, 0xa0, 0x47, 0xad, 0x0b,
    0x58, 0x77, 0x7b, 0x13, 0x9f, 0x7d, 0xfc, 0x71, 0x06, 0xef, 0xba, 0xbb, 0x94, 0x4f, 0x67, 0x06,
    0xbd, 0xa1, 0xce, 0xf9, 0x9b, 0xc5, 0xf8, 0x5f, 0xd2, 0x62, 0x90, 0xdf, 0x28, 0x49, 0x14, 0x96,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};
static const unsigned int ICON_HOVER_size = 2652;

// Global variables
HMODULE hSelf;
AddonDefinition_t AddonDef{};
AddonAPI_t* APIDefs = nullptr;
bool g_WindowVisible = false;

// UI State
static int g_SelectedCharIdx = -1;
static int g_SelectedTab = 0;          // 0 = Equipment, 1 = Build
static int g_SelectedEquipTab = 0;     // Equipment tab filter
static int g_SelectedBuildTab = 0;     // Build tab filter
static bool g_ShowQAIcon = true;
static bool g_CompactCharList = false;
static bool g_ShowCraftingIcons = true;
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
        auto ftime = std::filesystem::last_write_time(path);
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

// Clears tracker state
#define EV_AE_CLEARS_ACH_RESPONSE "EV_ALTER_EGO_CLEARS_ACH_RESP"
static void OnClearsAchResponse(void* eventArgs);
static void SendClearsAchQuery();

// Achievement categories: 88 = Daily Fractals, 475 = Daily Raid Bounties, 477 = Weekly Raids
static const uint32_t CAT_DAILY_FRACTALS = 88;
static const uint32_t CAT_DAILY_BOUNTIES = 475;
static const uint32_t CAT_WEEKLY_RAIDS   = 477;
static const uint32_t ACH_WEEKLY_STRIKES = 9125; // "Weekly Raid Encounters" — tracks all strikes

struct ClearEntry {
    uint32_t id = 0;
    std::string name;
    std::string tier;              // For fractals: "T1".."T4", "Rec"
    bool done = false;
    int32_t current = 0;
    int32_t max = 0;
    std::vector<std::string> bitNames;  // From API bits[].text
    std::vector<bool> bitDone;          // Per-bit completion from H&S
};

static std::vector<ClearEntry> g_DailyFractals;   // cat 88
static std::vector<ClearEntry> g_DailyBounties;   // cat 475
static std::vector<ClearEntry> g_WeeklyWings;     // cat 477 per-wing achievements
static ClearEntry g_WeeklyStrikes;                 // ach 9125

static std::mutex g_ClearsMutex;
static bool g_ClearsFetching = false;
static bool g_ClearsFetched = false;
static std::string g_ClearsStatusMsg;

// Reset time tracking
static std::chrono::system_clock::time_point g_LastDailyReset{};
static std::chrono::system_clock::time_point g_LastWeeklyReset{};
static std::chrono::steady_clock::time_point g_LastClearsCompletionQuery{};

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

static std::recursive_mutex g_AchMutex;
static bool g_AchGroupsFetched = false;
static bool g_AchGroupsFetching = false;
static bool g_AchNameIndexReady = false;
static bool g_AchNameIndexFetching = false;
static std::string g_AchStatusMsg;

#define ACH_NAME_INDEX_URL "https://raw.githubusercontent.com/PieOrCake/alter_ego/main/data/achievement_names.json"
#define ACH_WAYPOINTS_URL "https://raw.githubusercontent.com/PieOrCake/alter_ego/main/data/achievement_waypoints.json"

// Waypoint data: achId -> (bitIndex -> chatCode), bitIndex -1 = achievement-level waypoint
static std::unordered_map<uint32_t, std::unordered_map<int, std::string>> g_AchWaypoints;
static bool g_AchWaypointsReady = false;
static bool g_AchWaypointsFetching = false;

// UI state
static std::string g_AchSelectedGroupId;   // currently selected group UUID
static uint32_t g_AchSelectedCatId = 0;    // currently selected category ID
static bool g_AchCatFetching = false;      // fetching defs for selected category
static bool g_AchProgressFetching = false; // fetching account progress
static uint64_t g_AchProgressGen = 0;      // incremented when progress updates, triggers popout cache rebuild
static char g_AchSearchBuf[128] = "";
static bool g_AchPopoutVisible = false;    // popout tracker window visibility
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
static bool g_ClearsBootQueried = false;   // flag: have we queried clears completion after H&S became available?

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
static int g_LibImportMode = 0;        // GameMode for import
static bool g_LibShowImport = false;
static std::string g_LibImportError;
static bool g_LibDetailsFetched = false;
static int g_LibDragIdx = -1;          // drag-and-drop source index
static char g_LibEditName[128] = "";   // inline rename buffer
static char g_LibEditNotes[512] = ""; // inline notes buffer
static std::string g_LibEditBuildId;   // which build is being edited
static int g_LibCtxDeleteIdx = -1;     // deferred delete from context menu

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

// When H&S becomes available, query pinned achievement progress + clears completion
static void OnHoardPongForAch(void* eventArgs) {
    if (!eventArgs) return;
    auto* pong = (HoardPongPayload*)eventArgs;
    if (pong->api_version != HOARD_API_VERSION) return;

    // Query pinned achievement progress (once per session)
    if (!g_AchPinnedBootQueried) {
        g_AchPinnedBootQueried = true;
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        if (!g_AchPinned.empty()) {
            SendAchProgressQuery(g_AchPinned);
        }
    }

    // Query clears completion (once per session, only if we have cached defs)
    if (!g_ClearsBootQueried && g_ClearsFetched && !g_ClearsFetching) {
        g_ClearsBootQueried = true;
        SendClearsAchQuery();
    }
}

static void OnEvAlertAchievementCompleted(void* eventArgs) {
    if (!eventArgs) return;
    auto* payload = (AlertUnlockPayload*)eventArgs;
    if (payload->ID != 0) {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        auto it = g_AchProgress.find(payload->ID);
        if (it != g_AchProgress.end()) {
            it->second.done = true;
        } else {
            AchProgress p;
            p.id = payload->ID;
            p.done = true;
            g_AchProgress[payload->ID] = std::move(p);
        }
        g_AchProgressGen++;
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
    g_LastCharCount = chars.size();
}

// Character refresh selection popup state
static bool g_RefreshPopupOpen = false;
static bool g_RefreshListFetching = false;     // waiting for char list from H&S
static std::vector<std::string> g_RefreshNames; // all character names from API
static std::vector<bool> g_RefreshSelection;    // parallel checkbox state

// Gear customization dialog state
static bool g_GearDialogOpen = false;
static std::string g_GearDialogSlot;          // Which slot is being edited
static std::string g_GearDialogBuildId;       // Which saved build
static char g_GearStatSearch[128] = "";       // Search filter for stat combos
static uint32_t g_GearSelectedStatId = 0;     // Currently selected stat in dialog
static int g_GearSelectorTab = 0;             // 0 = Stats, 1 = Rune/Sigil

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
static void CopyToClipboard(const std::string& text) {
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
    try {
        auto j = nlohmann::json::parse(file);
        if (j.contains("show_qa_icon")) g_ShowQAIcon = j["show_qa_icon"].get<bool>();
        if (j.contains("compact_char_list")) g_CompactCharList = j["compact_char_list"].get<bool>();
        if (j.contains("show_crafting_icons")) g_ShowCraftingIcons = j["show_crafting_icons"].get<bool>();
        if (j.contains("show_age")) g_ShowAge = j["show_age"].get<bool>();
        if (j.contains("show_playtime")) g_ShowPlaytime = j["show_playtime"].get<bool>();
        if (j.contains("show_last_login")) g_ShowLastLogin = j["show_last_login"].get<bool>();
        if (j.contains("birthday_mode")) g_BirthdayMode = j["birthday_mode"].get<int>();
        if (j.contains("char_list_width")) g_CharListWidth = j["char_list_width"].get<float>();
        if (j.contains("lib_list_width")) g_LibListWidth = j["lib_list_width"].get<float>();
        if (j.contains("chat_build_detection")) g_ChatBuildDetection = j["chat_build_detection"].get<bool>();
        if (j.contains("toast_pos_x")) g_ToastPosX = j["toast_pos_x"].get<float>();
        if (j.contains("toast_pos_y")) g_ToastPosY = j["toast_pos_y"].get<float>();
    } catch (...) {}
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

    // Skinventory state
    j["skin_active_tab"] = g_SkinActiveTab;
    j["skin_type"] = g_SkinSelectedType;
    j["skin_weight"] = g_SkinSelectedWeightClass;
    j["skin_subtype"] = g_SkinSelectedSubtype;
    j["skin_selected_id"] = g_SkinSelectedId;
    j["skin_show_owned"] = g_SkinShowOwned;
    j["skin_show_unowned"] = g_SkinShowUnowned;

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

        // Skinventory
        if (j.contains("skin_active_tab")) g_SkinActiveTab = j["skin_active_tab"].get<int>();
        if (j.contains("skin_type")) g_SkinSelectedType = j["skin_type"].get<std::string>();
        if (j.contains("skin_weight")) g_SkinSelectedWeightClass = j["skin_weight"].get<std::string>();
        if (j.contains("skin_subtype")) g_SkinSelectedSubtype = j["skin_subtype"].get<std::string>();
        if (j.contains("skin_selected_id")) g_SkinSelectedId = j["skin_selected_id"].get<uint32_t>();
        if (j.contains("skin_show_owned")) g_SkinShowOwned = j["skin_show_owned"].get<bool>();
        if (j.contains("skin_show_unowned")) g_SkinShowUnowned = j["skin_show_unowned"].get<bool>();
    } catch (...) {}
}

// Clears cache persistence (forward declarations for functions defined in Clears backend section)
static std::chrono::system_clock::time_point CalcLastDailyReset(std::chrono::system_clock::time_point now);
static std::chrono::system_clock::time_point CalcLastWeeklyReset(std::chrono::system_clock::time_point now);
static void FetchClears();

static void SaveClearsCache() {
    std::string dir = AlterEgo::GW2API::GetDataDirectory();
    std::filesystem::create_directories(dir);
    std::string path = dir + "/clears_cache.json";

    nlohmann::json j;
    // Store fetch timestamp as seconds since epoch
    auto epochSecs = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    j["fetch_time"] = epochSecs;

    auto serializeEntry = [](const ClearEntry& e) -> nlohmann::json {
        nlohmann::json ej;
        ej["id"] = e.id;
        ej["name"] = e.name;
        ej["tier"] = e.tier;
        ej["done"] = e.done;
        ej["current"] = e.current;
        ej["max"] = e.max;
        ej["bitNames"] = e.bitNames;
        std::vector<int> bits;
        for (bool b : e.bitDone) bits.push_back(b ? 1 : 0);
        ej["bitDone"] = bits;
        return ej;
    };

    nlohmann::json fractals = nlohmann::json::array();
    for (const auto& e : g_DailyFractals) fractals.push_back(serializeEntry(e));
    j["dailyFractals"] = fractals;

    nlohmann::json bounties = nlohmann::json::array();
    for (const auto& e : g_DailyBounties) bounties.push_back(serializeEntry(e));
    j["dailyBounties"] = bounties;

    nlohmann::json wings = nlohmann::json::array();
    for (const auto& e : g_WeeklyWings) wings.push_back(serializeEntry(e));
    j["weeklyWings"] = wings;

    j["weeklyStrikes"] = serializeEntry(g_WeeklyStrikes);

    std::ofstream file(path);
    if (file.is_open()) file << j.dump(2);
}

static void LoadClearsCache() {
    std::string path = AlterEgo::GW2API::GetDataDirectory() + "/clears_cache.json";
    std::ifstream file(path);
    if (!file.is_open()) return;

    auto deserializeEntry = [](const nlohmann::json& ej) -> ClearEntry {
        ClearEntry e;
        e.id = ej.value("id", 0u);
        e.name = ej.value("name", "");
        e.tier = ej.value("tier", "");
        e.done = ej.value("done", false);
        e.current = ej.value("current", 0);
        e.max = ej.value("max", 0);
        if (ej.contains("bitNames") && ej["bitNames"].is_array()) {
            for (const auto& bn : ej["bitNames"]) e.bitNames.push_back(bn.get<std::string>());
        }
        if (ej.contains("bitDone") && ej["bitDone"].is_array()) {
            for (const auto& bd : ej["bitDone"]) e.bitDone.push_back(bd.get<int>() != 0);
        }
        return e;
    };

    try {
        auto j = nlohmann::json::parse(file);

        int64_t fetchEpoch = j.value("fetch_time", (int64_t)0);
        auto fetchTime = std::chrono::system_clock::from_time_t((time_t)fetchEpoch);

        // Check if a reset has occurred since the cached data was fetched
        // Add 60s buffer so we don't treat cache as stale before the API has updated
        auto now = std::chrono::system_clock::now();
        auto dailyReset = CalcLastDailyReset(now) + std::chrono::seconds(60);
        auto weeklyReset = CalcLastWeeklyReset(now) + std::chrono::seconds(60);
        bool dailyStale = fetchTime < dailyReset && now >= dailyReset;
        bool weeklyStale = fetchTime < weeklyReset && now >= weeklyReset;

        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);

            // Load daily data only if no daily reset has occurred since fetch
            if (!dailyStale) {
                if (j.contains("dailyFractals") && j["dailyFractals"].is_array()) {
                    for (const auto& ej : j["dailyFractals"])
                        g_DailyFractals.push_back(deserializeEntry(ej));
                }
                if (j.contains("dailyBounties") && j["dailyBounties"].is_array()) {
                    for (const auto& ej : j["dailyBounties"])
                        g_DailyBounties.push_back(deserializeEntry(ej));
                }
            }

            // Load weekly data only if no weekly reset has occurred since fetch
            if (!weeklyStale) {
                if (j.contains("weeklyWings") && j["weeklyWings"].is_array()) {
                    for (const auto& ej : j["weeklyWings"])
                        g_WeeklyWings.push_back(deserializeEntry(ej));
                    // Sort into canonical W1-W8 order
                    static const std::unordered_map<uint32_t, int> wingOrder = {
                        {9128, 1}, {9147, 2}, {9182, 3}, {9144, 4},
                        {9111, 5}, {9120, 6}, {9156, 7}, {9181, 8},
                    };
                    std::sort(g_WeeklyWings.begin(), g_WeeklyWings.end(),
                        [](const ClearEntry& a, const ClearEntry& b) {
                            auto ai = wingOrder.find(a.id);
                            auto bi = wingOrder.find(b.id);
                            int ao = (ai != wingOrder.end()) ? ai->second : 99;
                            int bo = (bi != wingOrder.end()) ? bi->second : 99;
                            return ao < bo;
                        });
                }
                if (j.contains("weeklyStrikes")) {
                    g_WeeklyStrikes = deserializeEntry(j["weeklyStrikes"]);
                }
            }

            // If we loaded any data, mark as fetched
            if (!g_DailyFractals.empty() || !g_WeeklyWings.empty()) {
                g_ClearsFetched = true;
                g_LastClearsCompletionQuery = std::chrono::steady_clock::now();
            }

            g_LastDailyReset = dailyReset;
            g_LastWeeklyReset = weeklyReset;
        }

        // If any data is stale, trigger a full re-fetch
        if (dailyStale || weeklyStale) {
            FetchClears();
        }
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

        // Check staleness (refresh weekly)
        int64_t fetchEpoch = j.value("fetch_time", (int64_t)0);
        auto now = std::chrono::system_clock::now();
        auto elapsed = now - std::chrono::system_clock::from_time_t((time_t)fetchEpoch);
        if (elapsed > std::chrono::hours(7 * 24)) return; // stale

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
            g_AchGroupsFetched = true;
            g_AchGroupsFetching = false;
            g_AchStatusMsg = "";
        }

        SaveAchGroupCache();
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
            if (json.empty()) continue;

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

        g_AchCatFetching = false;
        SaveAchDefCache();

        // Now fetch progress for this category
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        auto it = g_AchCategories.find(catId);
        if (it != g_AchCategories.end()) {
            SendAchProgressQuery(it->second.achievements);
        }
    }).detach();
}

static void SendAchProgressQuery(const std::vector<uint32_t>& ids) {
    if (ids.empty() || !APIDefs) return;

    // Send via H&S achievement query (same mechanism as Clears)
    HoardQueryAchievementRequest req{};
    req.api_version = HOARD_API_VERSION;
    strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
    strncpy(req.response_event, EV_AE_ACH_PROGRESS_RESPONSE, sizeof(req.response_event) - 1);

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
    if (resp->api_version != HOARD_API_VERSION) { delete resp; return; }

    if (resp->status != HOARD_STATUS_OK) {
        if (resp->status == HOARD_STATUS_PENDING) {
            // H&S is prompting user for API key — retry
            std::vector<uint32_t> retryIds;
            for (uint32_t i = 0; i < resp->entry_count; i++) {
                retryIds.push_back(resp->entries[i].id);
            }
            std::thread([retryIds]() {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                SendAchProgressQuery(retryIds);
            }).detach();
        }
        g_AchProgressFetching = false;
        delete resp;
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
            g_AchProgress[e.id] = std::move(p);
        }
        g_LastAchProgressQuery = std::chrono::steady_clock::now();
        g_AchProgressGen++;
    }
    g_AchProgressFetching = false;
    delete resp;
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
            if (batchJson.empty()) continue;

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
            if (json.empty()) continue;
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

// Forward declarations
static void RenderSectionHeader(const char* label, ImVec4 color, const char* suffix = nullptr);

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
        // Try to show skin icon first, fall back to item icon
        uint32_t display_id = eq->skin ? eq->skin : eq->id;
        bool is_skin = eq->skin != 0;

        const AlterEgo::ItemInfo* item_info = AlterEgo::GW2API::GetItemInfo(eq->id);
        const AlterEgo::SkinInfo* skin_info = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;

        std::string icon_url;
        std::string display_name = slotName;
        std::string rarity = "Basic";

        if (skin_info) {
            icon_url = skin_info->icon_url;
            display_name = skin_info->name;
            rarity = skin_info->rarity;
        } else if (item_info) {
            icon_url = item_info->icon_url;
            display_name = item_info->name;
            rarity = item_info->rarity;
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
static void RenderEquipRowReverse(const AlterEgo::Character& ch, const char* slot, int tab) {
    const auto* eq = FindEquipment(ch, slot, tab);

    // Determine display name and rarity
    std::string name = "---";
    std::string rarity = "Basic";
    if (eq && eq->id != 0) {
        const auto* info = AlterEgo::GW2API::GetItemInfo(eq->id);
        const auto* skin = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;
        if (info) rarity = info->rarity;
        if (std::string(slot) == "Relic" && info && info->rarity == "Legendary") {
            name = "Legendary Relic";
        } else if (skin) {
            name = skin->name;
        } else if (info) {
            name = info->name;
        } else {
            name = SlotDisplayName(slot);
        }
    }

    // Right-align: push label to the right so icon ends near the right edge
    float colW = ImGui::GetContentRegionAvail().x;
    float textW = ImGui::CalcTextSize(name.c_str()).x;
    float borderPad = 3.0f; // room for rarity border overhang
    float totalW = textW + 4.0f + ICON_SIZE + borderPad; // text + gap + icon + border
    float offset = colW - totalW;
    if (offset > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    ImGui::AlignTextToFramePadding();
    if (eq && eq->id != 0) {
        ImGui::TextColored(GetRarityColor(rarity), "%s", name.c_str());
    } else {
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "%s", name.c_str());
    }
    ImGui::SameLine();
    RenderEquipmentSlot(eq, SlotDisplayName(slot));
}

// Helper: render one equipment row (icon + name) for the paper-doll grid
static void RenderEquipRow(const AlterEgo::Character& ch, const char* slot, int tab) {
    const auto* eq = FindEquipment(ch, slot, tab);
    RenderEquipmentSlot(eq, SlotDisplayName(slot));
    ImGui::SameLine();
    if (eq && eq->id != 0) {
        const auto* info = AlterEgo::GW2API::GetItemInfo(eq->id);
        const auto* skin = eq->skin ? AlterEgo::GW2API::GetSkinInfo(eq->skin) : nullptr;
        std::string rarity = info ? info->rarity : "Basic";

        // Determine display name
        std::string name;
        if (std::string(slot) == "Relic" && info && info->rarity == "Legendary") {
            name = "Legendary Relic";
        } else if (skin) {
            name = skin->name;
        } else if (info) {
            name = info->name;
        } else {
            name = SlotDisplayName(slot);
        }

        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(GetRarityColor(rarity), "%s", name.c_str());
    } else {
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "---");
    }
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
        ImGui::Text("Equipment Tab:");
        for (int t = 1; t <= maxTab; t++) {
            ImGui::SameLine();
            // Use tab name if available, otherwise just the number
            std::string label;
            auto nameIt = ch.equipment_tab_names.find(t);
            if (nameIt != ch.equipment_tab_names.end() && !nameIt->second.empty()) {
                label = nameIt->second;
            } else {
                label = std::to_string(t);
            }
            bool active = (tab == t);
            bool isActiveTab = (ch.active_equipment_tab == t);
            if (isActiveTab) label += " *";
            if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.30f, 0.14f, 1.0f));
            ImGui::PushID(t);
            if (ImGui::SmallButton(label.c_str())) {
                g_SelectedEquipTab = t;
                g_DetailsFetched = false;
            }
            ImGui::PopID();
            if (active) ImGui::PopStyleColor();
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
            float halfW = (leftW - pad * 2 - gap - 8.0f) * 0.5f;
            ImGui::BeginGroup();
            ImGui::PushItemWidth(halfW);
            RenderEquipRow(ch, "WeaponA1", tab);
            RenderEquipRow(ch, "WeaponA2", tab);
            ImGui::PopItemWidth();
            ImGui::EndGroup();
            ImGui::SameLine(0, 8.0f);
            ImGui::BeginGroup();
            ImGui::PushItemWidth(halfW);
            RenderEquipRow(ch, "WeaponB1", tab);
            RenderEquipRow(ch, "WeaponB2", tab);
            ImGui::PopItemWidth();
            ImGui::EndGroup();
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
                try {
                    overlayTex = APIDefs->Textures_GetOrCreateFromFile(texId.c_str(), cacheIt->second.path.c_str());
                } catch (...) {}
                if (overlayTex && overlayTex->Resource) {
                    if (overlayTex->Height > 0)
                        overlayAspect = (float)overlayTex->Width / (float)overlayTex->Height;
                    overlayTint = IM_COL32(255, 255, 255, 55); // slightly more visible than race art
                }
            } else if (s_portraitMissing.find(ch.name) == s_portraitMissing.end()) {
                // Haven't checked yet — scan for portrait file (ensure dir exists)
                std::string portraitDir = AlterEgo::GW2API::GetDataDirectory() + "/portraits";
                std::filesystem::create_directories(portraitDir);
                static const char* exts[] = { ".png", ".jpg", ".jpeg" };
                bool found = false;
                for (const char* ext : exts) {
                    std::string path = portraitDir + "/" + ch.name + ext;
                    DWORD attrs = GetFileAttributesA(path.c_str());
                    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
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

            // Vignette: gradient edge fade into window background
            {
                ImVec4 bgF = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                ImU32 bg = ImGui::ColorConvertFloat4ToU32(bgF);
                ImU32 clear = ImGui::ColorConvertFloat4ToU32(ImVec4(bgF.x, bgF.y, bgF.z, 0.0f));
                float fadeX = artW * 0.25f; // horizontal fade width
                float fadeY = artH * 0.20f; // vertical fade height

                // Left fade
                dl->AddRectFilledMultiColor(
                    artPos, ImVec2(artPos.x + fadeX, artMax.y),
                    bg, clear, clear, bg);
                // Right fade
                dl->AddRectFilledMultiColor(
                    ImVec2(artMax.x - fadeX, artPos.y), artMax,
                    clear, bg, bg, clear);
                // Top fade
                dl->AddRectFilledMultiColor(
                    artPos, ImVec2(artMax.x, artPos.y + fadeY),
                    bg, bg, clear, clear);
                // Bottom fade
                dl->AddRectFilledMultiColor(
                    ImVec2(artPos.x, artMax.y - fadeY), artMax,
                    clear, clear, bg, bg);
            }
        }
        splitter.Merge(ImGui::GetWindowDrawList());
    }

    ImGui::Spacing();

    // ===== AQUATIC: Aquabreather + Aquatic Weapons =====
    ImGui::Indent(6.0f);
    RenderSectionHeader("Aquatic", ImVec4(0.70f, 0.58f, 0.20f, 1.0f));
    RenderEquipRow(ch, "HelmAquatic", tab);
    {
        float halfW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
        ImGui::BeginGroup();
        ImGui::PushItemWidth(halfW);
        RenderEquipRow(ch, "WeaponAquaticA", tab);
        ImGui::PopItemWidth();
        ImGui::EndGroup();
        ImGui::SameLine(0, 8.0f);
        ImGui::BeginGroup();
        ImGui::PushItemWidth(halfW);
        RenderEquipRow(ch, "WeaponAquaticB", tab);
        ImGui::PopItemWidth();
        ImGui::EndGroup();
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
                ImVec2(pos.x - 1, pos.y - 1),
                ImVec2(pos.x + size + 1, pos.y + size + 1),
                IM_COL32(100, 220, 255, 255), 0.0f, 0, 2.0f);
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

static void RenderSkillIcon(uint32_t skill_id, float size);
static bool DecodeBuildLink(const std::string& link, const std::string& name,
                            AlterEgo::GameMode mode, AlterEgo::SavedBuild& out);

static void RenderBuildPanel(const AlterEgo::Character& ch) {
    if (ch.build_tabs.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No build tabs available.");
        return;
    }

    // Build tab selector
    ImGui::Text("Build Tab:");
    for (int i = 0; i < (int)ch.build_tabs.size(); i++) {
        ImGui::SameLine();
        const auto& bt = ch.build_tabs[i];
        std::string label = bt.name.empty() ?
            ("Tab " + std::to_string(bt.tab)) : bt.name;
        if (bt.is_active) label += " *";

        bool active = (g_SelectedBuildTab == i);
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.30f, 0.14f, 1.0f));
        if (ImGui::SmallButton(label.c_str())) {
            g_SelectedBuildTab = i;
        }
        if (active) ImGui::PopStyleColor();
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
        ImGui::TextColored(specColor, "%s", specName.c_str());

        if (specInfo && specInfo->major_traits.size() >= 9) {
            // Positions for dotted lines
            ImVec2 specCenter(0, 0);
            ImVec2 minorCenters[3] = {};
            ImVec2 selectedCenters[3] = {};
            bool hasSelected[3] = {false, false, false};

            // Outer 2-column table: [Spec Icon] [6-col trait grid]
            char outerId[32];
            snprintf(outerId, sizeof(outerId), "##so_%u", spec.spec_id);
            if (ImGui::BeginTable(outerId, 2,
                    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                    SPEC_PORTRAIT_SIZE + 8.0f);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                // Column 0: Spec icon, aligned with middle trait row
                ImGui::TableNextColumn();
                float gridH = iconSz * 3 + ImGui::GetStyle().CellPadding.y * 6;
                float padY = (gridH - SPEC_PORTRAIT_SIZE) * 0.5f;
                if (padY > 0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padY);

                if (specInfo && !specInfo->icon_url.empty()) {
                    Texture_t* tex = AlterEgo::IconManager::GetIcon(spec.spec_id);
                    if (tex && tex->Resource) {
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Image(tex->Resource,
                            ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    } else {
                        AlterEgo::IconManager::RequestIcon(spec.spec_id, specInfo->icon_url);
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Dummy(ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    }
                    // Spec tooltip
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
                }

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

            // Draw dotted lines connecting: spec → minor0 → sel0 → minor1 → sel1 → minor2 → sel2
            // Lines stop at icon edges (inset by radius) so they don't overlap icons
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 lineCol = IM_COL32(180, 230, 255, 180);
            float specR = SPEC_PORTRAIT_SIZE * 0.5f;
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

            ImVec2 prev = specCenter;
            float prevR = specR;
            for (int tier = 0; tier < 3; tier++) {
                if (minorCenters[tier].x > 0) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, minorCenters[tier], prevR, minorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = minorCenters[tier];
                    prevR = minorR;
                }
                if (hasSelected[tier]) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, selectedCenters[tier], prevR, majorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = selectedCenters[tier];
                    prevR = majorR;
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
        if (ImGui::Button(hasPalette ? "Copy Build Chat Link" : "Copy Build Chat Link (loading...)") && hasPalette) {
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
            ImGui::Text("Loading profession data...");
        else
            ImGui::Text("Export this build as a GW2 chat link");
        ImGui::EndTooltip();
    }

    // Save to Library button — opens dialog
    ImGui::SameLine();
    if (ImGui::Button(hasPalette ? "Save to Library" : "Save to Library (loading...)") && hasPalette) {
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
            for (int t = 0; t < 3; t++)
                saveLink.specs[i].traits[t] = (uint8_t)bt.specializations[i].traits[t];
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

    // Draw profession/elite spec icon overlay in the top-right of the build panel
    {
        ImVec2 buildPanelEnd = ImGui::GetCursorScreenPos();
        float panelW = ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x - buildPanelStart.x;
        float overlaySize = 64.0f;
        float margin = 8.0f;

        // Determine which icon to show: elite spec if present, else base profession
        uint32_t overlayIconId = 0;
        std::string overlayIconUrl;
        std::string overlayTooltip;

        // Check for elite spec in this build's specializations
        for (int si = 0; si < 3; si++) {
            uint32_t sid = bt.specializations[si].spec_id;
            if (sid == 0) continue;
            const auto* sInfo = AlterEgo::GW2API::GetSpecInfo(sid);
            if (sInfo && sInfo->elite) {
                // Use offset ID to avoid collision with hex portrait icon
                overlayIconId = 7000000 + sid;
                overlayIconUrl = sInfo->profession_icon_big_url;
                overlayTooltip = sInfo->name;
                break;
            }
        }

        // Fall back to profession emblem from a core spec
        if (overlayIconId == 0) {
            for (int si = 0; si < 3; si++) {
                uint32_t sid = bt.specializations[si].spec_id;
                if (sid == 0) continue;
                const auto* sInfo = AlterEgo::GW2API::GetSpecInfo(sid);
                if (sInfo && !sInfo->profession_icon_big_url.empty()) {
                    overlayIconId = 7000000 + sid;
                    overlayIconUrl = sInfo->profession_icon_big_url;
                    overlayTooltip = ch.profession;
                    break;
                }
            }
        }

        if (overlayIconId != 0) {
            Texture_t* tex = AlterEgo::IconManager::GetIcon(overlayIconId);
            if (tex && tex->Resource) {
                ImVec2 iconPos(
                    buildPanelStart.x + panelW - overlaySize - margin,
                    buildPanelStart.y + margin
                );
                ImGui::GetWindowDrawList()->AddImage(
                    tex->Resource,
                    iconPos,
                    ImVec2(iconPos.x + overlaySize, iconPos.y + overlaySize),
                    ImVec2(0, 0), ImVec2(1, 1),
                    IM_COL32(255, 255, 255, 50)
                );
            } else if (!overlayIconUrl.empty()) {
                AlterEgo::IconManager::RequestIcon(overlayIconId, overlayIconUrl);
            }
        }
    }
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

// --- Shared Build Templates ---

static nlohmann::json ExportBuildToJson(const AlterEgo::SavedBuild& build) {
    nlohmann::json j;
    j["v"] = 1;
    j["name"] = build.name;
    j["chat_link"] = build.chat_link;
    j["profession"] = build.profession;
    j["game_mode"] = GameModeLabel(build.game_mode);
    if (!build.notes.empty()) j["notes"] = build.notes;

    // Gear
    if (!build.gear.empty()) {
        nlohmann::json gear = nlohmann::json::object();
        for (const auto& [slot, gs] : build.gear) {
            nlohmann::json s;
            if (gs.stat_id != 0) s["stat_id"] = gs.stat_id;
            if (!gs.stat_name.empty()) s["stat"] = gs.stat_name;
            if (!gs.rune.empty()) s["rune"] = gs.rune;
            if (!gs.sigil.empty()) s["sigil"] = gs.sigil;
            if (!gs.infusion.empty()) s["infusion"] = gs.infusion;
            if (!gs.weapon_type.empty()) s["weapon"] = gs.weapon_type;
            if (!s.empty()) gear[slot] = s;
        }
        if (!gear.empty()) j["gear"] = gear;
    }
    if (!build.rune_name.empty()) j["rune"] = build.rune_name;
    if (!build.relic_name.empty()) j["relic"] = build.relic_name;

    return j;
}

static std::string ExportBuildToBase64(const AlterEgo::SavedBuild& build) {
    std::string json = ExportBuildToJson(build).dump();
    std::string b64 = AlterEgo::ChatLink::Base64Encode(
        (const uint8_t*)json.data(), json.size());
    return "AE1:" + b64;
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
    for (const auto& [slot, gs] : build.gear) {
        if (slot.find("Weapon") == 0) {
            uint32_t sid = gs.sigil_id;
            if (sid == 0 && !gs.sigil.empty())
                sid = AlterEgo::GW2API::FindItemIdByName(gs.sigil);
            resolvedSigilIds[slot] = sid;
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
    buf.push_back(2); // version
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
    if (version != 2) { error = "Unsupported AE2 version."; return false; }

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
            if (pos + 6 > data.size()) break; // 4 (sigil) + 2 (weapon type)
            uint32_t sigilId = ReadU32LE(&data[pos]); pos += 4;
            uint16_t weaponTypeId = ReadU16LE(&data[pos]); pos += 2;
            const char* slotName = AE2SlotName(i);
            auto it = out.gear.find(slotName);
            if (it != out.gear.end()) {
                it->second.sigil_id = sigilId;
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

// Returns true if input is a shared build template (AE2, AE1, or JSON)
// On success, populates 'out' with the imported build
static bool ImportSharedBuild(const std::string& input, AlterEgo::SavedBuild& out, std::string& error) {
    // Try AE2 compact binary format first
    if (input.size() > 4 && input.substr(0, 4) == "AE2:") {
        return ImportBuildFromAE2(input, out, error);
    }

    std::string jsonStr;

    // Detect format
    if (input.size() > 4 && input.substr(0, 4) == "AE1:") {
        // Base64 encoded shared build
        auto bytes = AlterEgo::ChatLink::Base64Decode(input.substr(4));
        if (bytes.empty()) { error = "Failed to decode shared build data."; return false; }
        jsonStr.assign(bytes.begin(), bytes.end());
    } else if (!input.empty() && input[0] == '{') {
        // Raw JSON
        jsonStr = input;
    } else {
        return false; // Not a shared build format
    }

    try {
        auto j = nlohmann::json::parse(jsonStr);
        if (!j.contains("v") || !j.contains("chat_link") || !j.contains("name")) {
            error = "Invalid shared build: missing required fields.";
            return false;
        }

        std::string chatLink = j["chat_link"].get<std::string>();
        std::string name = j["name"].get<std::string>();
        std::string profession = j.value("profession", "");
        AlterEgo::GameMode mode = GameModeFromLabel(j.value("game_mode", "PvE"));

        // Need palette data to decode the build
        if (!profession.empty() && !AlterEgo::GW2API::HasPaletteData(profession)) {
            AlterEgo::GW2API::FetchProfessionPaletteAsync(profession);
            error = "Loading " + profession + " skill data... try again in a moment.";
            return false;
        }

        if (!DecodeBuildLink(chatLink, name, mode, out)) {
            error = "Failed to decode build chat link.";
            return false;
        }

        if (j.contains("notes")) out.notes = j["notes"].get<std::string>();

        // Restore gear data
        if (j.contains("gear")) {
            for (auto& [slot, gs_json] : j["gear"].items()) {
                AlterEgo::BuildGearSlot gs;
                gs.slot = slot;
                if (gs_json.contains("stat_id")) gs.stat_id = gs_json["stat_id"].get<uint32_t>();
                if (gs_json.contains("stat")) gs.stat_name = gs_json["stat"].get<std::string>();
                if (gs_json.contains("rune")) gs.rune = gs_json["rune"].get<std::string>();
                if (gs_json.contains("sigil")) gs.sigil = gs_json["sigil"].get<std::string>();
                if (gs_json.contains("infusion")) gs.infusion = gs_json["infusion"].get<std::string>();
                if (gs_json.contains("weapon")) gs.weapon_type = gs_json["weapon"].get<std::string>();
                out.gear[slot] = gs;
            }
        }
        if (j.contains("rune")) out.rune_name = j["rune"].get<std::string>();
        if (j.contains("relic")) out.relic_name = j["relic"].get<std::string>();

        return true;
    } catch (const std::exception& e) {
        error = std::string("Failed to parse shared build: ") + e.what();
        return false;
    }
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
            if (ae2data.size() >= 4 && ae2data[0] == 2) {
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

    PushGW2Theme();

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

    PopGW2Theme();
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
    {"Relic of the Pirate Queen",   106221, "Quickness on disable"},
    {"Relic of the Mist Stranger",  106206, "Siphon health on hit"},
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
            const char* upgradeLabel = isArmor ? "Rune" : "Sigil";
            const std::string& upgradeName = isArmor ? gs.rune : gs.sigil;

            ImVec2 p = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            float h = 30.0f;
            ImU32 bg = (g_GearSelectorTab == 1) ? IM_COL32(50, 60, 70, 200) : IM_COL32(35, 35, 40, 150);
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bg, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + w, p.y + h),
                (g_GearSelectorTab == 1) ? IM_COL32(100, 160, 220, 255) : IM_COL32(60, 60, 60, 180), 3.0f);

            ImGui::SetCursorScreenPos(p);
            if (ImGui::InvisibleButton("##upgradeArea", ImVec2(w, h))) {
                g_GearSelectorTab = 1;
                memset(g_GearStatSearch, 0, sizeof(g_GearStatSearch));
            }

            ImVec2 tp(p.x + 6, p.y + 3);
            if (!upgradeName.empty()) {
                char buf[256];
                snprintf(buf, sizeof(buf), "%s: %s", upgradeLabel, upgradeName.c_str());
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(200, 180, 255, 255), buf);
            } else {
                char buf[64];
                snprintf(buf, sizeof(buf), "%s: (click to select)", upgradeLabel);
                ImGui::GetWindowDrawList()->AddText(tp, IM_COL32(120, 120, 120, 200), buf);
            }
            tp.y += 14.0f;
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h + 4));
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
                    bool isSel = (gs.sigil == SIGIL_LIST[i].name);

                    ImVec2 cs = ImGui::GetCursorScreenPos();
                    float cw = ImGui::GetContentRegionAvail().x;
                    float ch = 32.0f;
                    ImU32 bg = isSel ? IM_COL32(60, 80, 100, 200) : IM_COL32(35, 35, 40, 180);
                    ImGui::GetWindowDrawList()->AddRectFilled(cs, ImVec2(cs.x+cw, cs.y+ch), bg, 3.0f);
                    ImGui::GetWindowDrawList()->AddRect(cs, ImVec2(cs.x+cw, cs.y+ch),
                        isSel ? IM_COL32(100, 160, 220, 255) : IM_COL32(80, 80, 80, 180), 3.0f);

                    ImGui::SetCursorScreenPos(cs);
                    if (ImGui::InvisibleButton("##ssel", ImVec2(cw, ch))) {
                        gs.sigil = SIGIL_LIST[i].name;
                        gs.sigil_id = AlterEgo::GW2API::FindItemIdByName(SIGIL_LIST[i].name);
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
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Loading weapon data...");
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
                            } else if (isWeapon) {
                                gs.sigil = upName;
                                gs.sigil_id = eq.upgrades[ui];
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
        // Render real icon with legendary border
        ImVec4 borderColor = hasData ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.5f, 0.3f, 0.6f, 0.8f);
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(pos.x - 1, pos.y - 1),
            ImVec2(pos.x + iconSz + 1, pos.y + iconSz + 1),
            ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, 2.0f);
        ImGui::Image(tex->Resource, ImVec2(iconSz, iconSz));
    } else {
        // Fallback: colored placeholder with slot initial
        ImU32 bgCol = hasData ? IM_COL32(50, 45, 30, 200) : IM_COL32(40, 40, 40, 180);
        ImU32 borderCol = hasData ? IM_COL32(255, 200, 60, 200) : IM_COL32(80, 80, 80, 200);
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
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(pos.x + (iconSz - textSz.x) * 0.5f, pos.y + (iconSz - textSz.y) * 0.5f),
            hasData ? IM_COL32(255, 210, 80, 200) : IM_COL32(100, 100, 100, 180), initial);
        ImGui::Dummy(ImVec2(iconSz, iconSz));
    }

    // Invisible button over icon area (overlaid on top for click handling)
    ImGui::SetCursorScreenPos(pos);
    if (ImGui::InvisibleButton("##icon", ImVec2(iconSz, iconSz))) {
        AlterEgo::BuildGearSlot current;
        if (it != build.gear.end()) current = it->second;
        current.slot = slot;
        OpenGearDialog(build.id, slot, current);
    }
    bool hovered = ImGui::IsItemHovered();

    // Name text next to icon
    bool isRelicSlot = (strcmp(slot, "Relic") == 0);
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    if (isRelicSlot) {
        if (!build.relic_name.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.8f, 1.0f), "%s", build.relic_name.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Relic");
        }
    } else if (hasStat && hasWeaponType) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s %s",
            it->second.stat_name.c_str(), WeaponDisplayName(it->second.weapon_type));
    } else if (hasStat) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s %s",
            it->second.stat_name.c_str(), GearSlotDisplayName(slot));
    } else if (hasWeaponType) {
        ImGui::TextColored(ImVec4(0.8f, 0.75f, 0.5f, 1.0f), "%s %s",
            WeaponDisplayName(it->second.weapon_type), GearSlotDisplayName(slot));
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", GearSlotDisplayName(slot));
    }
    if (!isRelicSlot && it != build.gear.end() && !it->second.rune.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "%s", it->second.rune.c_str());
    }
    if (!isRelicSlot && it != build.gear.end() && !it->second.sigil.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "%s", it->second.sigil.c_str());
    }
    ImGui::EndGroup();

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
    ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Gear");
    ImGui::Separator();

    if (ImGui::BeginTable("##buildGear", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("##gearLeft", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##gearRight", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        // Left column: Armor
        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Armor");
        RenderBuildGearSlot(build, "Helm");
        RenderBuildGearSlot(build, "Shoulders");
        RenderBuildGearSlot(build, "Coat");
        RenderBuildGearSlot(build, "Gloves");
        RenderBuildGearSlot(build, "Leggings");
        RenderBuildGearSlot(build, "Boots");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Weapons");
        RenderBuildGearSlot(build, "WeaponA1");
        RenderBuildGearSlot(build, "WeaponA2");
        RenderBuildGearSlot(build, "WeaponB1");
        RenderBuildGearSlot(build, "WeaponB2");

        // Right column: Trinkets + Relic + Rune
        ImGui::TableNextColumn();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Trinkets");
        RenderBuildGearSlot(build, "Backpack");
        RenderBuildGearSlot(build, "Accessory1");
        RenderBuildGearSlot(build, "Accessory2");
        RenderBuildGearSlot(build, "Amulet");
        RenderBuildGearSlot(build, "Ring1");
        RenderBuildGearSlot(build, "Ring2");
        RenderBuildGearSlot(build, "Relic");

        ImGui::EndTable();
    }
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
static void RenderSavedBuildPreview(const AlterEgo::SavedBuild& build) {
    // Try to resolve any placeholder traits (negative values from import without spec cache)
    // Safe to cast: we only mutate trait values from negative to positive
    ResolveBuildTraitPlaceholders(const_cast<AlterEgo::SavedBuild&>(build));

    ImVec4 profColor = GetProfessionColor(build.profession);
    ImGui::TextColored(profColor, "%s", build.name.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", build.profession.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 0.65f, 0.4f, 1.0f), "[%s]", GameModeLabel(build.game_mode));

    ImGui::Separator();

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
        ImGui::TextColored(specColor, "%s", specName.c_str());

        if (specInfo && specInfo->major_traits.size() >= 9) {
            ImVec2 specCenter(0, 0);
            ImVec2 minorCenters[3] = {};
            ImVec2 selectedCenters[3] = {};
            bool hasSelected[3] = {false, false, false};

            char outerId[32];
            snprintf(outerId, sizeof(outerId), "##lso_%u", spec.spec_id);
            if (ImGui::BeginTable(outerId, 2,
                    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                    SPEC_PORTRAIT_SIZE + 8.0f);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                float gridH = iconSz * 3 + ImGui::GetStyle().CellPadding.y * 6;
                float padY = (gridH - SPEC_PORTRAIT_SIZE) * 0.5f;
                if (padY > 0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padY);

                if (specInfo && !specInfo->icon_url.empty()) {
                    Texture_t* tex = AlterEgo::IconManager::GetIcon(spec.spec_id);
                    if (tex && tex->Resource) {
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Image(tex->Resource,
                            ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    } else {
                        AlterEgo::IconManager::RequestIcon(spec.spec_id, specInfo->icon_url);
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        specCenter = ImVec2(p.x + SPEC_PORTRAIT_SIZE * 0.5f,
                                            p.y + SPEC_PORTRAIT_SIZE * 0.5f);
                        ImGui::Dummy(ImVec2(SPEC_PORTRAIT_SIZE, SPEC_PORTRAIT_SIZE));
                    }
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
                }

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

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32 lineCol = IM_COL32(180, 230, 255, 180);
            float specR = SPEC_PORTRAIT_SIZE * 0.5f;
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

            ImVec2 prev = specCenter;
            float prevR = specR;
            for (int tier = 0; tier < 3; tier++) {
                if (minorCenters[tier].x > 0) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, minorCenters[tier], prevR, minorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = minorCenters[tier];
                    prevR = minorR;
                }
                if (hasSelected[tier]) {
                    ImVec2 la, lb;
                    if (InsetLine(prev, selectedCenters[tier], prevR, majorR, la, lb))
                        DrawDottedLine(dl, la, lb, lineCol);
                    prev = selectedCenters[tier];
                    prevR = majorR;
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
    if (ImGui::Button(g_LibShowImport ? "Cancel Import" : "+ Import Build")) {
        g_LibShowImport = !g_LibShowImport;
        g_LibImportBuf[0] = '\0';
        g_LibImportName[0] = '\0';
        g_LibImportMode = 0;
        g_LibImportError.clear();
    }

    // Events: Chat addon status
    ImGui::SameLine();
    if (g_ChatAddonConnected) {
        ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Chat: Connected");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Chat: Not detected");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Install 'Events: Chat' from the Nexus addon library");
            ImGui::Text("to import builds shared in GW2 chat.");
            ImGui::EndTooltip();
        }
    }

    // Import panel
    if (g_LibShowImport) {
        ImGui::Separator();
        ImGui::Text("Paste chat link, shared build code (AE1:...), or JSON:");
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

        ImGui::SameLine();
        if (ImGui::Button("Import")) {
            std::string input(g_LibImportBuf);
            std::string name(g_LibImportName);
            if (input.empty()) {
                g_LibImportError = "Paste a build first.";
            } else {
                // Try shared build format first (AE1: base64 or JSON)
                AlterEgo::SavedBuild sharedBuild;
                std::string sharedError;
                if (ImportSharedBuild(input, sharedBuild, sharedError)) {
                    AlterEgo::GW2API::AddSavedBuild(std::move(sharedBuild));
                    g_LibShowImport = false;
                    g_LibImportBuf[0] = '\0';
                    g_LibImportName[0] = '\0';
                    g_LibImportError.clear();
                    g_LibSelectedIdx = (int)AlterEgo::GW2API::GetSavedBuilds().size() - 1;
                    g_LibDetailsFetched = false;
                    if (APIDefs) APIDefs->GUI_SendAlert("Shared build imported!");
                } else if (!sharedError.empty()) {
                    // Shared format detected but failed
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
                                        AlterEgo::GW2API::AddSavedBuild(std::move(build));
                                        g_LibShowImport = false;
                                        g_LibImportBuf[0] = '\0';
                                        g_LibImportName[0] = '\0';
                                        g_LibImportError.clear();
                                        g_LibSelectedIdx = (int)AlterEgo::GW2API::GetSavedBuilds().size() - 1;
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

    if (builds.empty() && !g_LibShowImport) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
            "No saved builds. Click '+ Import Build' to add one from a chat link.");
        return;
    }

    ImGui::Spacing();

    // Filter + Search
    ImGui::SetNextItemWidth(80.0f);
    ImGui::Combo("##lib_filter", &g_LibFilterMode, GameModeNames, IM_ARRAYSIZE(GameModeNames));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputTextWithHint("##lib_search", "Search...", g_LibSearchBuf, sizeof(g_LibSearchBuf));

    // Left panel: build list, Right panel: preview
    float libAvailW = ImGui::GetContentRegionAvail().x;
    g_LibListWidth = (g_LibListWidth < 120.0f) ? 120.0f : (g_LibListWidth > libAvailW - 200.0f) ? libAvailW - 200.0f : g_LibListWidth;

    ImGui::BeginChild("LibList", ImVec2(g_LibListWidth, 0), true);
    {
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

            // Profession group header with gradient background
            if (b.profession != lastProf) {
                if (!lastProf.empty()) ImGui::Spacing();
                ImVec4 profColor = GetProfessionColor(b.profession);

                ImVec2 hdrPos = ImGui::GetCursorScreenPos();
                float hdrW = ImGui::GetContentRegionAvail().x;
                float hdrH = ImGui::GetTextLineHeightWithSpacing() + 2.0f;
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImU32 hdrLeft = ImGui::ColorConvertFloat4ToU32(
                    ImVec4(profColor.x * 0.25f, profColor.y * 0.25f, profColor.z * 0.25f, 0.60f));
                ImU32 hdrRight = IM_COL32(0, 0, 0, 0);
                dl->AddRectFilledMultiColor(
                    hdrPos, ImVec2(hdrPos.x + hdrW, hdrPos.y + hdrH),
                    hdrLeft, hdrRight, hdrRight, hdrLeft);
                // Gold underline
                dl->AddLine(
                    ImVec2(hdrPos.x, hdrPos.y + hdrH),
                    ImVec2(hdrPos.x + hdrW * 0.6f, hdrPos.y + hdrH),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(profColor.x, profColor.y, profColor.z, 0.35f)), 1.0f);

                ImGui::TextColored(profColor, "%s", b.profession.c_str());
                lastProf = b.profession;
            }

            ImGui::PushID(i);
            bool selected = (g_LibSelectedIdx == i);
            ImVec4 bProfColor = GetProfessionColor(b.profession);

            // Profession-tinted selection highlight
            ImVec4 bSelCol(bProfColor.x * 0.35f, bProfColor.y * 0.35f, bProfColor.z * 0.35f, 0.65f);
            ImVec4 bHovCol(bProfColor.x * 0.25f, bProfColor.y * 0.25f, bProfColor.z * 0.25f, 0.50f);
            ImVec4 bActCol(bProfColor.x * 0.40f, bProfColor.y * 0.40f, bProfColor.z * 0.40f, 0.75f);
            ImGui::PushStyleColor(ImGuiCol_Header, bSelCol);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, bHovCol);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, bActCol);

            if (ImGui::Selectable("##build", selected, 0, ImVec2(0, 32))) {
                g_LibSelectedIdx = i;
                g_LibDetailsFetched = false;
            }

            // Profession-colored left accent bar
            {
                ImVec2 bRMin = ImGui::GetItemRectMin();
                ImVec2 bRMax = ImGui::GetItemRectMax();
                ImDrawList* dl = ImGui::GetWindowDrawList();
                float barW = 3.0f;
                ImU32 barCol = ImGui::ColorConvertFloat4ToU32(
                    ImVec4(bProfColor.x, bProfColor.y, bProfColor.z,
                           selected ? 1.0f : (ImGui::IsItemHovered() ? 0.7f : 0.35f)));
                dl->AddRectFilled(
                    ImVec2(bRMin.x, bRMin.y + 1), ImVec2(bRMin.x + barW, bRMax.y - 1),
                    barCol, 1.0f);
                if (selected) {
                    ImU32 glowCol = ImGui::ColorConvertFloat4ToU32(
                        ImVec4(bProfColor.x, bProfColor.y, bProfColor.z, 0.08f));
                    dl->AddRectFilledMultiColor(
                        ImVec2(bRMin.x + barW, bRMin.y),
                        ImVec2(bRMin.x + barW + 30.0f, bRMax.y),
                        glowCol, IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), glowCol);
                }
            }

            ImGui::PopStyleColor(3);

            // Record rect for drag-and-drop
            ImVec2 rMin = ImGui::GetItemRectMin();
            ImVec2 rMax = ImGui::GetItemRectMax();
            libItemRects.push_back({ rMin.y, rMax.y, i });

            // Drag source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                g_LibDragIdx = i;
                ImGui::SetDragDropPayload("BUILD_REORDER", &i, sizeof(int));
                ImGui::Text("%s", b.name.c_str());
                ImGui::EndDragDropSource();
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::Selectable("Copy build-only chat link")) {
                    CopyToClipboard(b.chat_link);
                    if (APIDefs) APIDefs->GUI_SendAlert("Build link copied to clipboard!");
                }
                if (ImGui::Selectable("Copy entire build + equipment code")) {
                    std::string ae2 = ExportBuildToAE2(b);
                    if (!ae2.empty()) {
                        CopyToClipboard(ae2);
                        char info[196];
                        snprintf(info, sizeof(info), "Build code copied to clipboard (%d chars). Paste in GW2 chat to share!", (int)ae2.size());
                        if (APIDefs) APIDefs->GUI_SendAlert(info);
                    } else {
                        if (APIDefs) APIDefs->GUI_SendAlert("Failed to generate build code. Build may be missing a chat link.");
                    }
                }
                ImGui::Separator();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                if (ImGui::Selectable("Delete this build")) {
                    g_LibCtxDeleteIdx = i;
                }
                ImGui::PopStyleColor();
                ImGui::EndPopup();
            }

            ImGui::SameLine(8);
            ImGui::BeginGroup();
            ImGui::Text("%s", b.name.c_str());
            ImGui::TextColored(ImVec4(0.55f, 0.50f, 0.38f, 1.0f), "%s", GameModeLabel(b.game_mode));
            ImGui::EndGroup();
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
            g_LibDetailsFetched = true;
        }

        // Inline rename + notes editing
        if (g_LibEditBuildId != build.id) {
            g_LibEditBuildId = build.id;
            strncpy(g_LibEditName, build.name.c_str(), sizeof(g_LibEditName) - 1);
            g_LibEditName[sizeof(g_LibEditName) - 1] = '\0';
            strncpy(g_LibEditNotes, build.notes.c_str(), sizeof(g_LibEditNotes) - 1);
            g_LibEditNotes[sizeof(g_LibEditNotes) - 1] = '\0';
        }

        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(250.0f);
        if (ImGui::InputText("##edit_name", g_LibEditName, sizeof(g_LibEditName),
                ImGuiInputTextFlags_EnterReturnsTrue)) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName, g_LibEditNotes);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName, g_LibEditNotes);
        }

        ImGui::Text("Notes:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputTextMultiline("##edit_notes", g_LibEditNotes, sizeof(g_LibEditNotes),
                ImVec2(0, 60))) {
            // live typing — save handled on deactivate
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            AlterEgo::GW2API::UpdateSavedBuild(build.id, g_LibEditName, g_LibEditNotes);
        }

        ImGui::Separator();

        RenderSavedBuildPreview(build);
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a build from the list.");
    }
    ImGui::EndChild();
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
    s.PopupRounding     = 4.0f;
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
    s.PopupBorderSize   = 1.0f;
    s.FrameBorderSize   = 0.0f;
    s.TabBorderSize     = 0.0f;

    // Colors — dark slate base with warm gold accents
    ImVec4* c = s.Colors;

    // Backgrounds
    c[ImGuiCol_WindowBg]             = ImVec4(0.08f, 0.08f, 0.10f, 0.96f);
    c[ImGuiCol_ChildBg]              = ImVec4(0.07f, 0.07f, 0.09f, 0.80f);
    c[ImGuiCol_PopupBg]              = ImVec4(0.10f, 0.10f, 0.12f, 0.96f);

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
    APIDefs->Events_Subscribe(EV_AE_ACH_PROGRESS_RESPONSE, OnAchProgressResponse);

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
    LoadAchTrackerState();
    LoadAchGroupCache();
    LoadAchDefCache();
    LoadAchNameIndex();
    LoadAchWaypoints();

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

        if (Skinventory::SkinCache::LoadFromDisk()) {
            Skinventory::SkinCache::UpdateCacheAsync();
        } else {
            Skinventory::SkinCache::FetchAllSkinsAsync();
        }

        if (!Skinventory::Commerce::LoadItemMap()) {
            Skinventory::Commerce::BuildItemMapAsync();
        }

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
    APIDefs->Events_Unsubscribe(EV_AE_ACH_PROGRESS_RESPONSE, OnAchProgressResponse);

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

void OnMumbleIdentityUpdated(void* eventArgs) {
    if (!eventArgs) return;
    const MumbleIdentity* id = (const MumbleIdentity*)eventArgs;
    std::string newName(id->Name);
    if (!newName.empty() && newName != g_CurrentCharName) {
        g_LoginTimestamps[newName] = (int64_t)std::time(nullptr);
        g_LoginTimestampsDirty = true;
    }
    g_CurrentCharName = newName;
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
// Clears - Backend
// =========================================================================

// Calculate the most recent daily reset (00:00 UTC) before 'now'
static std::chrono::system_clock::time_point CalcLastDailyReset(std::chrono::system_clock::time_point now) {
    auto tt = std::chrono::system_clock::to_time_t(now);
    struct tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    utc.tm_hour = 0; utc.tm_min = 0; utc.tm_sec = 0;
    auto reset = std::chrono::system_clock::from_time_t(
#ifdef _WIN32
        _mkgmtime(&utc)
#else
        timegm(&utc)
#endif
    );
    if (reset > now) reset -= std::chrono::hours(24);
    return reset;
}

// Calculate the most recent weekly reset (Monday 07:30 UTC) before 'now'
static std::chrono::system_clock::time_point CalcLastWeeklyReset(std::chrono::system_clock::time_point now) {
    auto tt = std::chrono::system_clock::to_time_t(now);
    struct tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    // tm_wday: 0=Sun, 1=Mon, ...
    int daysSinceMonday = (utc.tm_wday + 6) % 7; // Mon=0, Tue=1, ... Sun=6
    utc.tm_hour = 7; utc.tm_min = 30; utc.tm_sec = 0;
    utc.tm_mday -= daysSinceMonday;
    auto reset = std::chrono::system_clock::from_time_t(
#ifdef _WIN32
        _mkgmtime(&utc)
#else
        timegm(&utc)
#endif
    );
    if (reset > now) reset -= std::chrono::hours(24 * 7);
    return reset;
}

// Format time until next reset as human-readable string
static std::string FormatTimeUntilReset(std::chrono::system_clock::time_point resetTime,
                                         std::chrono::hours period) {
    auto now = std::chrono::system_clock::now();
    auto nextReset = resetTime + period;
    if (nextReset <= now) return "now";
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(nextReset - now).count();
    int h = (int)(remaining / 3600);
    int m = (int)((remaining % 3600) / 60);
    if (h > 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%dh %dm", h, m);
        return buf;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%dm", m);
    return buf;
}

// Parse tier from fractal achievement name
static std::string ParseFractalTier(const std::string& name) {
    if (name.find("Tier 4") != std::string::npos) return "T4";
    if (name.find("Tier 3") != std::string::npos) return "T3";
    if (name.find("Tier 2") != std::string::npos) return "T2";
    if (name.find("Tier 1") != std::string::npos) return "T1";
    if (name.find("Recommended") != std::string::npos) return "Rec";
    return "";
}

// Send H&S achievement query for all currently tracked clears IDs
static void SendClearsAchQuery() {
    std::vector<uint32_t> allIds;
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        for (const auto& e : g_DailyFractals) allIds.push_back(e.id);
        for (const auto& e : g_DailyBounties) allIds.push_back(e.id);
        for (const auto& e : g_WeeklyWings)   allIds.push_back(e.id);
        if (g_WeeklyStrikes.id > 0) allIds.push_back(g_WeeklyStrikes.id);
    }
    if (allIds.empty() || !APIDefs) return;

    HoardQueryAchievementRequest req{};
    req.api_version = HOARD_API_VERSION;
    strncpy(req.requester, "Alter Ego", sizeof(req.requester) - 1);
    strncpy(req.response_event, EV_AE_CLEARS_ACH_RESPONSE, sizeof(req.response_event) - 1);
    req.id_count = (uint32_t)std::min(allIds.size(), (size_t)200);
    for (uint32_t i = 0; i < req.id_count; i++) {
        req.ids[i] = allIds[i];
    }
    APIDefs->Events_Raise(EV_HOARD_QUERY_ACHIEVEMENT, &req);
}

// H&S achievement response handler for all clears
static void OnClearsAchResponse(void* eventArgs) {
    if (!eventArgs) return;
    auto* resp = (HoardQueryAchievementResponse*)eventArgs;
    if (resp->api_version != HOARD_API_VERSION) { delete resp; return; }

    if (resp->status != HOARD_STATUS_OK) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        if (resp->status == HOARD_STATUS_PENDING) {
            g_ClearsStatusMsg = "Waiting for H&S permission...";
            // Retry after a delay — user is being prompted by H&S
            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                SendClearsAchQuery();
            }).detach();
        } else {
            if (resp->status == HOARD_STATUS_DENIED)
                g_ClearsStatusMsg = "H&S permission denied";
            else
                g_ClearsStatusMsg = "H&S error";
            g_ClearsFetching = false;
        }
        delete resp;
        return;
    }

    // Build lookup maps from response
    struct AchData {
        bool done;
        int32_t current;
        int32_t max;
        std::set<uint32_t> bits;
    };
    std::unordered_map<uint32_t, AchData> achMap;
    for (uint32_t i = 0; i < resp->entry_count; i++) {
        auto& e = resp->entries[i];
        AchData ad;
        ad.done = e.done;
        ad.current = e.current;
        ad.max = e.max;
        for (uint32_t b = 0; b < e.bit_count && b < 64; b++) {
            ad.bits.insert(e.bits[b]);
        }
        achMap[e.id] = std::move(ad);
    }

    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);

        // Apply to all tracked entries
        auto applyTo = [&](ClearEntry& ce) {
            auto it = achMap.find(ce.id);
            if (it != achMap.end()) {
                ce.done = it->second.done;
                ce.current = it->second.current;
                ce.max = it->second.max;
                ce.bitDone.assign(ce.bitNames.size(), false);
                for (uint32_t bitIdx : it->second.bits) {
                    if (bitIdx < ce.bitDone.size()) {
                        ce.bitDone[bitIdx] = true;
                    }
                }
            }
        };

        for (auto& e : g_DailyFractals)  applyTo(e);
        for (auto& e : g_DailyBounties)  applyTo(e);
        for (auto& e : g_WeeklyWings)    applyTo(e);
        applyTo(g_WeeklyStrikes);

        g_ClearsFetched = true;
        g_ClearsFetching = false;
        g_ClearsStatusMsg = "";
        g_LastClearsCompletionQuery = std::chrono::steady_clock::now();
    }

    // Persist to disk
    SaveClearsCache();

    delete resp;
}

// Helper: fetch a category's achievement IDs from public API
static std::vector<uint32_t> FetchCategoryIds(uint32_t catId) {
    std::string url = "https://api.guildwars2.com/v2/achievements/categories/" + std::to_string(catId);
    std::string json = Skinventory::HttpClient::Get(url);
    std::vector<uint32_t> ids;
    try {
        auto j = nlohmann::json::parse(json);
        if (j.contains("achievements") && j["achievements"].is_array()) {
            for (const auto& id : j["achievements"]) {
                ids.push_back(id.get<uint32_t>());
            }
        }
    } catch (...) {}
    return ids;
}

// Helper: fetch achievement details (name, bits) from public API
static std::vector<ClearEntry> FetchAchievementDetails(const std::vector<uint32_t>& ids) {
    std::vector<ClearEntry> entries;
    if (ids.empty()) return entries;

    std::string idStr;
    for (size_t i = 0; i < ids.size(); i++) {
        if (i > 0) idStr += ",";
        idStr += std::to_string(ids[i]);
    }
    std::string json = Skinventory::HttpClient::Get(
        "https://api.guildwars2.com/v2/achievements?ids=" + idStr);
    if (json.empty()) return entries;

    try {
        auto j = nlohmann::json::parse(json);
        if (j.is_array()) {
            for (const auto& ach : j) {
                ClearEntry ce;
                ce.id = ach.value("id", 0u);
                ce.name = ach.value("name", "");
                if (ce.id == 0 || ce.name.empty()) continue;

                // Parse bits (sub-objectives)
                if (ach.contains("bits") && ach["bits"].is_array()) {
                    for (const auto& bit : ach["bits"]) {
                        ce.bitNames.push_back(bit.value("text", ""));
                    }
                }

                // Parse tier count from tiers
                if (ach.contains("tiers") && ach["tiers"].is_array()) {
                    auto& tiers = ach["tiers"];
                    if (!tiers.empty()) {
                        ce.max = tiers.back().value("count", 0);
                    }
                }

                entries.push_back(std::move(ce));
            }
        }
    } catch (...) {}
    return entries;
}

// Fetch all clears data (runs on background thread)
static void FetchClears() {
    if (g_ClearsFetching) return;
    g_ClearsFetching = true;
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_ClearsStatusMsg = "Fetching achievement data...";
        g_ClearsFetched = false;
    }

    std::thread([]() {
        // Step 1: Fetch all category achievement IDs
        auto fractalIds = FetchCategoryIds(CAT_DAILY_FRACTALS);
        auto bountyIds  = FetchCategoryIds(CAT_DAILY_BOUNTIES);
        auto weeklyIds  = FetchCategoryIds(CAT_WEEKLY_RAIDS);

        if (fractalIds.empty() && bountyIds.empty() && weeklyIds.empty()) {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_ClearsStatusMsg = "Failed to fetch achievement categories";
            g_ClearsFetching = false;
            return;
        }

        // Step 2: Fetch all achievement details
        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_ClearsStatusMsg = "Fetching achievement details...";
        }

        auto fractalEntries = FetchAchievementDetails(fractalIds);
        for (auto& e : fractalEntries) {
            e.tier = ParseFractalTier(e.name);
        }

        auto bountyEntries = FetchAchievementDetails(bountyIds);
        auto weeklyEntries = FetchAchievementDetails(weeklyIds);

        // Separate weekly entries into wings vs strikes meta
        ClearEntry strikesEntry{};
        std::vector<ClearEntry> wingEntries;
        for (auto& e : weeklyEntries) {
            if (e.id == ACH_WEEKLY_STRIKES) {
                strikesEntry = std::move(e);
            } else if (!e.bitNames.empty()) {
                // Per-wing achievements have bits for encounters
                wingEntries.push_back(std::move(e));
            }
        }

        // Sort wings into canonical W1-W8 order by achievement ID
        static const std::unordered_map<uint32_t, int> wingOrder = {
            {9128, 1}, // Spirit Vale
            {9147, 2}, // Salvation Pass
            {9182, 3}, // Stronghold of the Faithful
            {9144, 4}, // Bastion of the Penitent
            {9111, 5}, // Hall of Chains
            {9120, 6}, // Mythwright Gambit
            {9156, 7}, // Key of Ahdashim
            {9181, 8}, // Mount Balrior
        };
        std::sort(wingEntries.begin(), wingEntries.end(),
            [](const ClearEntry& a, const ClearEntry& b) {
                auto ai = wingOrder.find(a.id);
                auto bi = wingOrder.find(b.id);
                int ao = (ai != wingOrder.end()) ? ai->second : 99;
                int bo = (bi != wingOrder.end()) ? bi->second : 99;
                return ao < bo;
            });

        {
            std::lock_guard<std::mutex> lock(g_ClearsMutex);
            g_DailyFractals = std::move(fractalEntries);
            g_DailyBounties = std::move(bountyEntries);
            g_WeeklyWings = std::move(wingEntries);
            g_WeeklyStrikes = std::move(strikesEntry);
            g_ClearsStatusMsg = "Querying completion...";
        }

        // Step 3: Query H&S for completion
        SendClearsAchQuery();
    }).detach();
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

static void RenderSkinCategoryNav() {
    ImGui::BeginChild("CategoryNav", ImVec2(180, 0), true);

    auto drawBullet = [](ImVec4 color) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float y_center = pos.y + ImGui::GetTextLineHeight() * 0.5f;
        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(pos.x + 4.0f, y_center), 4.0f,
            ImGui::GetColorU32(color));
        ImGui::Dummy(ImVec2(12.0f, ImGui::GetTextLineHeight()));
        ImGui::SameLine(0, 2);
    };

    ImGui::Text("Category");
    ImGui::Separator();

    drawBullet(ImVec4(0.45f, 0.65f, 0.85f, 1.0f));
    if (ImGui::Selectable("Armor", g_SkinSelectedType == "Armor")) {
        g_SkinSelectedType = "Armor";
        g_SkinSelectedWeightClass = "Heavy";
        g_SkinSelectedSubtype = "Helm";
        g_SkinSelectedId = 0;
    }
    drawBullet(ImVec4(0.85f, 0.45f, 0.35f, 1.0f));
    if (ImGui::Selectable("Weapons", g_SkinSelectedType == "Weapon")) {
        g_SkinSelectedType = "Weapon";
        g_SkinSelectedWeightClass = "";
        g_SkinSelectedSubtype = "Axe";
        g_SkinSelectedId = 0;
    }
    drawBullet(ImVec4(0.65f, 0.50f, 0.80f, 1.0f));
    if (ImGui::Selectable("Back", g_SkinSelectedType == "Back")) {
        g_SkinSelectedType = "Back";
        g_SkinSelectedWeightClass = "";
        g_SkinSelectedSubtype = "";
        g_SkinSelectedId = 0;
    }

    ImGui::Separator();

    if (g_SkinSelectedType == "Armor") {
        ImGui::Text("Weight Class");
        auto wcColor = [](const std::string& wc) -> ImVec4 {
            if (wc == "Heavy")  return ImVec4(0.65f, 0.65f, 0.70f, 1.0f);
            if (wc == "Medium") return ImVec4(0.65f, 0.50f, 0.30f, 1.0f);
            if (wc == "Light")  return ImVec4(0.55f, 0.45f, 0.70f, 1.0f);
            return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        };
        for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
            if (wc == "Clothing") continue;
            size_t count = Skinventory::SkinCache::GetCategorySkinCount("Armor", g_SkinSelectedSubtype, wc);
            std::string label = wc + " (" + std::to_string(count) + ")";
            drawBullet(wcColor(wc));
            if (ImGui::Selectable(label.c_str(), g_SkinSelectedWeightClass == wc)) {
                g_SkinSelectedWeightClass = wc;
                g_SkinSelectedId = 0;
            }
        }

        ImGui::Separator();
        ImGui::Text("Slot");
        for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(g_SkinSelectedWeightClass)) {
            size_t count = Skinventory::SkinCache::GetCategorySkinCount("Armor", slot, g_SkinSelectedWeightClass);
            std::string label = slot + " (" + std::to_string(count) + ")";
            drawBullet(ImVec4(0.45f, 0.65f, 0.85f, 1.0f));
            if (ImGui::Selectable(label.c_str(), g_SkinSelectedSubtype == slot)) {
                g_SkinSelectedSubtype = slot;
                g_SkinSelectedId = 0;
            }
        }
    } else if (g_SkinSelectedType == "Weapon") {
        ImGui::Text("Weapon Type");
        for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
            size_t count = Skinventory::SkinCache::GetCategorySkinCount("Weapon", wt, "");
            std::string label = wt + " (" + std::to_string(count) + ")";
            drawBullet(ImVec4(0.85f, 0.45f, 0.35f, 1.0f));
            if (ImGui::Selectable(label.c_str(), g_SkinSelectedSubtype == wt)) {
                g_SkinSelectedSubtype = wt;
                g_SkinSelectedId = 0;
            }
        }
    }

    ImGui::EndChild();
}

static void RenderSkinList() {
    ImGui::BeginChild("SkinList", ImVec2(300, 0), true);

    ImGui::PushItemWidth(-1);
    ImGui::InputTextWithHint("##skinfilter", "Filter skins...", g_SkinSearchFilter, sizeof(g_SkinSearchFilter));
    ImGui::PopItemWidth();

    if (Skinventory::OwnedSkins::HasData()) {
        ImGui::Checkbox("Owned", &g_SkinShowOwned);
        ImGui::SameLine();
        ImGui::Checkbox("Unowned", &g_SkinShowUnowned);
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
    ImGuiListClipper clipper;
    clipper.Begin((int)displaySkins.size());
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
            const auto& entry = displaySkins[row];
            auto skinOpt = Skinventory::SkinCache::GetSkin(entry.id);
            if (!skinOpt) continue;
            const auto& skin = *skinOpt;

            if (row % 2 == 1) {
                ImVec2 rowMin = ImGui::GetCursorScreenPos();
                ImVec2 rowMax(rowMin.x + ImGui::GetContentRegionAvail().x,
                              rowMin.y + ImGui::GetTextLineHeightWithSpacing());
                ImGui::GetWindowDrawList()->AddRectFilled(rowMin, rowMax,
                    ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.28f, 0.35f)));
            }

            bool selected = (g_SkinSelectedId == entry.id);
            if (hasOwnerData) {
                if (entry.owned) {
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "+");
                } else {
                    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), " -");
                }
                ImGui::SameLine(0, 4);
            }
            std::string label = skin.name + "##" + std::to_string(entry.id);
            ImGui::PushStyleColor(ImGuiCol_Text, GetSkinRarityColor(skin.rarity));
            if (ImGui::Selectable(label.c_str(), selected)) {
                g_SkinSelectedId = entry.id;
                Skinventory::WikiImage::RequestImage(entry.id, skin.name, skin.weight_class);
                Skinventory::Commerce::FetchPriceForSkin(entry.id);
            }
            ImGui::PopStyleColor();
            if (selected && g_SkinScrollToSkin) {
                g_SkinScrollToSkin = false;
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

    ImGui::PushStyleColor(ImGuiCol_Text, GetSkinRarityColor(skin.rarity));
    ImGui::TextWrapped("%s", skin.name.c_str());
    ImGui::PopStyleColor();

    ImGui::Text("%s%s%s",
        skin.type.c_str(),
        skin.subtype.empty() ? "" : (" / " + skin.subtype).c_str(),
        skin.weight_class.empty() ? "" : (" / " + skin.weight_class).c_str());
    ImGui::TextColored(GetSkinRarityColor(skin.rarity), "%s", skin.rarity.c_str());
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(ID: %u)", skin.id);

    if (Skinventory::OwnedSkins::HasData()) {
        bool owned = Skinventory::OwnedSkins::IsOwned(g_SkinSelectedId);
        if (owned) {
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "OWNED");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "NOT OWNED");
        }
    }

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
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Loading wiki image...");
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
    if (ImGui::Button("Refresh Prices") && !Skinventory::Commerce::IsFetching()) {
        g_SkinShopListDirty = true;
        s_needsFetch = true;
    }
    ImGui::SameLine();
    ImGui::Text("Category:");
    ImGui::SameLine();
    static int shopTypeFilter = 0;
    int prevFilter = shopTypeFilter;
    ImGui::RadioButton("All##shop", &shopTypeFilter, 0); ImGui::SameLine();
    ImGui::RadioButton("Armor##shop", &shopTypeFilter, 1); ImGui::SameLine();
    ImGui::RadioButton("Weapons##shop", &shopTypeFilter, 2);
    if (shopTypeFilter != prevFilter) {
        g_SkinShopListDirty = true;
    }

    ImGui::Text("Source:");
    ImGui::SameLine();
    static int shopSourceFilter = 0;
    int prevSource = shopSourceFilter;
    ImGui::RadioButton("All##src", &shopSourceFilter, 0); ImGui::SameLine();
    ImGui::RadioButton("TP##src", &shopSourceFilter, 1); ImGui::SameLine();
    ImGui::RadioButton("Vendor##src", &shopSourceFilter, 2);
    if (shopSourceFilter != prevSource) {
        g_SkinShopListDirty = true;
    }

    if (Skinventory::Commerce::IsFetching()) {
        std::string fetchMsg = Skinventory::Commerce::GetFetchStatus();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", fetchMsg.c_str());
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
    if (ImGui::BeginTable("ShoppingList", 5,
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Sortable,
        tableSize)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80, 1);
        ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed, 55, 2);
        ImGui::TableSetupColumn("Rarity", ImGuiTableColumnFlags_WidthFixed, 65, 3);
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
                        } else if (col == 3) {
                            cmp = sa->rarity.compare(sb->rarity);
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
                ImGui::TextColored(GetSkinRarityColor(skin.rarity), "%s", skin.rarity.c_str());

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

// Helper: strip fractal achievement name to short form
static std::string ShortenFractalName(const std::string& name) {
    // "Daily Recommended Fractal—Swampland" → "Swampland"
    auto pos = name.find("Fractal");
    if (pos != std::string::npos && pos + 7 < name.size()) {
        size_t start = pos + 7;
        while (start < name.size() && (name[start] == ' ' || (unsigned char)name[start] == 0xe2)) {
            if ((unsigned char)name[start] == 0xe2 && start + 2 < name.size())
                start += 3;
            else
                start++;
        }
        return name.substr(start);
    }
    // "Daily Tier 4 Swampland" → "Swampland"
    auto pos2 = name.find("Tier");
    if (pos2 != std::string::npos) {
        size_t afterTier = name.find(' ', pos2 + 5);
        if (afterTier != std::string::npos && afterTier + 1 < name.size())
            return name.substr(afterTier + 1);
    }
    return name;
}

// Helper: draw a gradient-backed section header with colored accent underline
static void RenderSectionHeader(const char* label, ImVec4 color, const char* suffix) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float h = ImGui::GetTextLineHeightWithSpacing() + 4.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Gradient background
    ImU32 left = ImGui::ColorConvertFloat4ToU32(
        ImVec4(color.x * 0.20f, color.y * 0.20f, color.z * 0.20f, 0.50f));
    ImU32 right = IM_COL32(0, 0, 0, 0);
    dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + w, pos.y + h), left, right, right, left);

    // Accent underline
    dl->AddLine(ImVec2(pos.x, pos.y + h), ImVec2(pos.x + w * 0.5f, pos.y + h),
        ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, 0.30f)), 1.0f);

    ImGui::SetCursorScreenPos(ImVec2(pos.x + 4.0f, pos.y + 2.0f));
    ImGui::TextColored(color, "%s", label);
    if (suffix) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.50f, 0.47f, 0.40f, 1.0f), "%s", suffix);
    }
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + h + 2.0f));
}

// Map verbose API encounter text to readable short names
static std::string ShortenEncounterName(const std::string& text) {
    static const std::unordered_map<std::string, std::string> nameMap = {
        // W1 — Spirit Vale
        {"Defeat the Vale Guardian.",                                      "Vale Guardian"},
        {"Traverse the Spirit Woods.",                                     "Spirit Woods"},
        {"Destroy Gorseval.",                                              "Gorseval"},
        {"Cull the bandits to lure out the bandit leader.",                ""},
        {"Defeat Sabetha the Saboteur.",                                   "Sabetha"},
        // W2 — Salvation Pass
        {"Defeat Slothasor.",                                              "Sloth"},
        {"Protect the caged prisoners.",                                   "Trio"},
        {"Cull the bandits in the ruins.",                                 ""},
        {"Defeat Inquisitor Matthias Gabrel.",                             "Matthias"},
        // W3 — Stronghold of the Faithful
        {"Escort Glenna to the stronghold's courtyard.",                   "Escort"},
        {"Defeat McLeod and breach the stronghold.",                       ""},
        {"Destroy the Keep Construct.",                                    "Keep Construct"},
        {"Traverse the Twisted Castle.",                                   "Twisted Castle"},
        {"Defeat Xera.",                                                   "Xera"},
        // W4 — Bastion of the Penitent
        {"Defeat Cairn the Indomitable.",                                  "Cairn"},
        {"Defeat the Mursaat Overseer.",                                   "Mursaat"},
        {"Defeat Samarog.",                                                "Samarog"},
        {"Free the prisoner from his bonds.",                              "Deimos"},
        // W5 — Hall of Chains
        {"Defeat the Soulless Horror.",                                    "Desmina"},
        {"Traverse the River of Souls.",                                   "River"},
        {"Restore the Statue of Ice.",                                     "Broken King"},
        {"Restore the Statue of Death and Resurrection.",                  "Eater"},
        {"Restore the Statue of Darkness.",                                "Eyes"},
        {"Defeat Dhuum.",                                                  "Dhuum"},
        // W6 — Mythwright Gambit
        {"Destroy the conjured amalgamate.",                               "Conjured Amalgamate"},
        {"Make your way through the sorting and appraisal rooms.",         "Sorting"},
        {"Defeat the twin largos.",                                        "Twin Largos"},
        {"Clear a path through Qadim's minions to the Mythwright Cauldron.", "Qadim's Minions"},
        {"Defeat Qadim.",                                                  "Qadim"},
        // W7 — Key of Ahdashim
        {"Get Glenna and the key to Ahdashim's front gate.",               "Gate"},
        {"Defeat Cardinal Adina.",                                         "Adina"},
        {"Defeat Cardinal Sabir.",                                         "Sabir"},
        {"Defeat Qadim the Peerless.",                                     "Qadim the Peerless"},
        // W8 — Mount Balrior
        {"Cleanse the camp of titanspawn.",                                "Camp"},
        {"Defeat the sentient conduit.",                                   "Conduit"},
        {"Defeat the blighted beast and his empowering allies.",           ""},
        {"Defeat Decima.",                                                 "Decima"},
        {"Defeat Greer.",                                                  "Greer"},
        {"Defeat Ura.",                                                    "Ura"},
    };
    auto it = nameMap.find(text);
    if (it != nameMap.end()) return it->second;
    return text; // fallback to original
}

// Helper: render a done/not-done/unknown indicator
static void RenderClearStatus(bool fetched, bool done, const char* label) {
    if (!fetched)
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "[?] %s", label);
    else if (done)
        ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "[x] %s", label);
    else
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "[ ] %s", label);
}

static void RenderClears() {
    auto now = std::chrono::system_clock::now();

    // Compute reset times (add 60s buffer so we don't refetch before the API has updated)
    auto dailyReset = CalcLastDailyReset(now);
    auto weeklyReset = CalcLastWeeklyReset(now);
    auto dailyResetBuffered = dailyReset + std::chrono::seconds(60);
    auto weeklyResetBuffered = weeklyReset + std::chrono::seconds(60);

    // Detect resets — clear stale data and auto re-fetch achievement lists
    bool resetTriggered = false;
    if (g_LastDailyReset != std::chrono::system_clock::time_point{} && dailyReset > g_LastDailyReset && now >= dailyResetBuffered) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_DailyFractals.clear();
        g_DailyBounties.clear();
        g_ClearsFetched = false;
        resetTriggered = true;
    }
    if (g_LastWeeklyReset != std::chrono::system_clock::time_point{} && weeklyReset > g_LastWeeklyReset && now >= weeklyResetBuffered) {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        g_WeeklyWings.clear();
        g_WeeklyStrikes = ClearEntry{};
        g_ClearsFetched = false;
        resetTriggered = true;
    }
    g_LastDailyReset = dailyReset;
    g_LastWeeklyReset = weeklyReset;

    // Auto re-fetch full data at reset boundaries
    if (resetTriggered && !g_ClearsFetching) {
        FetchClears();
    }

    // Auto re-query completion every 10 minutes (if we have data and not currently fetching)
    auto steadyNow = std::chrono::steady_clock::now();
    if (g_ClearsFetched && !g_ClearsFetching &&
        g_LastClearsCompletionQuery != std::chrono::steady_clock::time_point{} &&
        (steadyNow - g_LastClearsCompletionQuery) >= std::chrono::minutes(10)) {
        g_LastClearsCompletionQuery = steadyNow;
        SendClearsAchQuery();
    }

    // Refresh button + status
    bool fetching = g_ClearsFetching;
    if (fetching) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    if (ImGui::SmallButton("Refresh##clears") && !fetching) {
        FetchClears();
    }
    if (fetching) ImGui::PopStyleVar();
    ImGui::SameLine();
    {
        std::lock_guard<std::mutex> lock(g_ClearsMutex);
        if (fetching) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", g_ClearsStatusMsg.c_str());
        } else if (!g_ClearsFetched && g_DailyFractals.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Click Refresh to load data");
        } else if (!g_ClearsStatusMsg.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", g_ClearsStatusMsg.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Data loaded");
        }
    }

    ImGui::Separator();

    std::lock_guard<std::mutex> lock(g_ClearsMutex);

    std::string dailyResetStr = FormatTimeUntilReset(dailyReset, std::chrono::hours(24));
    std::string weeklyResetStr = FormatTimeUntilReset(weeklyReset, std::chrono::hours(24 * 7));

    // ---- Daily Fractals ----
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", dailyResetStr.c_str());
        RenderSectionHeader("Daily Fractals", ImVec4(0.3f, 0.7f, 0.9f, 1.0f), suffix);
    }

    if (g_DailyFractals.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        const char* tierOrder[] = {"T4", "T3", "T2", "T1", "Rec"};
        for (const char* tier : tierOrder) {
            std::vector<const ClearEntry*> tierEntries;
            for (const auto& e : g_DailyFractals) {
                if (e.tier == tier) tierEntries.push_back(&e);
            }
            if (tierEntries.empty()) continue;

            // Sort by shortened name so order is consistent across tiers
            std::sort(tierEntries.begin(), tierEntries.end(),
                [](const ClearEntry* a, const ClearEntry* b) {
                    return ShortenFractalName(a->name) < ShortenFractalName(b->name);
                });

            bool allDone = g_ClearsFetched;
            for (const auto* e : tierEntries) {
                if (!e->done) { allDone = false; break; }
            }

            if (allDone)
                ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "  %-4s", tier);
            else
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "  %-4s", tier);

            for (const auto* e : tierEntries) {
                ImGui::SameLine();
                std::string shortName = ShortenFractalName(e->name);
                RenderClearStatus(g_ClearsFetched, e->done, shortName.c_str());
            }
        }
        // Uncategorized
        for (const auto& e : g_DailyFractals) {
            if (!e.tier.empty()) continue;
            ImGui::Text("  ");
            ImGui::SameLine();
            RenderClearStatus(g_ClearsFetched, e.done, e.name.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Daily Raid Bounties ----
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", dailyResetStr.c_str());
        RenderSectionHeader("Daily Raid Bounties", ImVec4(0.9f, 0.6f, 0.3f, 1.0f), suffix);
    }

    if (g_DailyBounties.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        for (const auto& e : g_DailyBounties) {
            // Strip "Raid Bounty: " prefix
            std::string display = e.name;
            if (display.find("Raid Bounty: ") == 0)
                display = display.substr(13);
            ImGui::Text("  ");
            ImGui::SameLine();
            RenderClearStatus(g_ClearsFetched, e.done, display.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Weekly Strikes ----
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", weeklyResetStr.c_str());
        RenderSectionHeader("Weekly Strikes", ImVec4(0.7f, 0.4f, 0.9f, 1.0f), suffix);
    }

    if (g_WeeklyStrikes.id == 0) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        for (size_t i = 0; i < g_WeeklyStrikes.bitNames.size(); i++) {
            bool bitDone = (i < g_WeeklyStrikes.bitDone.size()) ? g_WeeklyStrikes.bitDone[i] : false;
            ImGui::Text("  ");
            ImGui::SameLine();
            RenderClearStatus(g_ClearsFetched, bitDone, g_WeeklyStrikes.bitNames[i].c_str());
        }
        if (g_ClearsFetched && g_WeeklyStrikes.max > 0) {
            int doneCount = 0;
            for (bool b : g_WeeklyStrikes.bitDone) { if (b) doneCount++; }
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  %d / %d completed",
                doneCount, (int)g_WeeklyStrikes.bitNames.size());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Weekly Raids ----
    {
        char suffix[64];
        snprintf(suffix, sizeof(suffix), "(resets in %s)", weeklyResetStr.c_str());
        RenderSectionHeader("Weekly Raids", ImVec4(0.9f, 0.7f, 0.3f, 1.0f), suffix);
    }

    if (g_WeeklyWings.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  No data");
    } else {
        if (ImGui::BeginTable("RaidWingsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Wing", ImGuiTableColumnFlags_WidthFixed, 220);
            ImGui::TableSetupColumn("Encounters", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto& wing : g_WeeklyWings) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // Strip "Weekly " prefix and add W# prefix
                std::string wingName = wing.name;
                if (wingName.find("Weekly ") == 0)
                    wingName = wingName.substr(7);
                {
                    static const std::unordered_map<uint32_t, int> wingNum = {
                        {9128, 1}, {9147, 2}, {9182, 3}, {9144, 4},
                        {9111, 5}, {9120, 6}, {9156, 7}, {9181, 8},
                    };
                    auto it = wingNum.find(wing.id);
                    if (it != wingNum.end())
                        wingName = "W" + std::to_string(it->second) + ": " + wingName;
                }

                bool allDone = g_ClearsFetched && wing.done;
                if (allDone)
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "%s", wingName.c_str());
                else
                    ImGui::Text("%s", wingName.c_str());

                ImGui::TableNextColumn();
                // Render encounters, skipping progress-only bits (empty short name)
                bool firstEnc = true;
                for (size_t i = 0; i < wing.bitNames.size(); i++) {
                    std::string shortName = ShortenEncounterName(wing.bitNames[i]);
                    if (shortName.empty()) continue; // skip progress-only bits
                    bool bitDone = (i < wing.bitDone.size()) ? wing.bitDone[i] : false;
                    if (!firstEnc) {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "|");
                        ImGui::SameLine();
                    }
                    RenderClearStatus(g_ClearsFetched, bitDone, shortName.c_str());
                    firstEnc = false;
                }
            }
            ImGui::EndTable();
        }
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

    // Achievement name row — clickable if has bits
    {
        // Build the header label
        std::string headerText;
        if (done) {
            headerText = "[x]  " + def.name;
        } else {
            headerText = "[ ]  " + def.name;
        }
        if (def.total_ap > 0) {
            headerText += "  (" + std::to_string(def.total_ap) + " AP)";
        }
        if (max > 0) {
            headerText += "  " + std::to_string(done ? max : current) + "/" + std::to_string(max);
        }

        ImVec4 headerColor = done ? ImVec4(0.35f, 0.82f, 0.35f, 1.0f) : ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

        if (hasBits) {
            std::string selectableLabel = headerText + "###ach" + std::to_string(achId);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_Text, headerColor);
            if (ImGui::Selectable(selectableLabel.c_str(), expanded, ImGuiSelectableFlags_AllowItemOverlap)) {
                if (expanded) g_AchExpandedInList.erase(achId);
                else g_AchExpandedInList.insert(achId);
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::TextColored(headerColor, "%s", headerText.c_str());
        }

        // Wiki link on right-click (on the header)
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            std::string wikiUrl = "https://wiki.guildwars2.com/wiki/" + def.name;
            std::replace(wikiUrl.begin(), wikiUrl.end(), ' ', '_');
            ShellExecuteA(NULL, "open", wikiUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }

    // Pin button (same line as header)
    if (showPinButton) {
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30.0f);
        bool pinned = IsAchPinned(achId);
        if (pinned) {
            if (ImGui::SmallButton("Unpin")) ToggleAchPin(achId);
        } else {
            if (ImGui::SmallButton("Pin")) ToggleAchPin(achId);
        }
    }

    // Progress bar
    if (max > 0) {
        float frac = done ? 1.0f : (float)current / (float)max;
        ImVec4 barColor = done ? ImVec4(0.3f, 0.6f, 0.25f, 0.6f) : ImVec4(0.4f, 0.7f, 0.3f, 1.0f);
        RenderAchProgressBar(frac, barColor);
    }

    // Requirement text + achievement-level waypoint button
    if (!done && !def.requirement.empty()) {
        std::string cleanReq = StripGW2Markup(def.requirement);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  %s", cleanReq.c_str());
    }
    // Achievement-level waypoint (bitIndex -1)
    {
        auto wpIt = g_AchWaypoints.find(achId);
        if (wpIt != g_AchWaypoints.end()) {
            auto wpBitIt = wpIt->second.find(-1);
            if (wpBitIt != wpIt->second.end()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("WP")) {
                    ImGui::SetClipboardText(wpBitIt->second.c_str());
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Copy waypoint: %s", wpBitIt->second.c_str());
                }
            }
        }
    }

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

    // Toolbar: search + popout toggle + refresh
    float searchWidth = ImGui::GetContentRegionAvail().x - 180.0f;
    if (searchWidth < 100.0f) searchWidth = 100.0f;
    ImGui::SetNextItemWidth(searchWidth);
    ImGui::InputTextWithHint("##AchSearch", "Search achievements...", g_AchSearchBuf, sizeof(g_AchSearchBuf));
    ImGui::SameLine();
    if (ImGui::Button(g_AchPopoutVisible ? "Hide Popout" : "Show Popout")) {
        g_AchPopoutVisible = !g_AchPopoutVisible;
        SaveAchTrackerState();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        g_AchGroupsFetched = false;
        FetchAchGroups();
        if (g_AchSelectedCatId > 0) FetchAchCategoryDefs(g_AchSelectedCatId);
    }

    // Status message
    {
        std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
        if (!g_AchStatusMsg.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", g_AchStatusMsg.c_str());
        }
    }

    if (!g_AchGroupsFetched) return;

    // Search mode — cached results to avoid iterating 8000+ names every frame
    bool searchActive = g_AchSearchBuf[0] != '\0';
    if (searchActive) {
        std::string query(g_AchSearchBuf);
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);

        if (!g_AchNameIndexReady) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Loading search index...");
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
        for (const auto& group : g_AchGroups) {
            if (group.name.empty()) continue;

            // Auto-open the saved group
            bool forceOpen = g_AchRestoreScroll && (group.id == g_AchSelectedGroupId);
            if (forceOpen) ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNode(group.name.c_str())) {
                // Sort categories by order within the group
                std::vector<const AchCategoryDef*> sortedCats;
                for (uint32_t catId : group.categories) {
                    auto it = g_AchCategories.find(catId);
                    if (it != g_AchCategories.end() && !it->second.name.empty()) {
                        sortedCats.push_back(&it->second);
                    }
                }
                std::sort(sortedCats.begin(), sortedCats.end(),
                    [](const AchCategoryDef* a, const AchCategoryDef* b) { return a->order < b->order; });

                for (const auto* cat : sortedCats) {
                    bool selected = (g_AchSelectedCatId == cat->id);
                    if (ImGui::Selectable(cat->name.c_str(), selected)) {
                        if (g_AchSelectedCatId != cat->id) {
                            g_AchSelectedCatId = cat->id;
                            g_AchSelectedGroupId = group.id;
                            FetchAchCategoryDefs(cat->id);
                        }
                    }
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
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Loading...");
                    }

                    for (uint32_t achId : catIt->second.achievements) {
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
    if (!g_AchPopoutVisible) return;
    if (g_CurrentCharName.empty()) return; // not yet logged in (character select screen)

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
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing)) {

        // Toolbar
        ImGui::Checkbox("Show completed steps", &g_AchShowCompletedSteps);
        ImGui::SameLine();
        if (ImGui::SmallButton("Refresh##pop")) {
            std::lock_guard<std::recursive_mutex> lock(g_AchMutex);
            if (!g_AchPinned.empty()) {
                SendAchProgressQuery(g_AchPinned);
            }
        }
        ImGui::Separator();

        if (s_popoutCache.empty()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No pinned achievements");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Pin achievements from the Achievements tab");
        }

        // Render from cache — no lock needed
        for (int idx = 0; idx < (int)s_popoutCache.size(); idx++) {
            const auto& disp = s_popoutCache[idx];
            ImGui::PushID(idx + 80000);
            ImGui::BeginGroup();

            if (disp.hasDef) {
                if (disp.done) {
                    ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "[x] %s", disp.name.c_str());
                } else if (disp.hasBits) {
                    bool expanded = g_AchExpandedInPopout[disp.achId];
                    std::string label = (expanded ? "v " : "> ") + disp.name;
                    if (disp.max > 0) label += "  " + std::to_string(disp.current) + "/" + std::to_string(disp.max);
                    if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap)) {
                        g_AchExpandedInPopout[disp.achId] = !expanded;
                        SaveAchTrackerState();
                    }
                } else {
                    std::string label = "  " + disp.name;
                    if (disp.max > 0) label += "  " + std::to_string(disp.current) + "/" + std::to_string(disp.max);
                    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", label.c_str());
                }

                if (!disp.done && disp.max > 0) {
                    float frac = (float)disp.current / (float)disp.max;
                    RenderAchProgressBar(frac, ImVec4(0.4f, 0.7f, 0.3f, 1.0f), 3.0f);
                }

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
            ImGui::Spacing();
        }
    }
    ImGui::End();
}

static void RenderSkinventory() {
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

        if (!canRefresh) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        if (ImGui::SmallButton("Refresh Owned") && canRefresh) {
            g_SkinRefreshOwned = true;
        }
        if (!canRefresh) ImGui::PopStyleVar();

        // Kick off batch query when requested
        if (g_SkinRefreshOwned && canRefresh) {
            g_SkinRefreshOwned = false;
            s_refreshPending = true;

            std::vector<uint32_t> allIds;
            auto collectAll = [&](const std::string& type, const std::string& subtype,
                                   const std::string& weight) {
                auto ids = Skinventory::SkinCache::GetSkinsByCategory(type, subtype, weight);
                allIds.insert(allIds.end(), ids.begin(), ids.end());
            };
            for (const auto& wc : Skinventory::SkinCache::GetArmorWeights()) {
                for (const auto& slot : Skinventory::SkinCache::GetArmorSlots(wc)) {
                    collectAll("Armor", slot, wc);
                }
            }
            for (const auto& wt : Skinventory::SkinCache::GetWeaponTypes()) {
                collectAll("Weapon", wt, "");
            }
            collectAll("Back", "", "");
            if (!allIds.empty()) {
                Skinventory::OwnedSkins::RequestOwnedSkins(allIds);
            }
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

    PushGW2Theme();

    // Render gear customize dialog (separate window, always checked)
    RenderGearCustomizeDialog();
    RenderSaveToLibraryDialog();

    // Render chat build detection toast (always visible, even when main window is hidden)
    RenderBuildToast();

    // Achievement tracker popout (independent of main window)
    RenderAchPopout();

    if (!g_WindowVisible) { PopGW2Theme(); return; }

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX));
    if (!ImGui::Begin("Alter Ego", &g_WindowVisible, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        PopGW2Theme();
        return;
    }

    // H&S status + Refresh button
    auto hoardStatus = AlterEgo::GW2API::GetHoardStatus();
    auto fetchStatus = AlterEgo::GW2API::GetFetchStatus();
    bool scanning = (fetchStatus == AlterEgo::FetchStatus::InProgress);
    bool hoardReady = (hoardStatus == AlterEgo::HoardStatus::Available ||
                       hoardStatus == AlterEgo::HoardStatus::Ready);
    bool disabled = scanning || !hoardReady;

    if (disabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    if (ImGui::Button("Refresh Characters") && !disabled) {
        // Fetch just the character name list, then show selection popup
        g_RefreshListFetching = true;
        AlterEgo::GW2API::RequestCharacterList();
    }
    if (disabled) ImGui::PopStyleVar();

    // When the list-only fetch completes, populate the popup
    if (g_RefreshListFetching &&
        AlterEgo::GW2API::GetFetchStatus() != AlterEgo::FetchStatus::InProgress) {
        g_RefreshListFetching = false;
        const auto& names = AlterEgo::GW2API::GetPendingCharNames();
        if (!names.empty()) {
            g_RefreshNames = names;
            // Pre-select: currently viewed character checked, rest unchecked
            g_RefreshSelection.assign(names.size(), false);
            const auto& chars = AlterEgo::GW2API::GetCharacters();
            std::string currentName;
            if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                if (ri >= 0 && ri < (int)chars.size()) currentName = chars[ri].name;
            }
            for (size_t i = 0; i < names.size(); i++) {
                if (names[i] == currentName)
                    g_RefreshSelection[i] = true;
            }
            g_RefreshPopupOpen = true;
            ImGui::OpenPopup("Select Characters to Refresh");
        }
    }

    ImGui::SameLine();

    // H&S connection indicator
    if (hoardStatus == AlterEgo::HoardStatus::Unknown) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Checking for H&S...");
        // Retry ping periodically
        static auto lastPing = std::chrono::steady_clock::time_point{};
        auto now = std::chrono::steady_clock::now();
        if (now - lastPing > std::chrono::seconds(5)) {
            lastPing = now;
            AlterEgo::GW2API::PingHoard();
        }
    } else if (hoardStatus == AlterEgo::HoardStatus::Unavailable) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "H&S not detected. Install Hoard & Seek.");
    } else if (hoardStatus == AlterEgo::HoardStatus::PermPending) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "Approve Alter Ego in H&S permission popup.");
        // Retry request every 3 seconds until user accepts/denies.
        static auto lastPermRetry = std::chrono::steady_clock::time_point{};
        auto permNow = std::chrono::steady_clock::now();
        if (permNow - lastPermRetry > std::chrono::seconds(3)) {
            lastPermRetry = permNow;
            AlterEgo::GW2API::RequestCharacterList();
        }
    } else if (hoardStatus == AlterEgo::HoardStatus::PermDenied) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "H&S permission denied. Enable in H&S settings.");
    } else if (fetchStatus == AlterEgo::FetchStatus::InProgress) {
        g_FetchDoneTime = std::chrono::steady_clock::time_point{}; // reset fade timer while in progress
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s",
            AlterEgo::GW2API::GetFetchStatusMessage().c_str());
    } else if (fetchStatus == AlterEgo::FetchStatus::Error ||
               fetchStatus == AlterEgo::FetchStatus::Success) {
        // Record completion time on first frame after finishing
        if (g_FetchDoneTime == std::chrono::steady_clock::time_point{})
            g_FetchDoneTime = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - g_FetchDoneTime).count();
        if (elapsed < 5.0f) {
            float alpha = (elapsed < 4.0f) ? 1.0f : (5.0f - elapsed); // fade during last second
            ImVec4 col = (fetchStatus == AlterEgo::FetchStatus::Error)
                ? ImVec4(1.0f, 0.3f, 0.3f, alpha)
                : ImVec4(0.35f, 0.82f, 0.35f, alpha);
            ImGui::TextColored(col, "%s",
                AlterEgo::GW2API::GetFetchStatusMessage().c_str());
        }
        // After 5s, fall through to show "Last updated" below
        if (elapsed >= 5.0f && !AlterEgo::GW2API::HasCharacterData()) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "H&S connected. Click Refresh to load characters.");
        } else if (elapsed >= 5.0f) {
            time_t last = AlterEgo::GW2API::GetLastUpdated();
            if (last > 0) {
                time_t now_t = std::time(nullptr);
                int el = (int)difftime(now_t, last);
                std::string ago;
                if (el < 60) ago = "just now";
                else if (el < 3600) ago = std::to_string(el / 60) + "m ago";
                else if (el < 86400) ago = std::to_string(el / 3600) + "h ago";
                else ago = std::to_string(el / 86400) + "d ago";
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Last updated %s", ago.c_str());
            }
        }
    } else if (!AlterEgo::GW2API::HasCharacterData()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
            "H&S connected. Click Refresh to load characters.");
    } else {
        time_t last = AlterEgo::GW2API::GetLastUpdated();
        if (last > 0) {
            time_t now_t = std::time(nullptr);
            int elapsed = (int)difftime(now_t, last);
            std::string ago;
            if (elapsed < 60) ago = "just now";
            else if (elapsed < 3600) ago = std::to_string(elapsed / 60) + "m ago";
            else if (elapsed < 86400) ago = std::to_string(elapsed / 3600) + "h ago";
            else ago = std::to_string(elapsed / 86400) + "d ago";
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Last updated %s", ago.c_str());
        }
    }

    // Character refresh selection popup
    if (ImGui::BeginPopup("Select Characters to Refresh")) {
        ImGui::Text("Select characters to refresh:");
        ImGui::Spacing();

        // All / None / Lv80 buttons
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

        // Scrollable checkbox list
        ImGui::BeginChild("##charCheckList", ImVec2(280, 300), true);
        const auto& cachedChars = AlterEgo::GW2API::GetCharacters();
        for (size_t i = 0; i < g_RefreshNames.size(); i++) {
            ImGui::PushID((int)i);
            bool checked = g_RefreshSelection[i];

            // Look up cached data for level/profession color
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

        // Count selected
        int selCount = 0;
        for (bool b : g_RefreshSelection) { if (b) selCount++; }

        ImGui::Spacing();
        char btnLabel[64];
        snprintf(btnLabel, sizeof(btnLabel), "Refresh Selected (%d)", selCount);
        bool canRefresh = (selCount > 0);
        if (!canRefresh) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        if (ImGui::Button(btnLabel) && canRefresh) {
            // Build the selected name list, with currently viewed character first
            std::vector<std::string> selected;
            const auto& chars = AlterEgo::GW2API::GetCharacters();
            std::string currentName;
            if (g_SelectedCharIdx >= 0 && g_SelectedCharIdx < (int)g_CharDisplayOrder.size()) {
                int ri = g_CharDisplayOrder[g_SelectedCharIdx];
                if (ri >= 0 && ri < (int)chars.size()) currentName = chars[ri].name;
            }

            // Add current character first if selected
            for (size_t i = 0; i < g_RefreshNames.size(); i++) {
                if (g_RefreshSelection[i] && g_RefreshNames[i] == currentName) {
                    selected.push_back(g_RefreshNames[i]);
                    break;
                }
            }
            // Add the rest
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

    ImGui::Separator();

    // Top-level tab bar: Characters | Build Library
    if (ImGui::BeginTabBar("##main_tabs")) {
        if (ImGui::BeginTabItem("Characters")) {
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
            if (!characters.empty()) {
                // Rebuild display order when character count changes
                if (characters.size() != g_LastCharCount) {
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
                struct CharItemRect { float yMin, yMax; };
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

                for (int di = 0; di < (int)g_CharDisplayOrder.size(); di++) {
                    int realIdx = g_CharDisplayOrder[di];
                    if (realIdx < 0 || realIdx >= (int)characters.size()) continue;
                    const auto& ch = characters[realIdx];

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

                    itemRects.push_back({ rMin.y, rMax.y });

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
                    float mouseY = ImGui::GetMousePos().y;
                    int insertIdx = (int)itemRects.size(); // default: end
                    float bestLineY = 0;

                    for (int di = 0; di < (int)itemRects.size(); di++) {
                        float midY = (itemRects[di].yMin + itemRects[di].yMax) * 0.5f;
                        if (mouseY < midY) {
                            insertIdx = di;
                            bestLineY = itemRects[di].yMin;
                            break;
                        }
                    }
                    if (insertIdx == (int)itemRects.size() && !itemRects.empty())
                        bestLineY = itemRects.back().yMax;

                    // Draw insertion line
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    float xMin = ImGui::GetWindowPos().x + 2;
                    float xMax = xMin + ImGui::GetWindowContentRegionMax().x - 4;
                    dl->AddLine(ImVec2(xMin, bestLineY), ImVec2(xMax, bestLineY),
                        IM_COL32(100, 180, 255, 220), 2.0f);

                    // Handle drop anywhere in the list area
                    if (ImGui::IsMouseReleased(0)) {
                        int srcDi = g_CharDragIdx;
                        // Adjust insert index if dragging from before the insertion point
                        int targetDi = insertIdx;
                        if (srcDi < targetDi) targetDi--;
                        if (srcDi != targetDi && srcDi >= 0 && srcDi < (int)g_CharDisplayOrder.size()) {
                            int movedIdx = g_CharDisplayOrder[srcDi];
                            g_CharDisplayOrder.erase(g_CharDisplayOrder.begin() + srcDi);
                            int finalPos = (srcDi < insertIdx) ? insertIdx - 1 : insertIdx;
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

                    // Character header
                    ImVec4 profColor = GetProfessionColor(ch.profession);
                    ImGui::TextColored(profColor, "%s", ch.name.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                        "Level %d %s %s", ch.level, ch.race.c_str(), ch.profession.c_str());

                    // Birthday countdown
                    {
                        int bdays = DaysUntilBirthday(ch.created);
                        int age = CharacterAgeYears(ch.created);
                        if (bdays >= 0 && age >= 0) {
                            if (bdays == 0) {
                                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
                                    "Happy Birthday! Turning %d today!", age + 1);
                            } else {
                                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                    "Age %d  -  Birthday in %d day%s", age, bdays, bdays == 1 ? "" : "s");
                            }
                        }
                    }

                    // Last login (from local MumbleLink tracking)
                    {
                        auto tsIt = g_LoginTimestamps.find(ch.name);
                        if (tsIt != g_LoginTimestamps.end()) {
                            time_t now_t = std::time(nullptr);
                            int elapsed = (int)difftime(now_t, (time_t)tsIt->second);
                            std::string ago;
                            if (elapsed < 60) ago = "Just Now";
                            else if (elapsed < 3600) ago = std::to_string(elapsed / 60) + "m ago";
                            else if (elapsed < 86400) ago = std::to_string(elapsed / 3600) + "h ago";
                            else ago = std::to_string(elapsed / 86400) + "d ago";
                            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Last login: %s", ago.c_str());
                        }
                    }

                    // Crafting disciplines
                    if (!ch.crafting.empty()) {
                        ImGui::SameLine(0, 20);
                        std::string craftStr;
                        for (size_t ci = 0; ci < ch.crafting.size(); ci++) {
                            if (ci > 0) craftStr += "  ";
                            craftStr += ch.crafting[ci];
                            if (ci < ch.crafting_levels.size())
                                craftStr += " " + std::to_string(ch.crafting_levels[ci]);
                        }
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", craftStr.c_str());
                    }

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
                        ImGui::EndTabBar();
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a character from the list.");
                }
                ImGui::EndChild();
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Build Library")) {
            g_MainTab = 1;
            RenderBuildLibrary();
            ImGui::EndTabItem();
        }

        ImGuiTabItemFlags skinTabFlags = 0;
        if (g_SwitchToSkinventory) {
            skinTabFlags = ImGuiTabItemFlags_SetSelected;
            g_SwitchToSkinventory = false;
        }
        if (g_SkinInitialized && ImGui::BeginTabItem("Skinventory", nullptr, skinTabFlags)) {
            g_MainTab = 2;
            RenderSkinventory();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Clears")) {
            g_MainTab = 3;
            RenderClears();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Achievements")) {
            g_MainTab = 4;
            RenderAchievements();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    PopGW2Theme();
}

// --- Options/Settings Render ---

void AddonOptions() {
    PushGW2Theme();
    // Header with links
    ImGui::Text("Alter Ego");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
    ImGui::SameLine();
    if (ImGui::SmallButton("Homepage")) {
        ShellExecuteA(NULL, "open", "https://pie.rocks.cc/projects/alter-ego/", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
    ImGui::SameLine();
    if (ImGui::SmallButton("Ko-fi")) {
        ShellExecuteA(NULL, "open", "https://ko-fi.com/pieorcake", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::Separator();

    // H&S connection status
    ImGui::Text("Data Source: Hoard & Seek");
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
            if (ImGui::SmallButton("Retry Connection")) {
                AlterEgo::GW2API::PingHoard();
            }
            break;
        case AlterEgo::HoardStatus::Ready:
            ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.35f, 1.0f), "Status: Ready");
            break;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("UI Settings:");
    if (ImGui::Checkbox("Show Quick Access icon", &g_ShowQAIcon)) {
        if (g_ShowQAIcon) {
            APIDefs->QuickAccess_Add(QA_ID, TEX_ICON, TEX_ICON_HOVER, "KB_ALTER_EGO_TOGGLE", "Alter Ego");
        } else {
            APIDefs->QuickAccess_Remove(QA_ID);
        }
        SaveSettings();
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Chat Build Detection:");
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Requires 'Events: Chat' addon from the Nexus library.");
    if (ImGui::Checkbox("Detect build links in chat", &g_ChatBuildDetection)) {
        SaveSettings();
    }
    if (g_ChatBuildDetection) {
        ImGui::Indent(16.0f);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
            "Drag the toast notification to reposition it.");
        if (ImGui::Button("Reset Toast Position")) {
            g_ToastPosX = -1.0f;
            g_ToastPosY = 100.0f;
            g_ToastPosInitialized = false;
            SaveSettings();
        }
        ImGui::Unindent(16.0f);
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Character List:");
    if (ImGui::Checkbox("Compact mode", &g_CompactCharList)) {
        SaveSettings();
    }
    if (!g_CompactCharList) {
        ImGui::Indent(16.0f);
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
        ImGui::Unindent(16.0f);
    }
    PopGW2Theme();
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
    AddonDef.Description = "Manage characters, builds, skins, clears, achievements";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = AF_None;
    AddonDef.Provider = UP_GitHub;
    AddonDef.UpdateLink = "https://github.com/PieOrCake/alter_ego";

    return &AddonDef;
}
