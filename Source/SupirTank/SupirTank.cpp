// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SupirTank.h"


IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SupirTank, "SupirTank" );

physx::PxFoundation* GModulePhysXFoundation = nullptr;

void ensurePhysXFoundationSetup() {

#if PLATFORM_MAC
    if (nullptr == GModulePhysXFoundation) {
        auto allocator = new SimpleMemAllocator();
        DummyErrorCallback* ErrorCallback = new DummyErrorCallback();

        GModulePhysXFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *allocator, *ErrorCallback);
    }
#endif

}

 