// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Public/SWeapon.h"

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the spring arm component
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	// Set the spring arm to rotate in tandem with the pawn's rotation
	// NOTE: this can be set in blueprints under "Camera Settings"
	SpringArmComp->bUsePawnControlRotation = true;
	// Attach the spring arm to be a child of the root component
	SpringArmComp->SetupAttachment(RootComponent);

	// Create the third person camera
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	// Attach the camera to be a child of the spring arm
	CameraComp->SetupAttachment(SpringArmComp);

	// This can be set in blueprints very easily
	// In the component hierarchy:
	//		Click CharacterMovement
	//		Scroll to the Nav Movement section
	//		Toggle "Can Crouch" boolean under the Movement Capabilities section
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Set default Zoom Field of View
	ZoomFOV = 65.0f;

	// Set default Zoom interpolation speed
	ZoomInterpSpeed = 20.0f;

	WeaponAttachSocketName = "WeaponSocket";

}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;

	// Spawn a default weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponAttachSocketName);
	}
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void ASCharacter::Fire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Fire();
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TERNARY OPERATOR
	// IF the zoom button is pressed (bWantsToZoom = TRUE)...
	// Set the TargetFOV to the ZoomFOV
	// ELSE ...
	// Set the TargetFOV to the DefaultFOV
	float TargetFOV = bWantsToZoom ? ZoomFOV : DefaultFOV;

	// set the new field of view to the interpolation of the current field of view TOWARDS the target field of view
	// with the passage of time mapping to DeltaTime
	// with a speed of the set ZoomInterpSpeed
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	// set the current field of view to the new field of view
	CameraComp->SetFieldOfView(NewFOV);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Binding Axis inputs and setting them to move forward and move right
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	// Binding Axis inputs and setting them to look up and turn
	// Since this is inherited from the Character class, we can directly hook in to ACharacter instead of ASCharacter
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	// Since this is inherited from the Character class, we can directly hook in to ACharacter instead of ASCharacter
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);

	// Binding Action inputs for zoom functionality
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::Fire);

}

// Override of GetPawnViewLocation
// Original was giving eye height of mesh
// We will rewrite this to return the camera component's location
FVector ASCharacter::GetPawnViewLocation() const
{
	// If the camera component exists...
	if (CameraComp)
	{
		// Return the component location of the camera component
		return CameraComp->GetComponentLocation();
	}

	// If the camera comp does not exist, then we fallback to the original implementation of GetPawnViewLocation
	return Super::GetPawnViewLocation();
}

