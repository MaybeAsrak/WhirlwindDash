

#include "include\PerkEntryPointExtenderAPI.h"

[[nodiscard]] inline RE::Actor* GetActorFromHkbCharacter(RE::hkbCharacter* a_hkbCharacter) {
    if (a_hkbCharacter) {
        const RE::BShkbAnimationGraph* animGraph =
            SKSE::stl::adjust_pointer<RE::BShkbAnimationGraph>(a_hkbCharacter, -0xC0);
        return animGraph->holder;
    }

    return nullptr;
}

void CastSpellsPerk(RE::MagicItem* a_MI, RE::Actor* caster, RE::Actor* Target) {
    if (a_MI->IsPermanent()) {
        RE::SpellItem* Spell = a_MI->As<RE::SpellItem>();
        if (Spell) {
            Target->AddSpell(Spell);
        }

    } else {
        caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
            ->CastSpellImmediate(a_MI, false, Target, 1.0f, false, 0.0f, nullptr);
    }
}

bool IsAnimPlaying(RE::TESObjectREFR* a_thisObj, std::string test) {
    auto ToClipGenerator = [](RE::hkbNode* a_node) -> RE::hkbClipGenerator* {
        if (a_node && a_node->GetClassType()) {
            if (_strcmpi(a_node->GetClassType()->name, "hkbClipGenerator") == 0)
                return skyrim_cast<RE::hkbClipGenerator*>(a_node);

            // if (_strcmpi(a_node->GetClassType()->name, "BSSynchronizedClipGenerator") == 0) {
            //     auto syncClip = skyrim_cast<RE::BSSynchronizedClipGenerator*>(a_node);
            //     if (syncClip) return syncClip->clipGenerator;
            // }
        }

        return nullptr;
    };

    if (a_thisObj) {
        RE::BSAnimationGraphManagerPtr graphMgr;
        if (a_thisObj->GetAnimationGraphManager(graphMgr) && graphMgr) {
            for (const auto project : graphMgr->graphs) {
                auto behaviourGraph = project ? project->behaviorGraph : nullptr;
                auto activeNodes = behaviourGraph ? behaviourGraph->activeNodes : nullptr;
                if (activeNodes) {
                    // CPrint("Behavior Project Name :  \"%s\"", project->projectName.c_str());
                    // auto API = GetAPI();
                    for (auto nodeInfo : *activeNodes) {
                        auto nodeClone = nodeInfo.nodeClone;
                        if (nodeClone && nodeClone->GetClassType()) {
                            auto clipGenrator = ToClipGenerator(nodeClone);
                            if (clipGenrator) {
                                // RE::ConsoleLog::GetSingleton()->Print(clipGenrator->animationName.c_str());
                                if (strstr(clipGenrator->animationName.c_str(), test.c_str())) {
                                    RE::ConsoleLog::GetSingleton()->Print(clipGenrator->animationName.c_str());
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

float GetEffectiveCastSpeed(RE::ActorMagicCaster* a_AMC, RE::Actor* a, float perkfactor) {
    float workingperkfactor = perkfactor;
    auto source = a_AMC->GetCastingSource();
    RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "CastTime", 2,
                         {a_AMC->currentSpell});  // Channel 2 will be the all purpose channel for everything

    if (source == RE::MagicSystem::CastingSource::kRightHand) {
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "CastTime", 3,
                             {a_AMC->currentSpell});  // Channel 3 will be the Right Hand Channel
    } else if (source == RE::MagicSystem::CastingSource::kLeftHand) {
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "CastTime", 4,
                             {a_AMC->currentSpell});  // Channel 4 will be the Left Hand Channel
    }

    return workingperkfactor;
}

float GetEffectiveConcentrationSpeed(RE::ActorMagicCaster* a_AMC, RE::Actor* a, float perkfactor) {
    float workingperkfactor = perkfactor;
    auto source = a_AMC->GetCastingSource();
    RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "ConcentrationTime", 2,
                         {a_AMC->currentSpell});  // Channel 2 will be the all purpose channel for everything

    if (source == RE::MagicSystem::CastingSource::kRightHand) {
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "ConcentrationTime", 3,
                             {a_AMC->currentSpell});  // Channel 3 will be the Right Hand Channel
    } else if (source == RE::MagicSystem::CastingSource::kLeftHand) {
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "ConcentrationTime", 4,
                             {a_AMC->currentSpell});  // Channel 4 will be the Left Hand Channel
    }

    return workingperkfactor;
}

float GetEffectiveDualCast(RE::ActorMagicCaster* a_AMC, RE::Actor* a, float perkfactor) {
    float workingperkfactor = perkfactor;
    RE::MagicItem* currentspell = a_AMC->currentSpell;
    auto source = a_AMC->GetCastingSource();
    if (source == RE::MagicSystem::CastingSource::kRightHand) {
        if (!a_AMC->currentSpell) {
            currentspell = a->GetActorRuntimeData().selectedSpells[1];
        }
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "DualCast", 3,
                             {currentspell});  // Channel 3 will be the Right Hand Channel
    } else if (source == RE::MagicSystem::CastingSource::kLeftHand) {
        if (!a_AMC->currentSpell) {
            currentspell = a->GetActorRuntimeData().selectedSpells[0];
        }
        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "DualCast", 4,
                             {currentspell});  // Channel 4 will be the Left Hand Channel
    }
    RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &workingperkfactor, "DualCast", 2,
                         {currentspell});  // Channel 2 will be the all purpose channel for everything
    return workingperkfactor;
}
// need to figure out a way to check if you are actually dualcasting a spell and to then have a separate layer of
// control

