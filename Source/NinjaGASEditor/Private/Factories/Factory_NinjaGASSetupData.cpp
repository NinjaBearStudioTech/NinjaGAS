// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "Factories/Factory_NinjaGASSetupData.h"

#include "Data/NinjaGASDataAsset.h"

UFactory_NinjaGASSetupData::UFactory_NinjaGASSetupData(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = UNinjaGASDataAsset::StaticClass();
    bEditAfterNew = true;
    bCreateNew = true;
}

UObject* UFactory_NinjaGASSetupData::FactoryCreateNew(UClass* Class, UObject* InParent, const FName Name,
    const EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    check(Class->IsChildOf(UNinjaGASDataAsset::StaticClass()));
    return NewObject<UNinjaGASDataAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
