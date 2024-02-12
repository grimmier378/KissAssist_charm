| -------------------------------------------------------------------------------------
| SUB: CombatPet
| -------------------------------------------------------------------------------------
    Sub CombatPet
        /declare i_TargetID int local 0
        DEBUGCOMBAT CombatPet: Enter - ${Me.Pet.ID} - ${Pet.Combat} - ${DPSPaused} - ${PetAttackRange}
        /if (!${Me.Pet.ID} || ${Pet.Combat} || ${DPSPaused} || !${PetCombatOn}) /return
        /if (${DPSMeter}) {
            /if (!${DPSAdv.MyStatus}) /DPSAdv mystart
        }
        /if (${Target.ID}) /varset i_TargetID ${Target.ID}
        /call CombatTargetCheck 1
        /if (!${MyTargetID}) /return
        /if (${Pulling}) {
            /if (${Me.AltAbility[Summon Companion]}) {
                /if (${Me.AltAbilityReady[Summon Companion]}) {
                    /if (${Me.Pet.Distance}>79.00) {
                        /echo Pet! Get over here!
                        /call CastAA "Summon Companion" ${Me.Pet.ID} CombatPet
                    }
                }
            }
        }
        /if (${i_TargetID}!=${Target.ID}) {
            /delay 20 ${Target.BuffsPopulated}
        }
        /if (${Target.Mezzed.ID}) {
            /if (${Select[${Role},pettank,pullerpettank,hunterpettank]} && ${PetBreakMezSpell.NotEqual[null]}) /call BreakMez
        }
        /if (!${Target.Mezzed.ID}) {
            /if (${Select[${Role},pettank,pullerpettank]}) {
                /if (${ReturnToCamp}) {
                    /if (${Me.Pet.Stance.NotEqual[FOLLOW]}) {
                        /if (${Math.Distance[${Me.Pet.Y},${Me.Pet.X}:${CampYLoc},${CampXLoc}]}>${CampRadius} || ${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}>${CampRadius}) {
                            /pet follow
                        } else /if (${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}<=${CampRadius} && ${Spawn[${MyTargetID}].Distance3D}>${PetAttackRange}) {
                            /pet follow
                        }
                    }
                    /if (${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}<=${CampRadius} && ${Spawn[${MyTargetID}].Distance3D}<${PetAttackRange}) {
                        /pet attack
                        /delay 5
                        /pet swarm
                    }
                } else {
                    /if (${Spawn[${MyTargetID}].Distance3D}<${PetAttackRange}) {
                        /pet attack
                        /delay 5
                        /pet swarm
                    } else {
                        /if (${Me.Pet.Stance.NotEqual[FOLLOW]}) /pet follow
                    }
                }
            } else {
                /if (${Spawn[${MyTargetID}].Distance3D}<${PetAttackRange}) {
                    /pet attack
                    /delay 5
                    /pet swarm
                } else {
                    /if (${Me.Pet.Stance.NotEqual[FOLLOW]}) /pet follow
                }
            }
            | Set timer to check if pet is attacking every 3s
            /varset PetAttack 30
        }
        DEBUGCOMBAT CombatPet: Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: PetStateCheck
