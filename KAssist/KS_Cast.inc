| -------------------------------------------------------------------------------------
| SUB: CastReady
| -------------------------------------------------------------------------------------
    Sub CastReady(string castWhat,string sentFrom)
        /varset SpellReadyL 0
        /if (!${Bool[${castWhat}]}) /return 0
        /if (${sentFrom.Equal[rezwithcheck]}) {
            /if (${Select[TRUE,${Me.AltAbilityReady[${castWhat}]},${Me.ItemReady[=${castWhat}]},${Me.CombatAbilityReady[${castWhat}]},${Me.AbilityReady[${castWhat}]}]}==0) {
                /if (${Me.Book[${castWhat}]}) {
                    /if (!${Me.Gem[${castWhat}]}) {
                        /varset SpellReadyL 5
                        /return ${SpellReadyL}
                    }
                }
            }
        } else /if (${sentFrom.Equal[singleheal]} || ${sentFrom.Equal[groupheal]}) {
            /if (${Me.Gem[${castWhat}]}) {
                /while (${Me.SpellInCooldown}) {
                    /delay 2
                }
            }
        }
        /varset SpellReadyL ${Select[TRUE,${Me.AltAbilityReady[${castWhat}]},${Me.ItemReady[=${castWhat}]},${Me.CombatAbilityReady[${castWhat}]},${Me.AbilityReady[${castWhat}]},${Me.SpellReady[${castWhat}]}]}
    /return ${SpellReadyL}
| -------------------------------------------------------------------------------------
| SUB: CastWhat
| -------------------------------------------------------------------------------------
    Sub CastWhat(string castWhat,int WhatID,string sentFrom,int CondNumber, int miscFlag)
        DEBUGCAST CastWhat Enter \aw ${castWhat} ID: ${WhatID} Target:${Spawn[id ${WhatID}].CleanName} SentFrom: ${sentFrom} Cond: ${CondNumber}
       | /if (!${IAmABard} && ${Me.Casting.ID} && ${Window[CastingWindow].Open}) /return CAST_CASTING
        | TODO MEDLEY ideally we would only skip casting if the castWhat has a castime
        /if (${IAmABard}) {
           | /if (${Medley.TTQE} > 0.0) /return CAST_CASTING
        }
        /declare ReadyToCast       int      local 0
        /declare MemReturn         string   local null
        /declare EventsCalcTimer   timer    local 0
        /declare str_TargetType    string   local ${Spell[${castWhat}].TargetType}
        /varset castReturn X
        /varset CastResult CAST_NO_RESULT
        /if (${castWhat.Find[command:]} || ${Me.AltAbility[${castWhat}]} || ${Me.CombatAbility[${castWhat}]} || ${FindItem[=${castWhat}].ID} || ${Me.Skill[${castWhat}]} || ${Me.Book[${castWhat}]}) {
            /if (${IAmABard}) {
                /varset ReadyToCast ${Select[TRUE,${Me.ItemReady[=${castWhat}]},${Me.AltAbilityReady[${castWhat}]},${Me.CombatAbilityReady[${castWhat}]},${If[${Me.AbilityReady[${castWhat}]} && ${Me.Skill[${castWhat}]},TRUE,FALSE]},${If[${Me.Gem[${castWhat}]} && ${Me.GemTimer[${castWhat}]}==0,TRUE,FALSE]},${Bool[${castWhat.Find[command:]}]}]}
                DEBUGCAST CastWhat \aw ${Me.ItemReady[=${castWhat}]},${Me.AltAbilityReady[${castWhat}]},${Me.CombatAbilityReady[${castWhat}]},${If[${Me.AbilityReady[${castWhat}]} && ${Me.Skill[${castWhat}]},TRUE,FALSE]},${If[${Me.Gem[${castWhat}]} && ${Me.GemTimer[${castWhat}]}==0,TRUE,FALSE]},${Bool[${castWhat.Find[command:]}]}
            } else {
                /varset ReadyToCast ${Select[TRUE,${Me.ItemReady[=${castWhat}]},${Me.AltAbilityReady[${castWhat}]},${Me.CombatAbilityReady[${castWhat}]},${If[${Me.AbilityReady[${castWhat}]} && ${Me.Skill[${castWhat}]},TRUE,FALSE]},${Me.SpellReady[${castWhat}]},${Bool[${castWhat.Find[command:]}]}]}
                DEBUGCAST CastWhat \aw ${Me.ItemReady[=${castWhat}]},${Me.AltAbilityReady[${castWhat}]},${Me.CombatAbilityReady[${castWhat}]},${If[${Me.AbilityReady[${castWhat}]} && ${Me.Skill[${castWhat}]},TRUE,FALSE]},${Me.SpellReady[${castWhat}]},${Bool[${castWhat.Find[command:]}]}
            }
            | Check for gems stuck or spell in book but not memed.
            /if (${ReadyToCast}==0 || ${Me.Casting.ID}) {
                /if (${Me.Book[${castWhat}]}) {
                    /if (!${IAmABard} && ${Me.Casting.ID} && !${Window[CastingWindow].Open}) {
                        /if (${CheckStuckGem}) /call CheckStuckGems
                        /if (${Me.Gem[${castWhat}]}) {
                            /varset ReadyToCast 5
                        } else {
                            /varset ReadyToCast 7
                        }
                    } else /if (!${FindItem[=${castWhat}].ID} && !${Me.Gem[${castWhat}]} && !${Me.AltAbility[${castWhat}]}) {
                        /varset ReadyToCast 7
                    }
                }
            }
            /if (${ReadyToCast}) {
                /if (${str_TargetType.Equal[null]}) {
                    /if (${ReadyToCast}==1) {
                        /varset str_TargetType ${FindItem[=${castWhat}].Spell.TargetType}
                    }
                }
                DEBUGCAST CastWhat \aw Target Info !${Target.ID} ${Target.ID}!=${WhatID} ${Spawn[id ${WhatID}].ID} ${ReadyToCast} ${str_TargetType}
                | Check if Castwhat has a conditon attached to it.
                /if (${CondNumber}) {
                    /if (${Cond[${CondNumber}].Find[TARGETCHECK]}) {
                        /if (${str_TargetType.NotEqual[Self]} && (!${Target.ID} || (${Target.ID}!=${WhatID} && ${Spawn[id ${WhatID}].ID}))) /call CastTarget ${WhatID}
                    }
                    /if (${If[${Cond[${CondNumber}]},0,1]}) /return CAST_COND_FAILED
                }
                /if (${str_TargetType.NotEqual[Self]} && (!${Target.ID} || (${Target.ID}!=${WhatID} && ${Spawn[id ${WhatID}].ID}))) /call CastTarget ${WhatID}
                | Check target for dps effects and stacking issues to prevent overcasting.
                /if (${sentFrom.Equal[DPS]} && ${Spell[${castWhat}].SpellType.Equal[Detrimental]}) {
                    /if (!${Me.CombatAbilityReady[${castWhat}]} && !${Me.AbilityReady[${castWhat}]} && (${Me.AltAbilityReady[${castWhat}]} || ${Me.SpellReady[${castWhat}]})) {
                        /call CastDPSSpellCheck "${castWhat}"
                        DEBUGCAST CastWhat ${Macro.Return}
                        /if (${Macro.Return}) {
                            DEBUGCAST CastWhat --${castWhat} is already on Mob. Skipping.
                            /return ${CastResult}
                        }
                    }
                | Check target for buffs and stacking issues to prevent overcasting.
                } else /if (${sentFrom.Find[Buffs]} && ${Spell[${castWhat}].SpellType.Find[Beneficial]} && ${WhatID}!=${Me.ID}) {
                    /call CastBuffsSpellCheck "${castWhat}"
                    DEBUGCAST CastWhat ${Macro.Return}
                    /if (${Macro.Return}) {
                        DEBUGCAST CastWhat --${castWhat} is already on Target. Skipping.
                        /varset CastResult CAST_HASBUFF
                        /varset HasBuffDuration ${Int[${Macro.Return}]}
                        /return ${CastResult}
                    }
                } else /if (${sentFrom.Find[pull]} && ${PullAggroTargetID}) {
                    /return
                }
                | Stop moving to Prevent cast interruption.
                /if (${Spell[${castWhat}].CastTime} && ${Me.Moving} && !${IAmABard}) /call StopMoving
                | Check Castwhat for type item/aa/disc/skill/spell/command and send to correct sub
                DEBUGCAST CastWhat 2: --${castWhat} ${ReadyToCast} ${Me.SpellReady[${castWhat}]} && ${Me.Book[${castWhat}]} ${Me.Gem[${castWhat}]} && ${Me.GemTimer[${castWhat}]}==0
                /if (${ReadyToCast}==1 && ${Me.ItemReady[=${castWhat}]} && ${FindItem[=${castWhat}].ID}) {
                    /call CastItem "${castWhat}" ${WhatID} ${sentFrom}
                } else /if (${ReadyToCast}==2 && ${Me.AltAbilityReady[${castWhat}]} && !${FindItem[=${castWhat}].ID}) {
                    /call CastAA "${castWhat}" ${WhatID} ${sentFrom}
                } else /if (${ReadyToCast}==3 && ${Me.CombatAbilityReady[${castWhat}]} && ${Spell[${castWhat}].EnduranceCost}<${Me.Endurance}) {
                    /call CastDisc "${castWhat}" ${WhatID} ${sentFrom}
                } else /if (${ReadyToCast}==4 && ${Me.AbilityReady[${castWhat}]}) {
                    /call CastSkill "${castWhat}" ${WhatID} ${sentFrom}
                } else /if (${ReadyToCast}==5) {
                    /if (${Spell[${castWhat}].Mana}<${Me.CurrentMana}) {
                        /if (${Me.SpellReady[${castWhat}]} && ${Me.Book[${castWhat}]}) {
                            /call CastSpell "${castWhat}" ${WhatID} ${sentFrom} ${miscFlag}
                        } else /if (${IAmABard} && ${Me.Gem[${castWhat}]} && !${Me.GemTimer[${castWhat}]}) {
                            /call CastSpell "${castWhat}" ${WhatID} ${sentFrom} ${miscFlag}
                        }
                        /varset MemReturn ${CastResult}
                    } else {
                        /varset CastResult CAST_NEEDMANA
                        /varset MemReturn CAST_NO_RESULT
                        |/return ${CastResult}
                    }
                } else /if (${ReadyToCast}==6 && ${castWhat.Find[command:]}) {
                    /call CastCommand "${castWhat}" ${WhatID}
                } else /if (${ReadyToCast}==7) {
                    /if (!${sentFrom.Find[combat]}) {
                        /if (${Spell[${castWhat}].Mana}<${Me.CurrentMana}) {
                            /call CastMem "${castWhat}" ${sentFrom}
                            /varset MemReturn ${Macro.Return}
                            /if (${Macro.Return.NotEqual[notready]} && ${Me.Gem[${castWhat}]}!=0) {
                                /call CastSpell "${castWhat}" ${WhatID} ${sentFrom} ${miscFlag}
                            }
                        } else {
                            /varset CastResult CAST_NEEDMANA
                            /varset MemReturn notready
                        }
                    } else {
                        /varset CastResult CAST_NO_RESULT
                        /varset MemReturn notready
                    }
                }
                /if (${sentFrom.Find[pull]} && ${PullAggroTargetID}) /return CAST_SUCCESS
                | Remem spell originally in Miscgem slot except if combat or just rezd
                | 0nly need to do this if ReadyToCast had a value other than 0.
                /if (${MiscGemRemem}) {
                    /if (${ReadyToCast}==7 && ${MemReturn.NotEqual[notready]}) {
                        /call CastReMem "${castWhat}" 0 ${sentFrom}
                    } else /if (${ReadyToCast}==5 && ${MemReturn.NotEqual[CAST_NO_RESULT]} && (${ReMemWaitLong.Equal[${castWhat}]} || ${ReMemWaitShort.Equal[${castWhat}]})) {
                        /call CastReMem "${castWhat}" 0 ${sentFrom}
                    }
                }
|--               /if (${IAmABard}) {
|--                   /if (!${Twist} && ${Twist.List.Length}) {
|--                       /if (${DPSTwisting}) {
|--                           /if (${Target.ID}==${Me.ID} || ${MyTargetID}!=${Target.ID}) {
|--                               /if (${MyTargetID}) {
|--                                   /target id ${MyTargetID}
|--                               } else {
|--                                   /target clear
|--                               }
|--                               /delay 5
|--                           }
|--                       }
|--                       /squelch /twist
|--                   }
|--               }
				/if (${IAmABard}) {
                  |  /if (!${Medley.Active} && ${MedleyName.Length}) /squelch /medley "${MedleyName}" -quiet
                }
                /varset SitToMedTimer ${SitToMedTimer.OriginalValue}
            } else {
                /varset CastResult CAST_RECOVER
            }
        }
        DEBUGCAST CastWhat \aw Leave ${CastResult}
    /return ${CastResult}
