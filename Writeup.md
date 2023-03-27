# Windows.Gaming.Input.Custom Writeup

This writeup goes into detail on how to make use of Windows.Gaming.Input.Custom. Aside from a single undocumented interface, this API is pretty much usable as-is, with only some catches here and there which will be mentioned where relevant.

Code examples make use of the C++/WinRT projection, which I would highly recommend for your own safety, sanity, and productivity if you're using C++.

## Table of Contents

- [The Device Class](#the-device-class)
  - [MIDL Runtime Class Definition](#midl-runtime-class-definition)
  - [The `IAggregable` Interface](#the-iaggregable-interface)
    - [Aggregation Warnings](#aggregation-warnings)
  - [The Input Sink Interfaces](#the-input-sink-interfaces)
  - [The `FromGameController` Function](#the-fromgamecontroller-function)
- [The Device Factory](#the-device-factory)
  - [The Factory Manager](#the-factory-manager)
  - [Creating and Handling Devices](#creating-and-handling-devices)
  - [`FromGameController` Implementation](#fromgamecontroller-implementation)
- [Closing Notes](#closing-notes)

## The Device Class

Creating a custom device class is fairly straightforward. Device connectivity status, input events, and providers are handled by Windows.Gaming.Input, and all other aspects of the device are handled by the implementer, such as keeping track of available devices and turning input events into something more consumable.

### MIDL Runtime Class Definition

The MIDL definition for your runtime class should implement `IGameController`, and should include the following:

- A static property to retrieve the current set of devices
- Two static events for when a device is added or removed
- A static function to get a typed instance of the class from an `IGameController` instance
- An instance function to get the current state of the device

```midl
namespace Demo
{
    struct CustomDeviceState
    {
        // ...
    }

    runtimeclass CustomDevice : Windows.Gaming.Input.IGameController
    {
        static IVectorView<CustomDevice> Devices { get; };
        static event Windows.Foundation.EventHandler<CustomDevice> DeviceAdded;
        static event Windows.Foundation.EventHandler<CustomDevice> DeviceRemoved;
        static CustomDevice FromGameController(Windows.Gaming.Input.IGameController gameController);

        CustomDeviceState GetCurrentReading();
    }
}
```

Any additional members you define depend on the device you're implementing. You'll want to provide ways to send force-feedback if the device supports it, for example. The only thing you should not define here is a constructor, as creation of device instances should only be managed internally.

### The `IAggregable` Interface

One complication arises when making the device class, as there's an interface that's required which was left undocumented (most likely deliberately). This interface is used for an aggregation pattern: you don't implement `IGameController` directly, instead you implement `IAggregable` and receive a pre-made instance of `IGameController`.

`IAggregable` is defined as the following (using MIDL 3.0):

```midl
namespace Windows.Gaming.Input.Custom
{
    [uuid(06E58977-7684-4DC5-BAD1-CDA52A4AA06D)]
    interface IAggregable : IInspectable
    {
        void Aggregate(IInspectable inner);
    }
}
```

To implement this aggregation properly, you will need to ignore the base class that C++/WinRT generates for you, and directly use `winrt::implements` to implement interfaces:

```cpp
namespace winrt::Demo::implementation
{
    // To make it so you don't have to write out all of template arguments in QueryInterface
    // to reference the base class
    template <typename D>
    using CustomDeviceT = implements<D, Demo::CustomDevice, IAggregable, /* other interfaces */>

    class CustomDevice : CustomDeviceT<CustomDevice>
    {
    private:
        IInspectable m_innerObject { nullptr };

    public:
        void Aggregate(IInspectable inner)
        {
            if (!inner) // Invalid arguments
                throw hresult_invalid_argument();
            // Unfortunately I'm not sure how easy it'll be to check if inner == this, doing that verbatim doesn't
            // compile and I can't currently be bothered to figure out what would work

            if (m_innerObject) // Already initialized
                throw hresult_error(CLASS_E_NOAGGREGATION);

            m_innerObject = inner;
        }

        int32_t QueryInterface(winrt::guid const& id, void** object) noexcept
        {
            // Query inner object if requesting IGameController
            if (winrt::is_guid_of<IGameController>(id) && m_innerObject)
                return m_innerObject.as(id, object);

            // Use the default QueryInterface otherwise
            return CustomDeviceT<CustomDevice>::QueryInterface(id, object);
        }
    }
}
```

#### Aggregation Warnings

There are a number of catches with this interface and the instance received through it.

The `Aggregate` function is called when the `IGameController` instance is being initialized. As such, you *must not* call any functions of the received instance while inside of it, otherwise you will risk crashes as it has not added you as its outer object yet.

Since aggregation should only occur at initialization of the device object, any future calls to Aggregate should fail with the `CLASS_E_NOAGGREGATION` result. Additionally, being passed either null or `this` should be treated as invalid parameters.

The instance should only be queried if the requested interface ID is that of `IGameController`, not when the interface ID isn't supported by the device class. An infinite loop will occur otherwise if neither the device class nor the instance support the requested interface, as they will end up querying each other infinitely.

The instance's `GetIids` function just redirects to its outer object's `GetIids`, so you should not call it if you make your own implementation or else you'll get an infinite loop.

C++/WinRT provides a `winrt::composing` marker type that can be specified in the interface list to indicate that you will be using an inner object, and provides an `m_inner` field and adjusts the default behavior for `IInspectable` functions to query this object where needed. However, due to the catches listed above, this marker type is not suitable for implementing `IAggregable`, as it will result in both the `QueryInterface` and `GetIids` infinite loops.

### The Input Sink Interfaces

For your device class to receive input, you will also need to implement `IGameControllerInputSink`, alongside at least one of the following: `IHidGameControllerInputSink`, `IXusbGameControllerInputSink`, `IGipGameControllerInputSink`. The functions exposed by these interfaces allow you to receive raw input data from devices. Typically, the received data should be turned into a normalized format and stored for later retrieval by `GetCurrentReading`.

```cpp
namespace winrt::Demo::implementation
{
    class CustomDevice : CustomDeviceT<CustomDevice>
    {
        void OnInputSuspended(uint64_t timestamp)
        {
            // Zero out the stored state here, or provide a mechanism that
            // GetCurrentReading will use to instead return a default state
        }

        void OnInputResumed(uint64_t timestamp)
        {
            // Restore the last known current state here
        }

        void OnInputReportReceived(uint64_t timestamp, uint8_t reportId, winrt::array_view<uint8_t const> reportBuffer)
        {
            // Translate the received input report into whatever format you wish to expose
        }
    }
}
```

### The `FromGameController` Function

To allow correlating different types of game controllers together, a static `FromGameController` function is exposed on device classes. The implementation for this can't be done with just the device class alone, however. You will need something else...

## The Device Factory

Alongside the device class, you will also need a class which implements `IGameControllerFactory` which will be used internally to instantiate the device. This factory (and everything it handles) is typically invisible to the user, with any relevant APIs for the device being exposed through just the device class.

### The Factory Manager

Windows.Gaming.Input provides a factory manager which is used to register controller factories with specific hardware IDs. You can register a factory for vendor/product IDs, XUSB types/subtypes, and GIP interface GUIDs. When or how this happens isn't important, but your factory won't receive devices until you register it.

```cpp
namespace winrt::Demo // ::implementation is dropped here to simplify and clarify some things
{
    uint16_t vendorId = ...;
    uint16_t productId = ...;
    XusbDeviceType type = ...;
    XusbDeviceSubtype subtype = ...;
    winrt::guid interfaceGuid = ...;

    class CustomDeviceFactory : implements<CustomDeviceFactory, ICustomGameControllerFactory>
    {
    private:
        static inline ICustomGameControllerFactory s_factory = winrt::make<CustomDeviceFactory>();

    public:
        CustomDeviceFactory()
        {
            // Register for any desired IDs
            GameControllerFactoryManager::RegisterCustomFactoryForHardwareId(*this, vendorId, productId);
            GameControllerFactoryManager::RegisterCustomFactoryForXusbType(*this, type, subtype);
            GameControllerFactoryManager::RegisterCustomFactoryForGipInterface(*this, interfaceGuid);
        }
    };
}
```

### Creating and Handling Devices

When a device is connected to the system, a provider for that device is created. This provider is given to all factories whose registrations match its hardware IDs, through the `CreateGameController` function. The factory is responsible for instantiating a new instance of the device class here:

```cpp
namespace winrt::Demo
{
    class CustomDeviceFactory : implements<CustomDeviceFactory, ICustomGameControllerFactory>
    {
        IInspectable CreateGameController(IGameControllerProvider const& provider)
        {
            // Perform any checks or type testing you desire to learn about the device,
            // then make the implementation class and return it
            if (...)
            {
                return winrt::make<implementation::CustomDevice>(provider);
            }

            // Throwing is recommended if you ignore the received provider
            throw hresult_invalid_argument();
        }
    };
}
```

After this, aggregation will be performed and the device will be handed to the factory again through `OnGameControllerAdded`:

```cpp
namespace winrt::Demo
{
    class CustomDeviceFactory : implements<CustomDeviceFactory, ICustomGameControllerFactory>
    {
        void OnGameControllerAdded(IGameController const& controller)
        {
            // Get the typed instance
            auto customDevice = controller.try_as<CustomDevice>();
            if (!customDevice)
                throw hresult_invalid_argument();

            // Add the instance to a collection that can be retrieved by a static function
        }
    };
}
```

And it will later be handed to `OnGameControllerRemoved` when the controller is disconnected:

```cpp
namespace winrt::Demo
{
    class CustomDeviceFactory : implements<CustomDeviceFactory, ICustomGameControllerFactory>
    {
        void OnGameControllerRemoved(IGameController const& controller)
        {
            // Get the typed instance
            auto customDevice = controller.try_as<CustomDevice>();
            if (!customDevice)
                throw hresult_invalid_argument();

            // Remove the instance from your collection
        }
    };
}
```

### `FromGameController` Implementation

`FromGameController` is implemented through the factory manager's `TryGetFactoryControllerFromGameController` function:

```cpp
namespace winrt::Demo
{
    class CustomDeviceFactory : implements<CustomDeviceFactory, ICustomGameControllerFactory>
    {
        CustomDevice FromGameController(IGameController controller)
        {
            return GameControllerFactoryManager::TryGetFactoryControllerFromGameController(*this, controller)
                .try_as<CustomDevice>(); // try_as will return null if the result is already null
        }
    };
}
```

## Closing Notes

With all of that said, here's some miscellaneous notes to finish things off.

While the APIs here are fairly nice, if you already have access to HID or XInput APIs then the HID and XUSB input sinks most likely won't be of any additional use to you. Making use of them just allows you to integrate with Windows.Gaming.Input if desired, and there aren't really any advantages asides from getting the full, raw XUSB report including unused bytes.

Unfortunately for XUSB devices, the enum provided for subtype values does not match that of the actual value reported by the controller. The order is changed around, and unrecognized subtype values are treated as `XusbDeviceSubtype::Unknown`. Since there is also no way to get raw capabilities info, this makes identifying devices that use undocumented subtype values unreliable and impractical. And since most devices that use the standard subtypes don't have any data in the unused bytes, again, all this would be practical for is integration with Windows.Gaming.Input.

The only API I personally see any practical use in is the GIP input sinks. There is no other official way to get raw GIP input reports on PC currently; the next closest thing would be GameInput but that still hasn't made its way over to PC yet. And unlike the XUSB part of the API, there is no tampering of the interface GUID values, so almost every GIP device can be implemented.
