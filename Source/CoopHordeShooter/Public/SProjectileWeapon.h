// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

class ASProjectile;

/**
 *
 */
UCLASS()
class COOPHORDESHOOTER_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()

protected:

	virtual void Fire() override;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
		TSubclassOf<ASProjectile> ProjectileClass;

};