| -------------------------------------------------------------------------------------
| SUB: CastWait
|--------------------------------------------------------------------------------------
    Sub CastWait(string WaitSpell,string sentFrom)
        DEBUGCAST CastWait Enter ${WaitSpell} ${Spell[${WaitSpell}].CastTime}
        /if (${Spell[${WaitSpell}].CastTime}) {
            /if (!${Me.Casting.ID} || !${Window[CastingWindow].Open}) /delay 5
            /while (${Me.Casting.ID} && ${Window[CastingWindow].Open}) {
                /if (${sentFrom.Find[pull]} && ${PullAggroTargetID}) {
                    /stopcast
                    /varset CastResult CAST_SUCCESS
                    /break
                }
                /delay 5
            }
        } else {
            /delay 5
            /varset CastResult CAST_SUCCESS
        }
        DEBUGCAST CastWait Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: CastAA
|--------------------------------------------------------------------------------------
    Sub CastAA(string WhatAA,int WhatID,string sentFrom)
        DEBUGCAST CastAA Enter ${WhatAA} ${Me.AltAbility[${WhatAA}].Spell.CastTime}
        /if (${Select[${WhatAA},Banestrike,15073]} && !${Banestrike.Find[|${Target.Race}|]} && ${Spawn[${MyTargetID}].Distance3D}>70 && ${CombatStart}) /return
        /if (${Me.Invis} && !${Select[${sentFrom},SingleHeal,GroupHeal]}) {
            /varset CastResult CAST_CANCELLED
            /return CAST_CANCELLED
        }

        /varset IAMCastingID ${Me.AltAbility[${WhatAA}].Spell.ID}        
        /if (${IAmABard}) {
            |/squelch /stopsong
            /echo DEBUGMD casting then waiting /medley queue "${WhatAA}"
            /if (${Me.AltAbility[${WhatAA}].Spell.CastTime}>0) {
                /squelch /multiline ; /stopsong; /medley queue "${WhatAA}" -interrupt
                /echo "/squelch /medley queue ${WhatAA}"
            }
            | TODO calculate better max wait
            /delay 30 ${Medley.TTQE}==0.0
            |/echo DEBUGMD delay over
            /call DoCastingEvents 5 0
            /return ${castReturn}
        } else {
            /alt act ${Me.AltAbility[${WhatAA}].ID}|/alt act ${Me.AltAbility[${WhatAA}].ID}
        }
        DEBUGCAST Before While: ${WhatAA} ${IAMCastingID} ${Me.Casting.ID} ${Window[CastingWindow].Open}
        /if (${Me.AltAbility[${WhatAA}].Spell.CastTime}>0) /delay 10 ${Window[CastingWindow].Open}
        /while (1) {
            /call DoCastingEvents
            /if (${sentFrom.Find[pull]} && ${PullAggroTargetID}) {
                /varset castReturn CAST_SUCCESS
                /break
            }
            /if (!${IAmABard}) {
                /if (!${Me.AltAbilityReady[${WhatAA}]} && !${Me.Casting.ID}) {
                    /varset castReturn CAST_SUCCESS
                    /break
                } else /if (!${Me.Casting.ID}) {
                    /if (${castReturn.Equal[x]}) /call DoCastingEvents 2
                    /break
                }
            } else {
                | 1 - singing Quick Cast AA and AA NotReady
                | 3 - Not Singing Cast AA and AA NotReady and Not Casting.
                /if (!${Me.AltAbilityReady[${WhatAA}]}) {
                    /if ((${Me.Casting.ID} && ${Me.Casting.ID}!=${IAMCastingID}) || !${Me.Casting.ID}) {
                        /varset castReturn CAST_SUCCESS
                        /break
                    }
                | 4 - Not Singing Cast AA and AA NotReady and still Casting. Continue/DropThrough
                } else {
                    | 2 - singing Quick Cast AA and AA Still Ready
                    | 5 - Not Singing Cast AA and AA Ready and Not Casting.
                    /if ((${Me.Casting.ID} && ${Me.Casting.ID}!=${IAMCastingID}) || (!${Me.Casting.ID} || !${Window[CastingWindow].Open})) {
                        /call DoCastingEvents 10 1
                        /break
                    }
                    | 6 - Not Singing Cast AA and AA Ready and still Casting. Continue/DropThrough
                }
                /if (!${Me.Casting.ID} || ${Me.Casting.ID}!=${IAMCastingID} || !${Window[CastingWindow].Open}) {
                    /break
                }
            }
        }
        /varset CastResult ${castReturn}
        DEBUGCAST After While: ${WhatAA} ${IAMCastingID} ${castReturn} ${WasTwisting}
        DEBUGCAST CastAA Leave ${CastResult} ${Me.AltAbilityReady[${WhatAA}]}
    /return ${CastResult}
|--------------------------------------------------------------------------------------
| SUB: CastItem
|--------------------------------------------------------------------------------------
    Sub CastItem(string WhatItem,int WhatID,string sentFrom)
        /declare ItemCastTime float local 0
        DEBUGCAST CastItem Enter
        | Prevent casting of Prestige items on silver and F2P accounts
        /if (${Me.Subscription.NotEqual[gold]} && ${FindItem[=${WhatItem}].Prestige}) /return CAST_NO_RESULT
        /if (${Me.Invis} && !${Select[${sentFrom},SingleHeal,GroupHeal]}) {
            /varset CastResult CAST_CANCELLED
            /return CAST_CANCELLED
        }
        /varset ItemCastTime ${FindItem[=${WhatItem}].Clicky.CastTime.Float}
        /if (${IAmABard}) {
            /if (${ItemCastTime}>0) {
                | TODO pause remove
                |/call CastBardCheck 0
                /echo DEBUGMD casting then waiting /medley queue "${WhatItem}"
                /squelch /medley queue "${WhatItem}"
                /delay 60 ${Medley.TTQE}==0.0
                /echo DEBUGMD delay over
            } else {
                /useitem "${WhatItem}" 
            }
            DEBUGCAST I am a bard CastItem ${WhatItem} ${ItemCastTime}
        } else {
            /useitem "${WhatItem}"            
        }
       | /useitem "${WhatItem}"
        /if (${ItemCastTime}>0) {
            /delay 10 ${Window[CastingWindow].Open}
            /while (1) {
                /call DoCastingEvents
                /if (${sentFrom.Find[pull]} && ${PullAggroTargetID}) {
                    /varset CastResult CAST_SUCCESS
                    /break
                }
                /if (!${Me.Casting.ID} || !${Window[CastingWindow].Open}) /break
            }
        } else {
            /call DoCastingEvents
        }
        DEBUGCAST CastItem ${ItemCastTime} ${FindItem[=${WhatItem}].Spell.ID} ${Me.Casting.ID} ${Window[CastingWindow].Open}
        /varset CastResult ${castReturn}
        | Assign cast success if item on cooldown or summoned item consumed
        /if (!${Me.ItemReady[=${WhatItem}]} || !${FindItem[=${WhatItem}].ID} && !${Select[${CastResult},CAST_IMMUNE,CAST_RESISTED]})  /varset CastResult CAST_SUCCESS
        /if (${IAmABard}) {
            /if (${ItemCastTime}>0) {
                /if (${Me.Casting.ID}) /stopcast
            }
            |/if (${WasTwisting} && !${Twist}) /squelch /twist
        }
        DEBUGCAST CastItem Leave
    /return ${CastResult}
|--------------------------------------------------------------------------------------
| SUB: CastDisc
|--------------------------------------------------------------------------------------
    Sub CastDisc(string WhatDisc,int WhatID,string sentFrom)
        /declare WaitTimerCD timer local
        DEBUGCAST CastDisc ${WhatDisc} Enter
        /if (${Me.Invis} && !${Select[${sentFrom},SingleHeal,GroupHeal]}) {
            /varset CastResult CAST_CANCELLED
            /return CAST_CANCELLED
        }
        /if (!${Spell[${WhatDisc}].Duration} || (${Spell[${WhatDisc}].Duration} && ${Spell[${WhatDisc}].TargetType.Equal[Self]} && !${Me.ActiveDisc.ID}) || ${Spell[${WhatDisc}].TargetType.NotEqual[Self]} || ${Spell[${WhatDisc}].DurationWindow}) {
            DEBUGCAST Well we passed the CastDisc checks...
            /if (${Spell[${WhatDisc}].RecastTime.TotalSeconds}) {
                /if (${Spell[${WhatDisc}].RecastTime.TotalSeconds} < 3) {
                    /varset WaitTimerCD ${Spell[${WhatDisc}].RecastTime.TotalSeconds}s
                } else {
                    /varset WaitTimerCD 30
                }
            } else {
                /varset WaitTimerCD 10
            }
            DEBUGCAST CastDisk ${WhatDisc} ${Me.CombatAbilityReady[${WhatDisc}]} ${WaitTimerCD}
            /while (${Me.CombatAbilityReady[${WhatDisc}]} && ${WaitTimerCD}) {
                /if (${MacroQuest.Build}!=4) {
                    /disc ${Me.CombatAbility[${Me.CombatAbility[${WhatDisc}]}].ID}
                } else {
                    |in old clients /disc only accepts name not ID
                    /disc ${WhatDisc}
                }
                /delay 1
                /if (${MacroQuest.Build}!=4) {
                    /if (${Spell[${WhatDisc}].MyCastTime}) /call CastWait "${WhatDisc}"
                } else {
                    /if (${Spell[${WhatDisc}].RecastTimerID} == -1) {
                        |we cant wait here for this on emu CombatAbilityReady always return true
                        /break
                    }
                }
                /delay 10 !${Me.CombatAbilityReady[${WhatDisc}]}
            }
            /varset CastResult CAST_SUCCESS
        } else {
            DEBUGCAST CastDisk ${WhatDisc} Skipping.
        }
        DEBUGCAST CastDisc Leave
    /return ${CastResult}
