// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

struct FPaperCharacterSetupHelper
{
	static void ConfigureFlipbookComponent(UPaperFlipbookComponent* Flipbook)
	{
		if (IsValid(Flipbook))
		{
			static FName CollisionProfileName(TEXT("CharacterMesh"));
			
			Flipbook->AlwaysLoadOnClient = true;
			Flipbook->AlwaysLoadOnServer = true;
			Flipbook->bOwnerNoSee = false;
			Flipbook->bAffectDynamicIndirectLighting = true;
			Flipbook->PrimaryComponentTick.TickGroup = TG_PrePhysics;
			Flipbook->SetCollisionProfileName(CollisionProfileName);
			Flipbook->SetGenerateOverlapEvents(false);
		}
	}
	
	/** Forces animation tick after movement component updates. */
	static void SetCMSTick(UPaperFlipbookComponent* Sprite, UCharacterMovementComponent* MovementComponent)
	{
		if (IsValid(Sprite) && IsValid(MovementComponent))
		{
			if (Sprite->PrimaryComponentTick.bCanEverTick && IsValid(MovementComponent))
			{
				Sprite->PrimaryComponentTick.AddPrerequisite(MovementComponent, MovementComponent->PrimaryComponentTick);
			}
		}
	}
};
