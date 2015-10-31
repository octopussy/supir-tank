// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#ifndef __SUPIRTANK_H__
#define __SUPIRTANK_H__

#include "EngineMinimal.h"

#include "PxPhysicsAPI.h"
#include "PxAllocatorCallback.h"
#include "PxErrorCallback.h"

#include "WheeledVehicleMovementComponentTank.h"
#include "SupirTankClasses.h"

//////////////////////////////////////////////////////
// for PhysX mem allocation purposes : consider this as hack!!

void ensurePhysXFoundationSetup();

class SimpleMemAllocator : public physx::PxAllocatorCallback {
    
public:
    
    SimpleMemAllocator() { }
    
    virtual ~SimpleMemAllocator() { }
    
    virtual void *allocate(size_t size, const char *typeName, const char *filename, int line) override {
        void *ptr = FMemory::Malloc(size, 16);
        return ptr;
    }
    
    virtual void deallocate(void *ptr) override {
        FMemory::Free(ptr);
    }
};

class DummyErrorCallback : public physx::PxErrorCallback {
public:
    virtual void reportError(physx::PxErrorCode::Enum e, const char *message, const char *file, int line) override {
    }
};

#endif