|--------------------------------------------------------------------------------------
| SUB: CastCommand
|--------------------------------------------------------------------------------------
    Sub CastCommand(string WhatSkill,int WhatID)
        DEBUGCAST CastCommand Enter
        /declare CommandText string local ${WhatSkill.Right[-8]}
        DEBUGCAST "${CommandText}"
        /docommand ${CommandText}
        /delay 5
        /varset CastResult CAST_SUCCESS
        DEBUGCAST CastCommand Leave
    /return ${CastResult}
|--------------------------------------------------------------------------------------
| SUB: CastSkill
|--------------------------------------------------------------------------------------
    Sub CastSkill(string WhatSkill,int WhatID,string sentFrom)
        DEBUGCAST CastSkill Enter
        /if (${Me.Invis} && ${sentFrom.NotEqual[SingleHeal]}) {
            /varset CastResult CAST_CANCELLED
        } else {
            /doability "${WhatSkill}"
            /delay 20 !${Me.AbilityReady[${WhatSkill}]}
            /if (!${Me.AbilityReady[${WhatSkill}]}) /varset CastResult CAST_SUCCESS
            DEBUGCAST CastSkill Leave
        }
    /return ${CastResult}
|--------------------------------------------------------------------------------------
| SUB: CastSpell
|--------------------------------------------------------------------------------------
    Sub CastSpell(string WhatSpell,int WhatID,string sentFrom, int miscFlag)
        DEBUGCAST CastSpell Enter Line#: ${WhatSpell} ID:${WhatID} SentFrom:${sentFrom} ${Me.GemTimer[${WhatSpell}]}
        /declare MaxTryNum int local 2
        /declare TryNum int local 0
        /declare int_amIWasSitting bool local ${Me.Sitting}

        /if (${Me.Invis} && !${Select[${sentFrom},SingleHeal,GroupHeal]}) {
            /varset CastResult CAST_CANCELLED
            /return CAST_CANCELLED
        }
        /if (!${Target.CanSplashLand} && ${Spell[${WhatSpell}].TargetType.Equal[Free Target]}) {
           /echo Splash Spell will not cast at targets location. Skipping ${WhatSpell}
           /return CAST_NO_RESULT
        }
        /if (${IAmABard}) {
            /echo DEBUGMD casting then waiting /medley queue "${WhatSpell}"
            /if (${sentFrom.Equal[MezMobs]} || ${sentFrom.Equal[CharmMobs]}) { 
                /squelch /medley queue "${WhatSpell}" -targetid|${WhatID} -interrupt
                |/echo "MEDLEY::CAST /medley queue "${WhatSpell}" -targetid|${WhatID} -interrupt"
            } else {
                /squelch /medley queue "${WhatSpell}"
            }
            | TODO calculate better max wait
            /delay 30 ${Medley.TTQE}==0.0
            /echo DEBUGMD delay over
            /call DoCastingEvents 5 0
            /return ${castReturn}
        }
        /if (${Me.Gem[${WhatSpell}]}) {
            /while (1) {
                /if (${Me.GemTimer[${WhatSpell}]}==0 || ${Me.SpellReady[${WhatSpell}]}) {
                    /cast "${WhatSpell}"
                    DEBUGCAST CastSpell cast "${WhatSpell}"
                    /call DoCastingEvents 0 0
                    DEBUGCAST CastSpell1 Return from DoCastingEvents 1 CastResult: ${CastResult} castReturn: ${castReturn}
                    /varcalc TryNum ${TryNum}+1
                    /while (${Me.Casting.ID} && ${Window[CastingWindow].Open} && ${CastResult.NotEqual[CAST_CANCELLED]}) {
                        /if (${CastingInterruptOn}) {
                            DEBUGCAST CastSpell ENTER Interrupts SentFrom ${sentFrom} Spell:${WhatSpell} CastID:${Me.Casting.ID}
                            |/if (${sentFrom.Equal[SingleHeal]} && (${CastingInterruptOn}&4)==4) {
                            /if (${Select[${sentFrom},SingleHeals,Cure]}>0  && (${CastingInterruptOn}&4)==4) {
                               /call CastInteruptHeals "${WhatSpell}" ${miscFlag}
                            } else /if (${sentFrom.Equal[MezMobs]} && (${CastingInterruptOn}&16)==16) {
                               /call CastInteruptMez "${WhatSpell}"
                            } else /if (${Select[${sentFrom},dps,gom,burn]}>0 && (${CastingInterruptOn}&8)==8) {
                               /call CastInteruptDPS "${WhatSpell}"
                            } else /if (${Select[${sentFrom},buffs,buffs-nomem,Regenother]}>0 && (${CastingInterruptOn}&2)==2) {
                               /call CastInteruptBuffs "${WhatSpell}"
                            }
                            |/delay 2
                            DEBUGCAST CastSpell LEAVE Interrupts SentFrom ${sentFrom}
                        }
                        /if (${sentFrom.Equal[pull]} && ${PullAggroTargetID}) {
                            /stopcast
                            /return CAST_SUCCESS
                        }
                    }
                    /call DoCastingEvents 5 0
                    DEBUGCAST CastSpell2 Return from DoCastingEvents 2 CastResult: ${CastResult} castReturn: ${castReturn}
                    /if (${CastResult.Equal[CAST_CANCELLED]} || ${castReturn.Equal[x]}) /break
                    /varset CastResult ${castReturn}
                }
                |Don't change this it has to be PullAggroTargetID or you break ChainPulling.
                /if (${sentFrom.Equal[Pull]} && ${PullAggroTargetID}) /return CAST_SUCCESS
                DEBUGCAST CastSpell3 ${Select[${CastResult},CAST_FIZZLE,CAST_INTERRUPTED,CAST_RESISTED]} ${Spell[${WhatSpell}].RecastTime.TotalSeconds} ${Me.GemTimer[${WhatSpell}]} ${Me.SpellReady[${WhatSpell}]}
                /if (${TryNum}<${MaxTryNum} && ${Select[${CastResult},CAST_FIZZLE,CAST_INTERRUPTED,CAST_RESISTED]}) {
                    /if  (${Spell[${WhatSpell}].RecastTime.TotalSeconds}<=2) {
                        /while (${Me.GemTimer[${WhatSpell}]} && !${Me.SpellReady[${WhatSpell}]}) {
                            /delay 5
                        }
                    } else {
                        /break
                    }
                } else {
                    /break
                }
            }
        } else {
            /echo Skip Casting ${WhatSpell}. Spell Not Memed.
            /varset CastResult CAST_NO_RESULT
        }
      |  /if (${IAmABard}) {
            |/if (${Me.Casting.ID}) /stopcast
            |/if (${WasTwisting} && !${Twist}) /squelch /twist
       | }
        /if (${int_amIWasSitting} && !${Me.Sitting} && !${CombatStart}) /sit
        DEBUGCAST CastSpell Leave ${Me.GemTimer[${WhatSpell}]} ${CastResult}
    /return  ${CastResult}
| ----------------------------------------------------------------------------
| SUB: Cast DPS Spell Check ${Spell[${Spell[Gift of T'Vem].ID}].RecastTime}
| ----------------------------------------------------------------------------
    Sub CastDPSSpellCheck(SpellToCheck)
        |might think about looping through all buffs looking for caster name.
        |necro dots and raid mobs.
        DEBUGCAST DPSSpellCheck \awEnter
        /declare i int local 0
        /declare k int local 0
        /if (${Target.Buff[${SpellToCheck}].ID} && ${Target.Buff[${SpellToCheck}].Caster.Equal[${Me}]}) {
            DEBUGCAST DPSSpellCheck ${SpellToCheck} is already on the target.
                /return TRUE
        } else {
            /if (${Spell[${SpellToCheck}].HasSPA[470]}==TRUE) {
                /for k 1 to ${Spell[${SpellToCheck}].NumEffects}
                    /if (${Spell[${SpellToCheck}].Attrib[${k}]}==470) {
                        DEBUGCAST \atDEBUGCOMBAT DPSSpellCheck ${k} ${Target.Buff[${Spell[${Spell[${SpellToCheck}].Base2[${k}]}].Name}].ID}==${Spell[${SpellToCheck}].Trigger[${k}].ID} ${Spell[${SpellToCheck}].Trigger[${k}].Name} && ${Target.Buff[${Spell[${Spell[${SpellToCheck}].Base2[${k}]}].Name}].Caster.Equal[${Me}]}
                        /if (${Target.Buff[${Spell[${SpellToCheck}].Trigger[${k}].Name}].ID} && ${Target.Buff[${Spell[${SpellToCheck}].Trigger[${k}].Name}].Caster.Equal[${Me}]}) {
                            DEBUGCAST \atDEBUGCOMBAT DPSSpellCheck The spell ${SpellToCheck} triggers ${Spell[${SpellToCheck}].Trigger[${k}].Name} is already on the target.
                            /return TRUE
                        }
                    }
                /next k
            }
        }
        DEBUGCAST DPSSpellCheck \awLeave
    /return FALSE
| ----------------------------------------------------------------------------
| SUB: Cast Buffs Spell Check
| ----------------------------------------------------------------------------
    Sub CastBuffsSpellCheck(SpellToCheck)
        DEBUGCAST CastBuffsSpellCheck \awEnter
        /declare i int local
        /declare k int local
        /declare numEffects int local
        /delay 5s ${Target.BuffsPopulated}
        | If spell is already on target skip
        /if (${Target.Buff[${SpellToCheck}].ID}) {
            DEBUGCAST \at The spell ${SpellToCheck} is already on the target.
            /return ${Target.BuffDuration[${SpellToCheck}].TotalSeconds}
            | Does buff have children spells like Unified Hand of Assurance or Talisman of the Wulthan
        } else /if (${Spell[${SpellToCheck}].HasSPA[374]} || ${Spell[${SpellToCheck}].HasSPA[340]}) {
            /varcalc numEffects ${Spell[${SpellToCheck}].NumEffects}
            /for k 1 to ${numEffects}
                | Double check spell attrib to make sure it has children spells
                /if (${Spell[${SpellToCheck}].Attrib[${k}]} == 374 || ${Spell[${SpellToCheck}].Attrib[${k}]} == 340) {
                    | If child spell is already on target skip
                    /if (${Target.Buff[${Spell[${Spell[${SpellToCheck}].Base2[${k}]}].Name}].ID}) {
                        DEBUGCAST \at${k} BuffSpellCheck: The spell ${Spell[${Spell[${SpellToCheck}].Base2[${k}]}].Name} is already on the target.
                        /return ${Target.BuffDuration[${Spell[${Spell[${SpellToCheck}].Base2[${k}]}].Name}].TotalSeconds}
                    }
                }
            /next k
            |If spell doesn't stack with targets buffs skip
            /for k 1 to ${numEffects}
                | Double check spell attrib to make sure it has children spells
                /if (${Spell[${SpellToCheck}].Attrib[${k}]} == 374 || ${Spell[${SpellToCheck}].Attrib[${k}]} == 340) {
                    | If  spell doesn't stack with target buff skip
                    /for i 1 to 85
                        /if (${Target.Buff[${i}].Name.Length}) {
                            /if (!${Spell[${Spell[${SpellToCheck}].Base2[${k}]}].WillStack[${Target.Buff[${i}]}]}) {
                                DEBUGCAST \at${i} ${k} BuffSpellCheck: The spell ${Spell[${Spell[${SpellToCheck}].Base2[${k}]}].Name} does not stack with ${Target.Buff[${i}].Name} on the target.
                                /return ${Target.BuffDuration[${i}].TotalSeconds}
                            }
                        }
                    /next i
                }
            /next k
        } else /if (${AltAbility[${SpellToCheck}].Spell.Trigger.ID}) {
            /if (${Target.Buff[${AltAbility[${SpellToCheck}].Spell.Trigger.Name}].ID}) {
                DEBUGCAST \at The spell ${SpellToCheck}(${AltAbility[${SpellToCheck}].Spell.Trigger}) is already on the target.
                /return ${Target.BuffDuration[${AltAbility[${SpellToCheck}].Spell.Trigger.Name}].TotalSeconds}
            }
        } else {
            /for i 1 to 85
                /if (${Target.Buff[${i}].Name.Length}) {
                    /if (!${Spell[${SpellToCheck}].WillStack[${Target.Buff[${i}]}]}) {
                    DEBUGCAST \at${i} The spell ${SpellToCheck} does not stack with ${Target.Buff[${i}].Name} on the target.
                        /return ${Target.BuffDuration[${i}].TotalSeconds}
                    }
                }
            /next i
        }
        DEBUGCAST CastBuffsSpellCheck \awLeave
    /return FALSE
| -------------------------------------------------------------------------------------
| SUB: Cast Weave
| -------------------------------------------------------------------------------------
    Sub CastWeave(TarID)
        /if (!${Select[${Me.State},stand,mount]}) /return
        DEBUGCAST CastWeave \awEnter
        /declare i int local
        /declare DPSat1 int Local 0
        /declare DPSWeaveDelay timer local 2
        /declare DPSWeaveSpell string local
        /declare DPSWeaveArg3 string local
        /declare DPSWeaveArg4 string local
        /for i 1 to ${WeaveArray.Size}
            /varset DPSat1 ${Int[${WeaveArray[${i}].Arg[2,|]}]}
            /varset DPSWeaveSpell ${WeaveArray[${i}].Arg[1,|]}
            DEBUGN Cast DPSWeaveSpell:(${DPSWeaveSpell}) at (${DPSat1}%) ${i}
            /if (${DPSat1}==0 || !${TarID} || ${Spawn[${TarID}].Type.Equal[corpse]}) /return
            /if (${DPSat1}<${Spawn[${TarID}].PctHPs} && ${DPSOn}==1) /continue
            /if (!${ConOn} || !${WeaveArray[${i}].Find[|cond]} || ${If[${Cond[${WeaveArray[${i}].Mid[${Math.Calc[${WeaveArray[${i}].Find[|cond]}+5]},3]}]},1,0]}) {
                /if (${Me.AltAbilityReady[${WeaveArray[${i}].Arg[1,|]}]} || ${Me.ItemReady[${WeaveArray[${i}].Arg[1,|]}]} || ${Me.CombatAbilityReady[${WeaveArray[${i}].Arg[1,|]}]}) {
                    /if (${IAmMA} && ${TargetSwitchingOn} && ${Target.ID} && ${Target.ID}!=${TarID}) {
                        /call CombatTargetCheck 1
                        /if (!${MyTargetID}) /break
                        /varset TarID ${MyTargetID}
                    }
                    /call CastWhat "${DPSWeaveSpell}" ${TarID} WeaveStuff 0 0
                    /while (${DPSWeaveDelay}) {
                        /doevents
                    }
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /echo -- Weaving: ${DPSWeaveSpell}
                        /return 1
                    }
                }
            }
        /next i
        DEBUGCAST CastWeave \awLeave
    /return 0
|--------------------------------------------------------------------------------------
| SUB: CastBardCheck
|--------------------------------------------------------------------------------------
    Sub CastBardCheck(int ignorePlaying)
        DEBUGCAST CastBardCheck \awEnter ${Me.Casting.ID} ${Window[CastingWindow].Open}
        /if (${Medley.Active}) {
            /if (!${ignorePlaying}) {
                /while (${Me.BardSongPlaying}) {
                    /squelch /medley off
                    /delay 5
                    /if (${Me.Casting.ID}) /stopsong
                    /delay 30 !${Me.Casting.ID}
                }
            }
        } else {
            /if (${Me.BardSongPlaying}) {
                /stopsong
                /delay 5
            }
        }
        /call DoCastingEvents 10 1
        /varset CastResult CAST_NO_RESULT
        DEBUGCAST CastBardCheck Leave ${Me.Casting.ID}
    /return 
|--------------------------------------------------------------------------------------
| SUB: CastTarget
|--------------------------------------------------------------------------------------
    Sub CastTarget(int CTWhatID)
        DEBUGCAST CastTarget Enter \aw ID:${CTWhatID} Target:${Spawn[id ${CTWhatID}].CleanName}
        /if (${CTWhatID}) {
            /squelch /target clear
            /delay 10 !${Target.ID}
            /squelch /target id ${CTWhatID}
            /delay 10 ${Target.ID}==${CTWhatID}
        }
        DEBUGCAST CastTarget Leave
    /return
|--------------------------------------------------------------------------------------
| SUB: CastMem
|--------------------------------------------------------------------------------------
    Sub CastMem(string WhatMemSpell,string sentFrom)
        /declare WaitTimerCM timer local 0
        DEBUGCAST CastMem Enter
        /if (!${IAmABard} && (${Me.Casting.ID} || ${Me.Moving})) /return notready
        /if (${Me.Invis} && !${Select[${sentFrom},SingleHeal,GroupHeal]}) {
            /varset CastResult CAST_CANCELLED
            /return notready
        }
        | Prevent tanks and heals trying to mem spells during combat.
        /if (${Attacking} && ${IAmMA}) {
      |      /echo "(${Attacking} && ${IAmMA})"
            /echo Cannot mem a spell during combat or while you have aggro. ${WhatMemSpell}
            /return notready
        }
        /if (${Cursor.ID}) /call CheckCursor CastMem 0
        /if (${MiscGemRemem} && ${MiscGemLW} && ${Spell[${WhatMemSpell}].RecastTime.TotalSeconds}>30) {
            /if (${ReMemWaitLong.Equal[null]}) {
                /echo RMWL: ${ReMemWaitLong} ${WhatMemSpell} ${ReMemWaitLong.Equal[${WhatMemSpell}]} ${Spell[${WhatMemSpell}].RecastTime.TotalSeconds}
                /varset ReMemWaitLong ${WhatMemSpell}
                /varset DontMoveMe 1
                /call CastMemSpell "${WhatMemSpell}" ${MiscGemLW} 0 CastMem
                /varset DontMoveMe 0
                /return notready
            } else {
                /echo Still Waiting on Long Wait Spell ${ReMemWaitLong}
                /return notready
            }
        } else {
            /if (${Spell[${WhatMemSpell}].Mana}>${Me.CurrentMana}) /return notready
            /varset ReMemWaitShort ${WhatMemSpell}
            /varset DontMoveMe 1
            /call CastMemSpell "${WhatMemSpell}" ${MiscGem} 0 CastMem
            /varset DontMoveMe 0
            /if (${Me.Gem[${WhatMemSpell}]}) {
                /varset WaitTimerCM 350
                /while (!${Me.SpellReady[${WhatMemSpell}]} && ${WaitTimerCM}) {
                    /if (${ChaseAssist}) {
                        /call DoWeChase 0 CastMem
                    } else {
                        DOPARSE
                        /delay 5
                    }
                    /if (${Select[${sentFrom},buffs,buffs-nomem,buffonce,checkaura,summonstuff-nomem,dopetstuff,pet,pet-nomem]} && ${AggroTargetID} && ${Spawn[${AggroTargetID}].Distance}<200) {
                        /varset CastResult CAST_CANCELLED
                        /return notready
                    }
                }
            }
            /if (!${Me.Gem[${WhatMemSpell}]}) /return notready
        }
        DEBUGCAST CastMem Leave
    /return 1
|--------------------------------------------------------------------------------------
| SUB: CastReMem
|--------------------------------------------------------------------------------------
    Sub CastReMem(string WhatMemSpell,int forceReMem,string sentFrom)
        DEBUGCAST CastReMem Enter
        |We need to check if the spell just cast was swaped out and other spell needs to bo rememed.
        |Need a way to carry a flag so remem will be done out of combat after the spell has been cast.
        /if (${CastResult.Equal[CAST_SUCCESS]}) {
            |Is it the MiscGem?
            /if (${WhatMemSpell.Equal[${ReMemWaitShort}]}) {
                /varset ReMemCast 1
            |Is it the MiscGemLW
            } else /if (${WhatMemSpell.Equal[${ReMemWaitLong}]}) {
                /varset ReMemCastLW 1
            }
        }
        /if (!${forceReMem}) /return
        /if (${sentFrom.Equal[buffs]} && ${AggroTargetID} && ${Spawn[${AggroTargetID}].Distance}<200) /return
        /if (${MiscGemRemem}==1 || ${MiscGemRemem}==2) {
            /if (!${Me.Gem[${ReMemMiscSpell}]} && ${ReMemCast} && !${CombatStart} && !${Me.Buff[Resurrection Sickness].ID} && !${sentFrom.Find[-nomem]}) {
                /if (${Cursor.ID}) /call CheckCursor CastReMem 0
                /varset DontMoveMe 1
                /call CastMemSpell "${ReMemMiscSpell}" ${MiscGem} 0 CastReMem
                /varset DontMoveMe 0
                /varset ReMemCast 0
                /varset ReMemWaitShort null
            }
        }
        |Remem spell originally in Long Wait Gem slot
        /if ((${MiscGemRemem}==1 || ${MiscGemRemem}==3) && ${MiscGemLW} && ${ReMemWaitLong.NotEqual[null]}) {
            /if (${ReMemCastLW} && !${Me.Buff[Resurrection Sickness].ID}) {
                /if (${Cursor.ID}) /call CheckCursor CastReMem2 0
                /varset DontMoveMe 1
                /call CastMemSpell "${ReMemMiscSpellLW}" ${MiscGemLW} ${Int[${Me.Gem[${ReMemMiscSpellLW}]}]} CastReMem
                /varset DontMoveMe 0
                /varset ReMemCastLW 0
                /varset ReMemWaitLong null
            }
        }
        DEBUGCAST CastReMem Leave
    /return
|--------------------------------------------------------------------------------------
| SUB: CastMemSpell
|--------------------------------------------------------------------------------------
    Sub CastMemSpell(SpellToMem, int gemNum, int ForceIt, string sentFrom)
        DEBUGCAST CastMemSpell Enter
        /if (${Int[${SpellToMem.Length}]}==0 || ${SpellToMem.Equal[null]} || ${gemNum}==0) /return

        /if (${Int[${Me.Gem[${SpellToMem}]}]}>0 && !${ForceIt}) /return
        /if (${Cursor.ID} && ${Cursor.NoRent}) {
            /autoinventory
            /delay 10
        }
        | Unmem a spell if already memmed in a diffent gem
        /if (${ForceIt}) {
            /if (${Int[${Me.Gem[${SpellToMem}]}]}>0 && ${gemNum}!=${ForceIt}) {
                /notify CastSpellWnd CSPW_Spell${Int[${Math.Calc[${ForceIt}-1]}]} rightmouseup
                /delay 20 ${Int[${Me.Gem[${gemNum}].ID}]}==0
            }
        }
        /if (${Me.Book[${SpellToMem}]}) {
            /if (!${Cursor.ID}) {
                /if (${Int[${Me.Gem[${gemNum}].ID}]}>0) {
                    /notify CastSpellWnd CSPW_Spell${Int[${Math.Calc[${gemNum}-1]}]} rightmouseup
                    /delay 20 ${Int[${Me.Gem[${gemNum}].ID}]}==0
                }
                /if (!${Int[${Me.Gem[${gemNum}].Name.Length}]} || ${Me.Gem[${gemNum}].Name.NotEqual[${SpellToMem}]}) {
                    /if (${IAmABard}) {
                        /call CastBardCheck 0
                    }
                    /while (${Me.Moving}) {
                        DOPARSE
                        /delay 5
                    }
                    /echo Memming ${SpellToMem} in slot ${gemNum}
                    /if (${Stick.Active}) {
                        /stick pause
                        /MemSpell ${gemNum} "${SpellToMem}"
                        /delay 150 ${Me.Gem[${gemNum}].Name.Equal[${SpellToMem}]}
                        /stick unpause
                    } else {
                        /MemSpell ${gemNum} "${SpellToMem}"
                        /delay 150 ${Me.Gem[${gemNum}].Name.Equal[${SpellToMem}]}
                    }
                }
            } else {
                /echo Cannot Mem a spell with Items on Cursor. Please drop item to Inventory.
            }
        } else {
            /echo Could Not find the spell ${SpellToMem} in your spell book.
        }
        /if (${Window[SpellBookWnd].Open}) /windowstate spellbookwnd close
        DEBUGCAST CastMemSpell ${SpellToMem} ${gemNum} ${ForceIt} ${sentFrom} ${Me.Gem[${gemNum}].Name}
        DEBUGCAST CastMemSpell Leave
    /return
|--------------------------------------------------------------------------------------
| SUB: CastInteruptHeals
|--------------------------------------------------------------------------------------
    Sub CastInteruptHeals(string CIHSpell, int overRide)
        DEBUGCAST CastInteruptHeals Enter
        /if (!${Int[${Me.Casting.ID}]}) /return
        | Check heal target is not an npc unless spell is detrimenatl like a tap or nuke type heal
        /if (${Target.Type.Equal[npc]} && ${Spell[${CIHSpell}].SpellType.NotEqual[Detrimental]} && !${overRide}) {
            /stopcast
            /varset CastResult CAST_CANCELLED
            /echo ${CIHSpell} interrupted. Target is an NPC
            DEBUGCAST CastInteruptHeals  CheckHP1: ${Target.Type}
        }
        | Override default heal interupts if caster MA and has his own healing set. ie pally or sk
        /if (${Target.Type.NotEqual[npc]} && ${SHealPct}>65 && ${Target.PctHPs}>${SHealPct} && !${CIHSpell.Find[promised]}) {
            /stopcast
            /varset CastResult CAST_CANCELLED
            /echo ${CIHSpell} interrupted. Target is above required ${SHealPct}% health. ${Target}
            DEBUGCAST CastInteruptHeals CheckHP2: ${Target.PctHPs} ${SHealPct}
        }
        DEBUGCAST CastInteruptHeals Leave
    /return
|--------------------------------------------------------------------------------------
| SUB: CastInteruptDPS
|--------------------------------------------------------------------------------------
    Sub CastInteruptDPS(string CIDSpell)
        DEBUGCAST CastInteruptDPS Enter
        /declare MAInterruptAt int local ${If[${SingleHealPointMA}<70,${SingleHealPointMA},70]}
        /if (!${Int[${Me.Casting.ID}]}) /return
        /if (${Target.Type.Equal[pc]}) /return
        /if (${Target.PctHPs}<1 || ${Target.Type.Equal[corpse]} || !${Int[${Target.ID}]}) {
            /stopcast
            /varset CastResult CAST_CANCELLED
            /echo ${CIDSpell} interrupted due to Killing corpses is not required.
            DEBUGCAST CastInteruptDPSCheckDPS1: ${Target.PctHPs} ${Target.Type} ${Int[${Target.ID}]}
        }
        /if (${HealsOn}) {
            /if (!${IAmMA} && ${Spawn[${MainAssist}].PctHPs}<${MAInterruptAt} && ${Spawn[${MainAssist} ${MainAssistType}].ID} && ${Spawn[${MainAssist} ${MainAssistType}].Type.NotEqual[corpse]} && ${Select[${Me.Class.ShortName},nec,mag]}==0) {
                /stopcast
                /varset CastResult CAST_CANCELLED
                /varset LastHealCheck 0
                /echo ${CIDSpell} interrupted due to The Main Assist Needs a Heal.
                DEBUGCAST CastInteruptDPS CheckDPS2: ${HealsOn}  ${Spawn[${MainAssist}].PctHPs} ${Spawn[${MainAssist} ${MainAssistType}].ID}  ${Spawn[${MainAssist} ${MainAssistType}].Type}
            } else /if (${Select[${Role},pettank,petpullertank,hunterpettank]}>0 && ${Me.Pet.ID} && ${Me.Pet.PctHPs}<${MAInterruptAt}) {
                /stopcast
                /varset CastResult CAST_CANCELLED
                /varset LastHealCheck 0
                /echo ${CIDSpell} interrupted due to my Pet Tank needs a heal.
                DEBUGCAST CastInteruptDPS CheckDPS3: ${HealsOn}  ${Me.Pet.ID} ${Me.Pet.PctHPs}
            }
        }
        DEBUGCAST CastInteruptDPS Leave
    /return
|--------------------------------------------------------------------------------------
| SUB: CastInteruptBuffs
|--------------------------------------------------------------------------------------
    Sub CastInteruptBuffs(string CIBSpell)
        DEBUGCAST CastInteruptBuffs Enter
        /if (!${Me.Casting.ID}) /return
        /if (${Me.Casting.Range}==0 && ${Me.Casting.AERange}==0) /return
        /if (${Me.Invis}) {
            /stopcast
            /varset CastResult CAST_CANCELLED
            /echo ${CIBSpell} interrupted because I am now invised.
            DEBUGCAST CheckBUFFS1: ${Me.Invis}
            /return
        }
        /if ((${Target.ID} && ${Target.PctHPs}<1) || (${Target.Type.Equal[corpse]})) {
            /stopcast
            /varset CastResult CAST_CANCELLED
            /echo ${CIBSpell} interrupted because target dead or not valid.
            DEBUGCAST CheckBUFFS2: ${Int[${Target.PctHPs}]} ${Target.Type}  ${Int[${Target.ID}]} ${Me.Casting} ${Me.Casting.TargetType}
        }
        /if (${Me.Casting.TargetType.NotEqual[self]}) {
            /if ((${Me.Casting.Range}>0 && ${Target.Distance}>${Me.Casting.Range}) || (${Me.Casting.AERange}>0 && ${Target.Distance}>${Me.Casting.AERange})) {
                /stopcast
                /varset CastResult CAST_CANCELLED
                /echo ${CIBSpell} interrupted because target out of range.
                DEBUGCAST CheckBUFFS3: ${Int[${Me.Casting.ID}]}  ${Target.Distance} ${Me.Casting.Range} ${Me.Casting.AERange} ${Me.Casting}
            }
        }
        /if (${AggroTargetID} && ${Spawn[${AggroTargetID}].Distance}<200) {
            /stopcast
            /varset CastResult CAST_CANCELLED
            /echo ${CIBSpell} interrupted because we now have aggro.
            DEBUGCAST CheckBUFFS4: ${AggroTargetID} && ${Spawn[${AggroTargetID}].Distance}
        }
        DEBUGCAST CastInteruptBuffs Leave
    /return
|--------------------------------------------------------------------------------------
| SUB: CastInteruptMez
|--------------------------------------------------------------------------------------
    Sub CastInteruptMez(string CIMSpell)
        DEBUGCAST CastInteruptMez Enter
        /if (!${Me.Casting.ID}) /return
        /if (!${Select[${Me.Class.ShortName},BRD,ENC]}) /return
        /if (${MezAECount}==0 || ${Select[${MezOn},1,3]}==0 || ${MezAETimer}) /return
        /if (${Spell[${MezAESpell}].Range}==0 && ${Spell[${MezAESpell}].AERange}==0) /return
        /varset MezMobAECount ${SpawnCount[npc xtarhater loc ${Target.X} ${Target.Y} radius ${Spell[${MezAESpell}].AERange}]}
        /if (!${MezMobAECount}) /return
        /if (${Me.XTarget[${XTSlot}].TargetType.NotEqual[Auto Hater]} && ${Me.XTarget[${XTSlot}].Type.NotEqual[pc]}) /varcalc MezMobAECount ${MezMobAECount}+1
        /if (${MezMobAECount}<${MezAECount}) /return
        /stopcast
        /varset MezAEClosest ${Target.ID}
        /varset CastResult CAST_CANCELLED
        /echo ${CIMSpell} interrupted because Need AE Mez.
        DEBUGCAST CastInteruptMez: ${Target.Type} ${Int[${Target.ID}]} ${Me.Casting} ${Me.Casting.TargetType} ${MezMobAECount} Leave
    /return
|----------------------------------------------------------------------------
| SUB: Single Heals
|----------------------------------------------------------------------------
    Sub SingleHeal(SHealName, SHealType, int SHealHPs, int WhoNum, String SHealFlag)
        /if (!${HealsOn} || ${Me.Moving} || ${Me.Hovering} || ${Spawn[${SHealName} ${SHealType}].ID}==0 || (${Select[${SHealType},PC,Pet,Mercenary]}==0 && !${XTarHeal}) || (${Me.Invis} && !${AggroTargetID})) /return
        DEBUGHEALS SingleHeal Enter: \ayName: ${SHealName} TType: ${SHealType} Hps%: ${SHealHPs} WhoNum:${WhoNum}
        /varset GoMByPass 1
        /doevents
        /varset GoMByPass 0
        /declare i               int     local   0
        /declare SHealSpell      string  local
        /declare SHealTag        string  local
        /declare SHealThem       int     local   ${Spawn[=${SHealName} ${SHealType}].ID}
        /declare SHealClass      string  local   ${Spawn[=${SHealName} ${SHealType}].Class.ShortName}
        /declare MainAssistID    int     local   ${Spawn[=${MainAssist}].ID}
        /declare SHealRange      int     local   0
        /declare HealLoopTimer   timer   local   10
        /declare CondNo          int     local   0
        /declare RemCheck        int     local   0
        | Set MA ID to 6 to keep spell durations correct
        /if (${WhoNum}!=6 && ${SHealThem}==${MainAssistID} && ${SHealFlag.NotEqual[X]}) /varset WhoNum 6
        /if (${SHealType.Equal[corpse]} || !${Spawn[=${SHealName} ${SHealType}].ID} || ${Spawn[=${SHealName}].Type.Equal[corpse]}) /return
        /for i 1 to ${SingleHeal.Size}
            | New Conditional Check to Skip spell
            /if (${ConOn} && ${SingleHeal[${i}].Find[|cond]}) {
                /varset CondNo ${SingleHeal[${i}].Mid[${Math.Calc[${SingleHeal[${i}].Find[|cond]}+5]},3]}
            } else {
                /varset CondNo 0
            }
            /if (${DebugHeals}) /delay 2
            | If heal is null or off |0 or spell/aa/item not ready skip it
            DEBUGHEALS SingleHeal \awSpell ${i}: ${SingleHeal[${i}]}  ${SingleHeal[${i}].Arg[1,|]}   ${SingleHeal[${i}].Arg[2,|]}   ${SingleHeal[${i}].Arg[3,|]}
            DEBUGHEALS SingleHeal \awSpellSkip: !${SingleHeal[${i}].Length} || ${SingleHeal[${i}].Arg[2,|].Equal[0]}
            /if (!${SingleHeal[${i}].Length} || ${SingleHeal[${i}].Arg[2,|].Equal[0]}) /continue
            /varset SHealSpell   ${SingleHeal[${i}].Arg[1,|]}
            /varset SHealPct     ${SingleHeal[${i}].Arg[2,|]}
            /varset SHealTag     ${SingleHeal[${i}].Arg[3,|]}
            /varset SHealRange ${Spell[${SHealSpell}].Range}
            /call CastReady "${SHealSpell}" singleheal
            /if (!${SpellReadyL}) /continue
            /if (${Spell[${SHealSpell}].TargetType.Find[Group v]}) /varset SHealRange ${Spell[${SHealSpell}].AERange}
            /if (!${SHealRange}) /varset SHealRange 100
            | Skip Heal if pet
            /if (${Spawn[${SHealThem}].Type.NotEqual[Pet]} && ${SHealTag.Equal[pet]}) /continue
            | Skip spells marked for ME and not healing self
            /if (${SHealTag.Equal[me]} && ${SHealThem}!=${Me.ID}) /continue
            /if (${SHealTag.Equal[!me]} && ${SHealThem}==${Me.ID}) /continue
            /if (${SHealTag.Equal[class]} && ${Select[${Spawn[id ${SHealThem}].Class.ShortName},${SingleHeal[${i}].Arg[4,|]}]}==0) /continue
            /if (${SHealTag.Equal[!class]} && ${Select[${Spawn[id ${SHealThem}].Class.ShortName},${SingleHeal[${i}].Arg[4,|]}]}>0) /continue
            /if (${SHealThem}!=${MainAssistID} && (${HealGroupPetsOn} && ${SHealTag.Equal[pet]} && ${Spawn[${SHealThem}].Type.NotEqual[pet]}) || (!${HealGroupPetsOn} && ${SHealTag.Equal[pet]})) /continue
            | If tag MA/Mob and heal target not MA or tag !MA and heal target MA skip
            DEBUGHEALS SingleHeal \awTAG: ${SHealTag.Equal[MA]} && ${SHealThem}!=${MainAssistID} || ${SHealTag.Equal[Mob]} && ${SHealThem}!=${MainAssistID} || ${SHealTag.Equal[!MA]} && ${SHealThem}==${MainAssistID}
            /if ((${SHealTag.Equal[MA]} && ${SHealThem}!=${MainAssistID}) || (${SHealTag.Equal[Mob]} && ${SHealThem}!=${MainAssistID}) || (${SHealTag.Equal[!MA]} && ${SHealThem}==${MainAssistID})) /continue
            | Cleric Divine Arbitration and Epics do not work on pets or people out of group
            /if (${Spell[${SHealSpell}].TargetType.Equal[group v1]} && ${Spawn[id ${SHealThem} group].ID}==0) /continue
            /if (${Spawn[${SHealThem}].Type.Equal[Pet]}  && (${SHealSpell.Find[Aegis of Superior Divinity]} || ${SHealSpell.Find[Harmony of the Soul]} ||  ${SHealSpell.Find[Divine Arbitration]})) /continue
            /if (!${Spawn[id ${SHealThem} group].ID} && (${SHealSpell.Find[Aegis of Superior Divinity]} || ${SHealSpell.Find[Harmony of the Soul]} ||  ${SHealSpell.Find[Divine Arbitration]})) /continue
            | intervention and survival lines only work on group members
            /if (!${Spawn[id ${SHealThem} group].ID} && ${Select[${Me.Class.Name},Druid,Shaman]} && (${SHealSpell.Find[Intervention]} || ${SHealSpell.Find[Survival]})) /continue
            | Check For Life Taps
            /if (${SHealTag.Find[Tap]}) {
                /if (!${Pulled} && ${CombatStart} && ${Me.PctHPs}<=${SHealPct} && ${Target.Type.NotEqual[Corpse]} && ${Spawn[${MyTargetID}].ID} && ${Spawn[${MyTargetID}].Distance}<=${SHealRange} && ${Spell${i}GM0}==0) {
                    /call CastWhat "${SHealSpell}" ${Spawn[${MyTargetID}].ID} SingleHeal ${CondNo} 1
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /call BroadCast o "${SHealSpell} for  >> ${Me.CleanName} <<"
                        /if (${Spell[${SHealSpell}].Duration.TotalSeconds}>0) {
                            /varcalc Spell${i}GM0 ${Spell[${SHealSpell}].MyDuration.TotalSeconds}*10
                        } else /if (${Spell[${SHealSpell}].RecastTime}>0) {
                            /varcalc Spell${i}GM0 ${Spell[${SHealSpell}].RecastTime}/100
                        } else {
                            /varset Spell${i}GM0 10
                        }
                        DEBUGHEALS SingleHeal Assign Timer:Spell${i}GM0 ${Spell[${SHealSpell}].MyDuration.TotalSeconds} ${Spell${i}GM0} Line#: ${Macro.CurLine}
                        /return
                    } else /if (${Macro.Return.Equal[CAST_CANCELLED]}) {
                        /return
                    }
                }
                /continue
            }
            | Check For Nuke Heals - contributed by thenomadman
            /if (${SHealTag.Find[Mob]}) {
                /if (!${AggroTargetID}) /continue
                /while (1) {
                    /varset EventFlag 0
                    /doevents Switch
                    /if (!${EventFlag}) /break
                }
                /if (!${MyTargetID} || ${Spawn[${MyTargetID}].Type.Equal[Corpse]}) /call CombatTargetCheck 0
                /if (${MyTargetID} && ${Spawn[${MainAssist}].PctHPs}<=${SHealPct} && ${Spawn[${MyTargetID}].LineOfSight} && ${Spawn[${MyTargetID}].Distance}<=${SHealRange} && ${Spawn[${MyTargetID}].Type.NotEqual[Corpse]}) {
                    /if (!${HealAgain}) /varset HealAgain 21
                    /call CastWhat "${SHealSpell}" ${MyTargetID} SingleHeal ${CondNo} 1
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /call BroadCast o "${SHealSpell} for >>${Spawn[${SHealName} ${SHealType}].CleanName} << cast on ${Spawn[${MyTargetID}].CleanName}"
                        /varset HealAgain 1
                        /return
                    } else /if (${Macro.Return.Equal[CAST_CANCELLED]}) {
                        /varset HealAgain 1
                        /return
                    } else /if (${Macro.Return.Equal[CAST_NEEDMANA]}) {
                        /varset HealAgain 0
                        /continue
                    } else /if (${Macro.Return.Equal[CAST_COND_FAILED]}) {
                        /if (${HealAgain}==21) /varset HealAgain 0
                        /continue
                    }
                }
                /continue
            }
            | Check conditons for heals
            DEBUGHEALS SingleHeal \aw${SHealHPs}<=${SHealPct} && ${Spawn[${SHealName} ${SHealType}].Distance}<=${SHealRange} && ${Spell${i}GM${WhoNum}}==0 ${Spell${i}XT${WhoNum}}==0
            /if (${SHealHPs}<=${SHealPct} && ${Spawn[${SHealName} ${SHealType}].Distance}<=${SHealRange} && ((${SHealFlag.Equal[G]} && ${Spell${i}GM${WhoNum}}==0) || (${SHealFlag.Equal[X]} && ${Spell${i}XT${WhoNum}}==0))) {
                | target has qualified for a heal
                /if (${Spell[${SHealSpell}].TargetType.Equal[Free Target]}) {
                    | Check if spalsh heal target is in line of sight
                    /if (${Target.ID}!=${SHealThem}) {
                        /target id ${SHealThem}
                        /delay 2s ${Target.ID}==${SHealThem}
                    }
                    /if (!${Target.CanSplashLand}) {
                        /echo Splash Spell will NOT land on target. Skipping.
                        /continue
                    }
                }
                /if (${Select[${EverQuest.Server},zek]} && ${Select[${Target.Type},PC]} && ${Me.Combat}) {
                    /attack off
                    /delay 25 !${Me.Combat}
                }
                /if (${SHealThem}==${MainAssistID} && ${Spawn[${MainAssist} ${MainAssistType}].ID}) {
                    /varset RemCheck ${Select[TRUE,${Bool[${Me.Song[${HealRemChk1}].ID}]},${Bool[${Me.Song[${HealRemChk2}].ID}]},${Bool[${Me.Song[${HealRemChk3}].ID}]}]}
                    /if (!${RemCheck}) /varset RemCheck ${Select[TRUE,${Bool[${Me.Buff[${HealRemChk1}].ID}]},${Bool[${Me.Buff[${HealRemChk2}].ID}]},${Bool[${Me.Buff[${HealRemChk3}].ID}]}]}
                    /if (${RemCheck}) {
                        /removebuff "${HealRemChk${RemCheck}}"
                        /echo Removing Invulnerability Buff: ${HealRemChk${RemCheck}}
                    }
                }
                /if (!${HealAgain}) /varset HealAgain 22
                /call CastWhat "${SHealSpell}" ${SHealThem} SingleHeal ${CondNo} 0
                DEBUGHEALS SingleHeal \awMR: ${Macro.Return}
                /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                    /call BroadCast o "${SHealSpell} on  >> ${Spawn[${SHealName} ${SHealType}].CleanName} <<"
                    /if (${SHealSpell.Find[Rune]}) {
                        /if (${SHealFlag.Equal[G]}) {
                            /varset Spell${i}GM${WhoNum} 0
                        } else {
                            /varset Spell${i}XT${WhoNum} 0
                        }
                    } else /if (${SHealSpell.Find[Promised]}) {
                        /if (${SHealFlag.Equal[G]}) {
                            /varset Spell${i}GM${WhoNum} 215
                        } else {
                            /varset Spell${i}XT${WhoNum} 215
                        }
                    } else {
                        /if (${SHealFlag.Equal[G]}) {
                            /if (${Spell[${SHealSpell}].Duration.TotalSeconds}>0) {
                            /varcalc Spell${i}GM${WhoNum} ${Spell[${SHealSpell}].MyDuration.TotalSeconds}*10
                            } else /if (${Spell[${SHealSpell}].RecastTime}>0) {
                                /varcalc Spell${i}GM${WhoNum} ${Spell[${SHealSpell}].RecastTime}/100
                            } else {
                                /varset Spell${i}GM${WhoNum} 10
                            }
                        } else {
                            /if (${Spell[${SHealSpell}].Duration.TotalSeconds}>0) {
                            /varcalc Spell${i}XT${WhoNum} ${Spell[${SHealSpell}].MyDuration.TotalSeconds}*10
                            } else /if (${Spell[${SHealSpell}].RecastTime}>0) {
                                /varcalc Spell${i}XT${WhoNum} ${Spell[${SHealSpell}].RecastTime}/100
                            } else {
                                /varset Spell${i}XT${WhoNum} 10
                            }
                        }
                    }
                    DEBUGHEALS SingleHeal Assign Timer:Spell${i}${If[${SHealFlag.Equal[G]},GM,XT]}${WhoNum} ${Spell[${SHealSpell}].MyDuration.TotalSeconds} ${Spell${i}${If[${SHealFlag.Equal[G]},GM,XT]}${WhoNum}}
                    | Prioritize heals by setting flag to check Health again
                    /if (${SHealClass.NotEqual[ber]}) /varset HealAgain 1
                    /return
                } else /if (${Macro.Return.Equal[CAST_CANCELLED]}) {
                    /varset HealAgain 1
                    /return
                } else /if (${Macro.Return.Equal[CAST_NEEDMANA]}) {
                    /varset HealAgain 0
                    /continue
                } else /if (${Macro.Return.Equal[CAST_COND_FAILED]}) {
                    /if (${HealAgain}==22) /varset HealAgain 0
                    /continue
                }
            | Check if target still needs a heal. spell might not be ready or timer not zero.
            } else /if (${SHealHPs}<=${SHealPct} && ${Spawn[${SHealName} ${SHealType}].Distance}<=${SHealRange}) {
                /if (${SHealClass.NotEqual[ber]}) /varset HealAgain 1
            }
        /next i
        DEBUGHEALS \aySingleHeal Leave ${HealAgain}
    /return
| ----------------------------------------------------------------------------
| SUB: Do Group Heal Stuff
| ----------------------------------------------------------------------------
    Sub DoGroupHealStuff(int GroupHealth)
        DEBUGHEALS DoGroupHealStuff Enter ${GroupHealth}
        /varset GoMByPass 1
        /doevents
        /varset GoMByPass 0
        /declare j              int     local
        /declare HealSpell      string  local
        /declare HealPct        string  local
        /declare CondNo         int     local 0
        /for j 1 to ${GroupHeal.Size}
            /if (!${Heals[${j}].Length} || ${Heals[${j}].Arg[2,|].Equal[0]}) /return
            /if (${ConOn} && ${GroupHeal[${j}].Find[|cond]}) {
                /varset CondNo ${GroupHeal[${j}].Mid[${Math.Calc[${GroupHeal[${j}].Find[|cond]}+5]},3]}
            } else {
                /varset CondNo 0
            }
            /varset HealSpell   ${GroupHeal[${j}].Arg[1,|]}
            /varset HealPct     ${GroupHeal[${j}].Arg[2,|]}
            DEBUGHEALS DoGroupHealStuff ${HealSpell} ${HealPct}
            DEBUGHEALS DoGroupHealStuff Group Heal Spell/Item:${Spell[${HealSpell}].TargetType.Find[group v]}/${Spell[${FindItem[=${HealSpell}].Spell}].TargetType.Find[group v]} Spell:${HealSpell} GpAvg:${GroupHealth} HealAt:${HealPct} Timer:${SpellGH${j}}==0
            /if (${Group.Injured[${HealPct}]} > 1  && ${SpellGH${j}}==0) {
                /call CastReady "${HealSpell}" groupheal
                /call CastWhat "${HealSpell}" ${Me.ID} GroupHeal ${CondNo} 0
                /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                    DEBUGHEALS DoGroupHealStuff ${HealSpell} on  >> Group <<
                    /call BroadCast o "${HealSpell} on  >> Group <<"
                    /varcalc SpellGH${j} ${Spell[${HealSpell}].MyDuration.TotalSeconds}*10
                    DEBUGHEALS DoGroupHealStuff Assign Timer:SpellGH${j} ${Spell[${HealSpell}].MyDuration.TotalSeconds} ${SpellGH${j}}
                    /varset HealAgain    1
                    /return
                }
            }
        /next j
        DEBUGHEALS DoGroupHealStuff Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: MassGroupBuff
