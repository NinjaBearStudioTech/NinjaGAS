// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AI/BehaviorTree/BTService_UpdateAttributes.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"

UBTService_UpdateAttributes::UBTService_UpdateAttributes()
{
    bNotifyBecomeRelevant = true;
    bNotifyTick = false;
	bNotifyCeaseRelevant = true;
    NodeName = "Update Attributes";
    Interval = 1.0f;
    RandomDeviation = 0.f;
	ApplyBlackboardFilters();
}

void UBTService_UpdateAttributes::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		for (FAttributeBlackboardMapping& Setup : AttributeMappings)
		{
			ApplyBlackboardFiltersToMapping(Setup);
			Setup.AttributeValueKey.ResolveSelectedKey(*BBAsset);
		}
	}
}

void UBTService_UpdateAttributes::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	FUpdateAttributesMemory* MyMemory = CastInstanceNodeMemory<FUpdateAttributesMemory>(NodeMemory);
	check(MyMemory);

	const int32 MappingsSize = AttributeMappings.Num();
	MyMemory->AttributeDelegateHandles.Reserve(MappingsSize);
}

void UBTService_UpdateAttributes::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BindToAttributes(OwnerComp, NodeMemory);
}

void UBTService_UpdateAttributes::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	BindToAttributes(OwnerComp, NodeMemory);
}

void UBTService_UpdateAttributes::HandleAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData, UBlackboardComponent* BlackboardComponent) const
{
	for (const FAttributeBlackboardMapping& Mapping : AttributeMappings)
	{
		if (Mapping.Attribute == OnAttributeChangeData.Attribute)
		{
			const float AttributeValue = OnAttributeChangeData.NewValue;
			BlackboardComponent->SetValueAsFloat(Mapping.AttributeValueKey.SelectedKeyName, AttributeValue);	
		}
	}
}

void UBTService_UpdateAttributes::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FUpdateAttributesMemory* MyMemory = CastInstanceNodeMemory<FUpdateAttributesMemory>(NodeMemory);
	check(MyMemory);
	
	const APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);

	if (IsValid(AbilitySystemComponent))
	{
		for (auto It = MyMemory->AttributeDelegateHandles.CreateIterator(); It; ++It)
		{
			const FGameplayAttribute Attribute = It->Key;
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(It->Value);
			It.RemoveCurrent();
		}
	}
}

uint16 UBTService_UpdateAttributes::GetInstanceMemorySize() const
{
	return sizeof(FUpdateAttributesMemory);
}

FString UBTService_UpdateAttributes::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s:\nAttributes to update: %d")
		, *Super::GetStaticDescription()
		, AttributeMappings.Num());
}

void UBTService_UpdateAttributes::ApplyBlackboardFilters()
{
	for (FAttributeBlackboardMapping& Setup : AttributeMappings)
	{
		ApplyBlackboardFiltersToMapping(Setup);
	}
}

void UBTService_UpdateAttributes::ApplyBlackboardFiltersToMapping(FAttributeBlackboardMapping& Setup)
{
	const FString FilterValue = Setup.Attribute.AttributeName + TEXT(".Value");
	Setup.AttributeValueKey.AllowedTypes.AddUnique(NewObject<UBlackboardKeyType_Float>(this, *FilterValue));
}

void UBTService_UpdateAttributes::BindToAttributes(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FUpdateAttributesMemory* MyMemory = CastInstanceNodeMemory<FUpdateAttributesMemory>(NodeMemory);
	check(MyMemory);
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (IsValid(Blackboard))
	{
		const APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
		UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);

		if (IsValid(AbilitySystemComponent))
		{
			int32 AttributeCount = 0;
			for (const FAttributeBlackboardMapping& Mapping : AttributeMappings)
			{
				if (AbilitySystemComponent->HasAttributeSetForAttribute(Mapping.Attribute))
				{
					const FGameplayAttribute Attribute = Mapping.Attribute;
					FDelegateHandle Delegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute)
						.AddUObject(this, &ThisClass::HandleAttributeChanged, Blackboard);

					MyMemory->AttributeDelegateHandles.Add(Attribute, Delegate);
				
					const float AttributeValue = AbilitySystemComponent->GetNumericAttribute(Attribute);
					Blackboard->SetValueAsFloat(Mapping.AttributeValueKey.SelectedKeyName, AttributeValue);
					AttributeCount++;
				}
			}

			bNotifyTick = AttributeCount != AttributeMappings.Num();
		}
	}
}

#if WITH_EDITOR
void UBTService_UpdateAttributes::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, AttributeMappings))
	{
		ApplyBlackboardFilters();
	}
}
#endif
