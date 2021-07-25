//
//  GenericUSBCardReaderController.cpp
//  GenericCardReaderFriend
//
//  Created by FireWolf on 7/24/21.
//

#include "GenericUSBCardReaderController.hpp"

//
// MARK: - Meta Class Definitions
//

OSDefineMetaClassAndStructors(GenericUSBCardReaderController, IOService);

//
// MARK: - IOKit Basics
//

///
/// Start the controller
///
/// @param provider An instance of USB host device that represents the card reader
/// @return `true` on success, `false` otherwise.
///
bool GenericUSBCardReaderController::start(IOService* provider)
{
    // Start the super class
    if (!super::start(provider))
    {
        return false;
    }
    
    // Fetch the USB host device
    this->device = OSDynamicCast(IOUSBHostDevice, provider);
    
    if (this->device == nullptr)
    {
        return false;
    }
    
    this->device->retain();
    
    // Publish the USB device information
    static const char* keys[] =
    {
        "idVendor",
        "idProduct",
        "kUSBSerialNumberString",
    };
    
    OSDictionary* deviceInfo = OSDictionary::withCapacity(3);
    
    for (const char* key : keys)
    {
        deviceInfo->setObject(key, this->device->getProperty(key));
    }
    
    this->setProperty("USB Device Info", deviceInfo);
    
    OSSafeReleaseNULL(deviceInfo);
    
    // Publish the revision level
    this->setProperty("Product Revision Level", "1.00");
    
    // Finished the decoration
    this->registerService();
    
    return true;
}

///
/// Stop the controller
///
/// @param provider An instance of USB host device that represents the card reader
///
void GenericUSBCardReaderController::stop(IOService* provider)
{
    OSSafeReleaseNULL(this->device);
    
    super::stop(provider);
}