| -------------------------------------------------------------------------------------
    Sub MassGroupBuff(SpellToCast1,int i1, int CondNumber)
        /if (!${SpellToCast1.Length} || ${i1}==0) /return 0
        /if (!${Spell[${SpellToCast1}].TargetType.Find[Group v]}) {
            /varset Buffs[${i1}] null
            /echo Spell ${SpellToCast1} cannot be MGB'd. Setting to null.
            /return 0
        } else /if (${Me.AltAbility[Tranquil Blessings]}==0) {
            /varset Buffs[${i1}] null
            /echo You don't have Tranquil Blessings AA. Setting ${SpellToCast1} to null.
            /return 0
        }
        /if (${Select[${Me.CombatState},active,resting]}==0) {
            /echo Your Not Resting. Will Have to Wait to cast MGB.
        } else /if (${Me.AltAbilityReady[Tranquil Blessings]} && ${Me.CurrentMana}>=${Math.Calc[${Spell[${SpellToCast1}].Mana}*2]}) {
            /if (!${CondNumber} || ${If[${Cond[${CondNumber}]},1,0]}) {
                /call CastWhat "Tranquil Blessings" ${Me.ID} Buffs-nomem 0 0
                /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                    /echo Casting Tranquil Blessings for MGB of ${SpellToCast1}
                    /call CastWhat "${SpellToCast1}" ${Me.ID} Buffs-nomem 0 0
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /echo MGB of ${SpellToCast1} Successful.
                    }
                }
            }
        }
    /return
