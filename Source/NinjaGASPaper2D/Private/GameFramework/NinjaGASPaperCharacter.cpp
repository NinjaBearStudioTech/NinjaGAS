// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPaperCharacter.h"

#include "FPaperCharacterSetupHelper.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"

FName ANinjaGASPaperCharacter::SpriteComponentName(TEXT("Sprite"));

ANinjaGASPaperCharacter::ANinjaGASPaperCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(MeshComponentName))
{
	// Try to create the sprite component.
	Sprite = CreateOptionalDefaultSubobject<UPaperFlipbookComponent>(SpriteComponentName);
	if (Sprite)
	{
		FPaperCharacterSetupHelper::ConfigureFlipbookComponent(Sprite);
		Sprite->SetupAttachment(GetCapsuleComponent());
	}
}

void ANinjaGASPaperCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	FPaperCharacterSetupHelper::SetCMSTick(Sprite, GetCharacterMovement());
}

UPaperFlipbookComponent* ANinjaGASPaperCharacter::GetSprite() const
{
	return Sprite;
}