| -------------------------------------------------------------------------------------
    Sub PetStateCheck
        DEBUGPET Checking Pet State.
	/if (${PetSuspend}!=0) {
        /if (${Me.AltAbility[Companion's Suspension]}>0) {
            /while (!${Me.AltAbilityReady[Companion's Suspension]}) {
                /echo Waiting on Suspend Minion AA to be ready.
                /delay 10
            }
            /alt act ${Me.AltAbility[Companion's Suspension].ID}
            /delay 5s !${Window[CastingWindow].Open}
            /doevents
        } else {
            /echo You do not have the "Companion's Suspension" AA, PetSuspend being turned off.
            /varset PetSuspend 0
        }
	}
        /if (${Me.Pet.ID}) /varset PetActiveState 1
		/if (${Me.Pet.ID} && ${CharmKeep}) /varset CharmPet ${Me.Pet.ID}
    /return
| -------------------------------------------------------------------------------------
| SUB: DoPetStuff
| -------------------------------------------------------------------------------------	
 Sub DoPetStuff
        /if ((${CampZone}!=${Zone.ID}) || (!${PetOn} && !${CharmOn})) /return
        /if (${AggroTargetID} || ${Me.Invis} || ${Me.Hovering}) /return
        /while (1) {
            /varset EventFlag 0
            /doevents
            /if (!${EventFlag}) /break
        }
        /declare PetSummonTimer timer local 20s
        DEBUGN DoPetStuff Enter
        /declare FocusPet string local ${PetFocus.Arg[1,|]}
        /declare FocusSlot string local ${PetFocus.Arg[2,|]}
        /declare FocusBuff string local ${PetFocus.Arg[3,|]}
        /declare FocusCurrent string local
        /declare FocusSwitch int local 0
        /if (!${FocusSlot.Length} || ${FocusSlot.Equal[null]}) {
            /varset FocusPet
            /varset FocusSlot
            /varset FocusCurrent
        } else /if (${FocusSlot.NotEqual[buff]}) {
            /varset FocusCurrent ${Me.Inventory[${FocusSlot}].Name}
        } else {
            /varset FocusCurrent ${FocusBuff}
        }

        /if (${Me.Pet.CleanName.Equal[${Me.Name}`s familiar]}) /pet get lost
        | Swap pet focus in pet cast
        DEBUGN DoPetStuff ${FocusPet} ${FocusSlot} ${FocusCurrent} ${Me.Pet.ID}
		/if (${CharmOn} && ${CharmPet} && !${Me.Pet.ID}) /call DoCharmStuff DoPetStuff
				
		/if (!${Me.Pet.ID} && ${Spell[${PetSpell}].Mana}<=${Me.CurrentMana} && !${CharmOn}) {
            DEBUGN DoPetStuff ${CharmOn} ${Spell[${PetSpell}].Mana}<=${Me.CurrentMana} ${FocusCurrent} ${Me.Pet.ID}
			| verbose

            /echo I have no pet. ${Me.Class}s live longer when we have pets.

            /varset PetActiveState 0
            /varset g_PetToysGave
            | it Gets changed to 2 when activated, so as not to keep turning it on.
            /if (${PetFocusOn}) /varset PetFocusOn 1
            /if (${PetHoldOn}) /varset PetHoldOn 1
            /if (${FocusPet.Length} && ${FindItemCount[=${FocusPet}]}) {
                /if (${FocusSlot.NotEqual[buff]}) {
                    /if (!${Cursor.ID} && ${FocusPet.NotEqual[${FocusCurrent}]}) {
                        /exchange "${FocusPet}" ${FocusSlot}
                        /varset FocusSwitch 1
                        /delay 10
                    }
                } else {
                    /if (${FocusBuff.Length} && ${FocusBuff.NotEqual[null]}) {
                        /if (!${Me.Buff[${FocusBuff}].ID} && !${Me.Song[${FocusBuff}].ID}) {
                            /call CastWhat "${FocusPet}" ${Me.ID} DoPetStuff 0 0
                        }
                    }
                }
            }
            /while (1) {
                /varset EventFlag 0
                /doevents
                /if (!${EventFlag}) /break
            }
            /delay 3
            | Pet spell mem check - Else try item like mage epic
            /call CastWhat "${PetSpell}" ${Me.ID} DoPetStuff 0 0
            /if (${Macro.Return.Equal[CAST_COMPONENTS]}) {
                /echo You are missing components to make this pet.
                /return CAST_COMPONENTS
            }
            | Summon Pet
            |- If using suspended pet function.
            /if (${PetSuspend}) {
               |- If I have a suspended pet bust him out!
                /if ((${PetTotCount}==1) && (${PetActiveState}==0) && (${PetSuspendState}==1)) {
                   /echo I have a suspended pet, summoning it now!
                   /call PetStateCheck
                }
                |- If I don't have a suspended or an active pet, make them.
                /if (${PetTotCount}<2 && ${PetSuspendState}==0 && ${PetActiveState}==0 && ${Me.Book[${PetSpell}]} && ${Spell[${PetSpell}].Mana}<=${Me.CurrentMana}) {
                    /while (1) {
                        /echo - Making pet now.
                        /echo ARISE ${PetSpell}
                        /call CastWhat "${PetSpell}" ${Me.ID} DoPetStuff 0 0
                        /if (${Macro.Return.Equal[CAST_COMPONENTS]}) {
                            /echo You are missing components to make this pet. Ending macro.
                            /endmacro
                        }
                        /delay 1s ${Me.Pet.ID}
                        /if (!${PetSummonTimer}) /break
                        /if (!${Me.Pet.ID}) /continue
                        /if (${Me.Pet.ID}) {
                            /echo My pet is now: ${Me.Pet.CleanName} from ${PetSpell}
                            /varset PetActiveState 1
                            /call CheckPetBuffs
                            /if (${PetToysOn}) {
                                /call PetToys ${Me.Pet.CleanName}
                                /if ((${PetSuspendState}==0) && (${PetActiveState}==1)) /varset PetToysDone 0
                                /if ((${PetSuspendState}==1) && (${PetActiveState}==1)) /varset PetToysDone 1
                            }
                        }
                        /call PetStateCheck
                        /if (${PetTotCount}==2 || !${PetSuspend} || !${PetSummonTimer}) /break
                    }
                }
            } else /if (${Me.Book[${PetSpell}]} && ${Spell[${PetSpell}].Mana}<=${Me.CurrentMana}) {
                | Normal pet function for not using suspend pet.
                /while (1) {
                    /echo ARISE ${PetSpell}
                    /call CastWhat "${PetSpell}" ${Me.ID} DoPetStuff 0 0
                    /if (${Macro.Return.Equal[CAST_COMPONENTS]}) {
                        /echo You are missing components to make this pet. Ending macro.
                        /endmacro
                    }
                    /delay 1s ${Me.Pet.ID}
                    /if (!${PetSummonTimer} || ${Me.Pet.ID}) /break
                }
                /if (${Me.Pet.ID}) {
                    /echo My pet is now: ${Me.Pet.CleanName} from ${PetSpell}
                    /varset PetActiveState 1
                }
                | Swap pet focus out for regular item
                /if (${FocusSwitch} && !${Cursor.ID}) {
                    /exchange "${FocusCurrent}" ${FocusSlot}
                    /delay 10
                }
            }
            | Reset PetToysGave in config after pet creation, as well as memory variable so we will pickup if our pet needs toys after death, under new condition checks.
            /ini "${IniFileName}" "Pet" "PetToysGave" ""
            /if (${Me.Pet.ID} && ${PetToysOn}) {
                /varset PetToysGave NULL
            }
            /if (${Select[${Role},puller,pullertank,pettank,pullerpettank]}) {
                /if (!${ChaseAssist}) {
                    /if (${Me.Pet.Distance}<=${CampRadius}) {
                        /if (${Me.Pet.Stance.NotEqual[guard]}) /pet guard
                    } else {
                        /if (${Me.Pet.Stance.NotEqual[follow]}) /pet follow
                    }
                } else {
                    /if (${Me.Pet.Stance.NotEqual[follow]}) /pet follow
                }
            }
            /if (${PetHoldOn}==1) {
                /pet ${PetHold} on
                /varset PetHoldOn 2
            }
            /if (${PetFocusOn}==1) {
                /pet focus on
                /varset PetFocusOn 2
            }

		}
		/if (${Me.Pet.ID}) {
            /if (${Bool[${Me.Pet.Target.Name.Equal[${MainAssist}]}]}) /pet back off
            /if (${Select[${Role},puller,pullertank,pettank,pullerpettank]}) {
                /if (!${ChaseAssist}) {
                    /if (${Me.Pet.Distance}<=${CampRadius}) {
                        /if (${Me.Pet.Stance.NotEqual[guard]}) /pet guard
                    } else {
                        /if (${Me.Pet.Stance.NotEqual[follow]}) /pet follow
                    }
                } else {
                    /if (${Me.Pet.Stance.NotEqual[follow]}) /pet follow
                }
            }
            /if (${PetHoldOn}==1) {
                /pet ${PetHold} on
                /varset PetHoldOn 2
            }
            /if (${PetFocusOn}==1) {
                /pet focus on
                /varset PetFocusOn 2
            }
        }

        /if (${Me.Pet.ID}) {
            /if (!${PetTauntOverride}) {
                /if (!${Pet.Taunt}) {
                    /if (${Select[${Role},pettank,pullerpettank,hunterpettank]}) /pet taunt on
                } else {
                    /if (!${Select[${Role},pettank,pullerpettank,hunterpettank]}) /pet taunt off
                }
            }
        }			
        /call CheckPetBuffs
        | Pet Toy routines
        | Condition revised to only call PetToys if I do not have my pet name in the PetToysGave string.
        /if (!${CharmOn} && (${Me.Pet.ID} && ${PetToysOn} && (!${PetToysGave.Find[${Me.Pet.CleanName}]} || !${PetToysGave.Find[Summoned]}))) {
            /call PetToys ${Me.Pet.CleanName}
            /varset g_PetToysGave ${PetToysGave}
        }
        | Check to have pet come to owner if owner out of camp too long.
        /if (${Select[${Role},pettank,hunterpettank]} && ${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}>${CampRadius} && (${Math.Distance[${Me.Pet.Y},${Me.Pet.X}:${CampYLoc},${CampXLoc}]}<=${CampRadius}) && ${Me.Pet.Stance.Equal[GUARD]}) /pet follow
        /if (${Me.Pet.ID} && ${Me.Gem[${MiscGem}].Name.NotEqual[${ReMemMiscSpell}]} && ${MiscGemRemem}) {
            /varset DontMoveMe 1
            /call CastMemSpell "${ReMemMiscSpell}" ${MiscGem} 0 DoPetStuff
            /varset DontMoveMe 0
        }
        DEBUGN DoPetStuff Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: CheckPetBuffs
| -------------------------------------------------------------------------------------
    Sub CheckPetBuffs
        /if (!${Me.Pet.ID}) /return
        /if (!${PetOn}) /return
        /if (!${PetBuffsOn}) /return
        /if (${CombatStart}) /return
        /if (${Pulling}) /return
        /if (${PetBuffCheck}) /return
        /if (${Me.Invis}) /return
        DEBUGN CheckPetBuffs Enter
        /declare PTempBuff string local
        /declare PTempSpell string local
        /declare i int local
        /declare j int local
        /declare 1stPart string local 0
        /declare 2ndPart string local 0
        /declare 3rdPart string local 0
        /declare FoundPetBuff int local 0
        /declare CondNo int local 0
        /varset PetBuffCheck 60s
        /for i 1 to ${PetBuffs.Size}
            /doevents
            | Stop pet buffing if aggro detected
            /if (${AggroTargetID}) /return
            /if (${PetBuffs[${i}].Equal[Null]}) /continue
            /varset 1stPart ${PetBuffs[${i}].Arg[1,|]}
            /varset 2ndPart ${PetBuffs[${i}].Arg[2,|]}
            /varset 3rdPart ${PetBuffs[${i}].Arg[3,|]}

            /if (${2ndPart.NotEqual[dual]}) /varset 3rdPart ${1stPart}

            /if (${ConOn} && ${PetBuffs[${i}].Find[|cond]}) {
                /varset CondNo ${PetBuffs[${i}].Mid[${Math.Calc[${PetBuffs[${i}].Find[|cond]}+5]},3]}
            } else {
                /varset CondNo 0
            }

            | Check for pet spells in book to prevent double casting of items
            /if (${Me.Book[${1stPart}]} || ${Me.AltAbility[${1stPart}]}) {
                /if (${1stPart.Find[ Rk.]}) {
                    /varset PTempSpell ${1stPart.Left[${Math.Calc[${1stPart.Find[ Rk.]}-1]}]}
                } else {
                    /varset PTempSpell ${1stPart}
                }
                /if (${3rdPart.Find[ Rk.]}) {
                    /varset PTempBuff ${3rdPart.Left[${Math.Calc[${3rdPart.Find[ Rk.]}-1]}]}
                } else {
                    /varset PTempBuff ${3rdPart}
                }
                DEBUGN CheckPetBuffs ${i} ${Me.PetBuff[${PTempBuff}]} ${PetBuffs[${i}]} ${PTempBuff}
                /for j 1 to 50
                    /if (${Me.PetBuff[${j}].Name.Find[${PTempBuff}]}) {
                        /varset FoundPetBuff 1
                        /break
                    }
                /next j
                /if (!${FoundPetBuff}) {
                    | Vebrose
                    /call CastWhat "${1stPart}" ${Me.Pet.ID} Pet-nomem ${CondNo} 0
                    /delay 20
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /echo Buffing ${Me.Pet.CleanName}, my pet, with ${1stPart}
                    } else /if (${Macro.Return.Equal[CAST_COMPONENTS]}) {
                        /echo You are missing components. Turning off ${1stPart}.
                        /varset PetBuffs[${i}] Null

                    }
                    /continue
                }
            } else /if (${FindItem[=${1stPart}].ID}) {
                /varset PTempSpell ${1stPart}
                /varset PTempBuff ${3rdPart}
                DEBUGN CheckPetBuffs ${FindItem[=${1stPart}].Spell} ${Me.PetBuff[${FindItem[=${1stPart}].Spell}]} ${Me.PetBuff[${3rdPart}]}
                /if (${Me.PetBuff[${FindItem[=${PTempSpell}].Spell}]}>0) /continue
                /if (${Me.PetBuff[${PTempBuff}]}>0) /continue
                DEBUGN CheckPetBuffs - Dual - ${1stPart} - ${3rdPart}
                /for j 1 to 50
                    /if (${Me.PetBuff[${j}].Name.Find[${PTempBuff}]}) {
                        /varset FoundPetBuff 1
                        /break
                    }
                /next j
                /if (!${FoundPetBuff}) {
                    |Vebrose
                    /call CastWhat "${1stPart}" ${Me.Pet.ID} Pet ${CondNo} 0
                    /delay 20
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /echo Buffing ${Me.Pet.CleanName}, my pet, with (${3rdPart})
                    } else /if (${Macro.Return.Equal[CAST_COMPONENTS]}) {
                        /echo You are missing components. Turning off ${1stPart}.
                        /varset PetBuffs[${i}] Null
                    }
                    /continue
                }
            } else /if (${1stPart.Equal[pettoys]} && ${2ndPart.Equal[begfor]}) {
                DEBUGN CheckPetBuffs begfor toys
                /if (${CondNo} && ${If[${Cond[${CondNo}]},0,1]}) /continue
                /if (${Defined[PetBuff${i}]}) {
                    /if (${PetBuff${i}}) /continue
                }
                /call BroadCast o "PetToysPlease ${Me.Pet.Name}"
                /if (!${Defined[PetBuff${i}]}) {
                    /declare PetBuff${i} timer outer 900
                } else {
                    /varset PetBuff${i} 900
                }
            }
            /varset FoundPetBuff 0
        /next i
        DEBUGN CheckPetBuffs Pet Height: ${Me.Pet.Height}
        /if (${Me.Pet.Height}>1.35 && ${PetShrinkOn}) {
            /call CastWhat "${PetShrinkSpell}" ${Me.Pet.ID} Pet 0 0
            /delay 20
        }
        /if (${Target.ID}==${Me.Pet.ID}) /squelch /target clear
        DEBUGN CheckPetBuffs Leave
    /return
| ----------------------------------------------------------------------------
| SUB: CastPetToys
| ----------------------------------------------------------------------------
    Sub CastPetToys(string PetToySpell0)
        /declare retryCount int local 1
        /declare cancelFlag int local 0
        /while (1) {
            /varcalc retryCount ${retryCount}+1
            /call CastWhat "${PetToySpell0}" ${Me.ID} Pet-nomem 0 0
            /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                /echo Casting pet toy spell >> ${PetToySpell0} <<
                /break
            } else /if (${Macro.Return.Equal[CAST_FIZZLE]}) {
                /if (${retryCount}>4) {
                    /varset cancelFlag 1
                    /break
                }
                /if (!${Me.GemTimer[${PetToySpell0}]} && ${Me.SpellReady[${PetToySpell0}]}) {
                    /delay 5
                    /continue
                } else {
                    /while (${Me.GemTimer[${PetToySpell0}]}) {
                        /delay 5
                    }
                    /continue
                }
            } else /if (${Macro.Return.Equal[CAST_RECOVER]}) {
                /varcalc retryCount ${retryCount}-1
                /while (1) {
                    /delay 5
                    /if (${Me.SpellInCooldown}) /continue
                    /if (${Me.GemTimer[${PetToySpell0}]}) /continue
                    /if (!${Me.SpellReady[${PetToySpell0}]}) /continue
                    /break
                }
                /continue
            }
            /varset cancelFlag 1
            /break
        }
    /return ${cancelFlag}
| ----------------------------------------------------------------------------
| SUB: Check PetToys
| ----------------------------------------------------------------------------
    Sub PetToys(string petName)
        /if (!${Me.Pet.ID}) /return w
        DEBUGPET PetToys Enter
        /declare i            int    local
        /declare j            int    local
        /declare GrabItem     int    local 0
        /declare PetToySpell  string local 0
        /declare FullText     string local
        /declare 2ndPart      string local
        /declare LastPart     string local
        /declare PetToysTemp  string local ${Ini[${IniFileName},Pet,PetToysGave]}
        /declare PetToyCheck  string local
        /declare GaveItem     int    local 1
        /declare CondNo       int    local 0
        /declare pCount1      int    local 0
        /declare pCount2      int    local 0
        /declare pIdx         int    local 0
        /declare isMyPet      int    local 0
        /declare retryCount   int    local 0
        /declare cancelFlag   int    local 0
        /declare SlotOpen     int    local 0
        /declare CastFlag1    int    local 0

        /varset BagNum 0
        /call OpenInvSlot
        /declare petID        int    local ${Spawn[pet ${petName}].ID}
        /declare petLevel     int    local ${Spawn[id ${petID}].Level}
        /if (!${Defined[PetToysOn]}) /return
        /if (!${PetToysOn}) /return 0
        /if (${BagNumLast}==99) {
            /echo Inventory is full
            /varset PetToysOn 0
            /return X
        }
        /if (${BagNum}==0) {
            /echo You must have an empty Top Inventory slot for Pet Toys to work.
            /varset PetToysOn 0
            /return X
        }
        | reset if different pet name only on my pet
        /if (${Me.Pet.CleanName.Equal[${petName}]}) {
            /varset isMyPet 1
            /if (!${PetToysTemp.Find[${petName}]}) {
                /ini "${IniFileName}" "Pet" "PetToysGave" "${petName}"
                /varset PetToysTemp ${petName}
            }
        } else {
            /varset PetToysTemp
        }
        /if (!${Window[InventoryWindow].Open}) /varset GaveItem 0
        /for i 1 to ${PetToys.Size}
            DEBUGPET CHECKING: ${i} - ${PetToys[${i}]}
            /if (${ConOn} && ${PetToys[${i}].Find[|cond]}) {
                /varset CondNo ${PetToys[${i}].Mid[${Math.Calc[${PetToys[${i}].Find[|cond]}+5]},3]}
            } else {
                /varset CondNo 0
            }
            /if (${DebugPet}) {
                /echo \atPetDebug PetToys 1: ${PetToys[${i}]}
                /echo \atPetDebug PetToys 2: ${PetToysTemp}
                /echo \atPetDebug PetToys 3: ${PetToys[${i}].Equal[Null]} || ${PetToysTemp.Find[${petName}]} && ${PetToysTemp.Find[${PetToys[${i}]}]}
                /echo \atPetDebug PetToys 4: ${If[${CondNo},${Cond[${CondNo}]},0]}
            }
            /if (${CondNo}) {
                /if (${If[${Cond[${CondNo}]},0,1]}) /continue
            }
            | Had to add pet name check else it will always skip giving weapons to OTHER pets.
            DEBUGPET PetToys ${i} ${PetToys[${i}].Equal[Null]} SkipPetToy 1
            /if (${PetToys[${i}].Equal[Null]}) /continue
            /if (${PetToys[${i}].Find[|cond]}) {
                /varset FullText ${PetToys[${i}].Left[${Math.Calc[${PetToys[${i}].Find[|cond]}-1]}]}
            } else {
                /varset FullText ${PetToys[${i}]}
            }
            | Are we handing weapons to the pet
            /varcalc pCount1 ${FullText.Count[|]}+1
            /varset PetToySpell ${FullText.Arg[1,|]}
            /if (${pCount1}>1) {
                /varset pIdx 2
                /varset 2ndPart ${FullText.Arg[${pIdx},|]}
                /varset LastPart ${FullText.Arg[${Math.Calc[${pIdx}+1]},|]}
                /if (${2ndPart.Length} && ${2ndPart.NotEqual[null]} && ${LastPart.Equal[${2ndPart}]}) {
                    /varset PetToyCheck :${2ndPart}2
                } else {
                    /varset PetToyCheck :${2ndPart}1
                }
            } else {
                /varset pIdx 1
                /varset PetToySpell ${FullText}
                /varset 2ndPart
                /varset PetToyCheck ${PetToySpell}:
            }
            /varset LastPart
            DEBUGPET PetToys ${isMyPet} - ${2ndPart.Length} - ${PetToysTemp.Find[PetToyCheck]} - ${Spell[${PetToySpell}].Level} SkipPetToy 2
            /if (${isMyPet}) {
                /if (${PetToysTemp.Find[${petName}]} && ${PetToysTemp.Find[${PetToyCheck}]}) /continue
                | Check if spell level less than 76 because mage pets auto equipped after that.
                /if (${petLevel}>=76 && (${PetToySpell.Find[muzzle]} || ${PetToySpell.Find[visor]} || ${PetToySpell.Find[belt]} || ${PetToySpell.Find[plate]})) /continue
            }
            | Check for pet toy spells in book to prevent double casting of items
            /if (${Me.Book[${PetToySpell}]} || (${PetToySpell.Equal[inventory]} && ${FindItemCount[=${2ndPart}]})) {
                | Cast to get Item
                /if (${Me.Book[${PetToySpell}]}) {
                    /call CastPetToys "${PetToySpell}"
                    /if (${Macro.Return.Equal[1]}) /continue
                    /delay 15s ${Cursor.ID}
                    /if (!${Cursor.ID}) /return 0
                }
                | If it is a Bag we need to drop it into inventory
                /if (${Cursor.Container} || ${Cursor.Name.Find[Folded]}) {
                    | If item is in Inv Slot exchange it with bag on cursor
                    /varset CastFlag1 0
                    /if (${Me.Inventory[pack${BagNum}].ID}) {
                        /varset SlotOpen 0
                    } else {
                        /varset SlotOpen 1
                    }
                    /nomodkey /itemnotify pack${BagNum} leftmouseup
                    /while (1) {
                        /if (!${SlotOpen}) {
                            /delay 30 ${Cursor.ID}
                        } else {
                            /delay 20 !${Cursor.ID}
                        }
                        | Drop bag or item into inventory
                        /if (${Cursor.ID}) {
                            /if (!${Cursor.Container}) {
                                /autoinventory
                            } else {
                                /nomodkey /itemnotify pack${BagNum} leftmouseup
                            }
                        }
                        /delay 20 !${Cursor.ID}
                        | Drop exchanged item into inventory after bag is dropped
                        /if (${Cursor.ID}) /autoinventory
                        | If folded pack right click to convert to phantom satchel
                        /if (${Me.Inventory[pack${BagNum}].Name.Find[folded]}) {
                            /nomodkey /itemnotify pack${BagNum} rightmouseup
                            /echo Opening ${Me.Inventory[pack${BagNum}].Name}
                            /delay 10
                            /delay 30 ${Window[CastingWindow].Open}
                            /varset SlotOpen 0
                        } else {
                            /delay 10
                            /break
                        }
                    }
                    /if (${Me.Inventory[pack${BagNum}].Container}) {
                        /nomodkey /itemnotify pack${BagNum} rightmouseup
                        /delay 10
                    }
                } else /if (${Cursor.Name.Find[Summoned:]}) {
                    /varset CastFlag1 1
                } else {
                    /varset CastFlag1 0
                }

                /while (1) {
                    /if (${FindItemCount[=${2ndPart}]}) {
                        /call GiveTo "${2ndPart}" ${petID} 0
                    } else /if (${Cursor.ID}) {
                        /varset 2ndPart ${Cursor.Name}
                        /call GiveTo "${Cursor.Name}" ${petID} 1
                    } else /if (${pIdx}==1 && !${2ndPart.Length}) {
                        /varset CastFlag1 2
                        /break
                    } else {
                        /break
                    }
                    /delay 10
                    /if (${PetToysTemp.Find[:${2ndPart}1]}) {
                        /varset PetToysTemp ${PetToysTemp}|${PetToySpell}:${2ndPart}2
                    } else {
                        /varset PetToysTemp ${PetToysTemp}|${PetToySpell}:${2ndPart}1
                    }
                    /if (${isMyPet}) /ini "${IniFileName}" "Pet" "PetToysGave" "${PetToysTemp}"

                    /while (1) {
                        /varset LastPart ${2ndPart}
                        /if (${pIdx}==${pCount1}) {
                            /varset 2ndPart
                            /break
                        }
                        /varcalc pIdx ${pIdx}+1
                        /varset 2ndPart ${FullText.Arg[${pIdx},|]}

                        /if (${2ndPart.Length}) {
                            /if (${2ndPart.NotEqual[null]} && ${2ndPart.Equal[${LastPart}]}) {
                                /varset PetToyCheck :${2ndPart}2
                            } else {
                                /varset PetToyCheck :${2ndPart}1
                            }
                        } else {
                            /break
                        }

                        /if (${isMyPet}) {
                            /if (${PetToysTemp.Find[${petName}]} && ${PetToysTemp.Find[${PetToyCheck}]}) /continue
                            /if (${petLevel}>=76 && (${PetToySpell.Find[muzzle]} || ${PetToySpell.Find[visor]} || ${PetToySpell.Find[belt]} || ${PetToySpell.Find[plate]})) /continue
                        }
                        /break
                    }
                    /if (!${2ndPart.Length} || ${2ndPart.Equal[null]}) /break
                    /if (${CastFlag1}==1) {
                        /if (${Window[GiveWnd].Open}) /call GiveTo "giveitems" ${petID} 1
                        /call CastPetToys "${PetToySpell}"
                        /if (${Macro.Return.Equal[1]}) /break
                        /delay 15s ${Cursor.ID}
                    }
                }
                /if (${CastFlag1}==2) {
                    /varset 2ndPart
                    /for j 1 to ${Me.Inventory[pack${BagNum}].Container}
                        /if (${PetToySpell.Find[heirloom]} && ${isMyPet} && ${j}<4) /continue
                        /if (${Me.Inventory[pack${BagNum}].Item[${j}].ID} && ${Me.Inventory[pack${BagNum}].Item[${j}].Name.Length}) {
                            /varset 2ndPart ${Me.Inventory[pack${BagNum}].Item[${j}].Name}
                            /call GiveTo "${2ndPart}" ${petID} 0
                        }
                        /delay 10
                    /next j
                    /if (${2ndPart.Length} && !${PetToysTemp.Find[${PetToySpell}]}) {
                        /varset PetToysTemp ${PetToysTemp}|${PetToySpell}:AllContents
                        /if (${isMyPet}) /ini "${IniFileName}" "Pet" "PetToysGave" "${PetToysTemp}"
                    }
                }
                /if (${Window[GiveWnd].Open}) /call GiveTo "giveitems" ${petID} 1
                /if (${Me.Inventory[pack${BagNum}].Name.Find[Phantom Satchel]} || ${Me.Inventory[pack${BagNum}].Name.Find[Pouch of Quellious]}) /call DestroyBag
            }
        /next i
        | MUST reset PetToysGave after cycle complete in order to stop calling PetToys all the time.
        /varset PetToysGave ${PetToysTemp}
        /if (${Window[InventoryWindow].Open} && !${GaveItem}) /keypress inventory
        /varset PetToysDone 1
        /call DoWeMove ${ReturnToCamp} PetToys
        DEBUGPET PetToys Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: DoPetHealStuff
| -------------------------------------------------------------------------------------
    Sub DoPetHealStuff
        | Check if my pet needs healing
        /if (!${Me.Pet.ID}) /return
        DEBUGHEALS DoPetHealStuff Enter
        /declare j int local
        /for j 1 to ${SingleHeal.Size}
            /if (${SingleHeal[${j}].Arg[3,|].NotEqual[pet]} || !${SingleHeal[${j}].Length} || ${SingleHeal[${j}].Arg[2,|]}==0) /continue
            /if (${Me.Pet.PctHPs}<=${SingleHeal[${j}].Arg[2,|]} && ${Me.Pet.ID} && ${Me.Pet.Distance}<${Spell[${SingleHeal[${j}].Arg[1,|]}].Range}) {
                DEBUGHEALS DoPetHealStuff (${Me.Pet.PctHPs}<=${SingleHeal[${j}].Arg[2,|]} && ${Me.Pet.ID} && ${Me.Pet.Distance}<${Spell[${SingleHeal[${j}].Arg[1,|]}].Range})
                /call CastWhat "${SingleHeal[${j}].Arg[1,|]}" ${Me.Pet.ID} Heal 0 0
                /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                    /call BroadCast o "${SingleHeal[${j}].Arg[1,|]} on  >> ${Me.Pet.CleanName} <<"
                    /varcalc PetHealTimer${j} ${Spell[${SingleHeal[${j}].Arg[1,|]}].MyDuration.TotalSeconds}*10
                    /varset HealAgain    1
                }
            }
        /next j
        DEBUGHEALS DoPetHealStuff Leave
    /return
|--------------------------------------------------------------------------------
|SUB: CheckRampPets - Writtten by Trehuggindruid for KissAssist
|--------------------------------------------------------------------------------
    Sub CheckRampPets
        /declare i int local
        /declare DoLoop int local 1
        /if (!${Me.CombatState.Equal[COMBAT]}) {
            | Wait for rampage pets to poof before pulling.
            /for i 0 to 20
                /if (${Spawn[${Me.CleanName}`s_pet0${i}].ID}) /echo +++ My rampage pet is up: (${Spawn[${Me.CleanName}`s_pet0${i}]}|${Spawn[${Me.CleanName}`s_pet0${i}].ID}), HOLDING . . .
                /while (${DoLoop}) {
                    /delay 1
                    /if (!${Me.CombatState.Equal[COMBAT]} && ${Spawn[${Me.CleanName}`s_pet0${i}].ID}) /continue
                    /varset DoLoop 0
                }
            /next i
        }
    /return
|-----------------------------------------------------------------------------
| SUB: PetSusStateAdd1
| ----------------------------------------------------------------------------
    Sub Event_PetSusStateAdd1
        /varset EventFlag 1
        /varset PetActiveState  0
        /varset PetSuspendState 1
        /varset PetTotCount     1
        DEBUGPET PetSusStateAdd1 triggered.
        DEBUGPET PetTotCount(${PetTotCount}) PetActiveState(${PetActiveState}) PetSuspendState(${PetSuspendState})
    /return
|-----------------------------------------------------------------------------
| SUB: PetSusStateAdd2
| ----------------------------------------------------------------------------
    Sub Event_PetSusStateAdd2
        /varset EventFlag 1
        /varset PetActiveState  1
        /varset PetSuspendState 1
        /varset PetTotCount     2
        DEBUGPET PetSusStateAdd2 triggered.
        DEBUGPET PetTotCount(${PetTotCount}) PetActiveState(${PetActiveState}) PetSuspendState(${PetSuspendState})
    /return
|-----------------------------------------------------------------------------
| SUB: PetSusStateSub
| ----------------------------------------------------------------------------
    Sub Event_PetSusStateSub
        /varset EventFlag 1
        /varset PetActiveState  1
        /varset PetSuspendState 0
        /varset PetTotCount     1
        DEBUGPET PetSusStateSub triggered.
        DEBUGPET PetTotCount(${PetTotCount}) PetActiveState(${PetActiveState}) PetSuspendState(${PetSuspendState})
    /return