| -------------------------------------------------------------------------------------
| SUB: SummonStuff
| -------------------------------------------------------------------------------------
    Sub SummonStuff(SSpell,SumItem,int SumNum,int CondNumber)
        /if (${Me.Invis}) /return
        /if (${FindItemCount[=${SumItem}]}>=${SumNum}) /return 0
        /declare SumResult string local FALSE
        /declare SummonAttempts int local 0
        /declare CurrentCount int local 0
        /declare OriginalCount int local ${FindItemCount[=${SumItem}]}
        DEBUGBUFF SummonStuff Enter SSpell: ${SSpell} SumItem: ${SumItem} SumNum: ${SumNum}
        /if (${CondNumber} && ${If[${Cond[${CondNumber}]},0,1]}) /return 0
        /while (${FindItemCount[=${SumItem}]}<${SumNum}) {
            /if (${Me.FreeInventory}==0) {
                /echo No room in inventory skipping summoning >> ${SumItem} <<.
                /break
            }
            /varset CurrentCount ${FindItemCount[=${SumItem}]}
            | Summon axes for zerkers had to be hard coded because spell is same name as the item summoned.
            /if (${Select[${Me.Class.ShortName},BER]} && ${SSpell.Find[axe]}) {
                /doevents flush Missing
                /if (!${Me.CombatAbility[${SSpell}]}) {
                    /varset SumResult CAST_COMPONENTS
                    /return ${SumResult}
                } else /if (!${Me.CombatAbilityReady[${SSpell}]}) {
                    DEBUGBUFF ${SSpell} Combat Ability not ready skipping
                    /varset SumResult CAST_NOT_READY
                    /return ${SumResult}
                }
                /if (${Cursor.ID}) /call CheckCursor SummonStuff 1
                /docommand /disc ${SSpell}
                /varset MissingComponent 0
                /call CastWait "${SSpell}" SummonStuff
                /doevents Missing
                /if (!${MissingComponent} && !${Cursor.ID}) {
                    /delay 20 ${Cursor.ID}
                }
                /if (${Cursor.ID}) {
                    /varcalc SummonAttempts ${SummonAttempts}+1
                    /varset SumResult CAST_SUCCESS
                } else {
                    /if (!${MissingComponent}) {
                        /doevents Missing
                    } else {
                        /varset SumResult CAST_COMPONENTS
                    }
                }
            } else {
                | Check if summoning from an item
                /if (${FindItemCount[=${SSpell}]} > 0) {
                    | Check if item is ready to cast
                    /if (${FindItem[=${SSpell}].Timer}!=0) {
                        DEBUGBUFF ${SSpell} Item not ready yet skipping
                        /varset SumResult CAST_NOT_READY
                        /return ${SumResult}
                    }
                }
                /if (${Cursor.ID}) /call CheckCursor SummonStuff 1
                /call CastWhat "${SSpell}" ${Me.ID} SummonStuff-nomem 0 0
                /varset SumResult ${Macro.Return}
            }
            DEBUGBUFF CheckBuffs summon item cast result ${Macro.Return} ${SumResult}
            /if (${SumResult.Equal[CAST_SUCCESS]}) {
                /delay 5s ${Cursor.ID}
                /if (${Cursor.ID}) {
                    /echo Summoned  >> ${SumItem} <<
                    /varcalc SummonAttempts ${SummonAttempts}+1
                    /call CheckCursor SummonStuff 1
                } else /if (${FindItemCount[=${SumItem}]}==${CurrentCount} && ${SummonAttempts}>0) {
                    /echo Summoning >> ${SumItem} << Failed - Check reagents, timer, etc
                    /echo -- Turning off ${SSpell} - ${SSpell}|Summon|${SumItem}|0
                    /varset SummonAttempts 0
                    /varset SumResult CAST_COMPONENTS
                    /break
                }
            } else /if (${SumResult.Equal[CAST_COMPONENTS]}) {
                /echo You are missing components. Turning Off ${SSpell}.
                /break
            } else /if (${Select[${SumResult},CAST_NO_RESULT,CAST_RECOVER]}) {
                /break
            }
            /if (${SummonAttempts}>5) /break
        }
        /if (${Cursor.ID}) /call CheckCursor SummonStuff 0
        /if (${FindItemCount[=${SumItem}]}>${OriginalCount}) /echo I now have ${FindItemCount[=${SumItem}]} of >> ${SumItem} <<
        DEBUGBUFF SummonStuff Leave ${Me.FreeInventory} ${SumResult} ${SummonAttempts}
    /return ${SumResult}
