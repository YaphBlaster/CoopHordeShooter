// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectileWeapon.h"
#include "Public/SProjectile.h"

void ASProjectileWeapon::Fire()
{
	AActor* MyOwner = GetOwner();

	// try and fire a projectile
	if (ProjectileClass && MyOwner)
	{

		FVector MuzzleLocation = SkelMeshComp->GetSocketLocation(MuzzleSocketName);

		// Initialize variables
		FVector EyeLocation;
		FRotator EyeRotation;


		// Since the parameters from GetActorEyesViewPoint are references,
		// It will fill in EyeLocation and EyeRotation when the function executes
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);


		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Set the instigator
		ActorSpawnParams.Instigator = MyOwner->GetInstigatorController()->GetPawn();

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);


	}

}



