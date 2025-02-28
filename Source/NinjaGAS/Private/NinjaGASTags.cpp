// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "NinjaGASTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_GAS_Ability_Passive, "Ability.Passive", "If present, activates the ability as soon as the avatar is set.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_GAS_Ability_InitialCooldown, "Ability.InitialCooldown", "If present, applies the cooldown Gameplay Effect as soon as the avatar is set.");
UE_DEFINE_GAMEPLAY_TAG(Tag_GAS_Activation_Fail_BlockedByTags, "Activation.Fail.BlockedByTags");
UE_DEFINE_GAMEPLAY_TAG(Tag_GAS_Activation_Fail_CantAffordCost, "Activation.Fail.CantAffordCost");
UE_DEFINE_GAMEPLAY_TAG(Tag_GAS_Activation_Fail_IsDead, "Activation.Fail.IsDead");
UE_DEFINE_GAMEPLAY_TAG(Tag_GAS_Activation_Fail_MissingTags, "Activation.Fail.MissingTags");
UE_DEFINE_GAMEPLAY_TAG(Tag_GAS_Activation_Fail_Networking, "Activation.Fail.Networking");
UE_DEFINE_GAMEPLAY_TAG(Tag_GAS_Activation_Fail_OnCooldown, "Activation.Fail.OnCooldown");