// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPaperPlayerCharacter.h"

#include "FPaperCharacterSetupHelper.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"

FName ANinjaGASPaperPlayerCharacter::SpriteComponentName(TEXT("Sprite"));

ANinjaGASPaperPlayerCharacter::ANinjaGASPaperPlayerCharacter(const FObjectInitializer& ObjectInitializer)
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

void ANinjaGASPaperPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	FPaperCharacterSetupHelper::SetCMSTick(Sprite, GetCharacterMovement());
}

UPaperFlipbookComponent* ANinjaGASPaperPlayerCharacter::GetSprite() const
{
	return Sprite;
}
