// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASCharacter.h"
#include "NinjaGASPaperCharacter.generated.h"

class UPaperFlipbookComponent;

/**
 * Base Paper Character class, with a pre-configured Ability System Component.
 */
UCLASS(Abstract)
class NINJAGASPAPER2D_API ANinjaGASPaperCharacter : public ANinjaGASCharacter
{
	GENERATED_BODY()

public:
	
	// Name of the Sprite component
	static FName SpriteComponentName;
	
	ANinjaGASPaperCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin character implementation
	virtual void PostInitializeComponents() override;
	// -- End character implementation
	
	/** 
	 * Provides the sprint that represents this character.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GAS|Paper Character")
	UPaperFlipbookComponent* GetSprite() const;
	
private:
	
	/** The flipbook component associated with this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess))
	TObjectPtr<UPaperFlipbookComponent> Sprite;	
	
};
