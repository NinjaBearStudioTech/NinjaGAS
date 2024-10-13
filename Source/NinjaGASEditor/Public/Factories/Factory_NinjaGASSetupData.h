// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_NinjaGASSetupData.generated.h"

UCLASS()
class NINJAGASEDITOR_API UFactory_NinjaGASSetupData : public UFactory
{
    
    GENERATED_BODY()

public:
    
    UFactory_NinjaGASSetupData(const FObjectInitializer& ObjectInitializer);
    
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
        UObject* Context, FFeedbackContext* Warn) override;
    
};
