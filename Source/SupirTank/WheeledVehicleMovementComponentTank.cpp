#include "SupirTank.h"
#include "WheeledVehicleMovementComponentTank.h"

#include "PhysicsPublic.h"
#include "PhysXPublic.h"


UWheeledVehicleMovementComponentTank::UWheeledVehicleMovementComponentTank(const FObjectInitializer &ObjectInitializer)
        : Super(ObjectInitializer) {

    ensurePhysXFoundationSetup();

    RightThrustRate.FallRate = 5.f;
    RightThrustRate.RiseRate = 2.5f;
    LeftThrustRate.FallRate = 5.f;
    LeftThrustRate.RiseRate = 2.5f;
    LeftBrakeRate.RiseRate = 6.f;
    LeftBrakeRate.FallRate = 10.f;
    RightBrakeRate.RiseRate = 6.f;
    RightBrakeRate.FallRate = 10.f;

    PxVehicleEngineData DefEngineData;
    EngineSetup.MOI = DefEngineData.mMOI;
    EngineSetup.MaxRPM = OmegaToRPM(DefEngineData.mMaxOmega);
    EngineSetup.DampingRateFullThrottle = DefEngineData.mDampingRateFullThrottle;
    EngineSetup.DampingRateZeroThrottleClutchEngaged = DefEngineData.mDampingRateZeroThrottleClutchEngaged;
    EngineSetup.DampingRateZeroThrottleClutchDisengaged = DefEngineData.mDampingRateZeroThrottleClutchDisengaged;

    EngineSetup.Torque0 = 400.f;
    EngineSetup.Torque25 = 500.f;
    EngineSetup.Torque75 = 600.f;
    EngineSetup.Torque100 = 400.f;

    PxVehicleClutchData DefClutchData;
    TransmissionSetup.ClutchStrength = DefClutchData.mStrength;

    PxVehicleGearsData DefGearSetup;
    TransmissionSetup.GearSwitchTime = DefGearSetup.mSwitchTime;
    TransmissionSetup.ReverseGearRatio = DefGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
    TransmissionSetup.FinalRatio = DefGearSetup.mFinalRatio;

    PxVehicleAutoBoxData DefAutoBoxSetup;
    TransmissionSetup.NeutralGearUpRatio = DefAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
    TransmissionSetup.GearAutoBoxLatency = DefAutoBoxSetup.getLatency();
    TransmissionSetup.bUseGearAutoBox = true;

    for (uint32 i = PxVehicleGearsData::eFIRST; i < DefGearSetup.mNbRatios; i++) {
        FTankGearData GearData;
        GearData.DownRatio = DefAutoBoxSetup.mDownRatios[i];
        GearData.UpRatio = DefAutoBoxSetup.mUpRatios[i];
        GearData.Ratio = DefGearSetup.mRatios[i];
        TransmissionSetup.ForwardGears.Add(GearData);
    }

    WheelSetups.SetNum(4);
}


float FTankEngineData::FindPeakTorque() const {
    return FMath::Max3(Torque0, Torque25, FMath::Max(Torque75, Torque100));
}

static void GetTankVehicleEngineSetup(const FTankEngineData &Setup, PxVehicleEngineData &PxSetup) {
    PxSetup.mMOI = M2ToCm2(Setup.MOI);
    PxSetup.mMaxOmega = RPMToOmega(Setup.MaxRPM);
    PxSetup.mDampingRateFullThrottle = M2ToCm2(Setup.DampingRateFullThrottle);
    PxSetup.mDampingRateZeroThrottleClutchEngaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchEngaged);
    PxSetup.mDampingRateZeroThrottleClutchDisengaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchDisengaged);

    float PeakTorque = Setup.FindPeakTorque(); // In Nm
    PxSetup.mPeakTorque = M2ToCm2(PeakTorque);    // convert Nm to (kg cm^2/s^2)

    // Convert from our curve to PhysX
    PxSetup.mTorqueCurve.clear();
    PxSetup.mTorqueCurve.addPair(0.f, Setup.Torque0 / PeakTorque);
    PxSetup.mTorqueCurve.addPair(0.25f, Setup.Torque25 / PeakTorque);
    PxSetup.mTorqueCurve.addPair(0.75f, Setup.Torque75 / PeakTorque);
    PxSetup.mTorqueCurve.addPair(1.f, Setup.Torque100 / PeakTorque);
}

