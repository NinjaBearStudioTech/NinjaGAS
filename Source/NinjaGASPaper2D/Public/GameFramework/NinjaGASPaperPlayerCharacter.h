// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASPlayerCharacter.h"
#include "NinjaGASPaperPlayerCharacter.generated.h"

class UPaperFlipbookComponent;

/**
 * Base Paper Character class, with a pre-configured Ability System Component.
 * This class expects the ASC to be provided by the Player State.
 */
UCLASS(Abstract)
class NINJAGASPAPER2D_API ANinjaGASPaperPlayerCharacter : public ANinjaGASPlayerCharacter
{
	
	GENERATED_BODY()
	
public:
	
	// Name of the Sprite component
	static FName SpriteComponentName;
	
	ANinjaGASPaperPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin character implementation
	virtual void PostInitializeComponents() override;
	// -- End character implementation
	
	/** 
	 * Provides the sprint that represents this character.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GAS|Paper Player Character")
	UPaperFlipbookComponent* GetSprite() const;
	
private:
	
	/** The flipbook component associated with this character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperFlipbookComponent> Sprite;	
	
};