| ----------------------------------------------------------------------------
| SUB: AE check - cast area effect spells depending on set number of mobs
| ----------------------------------------------------------------------------
    Sub AECheck(int prm_ListMobs)
        /if (!${AEOn}) /return
        /if (${Target.Type.Equal[corpse]}) /return
        /if (!${AggroTargetID}) /return
        DEBUGCOMBAT AECheck Enter
        /declare i int local
        /declare j int local
        /declare AESpell string local
        /declare AEMobCount int local
        /declare AETarget string local
        /declare AETargetID int local
        /declare AERange int local
        /declare MobCountTemp int local 0
        /declare int_SpawnCountXT int local 0
        /declare int_SpawnCountAE int local 0
        /declare s_WorkSpawn spawn local

        /call MobRadar los ${AERadius} AECheck
        | Adjust mobcount by subtracting mobs not on xtarget
        DEBUGCOMBAT AECheck Mobcount: ${MobCount} ${prm_ListMobs}
        /if (!${MobCount}) /return
        /varset MobCountTemp ${SpawnCount[npc xtarhater targetable los radius ${AERadius} zradius 50 noalert 3]}
        /if (${MobCountTemp}<=0) /return
        /if (${IAmMA} && ${TargetSwitchingOn} && ${Target.ID} && ${Target.ID}!=${MyTargetID} && ${CombatStart}) /call CombatTargetCheck 1
        /for i 1 to ${AE.Size}
            /varset AESpell ${AE[${i}].Arg[1,|]}
            /if (${AESpell.Equal[null]}) /continue
            /varset AEMobCount ${AE[${i}].Arg[2,|]}
            /varset AETarget ${AE[${i}].Arg[3,|]}
            | Check if AEspell is ready to use if not skip it
            /if (${AESpell.NotEqual[burn]} && !${Me.SpellReady[${Spell[${AESpell}].RankName}]} && !${Me.AltAbilityReady[${AESpell}]} && !${Me.CombatAbilityReady[${Spell[${AESpell}].RankName}]} && !${Me.AbilityReady[${AESpell}]} && !${AESpell.Find[command:]}) /continue
            /if (${FindItem[=${AESpell}].ID} && !${Me.ItemReady[=${AESpell}]}) /continue
            /if (${AEMobCount}<=${MobCountTemp}) {
                /if (${AETarget.Equal[null]} || ${AETarget.Equal[Mob]} || ${AETarget.Equal[Single]}) {
                    /varset AETargetID ${MyTargetID}
                } else /if (${AETarget.Equal[Me]}) {
                    /varset AETargetID ${Me.ID}
                } else /if (${AETarget.Equal[MA]}) {
                    /varset AETargetID ${Spawn[=${MainAssist}].ID}
                } else /if (${AETarget.Equal[Pet]}) {
                    /varset AETargetID ${Me.Pet.ID}
                } else /if (${AESpell.Equal[burn]} && !${BurnActive}) {
                    /echo AE-> ${MobCountTemp} Mobs: Activating BURN
                    /call Burn
                    /return
                }
                /varset AERange ${Spell[${AESpell}].AERange}
                /if (!${AERange}) {
                    /varset AERange ${AERadius}
                }
                /if (${Spawn[id ${AETargetID}].ID}) {
                    /vardata s_WorkSpawn Spawn[id ${AETargetID}].ID
                    /varset int_SpawnCountXT ${SpawnCount[npc xtarhater loc ${s_WorkSpawn.X} ${s_WorkSpawn.Y} radius ${AERange}]}
                    /varset int_SpawnCountAE ${SpawnCount[npc loc ${s_WorkSpawn.X} ${s_WorkSpawn.Y} radius ${AERange}]}
                    /if (${Me.XTarget[${XTSlot}].ID} && ${Me.XTarget[${XTSlot}].TargetType.NotEqual[Auto Hater]}) /varcalc int_SpawnCountXT ${int_SpawnCountXT}+1
                    DEBUGCOMBAT AECheck SpawnCountXT: ${int_SpawnCountXT} SpawnCountAE: ${int_SpawnCountAE} AERange: ${AERange}
                    /if (${AETarget.NotEqual[Single]}) {
                        /if (${s_WorkSpawn.ID} && ${int_SpawnCountXT} < ${int_SpawnCountAE}) {
                            /if (!${SpamTimer2}) {
                                /echo AE-> Casting ${AESpell} now would aggro more mobs(${int_SpawnCountAE}) than we have on xtarget(${int_SpawnCountXT})
                                /varset SpamTimer2 20
                            }
                            /if (${prm_ListMobs}) {
                                /for j 1 to ${int_SpawnCountAE}
                                    /vardata WorkSpawn NearestSpawn[${j},npc loc ${s_WorkSpawn.X} ${s_WorkSpawn.Y} radius ${AERange}].ID
                                    /echo Spawn Info: ${j} - ${WorkSpawn.Name} - Direction from you: ${WorkSpawn.HeadingTo} Mob X,Y: ${WorkSpawn.X},${WorkSpawn.Y}
                                /next j
                                /vardata WorkSpawn ClearSpawnValue
                            }
                            /continue
                        }
                    }
                    /if (!${ConOn} || !${AE[${i}].Find[|cond]} || ${If[${Cond[${AE[${i}].Mid[${Math.Calc[${AE[${i}].Find[|cond]}+5]},3]}]},1,0]}) {
                        /if (${AETarget.Equal[Single]}) {
                            /call CastWhat "${AESpell}" ${AETargetID} AoE 0 0
                            /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                                /echo AE-> ${AESpell} on Single target  >> ${Spawn[${AETargetID}].CleanName} <<
                            }
                            /continue
                        } else {
                            /call CastWhat "${AESpell}" ${AETargetID} AoE 0 0
                            /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                                /echo AE-> ${MobCountTemp} Mobs: Casting AE ${AESpell}
                            }
                        }
                    }
                } else {
                    /break
                }
            }
        /next i
        DEBUGCOMBAT AECheck Leave
    /return
