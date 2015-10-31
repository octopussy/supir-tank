#include "SupirTank.h"
#include "TankVehicle.h"

class UWheeledVehicleMovementComponentTank;

TankVehicle::TankVehicle(const FObjectInitializer &ObjectInitializer) :
        Super(ObjectInitializer.SetDefautSubobjectClass<UWheeledVehicleMovementComponentTank>(
                VehicleMovementComponentName)) {
}

