// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateAttributes.generated.h"

/**
 * Maps a Gameplay Attribute to the blackboard key used to store its value.
 */
USTRUCT(BlueprintType)
struct FAttributeBlackboardMapping
{
	
	GENERATED_BODY()

	/** Gameplay Tag that identifies the token. */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	FGameplayAttribute Attribute;
	
	/** Blackboard entry that will store the attribute's value. */
	UPROPERTY(EditAnywhere, Category = "Attribute")    
	FBlackboardKeySelector AttributeValueKey;

};

/**
 * Memory used to store persistent values for this Service node.
 */
struct FUpdateAttributesMemory
{
	/** Delegate handle for the attribute callback. */
	TMap<FGameplayAttribute, FDelegateHandle> AttributeDelegateHandles;
};

/**
 * Transfers combat attributes to the Blackboard, which may be useful for the agent's decision-making.
 */
UCLASS(DisplayName = "Update Attributes", Category = "Ninja GAS")
class NINJAGAS_API UBTService_UpdateAttributes : public UBTService
{

    GENERATED_BODY()

public:

	/** Maps all gameplay tags to their appropriate blackboard keys. */
	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (TitleProperty = "Attribute"))
	TArray<FAttributeBlackboardMapping> AttributeMappings;
	
    UBTService_UpdateAttributes();

protected:

	// -- Begin Service implementation
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
    virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	// -- End Service implementation

	/**
	 * Reacts to a change in an Attribute Value.
	 */
	void HandleAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData, UBlackboardComponent* BlackboardComponent) const;
	
	/**
	 * Reinforces blackboard filters to all registered token mappings.
	 */
	void ApplyBlackboardFilters();

	/**
	 * Reinforces blackboard filters to a specific token mapping.
	 */
	void ApplyBlackboardFiltersToMapping(FAttributeBlackboardMapping& Setup);

	void BindToAttributes(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

#if WITH_EDITOR
public:

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif
	
};