//
// void EvaluateDynamicMGEFs(RE::Actor* a_Actor) {
// RE::SendHUDMessage

//        auto ActiveEffects = a_Actor->AsMagicTarget()->GetActiveEffectList();
//        float updatefactor = 1.0f;
//        if (ActiveEffects) {
//            for (const auto& effect : *ActiveEffects) {
//                const auto& setting = effect ? effect->GetBaseObject() : nullptr;
//
//                if (setting) {
//                    //            logger::info("The effect we're checking is {}. The effect we're looking for is {}.",
//                    //            setting->formID,
//                    //                         akMagicEffect->formID);
//                    if (setting->HasKeyword("ASR_TestKeyword")) {
//                        updatefactor = 1.0f;
//                        //updatefactor = std::bit_cast<float>(effect->pad8C);
//                        //float basemagnitude = effect->effect->GetMagnitude();
//
//                        RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a_Actor, &updatefactor,
//                                             "DynamicMagnitude", 2,
//                                             {effect->spell});
//
//                        //float oldupdatefactor = std::bit_cast<float>(effect->pad8C);
//
//                        //effect->pad8C = updatefactor;
//
//
//                        effect->magnitude = effect->magnitude * updatefactor;
//                    }
//                }
//            }
//        }
//}

struct Hooks {
    struct Update {
        static void thunk(RE::ActorMagicCaster* a_AMC, float a_deltatime) {
            float updatedtime = a_deltatime;
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            float castperkfactor = 1.00f;
            float concentrationperkfactor = 1.00f;

            if (source == RE::MagicSystem::CastingSource::kRightHand ||
                source == RE::MagicSystem::CastingSource::kLeftHand) {
                if (a_AMC->state.get() == RE::MagicCaster::State::kUnk02) {
                    castperkfactor = GetEffectiveCastSpeed(a_AMC, a, castperkfactor);
                    // perkfactor = std::min((a_AMC->currentSpell->GetChargeTime()) / a_deltatime, perkfactor);
                    castperkfactor = std::max(0.05f, castperkfactor);
                    updatedtime = a_deltatime * castperkfactor;

                }

                else if ((a_AMC->state.get() == RE::MagicCaster::State::kCasting) &&
                         (a_AMC->currentSpell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration)) {
                    concentrationperkfactor = GetEffectiveConcentrationSpeed(a_AMC, a, concentrationperkfactor);
                    // perkfactor = std::min((a_AMC->currentSpell->GetChargeTime()) / a_deltatime, perkfactor);
                    concentrationperkfactor = std::max(0.05f, concentrationperkfactor);
                    updatedtime = a_deltatime * concentrationperkfactor;
                }
            }
            func(a_AMC, updatedtime);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct DualCasting {
        static bool thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            float dualcastfactor = 1.00f;
            if (source == RE::MagicSystem::CastingSource::kRightHand ||
                source == RE::MagicSystem::CastingSource::kLeftHand) {
                // RE::HandleEntryPoint(RE::PerkEntryPoint::kModSpellCost, a, &dualcastfactor, "DualCast", 2,
                //                      {a_AMC->currentSpell});
                dualcastfactor = GetEffectiveDualCast(a_AMC, a, dualcastfactor);
                if (dualcastfactor >= 5) {
                    // a_AMC->flags.set(RE::ActorMagicCaster::Flags::kDualCasting);
                    return true;
                }
            }
            return func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct StartChargingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            RE::ConsoleLog::GetSingleton()->Print("StartCharge!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct FinishChargingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            RE::ConsoleLog::GetSingleton()->Print("FinishCharge!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct StartCastingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            RE::ConsoleLog::GetSingleton()->Print("StartCast!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct FinishCastingHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            RE::ConsoleLog::GetSingleton()->Print("FinishCast!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct OverrideSpellHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            RE::ConsoleLog::GetSingleton()->Print("FinishCast!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct OverrideSpellCostHook {
        static void thunk(RE::ActorMagicCaster* a_AMC) {
            auto a = a_AMC->GetCasterAsActor();
            auto source = a_AMC->GetCastingSource();
            RE::ConsoleLog::GetSingleton()->Print("FinishCast!");
            func(a_AMC);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct OnHitHook {
        static void thunk(RE::Actor* a_AMC, RE::HitData& hit) {
            // auto a = a_AMC->GetCasterAsActor();
            // auto source = a_AMC->GetCastingSource();

            func(a_AMC, hit);
            if (hit.criticalDamageMult) {
                RE::ConsoleLog::GetSingleton()->Print("Hello, critski!");
            }
            if (hit.flags.any(RE::HitData::Flag::kCritical) == 1) {
                RE::ConsoleLog::GetSingleton()->Print("Hello, critter!");
            }
            if (hit.flags.any(RE::HitData::Flag::kSneakAttack) == 1) {
                RE::ConsoleLog::GetSingleton()->Print("Hello, scritter!");
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct StartHitting {
        static void thunk(RE::Actor* a_AI) {
            // auto a = a_AMC->GetCasterAsActor();
            // auto source = a_AMC->GetCastingSource();
            // if (a_AI->GetActorRuntimeData().currentProcess)
            //   if (a_AI->GetActorRuntimeData().currentProcess->high)
            if (a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()) {
                RE::AttackData A_Data = a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data;

                // RE::ConsoleLog::GetSingleton()->Print(a_AI->GetActorRuntimeData()
                //                                           .currentProcess->high->attackData.get()
                //                                           ->data.attackType->GetName());
                // RE::ConsoleLog::GetSingleton()->Print("Hello, normalcy!");
                // if (A_Data.flags.any(RE::AttackData::AttackFlag::kRotatingAttack) == 1) {
                //    RE::ConsoleLog::GetSingleton()->Print("Hello, normalcy!");
                //}
                // if (A_Data.flags.any(RE::AttackData::AttackFlag::kContinuousAttack) == 1) {
                //    RE::ConsoleLog::GetSingleton()->Print("Hello, prom!");
                //}
                // if (A_Data.flags.any(RE::AttackData::AttackFlag::kChargeAttack) == 1) {
                //        RE::ConsoleLog::GetSingleton()->Print("Hello, carhe!");
                //    }
                // if (A_Data.flags.any(RE::AttackData::AttackFlag::kNone) == 1) {
                //    RE::ConsoleLog::GetSingleton()->Print("Hello, carheee!");
                //}
                // if (A_Data.flags.any(RE::AttackData::AttackFlag::kIgnoreWeapon) == 1) {
                //    RE::ConsoleLog::GetSingleton()->Print("Hello, carhdfe!");
                //}
                // if (A_Data.flags.any(RE::AttackData::AttackFlag::kOverrideData) == 1) {
                //    RE::ConsoleLog::GetSingleton()->Print("Hello, carhdfe!");
                //}
                if (A_Data.flags.any(RE::AttackData::AttackFlag::kPowerAttack) == 1 && a_AI->IsSneaking()) {
                    if (a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data.pad34 != 77) {
                        RE::ConsoleLog::GetSingleton()->Print("Hello, power!");
                        a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data.pad34 = 77;
                    } else {
                        RE::ConsoleLog::GetSingleton()->Print("Hello, spower!");
                        a_AI->GetActorRuntimeData().currentProcess->high->attackData.get()->data.pad34 = 74;
                    }
                } else {
                    RE::ConsoleLog::GetSingleton()->Print("Hello, normalcy!");
                }
            }
            func(a_AI);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct StartDying {
        static bool thunk(RE::Character* a_Killed, RE::Character* a_Killer) {
            if (a_Killer == RE::PlayerCharacter::GetSingleton()) {
                RE::ConsoleLog::GetSingleton()->Print("Hello, playerski!");
            }
            if (a_Killed == RE::PlayerCharacter::GetSingleton()) {
                RE::ConsoleLog::GetSingleton()->Print("GoodBye, playerski!");
            }

            return func(a_Killed, a_Killer);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct AnimationSpeed {
        static void thunk(RE::hkbClipGenerator* a_Killed, const RE::hkbContext& a_context, float a_timesteps) {
            if (a_context.character) {
                auto a_actor = GetActorFromHkbCharacter(a_context.character);

                if (a_actor && a_actor == RE::PlayerCharacter::GetSingleton()) {
                    a_Killed->triggers.get()->triggers[0].event.id;
                    RE::ConsoleLog::GetSingleton()->Print(a_Killed->animationName.c_str());
                }
            }
            return func(a_Killed, a_context, a_timesteps);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct StartApplying {
        static bool thunk(RE::MagicTarget* a_MT, RE::MagicTarget::AddTargetData& ATD) {
            // auto a = a_AMC->GetCasterAsActor();
            // auto source = a_AMC->GetCastingSource();
            RE::ConsoleLog::GetSingleton()->Print("Hello, scritter!");
            RE::Effect* firsteffect = ATD.magicItem->effects[0];
            if (ATD.effect == firsteffect) {
                if (ATD.magicItem->GetFullName()) RE::ConsoleLog::GetSingleton()->Print((ATD.magicItem->GetFullName()));
            }
            return func(a_MT, ATD);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct StartApplyinge {
        static void thunk(RE::BSTEventSource<RE::TESHitEvent>& a_source, RE::TESHitEvent& a_event) {
            // auto a = a_AMC->GetCasterAsActor();
            // auto source = a_AMC->GetCastingSource();
            const auto aggressor = a_event.cause.get();
            const auto target = a_event.target.get();
            const auto source = RE::TESForm::LookupByID(a_event.source);
            const auto projectile = RE::TESForm::LookupByID<RE::BGSProjectile>(a_event.projectile);

            if (aggressor && target && source && projectile) {
                const auto aggressorActor = aggressor->As<RE::Actor>();
                const auto targetActor = target->As<RE::Actor>();

                auto spell = source->As<RE::MagicItem>();
                if (!spell) {
                    if (const auto weapon = source->As<RE::TESObjectWEAP>(); weapon && weapon->IsStaff()) {
                        spell = weapon->formEnchanting;
                    }
                }

                RE::ConsoleLog::GetSingleton()->Print("Hello, sdf!");
                if (spell->GetFullName()) RE::ConsoleLog::GetSingleton()->Print((spell->GetFullName()));
            }
            func(a_source, a_event);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct ModifyAnimationData {
        static void thunk(RE::Character* a_this, RE::BSAnimationUpdateData& a_data) {
            float timescale = 1.0f;
            // float timescaler = a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition);

            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_this, &timescale, "TimeScale", 2, {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_this, &timescale, "TimeScale", 3, {});
            a_data.deltaTime *= timescale;
            // a_data.deltaTime *= timescaler / 100.0f;
            func(a_this, a_data);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct ModifyMovement {
        static void thunk(RE::Actor* a_this, float a_deltaTime) {
            float timescale = 1.0f;
            float timescaler = a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition);
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_this, &timescale, "TimeScale", 2, {});
            RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_this, &timescale, "TimeScale", 4, {});
            func(a_this, timescale * a_deltaTime * timescaler / 100.0f);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ModifyMovementActor {
        static RE::bhkCharacterController* thunk(RE::Actor* a_this, float a_arg2, const RE::NiPoint3& a_position) {
            // float timescaler = a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightMobilityCondition);
            float timescaler = a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightAttackCondition);
            // float timescaler = a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftAttackCondition);
            // float timescaler = a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition);
            auto Delta = a_position;
            if (a_this == RE::PlayerCharacter::GetSingleton()) {
                bool boob = IsAnimPlaying(a_this, std::string("Whirlwind"));

                Delta.z = 0.0f;
                float distance = Delta.Length();
                // if (Delta.y > -0.01f) {
                //     Delta.y = std::abs(Delta.y);
                // }
                // if (Delta.x > -0.01f) {
                //     Delta.x = std::abs(Delta.x);
                // }
                double angleOfmovement = std::atan2(Delta.y, Delta.x) * 180 / (atan(1) * 4);
                if (boob) {
                    // double angleOfmovement = std::atan2(Delta.y, Delta.x) * 180 / (atan(1) * 4);
                    double newangle = double(timescaler);
                    float newY = std::sin(float(newangle) * atan(1) * 4 / 180) * distance;
                    float newX = std::cos(float(newangle) * atan(1) * 4 / 180) * distance;
                    char buffer[150];
                    RE::ConsoleLog::GetSingleton()->Print("gottem");
                    sprintf_s(buffer, "angles: %f, %f, %f, %f, %f", Delta.y, Delta.x, newangle, newY, newX);
                    RE::ConsoleLog::GetSingleton()->Print(buffer);

                    Delta.x = newX;
                    Delta.y = newY;
                } else if (distance > 0.5f) {
                    // a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightAttackCondition);

                    a_this->AsActorValueOwner()->SetActorValue(
                        RE::ActorValue::kRightAttackCondition,
                        (2 * angleOfmovement +
                         a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightAttackCondition)) /
                            3);
                    char buffer[150];

                    sprintf_s(buffer, "angle2s: %f, %f, %f, %f", Delta.y, Delta.x, angleOfmovement,
                              a_this->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightAttackCondition));
                    RE::ConsoleLog::GetSingleton()->Print(buffer);
                }
            }
            Delta.z = a_position.z;
            return func(a_this, a_arg2, Delta);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct ProjectileUpdateHook {
        static void thunk(RE::Projectile* a_this, float a_deltTime) {
            float timescale = 1.0f;
            auto a_actor = a_this->GetActorCause()->actor.get().get();
            float timescaler = a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition);
            RE::ConsoleLog::GetSingleton()->Print("Hello, weakerboys!");
            if (a_actor == RE::PlayerCharacter::GetSingleton()) {
                RE::ConsoleLog::GetSingleton()->Print("Hello, projectile!");
                // RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescale, "TimeScale", 5,
                // {}); timescaler =
                // a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition);

            } else if (a_actor) {
                RE::ConsoleLog::GetSingleton()->Print("Hello, weakerestboys!");
                // RE::HandleEntryPoint(RE::PerkEntryPoint::kModPercentBlocked, a_actor, &timescale, "TimeScale", 4,
                // {}); timescaler =
                // a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftMobilityCondition);
            }
            char buffer[100];
            sprintf_s(buffer, "time %f", timescaler * a_deltTime);
            RE::ConsoleLog::GetSingleton()->Print(buffer);
            func(a_this, (a_deltTime * timescaler));
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct PlayerCharacter_Update {
        static void thunk(RE::PlayerCharacter* a_player, float a_delta) {
            /// add manual regen perk entry points. Need to add slow time perk entry points too

            func(a_player, a_delta);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    struct Character_Update {
        static void thunk(RE::Character* a_char, float a_delta) { func(a_char, a_delta); }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    static void Install() {
        stl::write_vfunc<RE::PlayerCharacter, 0xAD, PlayerCharacter_Update>();
        stl::write_vfunc<RE::Character, 0xAD, Character_Update>();

        stl::write_vfunc<RE::ActorMagicCaster, 0x1D, Update, 0>();
        stl::write_vfunc<RE::ActorMagicCaster, 0x16, DualCasting, 0>();

        stl::write_vfunc<RE::ActorMagicCaster, 0x04, StartChargingHook, 0>();
        stl::write_vfunc<RE::ActorMagicCaster, 0x05, FinishChargingHook, 0>();
        stl::write_vfunc<RE::ActorMagicCaster, 0x06, StartCastingHook, 0>();
        stl::write_vfunc<RE::ActorMagicCaster, 0x07, FinishCastingHook, 0>();

        stl::write_vfunc<RE::PlayerCharacter, 0x0EF, StartHitting, 0>();
        stl::write_vfunc<RE::PlayerCharacter, 0x0C8, ModifyMovementActor, 0>();

        // stl::write_vfunc<RE::PlayerCharacter, 0x1, StartApplying, 4>();
        // stl::write_vfunc<RE::Character, 0x1, StartApplying, 4>();
        // stl::write_vfunc<RE::PlayerCharacter, 0x79, ModifyAnimationData>();
        // stl::write_vfunc<RE::Character, 0x79, ModifyAnimationData>();
        // stl::write_vfunc<RE::hkbClipGenerator, 0x05, AnimationSpeed>();

        // stl::write_vfunc<RE::Projectile, 0xAB, ProjectileUpdateHook>();
        // stl::write_vfunc<RE::ConeProjectile, 0xAB, ProjectileUpdateHook>();
        // stl::write_vfunc<RE::MissileProjectile, 0xAB, ProjectileUpdateHook>();
        // stl::write_vfunc<RE::ArrowProjectile, 0xAB, ProjectileUpdateHook>();
        // stl::write_vfunc<RE::BeamProjectile, 0xAB, ProjectileUpdateHook>();
        // stl::write_vfunc<RE::FlameProjectile, 0xAB, ProjectileUpdateHook>();

        REL::Relocation<std::uintptr_t> FunctionModifyMovement{RELOCATION_ID(36359, 37350), REL::Relocate(0xF0, 0xFB)};
        stl::write_thunk_call<ModifyMovement>(FunctionModifyMovement.address());

        REL::Relocation<std::uintptr_t> functionA{RELOCATION_ID(37673, 38627), REL::Relocate(0x3C0, 0x4A8)};
        stl::write_thunk_call<OnHitHook>(functionA.address());

        // REL::Relocation<std::uintptr_t> functionB{RELOCATION_ID(39577, 40663), REL::Relocate(0xD, 0xD)};
        // stl::write_thunk_call<StartHitting>(functionB.address());

        REL::Relocation<std::uintptr_t> functionC{RELOCATION_ID(37673, 37896), REL::Relocate(0x3C0, 0x5F8)};
        stl::write_thunk_call<StartDying>(functionC.address());

        // REL::Relocation<std::uintptr_t> functionD{RELOCATION_ID(37832, 38786), REL::Relocate(0x1C3, 0x29B)};
        // stl::write_thunk_call<StartApplyinge>(functionD.address());

        // REL::Relocation<std::uintptr_t> functionB{RELOCATION_ID(37673, 38581), REL::Relocate(0x54, 0x54)};
        // stl::write_thunk_call<StartHitting>(functionB.address());
        // 6EE0F0+D
        // 63D4E0+5F8
        // 140628C20 + 3C0
        // 65DB00+54
    }
};

// void CastSpellMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::SpellItem* akSpell,
//                      RE::Actor* akTarget, float a_effectiveness) {
//     akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//         ->CastSpellImmediate(akSpell, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
// }
//
// void CastEnchantmentMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::EnchantmentItem* akEnchantment,
//                          RE::Actor* akTarget, float a_effectiveness) {
//     akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//         ->CastSpellImmediate(akEnchantment, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
// }
//
// void CastPotionMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::AlchemyItem* akPotion, RE::Actor* akTarget,
// float a_effectiveness) {
//     akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//         ->CastSpellImmediate(akPotion, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
// }
//
//
// void CastIngredientMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::IngredientItem* akIngredient,
//                     RE::Actor* akTarget, float a_effectiveness) {
//     akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
//         ->CastSpellImmediate(akIngredient, false, akTarget, a_effectiveness, false, 0.0f, nullptr);
// }

void CastSpellItemMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::SpellItem* akSpell,
                       RE::EnchantmentItem* akEnchantment, RE::AlchemyItem* akPotion, RE::IngredientItem* akIngredient,
                       RE::TESObjectREFR* akTarget, float a_effectiveness, float a_override) {
    RE::MagicItem* akMagic = nullptr;
    if (akSpell) {
        akMagic = akSpell;
    } else if (akEnchantment) {
        akMagic = akEnchantment;
    } else if (akPotion) {
        akMagic = akPotion;
    } else if (akIngredient) {
        akMagic = akIngredient;
    }
    auto akCastedMagic = akMagic;
    akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
        ->CastSpellImmediate(akCastedMagic, false, akTarget, a_effectiveness, false, a_override, akSource);
}

void CastSpellMult(RE::StaticFunctionTag*, RE::Actor* akSource, RE::TESForm* MagicItem, RE::TESObjectREFR* akTarget,
                   float a_effectiveness, float a_override) {
    if (MagicItem) {
        RE::MagicItem* akCastedMagic = MagicItem->As<RE::MagicItem>();

        akSource->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
            ->CastSpellImmediate(akCastedMagic, false, akTarget, a_effectiveness, false, a_override, akSource);
    }
}

bool GetEffectWasDualCast(RE::StaticFunctionTag*, RE::ActiveEffect* a_activeEffect) {
    if (a_activeEffect) {
        if (a_activeEffect->flags.any(RE::ActiveEffect::Flag::kDual) == true) {
            return true;
        }
        return false;
    } else {
        return false;
    }
}

bool GetEnchantCostOverrideFlag(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant) {
    if (a_enchant) {
        if (a_enchant->data.flags.any(RE::EnchantmentItem::EnchantmentFlag::kCostOverride) == true) {
            return true;
        }
        return false;
    } else {
        return false;
    }
}

void SetEnchantCostOverrideFlag(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant, bool set) {
    if (a_enchant) {
        if (set == 1) {
            (a_enchant->data.flags.set(RE::EnchantmentItem::EnchantmentFlag::kCostOverride));

        } else if (set == 0) {
            (a_enchant->data.flags.reset(RE::EnchantmentItem::EnchantmentFlag::kCostOverride));
        }
    }
}

int GetEnchantCostOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant) {
    if (a_enchant) {
        return a_enchant->data.costOverride;
    } else {
        return 11111;
    }
}

int GetEnchantChargeOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant) {
    if (a_enchant) {
        return a_enchant->data.chargeOverride;
    } else {
        return 11111;
    }
}

void setEnchantCostOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant, int CostOverride) {
    if (a_enchant) {
        a_enchant->data.costOverride = CostOverride;
    }
}

void setEnchantChargeOverrideValue(RE::StaticFunctionTag*, RE::EnchantmentItem* a_enchant, int ChargeOverride) {
    if (a_enchant) {
        a_enchant->data.chargeOverride = ChargeOverride;
    }
}

void AdjustActiveEffectMagnitude(RE::StaticFunctionTag*, RE::ActiveEffect* a_activeEffect, float a_power) {
    if (a_activeEffect) {
        float magnitude = a_activeEffect->magnitude;
        a_activeEffect->magnitude = magnitude * a_power;
    }
}

void AdjustActiveEffectDuration(RE::StaticFunctionTag*, RE::ActiveEffect* a_activeEffect, float a_duration) {
    if (a_activeEffect) {
        float duration = a_activeEffect->duration;
        a_activeEffect->magnitude = duration * a_duration;
    }
}

bool PapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    vm->RegisterFunction("CastSpellItemMult", "ASR_PapyrusFunctions", CastSpellItemMult);
    vm->RegisterFunction("CastSpellMult", "ASR_PapyrusFunctions", CastSpellMult);
    vm->RegisterFunction("GetEffectWasDualCast", "ASR_PapyrusFunctions", GetEffectWasDualCast);
    vm->RegisterFunction("AdjustActiveEffectMagnitude", "ASR_PapyrusFunctions", AdjustActiveEffectMagnitude);
    vm->RegisterFunction("AdjustActiveEffectDuration", "ASR_PapyrusFunctions", AdjustActiveEffectDuration);
    vm->RegisterFunction("GetEnchantCostOverrideFlag", "ASR_PapyrusFunctions", GetEnchantCostOverrideFlag);
    vm->RegisterFunction("SetEnchantCostOverrideFlag", "ASR_PapyrusFunctions", SetEnchantCostOverrideFlag);
    vm->RegisterFunction("GetEnchantCostOverrideValue", "ASR_PapyrusFunctions", GetEnchantCostOverrideValue);
    vm->RegisterFunction("GetEnchantChargeOverrideValue", "ASR_PapyrusFunctions", GetEnchantChargeOverrideValue);
    vm->RegisterFunction("setEnchantCostOverrideValue", "ASR_PapyrusFunctions", setEnchantCostOverrideValue);
    vm->RegisterFunction("setEnchantChargeOverrideValue", "ASR_PapyrusFunctions", setEnchantChargeOverrideValue);
    return true;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded)
            RE::ConsoleLog::GetSingleton()->Print("Hello, world!");
    });
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);
    Hooks::Install();
    return true;
}