static void GetTankVehicleGearSetup(const FTankTransmissionData &Setup, PxVehicleGearsData &PxSetup) {
    PxSetup.mSwitchTime = Setup.GearSwitchTime;
    PxSetup.mRatios[PxVehicleGearsData::eREVERSE] = Setup.ReverseGearRatio;
    for (int32 i = 0; i < Setup.ForwardGears.Num(); i++) {
        PxSetup.mRatios[i + PxVehicleGearsData::eFIRST] = Setup.ForwardGears[i].Ratio;
    }
    PxSetup.mFinalRatio = Setup.FinalRatio;
    PxSetup.mNbRatios = Setup.ForwardGears.Num() + PxVehicleGearsData::eFIRST;
}

static void GetTankVehicleAutoBoxSetup(const FTankTransmissionData &Setup, PxVehicleAutoBoxData &PxSetup) {
    for (int32 i = 0; i < Setup.ForwardGears.Num(); i++) {
        const FTankGearData &GearData = Setup.ForwardGears[i];
        PxSetup.mUpRatios[i] = GearData.UpRatio;
        PxSetup.mDownRatios[i] = GearData.DownRatio;
    }
    PxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL] = Setup.NeutralGearUpRatio;
    PxSetup.setLatency(Setup.GearAutoBoxLatency);
}

void SetupTankDriveHelper(const UWheeledVehicleMovementComponentTank *VehicleData,
                          PxVehicleDriveSimData &DriveData) {


    PxVehicleEngineData EngineSetup;
    GetTankVehicleEngineSetup(VehicleData->EngineSetup, EngineSetup);
    DriveData.setEngineData(EngineSetup);

    PxVehicleClutchData ClutchSetup;
    ClutchSetup.mStrength = M2ToCm2(VehicleData->TransmissionSetup.ClutchStrength);
    DriveData.setClutchData(ClutchSetup);

    PxVehicleGearsData GearSetup;
    GetTankVehicleGearSetup(VehicleData->TransmissionSetup, GearSetup);
    DriveData.setGearsData(GearSetup);

    PxVehicleAutoBoxData AutoBoxSetup;
    GetTankVehicleAutoBoxSetup(VehicleData->TransmissionSetup, AutoBoxSetup);
    DriveData.setAutoBoxData(AutoBoxSetup);

}

void UWheeledVehicleMovementComponentTank::SetupVehicle() {
    if (!UpdatedPrimitive) {
        return;
    }

    if (WheelSetups.Num() % 2 != 0 && WheelSetups.Num() > 20) {
        PVehicle = NULL;
        PVehicleDrive = NULL;
        return;
    }

    for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx) {
        const FWheelSetup &WheelSetup = WheelSetups[WheelIdx];
        if (WheelSetup.BoneName == NAME_None) {
            return;
        }
    }

    // Setup the chassis and wheel shapes
    SetupVehicleShapes();

    // Setup mass properties
    SetupVehicleMass();

    // Setup the wheels
    PxVehicleWheelsSimData *PWheelsSimData = PxVehicleWheelsSimData::allocate(WheelSetups.Num());

    SetupWheels(PWheelsSimData);

    // Setup drive data
    PxVehicleDriveSimData DriveData;
    SetupTankDriveHelper(this, DriveData);

    // Create the vehicle
    PxVehicleDriveTank *PVehicleDriveTank = PxVehicleDriveTank::allocate(WheelSetups.Num());
    check(PVehicleDriveTank);

    PVehicleDriveTank->setup(GPhysXSDK, UpdatedPrimitive->GetBodyInstance()->GetPxRigidDynamic_AssumesLocked(),
                             *PWheelsSimData,
                             DriveData, WheelSetups.Num());

    PVehicleDriveTank->setToRestState();

    PVehicleDriveTank->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);

    // cleanup
    PWheelsSimData->free();

    // cache values
    PVehicle = PVehicleDriveTank;
    PVehicleDrive = PVehicleDriveTank;

    PxVehicleChassisData d;

    SetUseAutoGears(TransmissionSetup.bUseGearAutoBox);
}


