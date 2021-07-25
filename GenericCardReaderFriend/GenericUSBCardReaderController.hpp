//
//  GenericUSBCardReaderController.hpp
//  GenericCardReaderFriend
//
//  Created by FireWolf on 7/24/21.
//

#ifndef GenericUSBCardReaderController_hpp
#define GenericUSBCardReaderController_hpp

// Get rid of a bunch of documentation warnings in the USB headers
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <IOKit/usb/IOUSBHostDevice.h>
#pragma clang diagnostic pop

///
/// A generic USB-based card reader controller that injects properties for decoration
///
class GenericUSBCardReaderController: public IOService
{
    //
    // MARK: - Constructors & Destructors
    //
    
    OSDeclareDefaultStructors(GenericUSBCardReaderController);
    
    using super = IOService;
    
    //
    // MARK: - IOKit Basics
    //
    
    /// The USB host device (provider)
    IOUSBHostDevice* device;
    
public:
    ///
    /// Start the controller
    ///
    /// @param provider An instance of USB host device that represents the card reader
    /// @return `true` on success, `false` otherwise.
    ///
    bool start(IOService* provider) override;
    
    ///
    /// Stop the controller
    ///
    /// @param provider An instance of USB host device that represents the card reader
    ///
    void stop(IOService* provider) override;
};

#endif /* GenericUSBCardReaderController_hpp */
