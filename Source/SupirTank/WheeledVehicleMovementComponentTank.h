#pragma once

#include "Vehicles/WheeledVehicleMovementComponent.h"
#include "WheeledVehicleMovementComponentTank.generated.h"


USTRUCT()
struct FTankEngineData
{
    GENERATED_USTRUCT_BODY()

    /** Torque (Nm) at a given RPM*/
    UPROPERTY(EditAnywhere, Category = Setup)
            FRuntimeFloatCurve TorqueCurve;

    /** Maximum revolutions per minute of the engine */
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
    float MaxRPM;

    /** Moment of inertia of the engine around the axis of rotation (Kgm^2). */
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
    float MOI;

    /** Damping rate of engine when full throttle is applied (Kgm^2/s) */
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateFullThrottle;

    /** Damping rate of engine in at zero throttle when the clutch is engaged (Kgm^2/s)*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateZeroThrottleClutchEngaged;

    /** Damping rate of engine in at zero throttle when the clutch is disengaged (in neutral gear) (Kgm^2/s)*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float DampingRateZeroThrottleClutchDisengaged;

    /** Find the peak torque produced by the TorqueCurve */
    float FindPeakTorque() const;
};


USTRUCT()
struct FTankGearData
{
    GENERATED_USTRUCT_BODY()

    /** Determines the amount of torque multiplication*/
    UPROPERTY(EditAnywhere, Category = Setup)
    float Ratio;

    /** Value of engineRevs/maxEngineRevs that is low enough to gear down*/
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
    float DownRatio;

    /** Value of engineRevs/maxEngineRevs that is high enough to gear up*/
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
    float UpRatio;
};

USTRUCT()
struct FTankTransmissionData
{
    GENERATED_USTRUCT_BODY()
    /** Whether to use automatic transmission */
    UPROPERTY(EditAnywhere, Category = VehicleSetup, meta=(DisplayName = "Automatic Transmission"))
    bool bUseGearAutoBox;

    /** Time it takes to switch gears (seconds) */
    UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float GearSwitchTime;

    /** Minimum time it takes the automatic transmission to initiate a gear change (seconds)*/
    UPROPERTY(EditAnywhere, Category = Setup, meta = (editcondition = "bUseGearAutoBox", ClampMin = "0.0", UIMin="0.0"))
    float GearAutoBoxLatency;

    /** The final gear ratio multiplies the transmission gear ratios.*/
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
    float FinalRatio;

    /** Forward gear ratios (up to 30) */
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
    TArray<FTankGearData> ForwardGears;

    /** Reverse gear ratio */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
    float ReverseGearRatio;

    /** Value of engineRevs/maxEngineRevs that is high enough to increment gear*/
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
    float NeutralGearUpRatio;

    /** Strength of clutch (Kgm^2/s)*/
    UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float ClutchStrength;
};

UCLASS()
class ENGINE_API UWheeledVehicleMovementComponentTank : public UWheeledVehicleMovementComponent {
    GENERATED_UCLASS_BODY()

public:

    /** Engine */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    FTankEngineData EngineSetup;

    /** Transmission data */
    UPROPERTY(EditAnywhere, Category = MechanicalSetup)
    FTankTransmissionData TransmissionSetup;

    UPROPERTY(EditAnywhere, Category = TankInput, AdvancedDisplay)
    FVehicleInputRate LeftThrustRate;
    UPROPERTY(EditAnywhere, Category = TankInput, AdvancedDisplay)
    FVehicleInputRate RightThrustRate;
    UPROPERTY(EditAnywhere, Category = TankInput, AdvancedDisplay)
    FVehicleInputRate RightBrakeRate;
    UPROPERTY(EditAnywhere, Category = TankInput, AdvancedDisplay)
    FVehicleInputRate LeftBrakeRate;

    UPROPERTY(EditAnywhere, Category = TankSetup, AdvancedDisplay)
    FName TurretBoneName;

    UPROPERTY(EditAnywhere, Category = TankSetup, AdvancedDisplay)
    FName CannonBoneName;
    /*
     10.0f,	//fall rate eANALOG_INPUT_ACCEL=0
     10.0f,	//fall rate eANALOG_INPUT_BRAKE_LEFT
     10.0f,	//fall rate eANALOG_INPUT_BRAKE_RIGHT
     5.0f,	//fall rate eANALOG_INPUT_THRUST_LEFT
     5.0f	//fall rate eANALOG_INPUT_THRUST_RIGHT
     //6.0f,	//rise rate eANALOG_INPUT_ACCEL=0,
     //6.0f,	//rise rate eANALOG_INPUT_BRAKE,
     //6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE,
     //2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT,
     //2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT,
     */
    UPROPERTY(BlueprintReadOnly, Transient, Category = "Track Data")
    float RightTrackSpeed;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "Track Data")
    float LeftTrackSpeed;

protected:
    virtual void SetupVehicle() override;
    virtual void UpdateSimulation(float DeltaTime) override;

protected:
    UPROPERTY(Transient)
    float Acceleration;
    UPROPERTY(Transient)
    float LeftBreak;
    UPROPERTY(Transient)
    float RightBreak;
    UPROPERTY(Transient)
    float LeftThrust;
    UPROPERTY(Transient)
    float RightThrust;

};