#pragma once

#include "GameFramework/WheeledVehicle.h"
#include "TankVehicle.generated.h"

UCLASS(abstract)
class SUPIRTANK_API ATankVehicle : public AWheeledVehicle {
    GENERATED_UCLASS_BODY()

public:
    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

protected:

    void SetThrottleInput(float value);
    void SetSteeringInput(float Value);

protected:

    virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

};
