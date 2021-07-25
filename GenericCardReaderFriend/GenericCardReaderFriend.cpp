//
//  GenericCardReaderFriend.cpp
//  GenericCardReaderFriend
//
//  Created by FireWolf on 7/24/21.
//

#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>
#include <Headers/kern_user.hpp>

//
// MARK: - Constants & Patches
//

static const char* kCardReaderReporterPath = "/System/Library/SystemProfiler/SPCardReaderReporter.spreporter";

static const size_t kCardReaderReporterPathLength = strlen(kCardReaderReporterPath);

// MARK: USB-based Card Reader

// Function: SPCardReaderReporter::updateDictionary()
// Find: IOServiceMatching("com_apple_driver_AppleUSBCardReaderSBC")
// Repl: IOServiceMatching("GenericUSBCardReaderController")
// Note: Patch the name of the controller class
static const uint8_t kAppleUSBCardReaderSBC[] =
{
    0x63, 0x6F, 0x6D,                   // "com"
    0x5F,                               // "_"
    0x61, 0x70, 0x70, 0x6C, 0x65,       // "apple"
    0x5F,                               // "_"
    0x64, 0x72, 0x69, 0x76, 0x65, 0x72, // "driver"
    0x5F,                               // "_"
    0x41, 0x70, 0x70, 0x6C, 0x65,       // "Apple"
    0x55, 0x53, 0x42,                   // "USB"
    0x43, 0x61, 0x72, 0x64,             // "Card"
    0x52, 0x65, 0x61, 0x64, 0x65, 0x72, // "Reader"
    0x53, 0x42, 0x43,                   // "SBC"
    0x00                                // "\0"
};

static const uint8_t kGenericUSBCardReaderController[] =
{
    0x47, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63,
    0x55, 0x53, 0x42,
    0x43, 0x61, 0x72, 0x64,
    0x52, 0x65, 0x61, 0x64, 0x65, 0x72,
    0x43, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Function: SPCardReaderReporter::updateDictionary()
// Find: IORegistryEntrySearchCFProperty(entry, "IOService", kCFBundleIdentifierKey, kCFAllocatorDefault, kIORegistryIterateRecursively)
//       NSString isEqualToString("com.apple.driver.AppleUSBCardReader")
// Repl: NSString isEqualToString("science.firewolf.gcrf")
// Note: Patch the bundle identifier
static const uint8_t kAppleUCRBundleIdentifier[] =
{
    0x63, 0x6F, 0x6D,                   // "com"
    0x2E,                               // "."
    0x61, 0x70, 0x70, 0x6C, 0x65,       // "apple"
    0x2E,                               // "."
    0x64, 0x72, 0x69, 0x76, 0x65, 0x72, // "driver"
    0x2E,                               // "."
    0x41, 0x70, 0x70, 0x6C, 0x65,       // "Apple"
    0x55, 0x53, 0x42,                   // "USB"
    0x43, 0x61, 0x72, 0x64,             // "Card"
    0x52, 0x65, 0x61, 0x64, 0x65, 0x72, // "Reader"
    0x00                                // "\0"
};

static const uint8_t kDummyUCRBundleIdentifier[] =
{
    0x73, 0x63, 0x69, 0x65, 0x6E, 0x63, 0x65,
    0x2E,
    0x66, 0x69, 0x72, 0x65, 0x77, 0x6F, 0x6C, 0x66,
    0x2E,
    0x67, 0x63, 0x72, 0x66,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
};

//
// MARK: - Helper Functions
//

static inline bool matchReporterPath(const char* path)
{
    return strncmp(path, kCardReaderReporterPath, kCardReaderReporterPathLength) == 0;
}

static void patchReporter(const void* data, vm_size_t size)
{
    void* memory = const_cast<void*>(data);
    
    SYSLOG_COND(UNLIKELY(KernelPatcher::findAndReplace(memory, size, kAppleUSBCardReaderSBC, kGenericUSBCardReaderController)), "GCRF", "Patched the USB controller name.");
    
    SYSLOG_COND(UNLIKELY(KernelPatcher::findAndReplace(memory, size, kAppleUCRBundleIdentifier, kDummyUCRBundleIdentifier)), "GCRF", "Patched the USB bundle identifier.");
}

//
// MARK: - Routed Functions
//

// macOS Catalina and earlier
static boolean_t (*orgCSValidateRange)(vnode_t, memory_object_t, memory_object_offset_t, const void*, vm_size_t, unsigned int*) = nullptr;

static boolean_t wrapCSValidateRange(vnode_t vp, memory_object_t pager, memory_object_offset_t offset, const void* data, vm_size_t size, unsigned int* result)
{
    char path[PATH_MAX];
    
    int pathlen = PATH_MAX;
    
    boolean_t retVal = (*orgCSValidateRange)(vp, pager, offset, data, size, result);
    
    if (retVal && vn_getpath(vp, path, &pathlen) == 0 && matchReporterPath(path))
    {
        patchReporter(data, size);
    }
    
    return retVal;
}

// macOS Big Sur and later
static void (*orgCSValidatePage)(vnode_t, memory_object_t, memory_object_offset_t, const void*, int*, int*, int*) = nullptr;

static void wrapCSValidatePage(vnode_t vp, memory_object_t pager, memory_object_offset_t page_offset, const void* data, int* validated_p, int* tainted_p, int* nx_p)
{
    char path[PATH_MAX];
    
    int pathlen = PATH_MAX;
    
    (*orgCSValidatePage)(vp, pager, page_offset, data, validated_p, tainted_p, nx_p);
    
    if (vn_getpath(vp, path, &pathlen) == 0 && matchReporterPath(path))
    {
        patchReporter(data, PAGE_SIZE);
    }
}

//
// MARK: - Boot Args
//

static const char *bootargOff[] =
{
    "-gcrfoff"
};

static const char *bootargDebug[] =
{
    "-gcrfdbg"
};

static const char *bootargBeta[] =
{
    "-gcrfbeta"
};

//
// MARK: - Plugin Start Routine
//

static KernelPatcher::RouteRequest gRequestLegacy =
{
    "_cs_validate_range",
    wrapCSValidateRange,
    orgCSValidateRange
};

static KernelPatcher::RouteRequest gRequestCurrent =
{
    "_cs_validate_page",
    wrapCSValidatePage,
    orgCSValidatePage
};

static void start()
{
    DBGLOG("GCRF", "Realtek card reader friend started.");
    
    auto action = [](void*, KernelPatcher& patcher) -> void
    {
        KernelPatcher::RouteRequest* request = getKernelVersion() >= KernelVersion::BigSur ? &gRequestCurrent : &gRequestLegacy;
        
        if (!patcher.routeMultipleLong(KernelPatcher::KernelID, request, 1))
        {
            SYSLOG("GCRF", "Failed to route the function.");
        }
    };
    
    lilu.onPatcherLoadForce(action);
}

//
// MARK: - Plugin Configuration
//

PluginConfiguration ADDPR(config) =
{
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
    KernelVersion::Mojave,
    KernelVersion::Monterey,
    start
};
