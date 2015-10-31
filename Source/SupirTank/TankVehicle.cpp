#include "SupirTank.h"
#include "TankVehicle.h"

class UWheeledVehicleMovementComponentTank;

ATankVehicle::ATankVehicle(const FObjectInitializer &ObjectInitializer) :
        Super(ObjectInitializer.SetDefaultSubobjectClass<UWheeledVehicleMovementComponentTank>(
                VehicleMovementComponentName)) {

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->AttachTo(RootComponent);
    CameraBoom->TargetArmLength = 1000.0f; // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->AttachTo(CameraBoom,
                           USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}


void ATankVehicle::SetupPlayerInputComponent(class UInputComponent *InputComponent) {
    check(InputComponent);

    //InputComponent->BindAction("Jump", IE_Pressed, this, &ATankVehicle::Jump);

    InputComponent->BindAxis("MoveForward", this, &ATankVehicle::SetThrottleInput);
    InputComponent->BindAxis("MoveRight", this, &ATankVehicle::SetSteeringInput);

    // We have 2 versions of the rotation bindings to handle different kinds of devices differently
    // "turn" handles devices that provide an absolute delta, such as a mouse.
    // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
    InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void ATankVehicle::SetThrottleInput(float value) {
    UWheeledVehicleMovementComponent* movementComponent = GetVehicleMovement();

    if (nullptr != movementComponent){
        movementComponent->SetThrottleInput(value);
    }
}

void ATankVehicle::SetSteeringInput(float value) {
    UWheeledVehicleMovementComponent* movementComponent = GetVehicleMovement();

    if (nullptr != movementComponent){
        movementComponent->SetSteeringInput(value);
    }
}