| ----------------------------------------------------------------------------
| SUB: WriteDebuffs
| ----------------------------------------------------------------------------
    Sub WriteDebuffs
        /if (${EverQuest.GameState.NotEqual[ingame]}) /return
        /if (${DanNetOn}) /return
        DEBUGHEALS WriteDebuffs Enter - ${NeedCuring}
        /declare DebuffList string local
        /declare IAmDeBuffed int local ${Math.Calc[${Int[${Me.Poisoned.ID}]}+${Int[${Me.Diseased.ID}]}+${Int[${Me.Cursed.ID}]}+${Int[${Me.Corrupted.ID}]}+${Int[${Me.Mezzed.ID}]}+${Int[${Me.Song[Restless Curse].ID}]}]}

        /if (${IAmDeBuffed}) {
            /if (!${NeedCuring}) {
                /varset NeedCuring 1
                /varset DebuffList ${IAmDeBuffed}|${Int[${Me.Poisoned.ID}]}|${Int[${Me.Diseased.ID}]}|${Math.Calc[${Int[${Me.Cursed.ID}]}+${Int[${Me.Song[Restless Curse].ID}]}]}|${Int[${Me.Corrupted.ID}]}|${Int[${Me.Mezzed.ID}]}
                /ini "KissAssist_Buffs.ini" "${Me.ID}" Debuffs "${DebuffList}"
                DEBUGHEALS WriteDebuffs Writing debuffs to ini file
            }
        } else {
            /if (${NeedCuring}) {
                /varset NeedCuring 0
                /varset DebuffList
                /ini "KissAssist_Buffs.ini" "${Me.ID}" Debuffs "${DebuffList}"
                DEBUGHEALS WriteDebuffs Clearing debuffs from ini file
            }
        }
        DEBUGHEALS WriteDebuffs Leave - ${NeedCuring}
    /return
