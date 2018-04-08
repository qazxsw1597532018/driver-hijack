#pragma once

#include <Fltkernel.h>
#include <ntddk.h>


extern "C" NTSTATUS DriverEntry(_In_ struct _DRIVER_OBJECT * DriverObject, PUNICODE_STRING RegistryPath);
extern "C" __declspec(dllexport) VOID DriverUnload(_In_ struct _DRIVER_OBJECT *);
extern "C" NTSTATUS DriverInitialize(_In_ struct _DRIVER_OBJECT * DriverObject, PUNICODE_STRING);
extern "C" NTSTATUS FindDriver(PDRIVER_OBJECT*, PDRIVER_OBJECT Ignore);
extern "C" NTSTATUS DestroyDevice();
extern "C" _Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
NTSTATUS CatchDeviceCtrl(PDEVICE_OBJECT, PIRP);
_Dispatch_type_(IRP_MJ_CREATE)
extern "C" NTSTATUS CatchCreate(PDEVICE_OBJECT, PIRP);
_Dispatch_type_(IRP_MJ_CLOSE)
extern "C"  NTSTATUS CatchClose(PDEVICE_OBJECT, PIRP);
extern "C" NTSTATUS RestoreIRPHandler();

#define DEVICE_NAME(name) L"\\Device\\"#name
#define DOSDEVICE_NAME(name) L"\\DosDevices\\"#name
#define DRIVER_NAME(name) L"\\Driver\\"#name

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DriverInitialize)
#pragma alloc_text(INIT, FindDriver)

#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, DestroyDevice)
#pragma alloc_text(PAGE, RestoreIRPHandler)

#pragma alloc_text(NONPAGED, CatchCreate)
#pragma alloc_text(NONPAGED, CatchDeviceCtrl)
#pragma alloc_text(NONPAGED, CatchClose)

// disable warning for unnamed structs/unions because i cant be arsed to name undocumented win structs lol
#pragma warning(disable: 4201)

typedef struct _OBJECT_CREATE_INFORMATION
{
    ULONG Attributes;
    PVOID RootDirectory;
    PVOID ParseContext;
    CHAR ProbeMode;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG SecurityDescriptorCharge;
    PVOID SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_CREATE_INFORMATION, *POBJECT_CREATE_INFORMATION;


// This structure is not correct on Windows 7, but the offsets we need are still correct.
typedef struct _OBJECT_HEADER
{
    LONG PointerCount;
    union
    {
        LONG HandleCount;
        PVOID NextToFree;
    };
    EX_PUSH_LOCK Lock;
    UCHAR TypeIndex;
    union
    {
        UCHAR TraceFlags;
        struct
        {
            UCHAR DbgRefTrace : 1;
            UCHAR DbgTracePermanent : 1;
            UCHAR Reserved : 6;
        };
    };
    UCHAR InfoMask;
    union
    {
        UCHAR Flags;
        struct
        {
            UCHAR NewObject : 1;
            UCHAR KernelObject : 1;
            UCHAR KernelOnlyAccess : 1;
            UCHAR ExclusiveObject : 1;
            UCHAR PermanentObject : 1;
            UCHAR DefaultSecurityQuota : 1;
            UCHAR SingleHandleEntry : 1;
            UCHAR DeletedInline : 1;
        };
    };
    union
    {
        POBJECT_CREATE_INFORMATION ObjectCreateInfo;
        PVOID QuotaBlockCharged;
    };
    PVOID SecurityDescriptor;
    QUAD Body;
} OBJECT_HEADER, *POBJECT_HEADER;



typedef struct _OBJECT_TYPE_INITIALIZER
{
    USHORT Length;
    UCHAR ObjectTypeFlags;
    ULONG CaseInsensitive : 1;
    ULONG UnnamedObjectsOnly : 1;
    ULONG UseDefaultObject : 1;
    ULONG SecurityRequired : 1;
    ULONG MaintainHandleCount : 1;
    ULONG MaintainTypeList : 1;
    ULONG ObjectTypeCode;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    POOL_TYPE PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
    PVOID DumpProcedure;
    LONG * OpenProcedure;
    PVOID CloseProcedure;
    PVOID DeleteProcedure;
    LONG * ParseProcedure;
    LONG * SecurityProcedure;
    LONG * QueryNameProcedure;
    UCHAR * OkayToCloseProcedure;
} OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;

typedef struct _OBJECT_TYPE
{
    // ERESOURCE Mutex; -> not in WinDbg probably negative offset 
    LIST_ENTRY TypeList;
    UNICODE_STRING Name;
    PVOID DefaultObject;
    UCHAR Index;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    OBJECT_TYPE_INITIALIZER TypeInfo;
    EX_PUSH_LOCK TypeLock;
    ULONG Key;
    LIST_ENTRY CallbackList;
} OBJECT_TYPE, *POBJECT_TYPE;

typedef struct _DEVICE_MAP *PDEVICE_MAP;

struct _OBJECT_DIRECTORY_ENTRY
{
    _OBJECT_DIRECTORY_ENTRY* ChainLink;
    PVOID Object;
    ULONG HashValue;
};

using OBJECT_DIRECTORY_ENTRY = _OBJECT_DIRECTORY_ENTRY;
using POBJECT_DIRECTORY_ENTRY = OBJECT_DIRECTORY_ENTRY * ;



typedef struct _OBJECT_DIRECTORY
{
    POBJECT_DIRECTORY_ENTRY HashBuckets[37];
    EX_PUSH_LOCK Lock;
    PDEVICE_MAP DeviceMap;
    ULONG SessionId;
    PVOID NamespaceEntry;
    ULONG Flags;
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;

// Creates a driver object if you pass an initialization routine
EXTERN_C NTSYSCALLAPI NTSTATUS ZwOpenDirectoryObject(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
EXTERN_C NTSYSCALLAPI VOID ExAcquirePushLockExclusiveEx(PEX_PUSH_LOCK, ULONG Flags);
EXTERN_C NTSYSCALLAPI VOID ExReleasePushLockExclusiveEx(PEX_PUSH_LOCK, ULONG Flags);