void UWheeledVehicleMovementComponentTank::PreTick(float DeltaTime) {
    UWheeledVehicleMovementComponent::PreTick(DeltaTime);
}


float UWheeledVehicleMovementComponentTank::CalcThrottleInput() {
    if (FMath::Abs(RawSteeringInput) > 0 && FMath::Abs(RawThrottleInput) <= 0.1f) {
        if (GetCurrentGear() > 0 && GetTargetGear() > 0) {
            SetTargetGear(1, true);
        } else if (GetCurrentGear() < 0 && GetTargetGear() < 0) {
            SetTargetGear(-1, true);
        }

        RawThrottleInput = FMath::Abs(RawSteeringInput);
    }
    return UWheeledVehicleMovementComponent::CalcThrottleInput();
}

void UWheeledVehicleMovementComponentTank::UpdateSimulation(float DeltaTime) {

    if (PVehicleDrive == NULL)
        return;

    PxVehicleDriveTankRawInputData VehicleInputData(PxVehicleDriveTankControlModel::Enum::eSTANDARD);

    float leftThrust = FMath::Clamp(1.f + SteeringInput, 0.f, 1.f);
    float rightThrust = FMath::Clamp(1.f - SteeringInput, 0.f, 1.f);
    float MAX_SPEED = 1000.f;
    float speedScale =
            FMath::Clamp(((MAX_SPEED - FMath::Abs(GetForwardSpeed())) / MAX_SPEED), 0.f, 1.f) * (1.f - ThrottleInput) *
            0.7f;
    float leftBrake = FMath::Max(BrakeInput, rightThrust * 0.7f);
    float rightBrake = FMath::Max(BrakeInput, leftThrust * 0.7f);
    float throttle = FMath::Max(ThrottleInput, FMath::Abs(SteeringInput));

    VehicleInputData.setAnalogAccel(throttle);

    VehicleInputData.setAnalogLeftThrust(BrakeInput  <= 0.1f ? leftThrust: 0.f);
    VehicleInputData.setAnalogRightThrust(BrakeInput  <= 0.1f ? rightThrust : 0.f);
    VehicleInputData.setAnalogLeftBrake(leftBrake);
    VehicleInputData.setAnalogRightBrake(rightBrake);

    print(FString::Printf(TEXT("g: (%d %d)\t s:%03.2f\t t: %03.2f\t l: %03.2f\t r: %03.2f\t lb: %03.2f\t rb: %03.2f"),
                          GetCurrentGear(), GetTargetGear(), GetForwardSpeed(),
                          throttle, leftThrust, rightThrust, leftBrake, rightBrake));

    if (!PVehicleDrive->mDriveDynData.getUseAutoGears()) {
        VehicleInputData.setGearUp(bRawGearUpInput);
        VehicleInputData.setGearDown(bRawGearDownInput);
    }

    // Convert from our curve to PxFixedSizeLookupTable

    PxVehiclePadSmoothingData SmoothData = {
            {ThrottleInputRate.RiseRate, LeftBrakeRate.RiseRate, RightBrakeRate.RiseRate, LeftThrustRate.RiseRate, RightThrustRate.RiseRate},
            {ThrottleInputRate.FallRate, LeftBrakeRate.FallRate, RightBrakeRate.FallRate, LeftThrustRate.FallRate, RightThrustRate.FallRate}
    };

    PxVehicleDriveTank *PVehicleDriveTank = (PxVehicleDriveTank *) PVehicleDrive;
    PxVehicleDriveTankSmoothAnalogRawInputsAndSetAnalogInputs(SmoothData, VehicleInputData, DeltaTime,
                                                              *PVehicleDriveTank);

    LeftTrackSpeed = PVehicle->mWheelsDynData.getWheelRotationSpeed(0) / 1000.f;
    RightTrackSpeed = PVehicle->mWheelsDynData.getWheelRotationSpeed(1) / 1000.f;
}