| -------------------------------------------------------------------------------------
| SUB: Cast Mount
| -------------------------------------------------------------------------------------
    Sub CastMount
        /if (${Me.Mount.ID} || !${MountOn} || (${HealsOn} && ${AggroTargetID})) /return
        /declare i int local
        /for i 1 to ${Buffs.Size}
        | Cast mount
        /if (${Buffs[${i}].Arg[2,|].Equal[Mount]} && !${Buffs[${i}].Find[0]} && !${Me.Mount.ID} && !${Attacking} && ${MountOn} && !${Me.FeetWet}) {
            DEBUGBUFF CheckBuffs Mount
                /if (${ConOn} && ${Buffs[${i}].Find[|cond]} && ${If[${Cond[${Buffs[${i}].Mid[${Math.Calc[${Buffs[${i}].Find[|cond]}+5]},3]}]},0,1]}) /continue
                /call CastWhat "${Buffs[${i}].Arg[1,|]}" ${Me.ID} CastMount 0 0
            /delay 1s
            /if (${Macro.Return.Equal[CAST_NOMOUNT]}) /varset MountOn 0
        }
        /next i
    /return
| -------------------------------------------------------------------------------------
| SUB: Cast Mana
| -------------------------------------------------------------------------------------
    Sub CastMana(string SentFrom)
        /if (${Me.Invis}) /return
        DEBUGBUFF CastMana Enter ${SentFrom}
        /declare i int local
        /declare CondNo int local 0
        /if (${MezOn}) {
            /varset MezBroke 0
            /doevents MezBroke
            /if (!${MezBroke}) {
                /varset GoMByPass 1
                /call DoMezStuff CastMana
                /varset GoMByPass 0
            }
        }

        /for i 1 to ${Buffs.Size}
            /if (${Buffs[${i}].Find[|0]} || ${Buffs[${i}].Equal[null]}) /continue
            /if (${Select[${Buffs[${i}].Arg[2,|]},mana,managroup]}==0) /continue
            /if (${ConOn} && ${Buffs[${i}].Find[|cond]}) {
                /varset CondNo ${Buffs[${i}].Mid[${Math.Calc[${Buffs[${i}].Find[|cond]}+5]},3]}
            } else {
                /varset CondNo 0
            }
            | Check and Cast mana type spells/aas/items - Canni/Paragon/Harvest - prevent toon from using before rez after zoning for 2 min
            /if (${Buffs[${i}].Arg[2,|].Equal[Mana]} && !${Me.Buff[Revival Sickness].ID} && !${JustZoned}) {
                /if (${Buffs[${i}].Arg[1,|].Equal[Dichotomic Psalm]} && (${IAmABard} || ${Me.CurrentEndurance}<6600)) /continue
                /if (${Me.PctMana}<=${Buffs[${i}].Arg[3,|]} && ${Me.PctHPs}>${Int[${Buffs[${i}].Arg[4,|]}]}) {
                    /if (${Cursor.ID}) /call CheckCursor CastMana 0
                    DEBUGBUFF Canni/Paragon/Harvest
                    /call CastWhat "${Buffs[${i}].Arg[1,|]}" ${Me.ID} CastMana ${CondNo} 0
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /echo Casting >> ${Buffs[${i}].Arg[1,|]} << for mana
                        /if (${AggroTargetID}) /break
                    }
                }
            } else /if (${Buffs[${i}].Arg[2,|].Equal[Managroup]} && (!${Medding} || (!${MedCombat} && ${Medding}))) {
                /call RegenOther "${Buffs[${i}].Arg[1,|]}" Mana ${Buffs[${i}].Arg[3,|]} 0 ${CondNo}
            }
        /next i
        DEBUGBUFF CastMana Leave ${SentFrom}
    /return
| ----------------------------------------------------------------------------
| SUB: Check and Fix Stuck Gems
| ----------------------------------------------------------------------------
    Sub CheckStuckGems
        /if (${Window[CastingWindow].Open} || ${Me.Invis}) /return
        /if (${Me.AltAbilityReady[origin]}) {
            /echo ***** Stuck Gem Detected ***** ${Me.SpellInCooldown}
            /alt act ${Me.AltAbility[origin].ID}
            /delay 50 ${Window[CastingWindow].Open}
            /stopcast
            /delay 10
        } else {
            /echo Origin AA not ready.
            /delay 20
        }
    /return
| ----------------------------------------------------------------------------
| SUB: Fix Burn Spells
| ----------------------------------------------------------------------------
    Sub FixBurnSpells
        /declare i        int    local 0
        /declare CondNo   int    local 0
        /declare BurnArg1 string local
        /declare BurnArg2 string local
        /declare BurnArg3 string local
        /for i 1 to ${Burn.Size}
            /if (${Burn[${i}].NotEqual[null]}) {
                /if (${Burn[${i}].Find[|cond]}) {
                    /varset CondNo ${Burn[${i}].Mid[${Math.Calc[${Burn[${i}].Find[|cond]}+5]},3]}
                } else {
                    /varset CondNo 0
                }
                /varset BurnArg1 ${Burn[${i}].Arg[1,|]}
                /if (${Select[${Burn[${i}].Arg[2,|]},mob,me,ma,pet]}) {
                    /varset BurnArg2 ${Burn[${i}].Arg[2,|]}
                } else {
                    /varset BurnArg2 mob
                }
                /if (${Select[${Burn[${i}].Arg[3,|]},1,2,3]}) {
                    /varset BurnArg3 ${Burn[${i}].Arg[3,|]}
                } else {
                    /varset BurnArg3 1
                }
                /if (${CondNo}) {
                    /varset Burn[${i}] ${BurnArg1}|${BurnArg2}|${BurnArg3}|cond${CondNo}
                } else {
                    /varset Burn[${i}] ${BurnArg1}|${BurnArg2}|${BurnArg3}
                }
            }
        /next i
    /return
