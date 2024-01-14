| -------------------------------------------------------------------------------------
|   Sub MezRadar
| -------------------------------------------------------------------------------------
    Sub MezRadar
        /if ((${DMZ} && ${Me.InInstance}==FALSE)) /return
        DEBUGMEZ MezRadar Enter
        /declare NMMob int local 0
        /declare i int local
        /varset MezMobAECount 0
        /varset MezAEClosest 0
        /varset MezMobCount ${SpawnCount[xtarhater]}
        /varset MezMobAECount ${SpawnCount[xtarhater radius ${MezRadius}]}
        /if (${Me.XTarget[${XTSlot}].TargetType.NotEqual[Auto Hater]} && ${Me.XTarget[${XTSlot}].ID}) {
            /varcalc MezMobCount ${MezMobCount}+1
            /varcalc MezMobAECount ${MezMobAECount}+1
        }
        /if (${MezMobAECount}) {
            /for i 1 to ${MezMobAECount}
                /varset NMMob ${NearestSpawn[${i},xtarhater radius ${MezRadius}].ID}
                DEBUGMEZ MezRadar:  ${i} ${NMMob} ${Spawn[id ${NMMob}].Type}
                /if (${NMMob} && ${Select[${NMMob},${MezArray[1,1]},${MezArray[2,1]},${MezArray[3,1]},${MezArray[4,1]},${MezArray[5,1]},${MezArray[6,1]},${MezArray[7,1]},${MezArray[8,1]},${MezArray[9,1]},${MezArray[10,1]},${MezArray[11,1]},${MezArray[12,1]},${MezArray[13,1]}]}==0) {
                    DEBUGMEZ MezRadar: ADDING -> Name: ${Spawn[${NMMob}].Name} ID: ${NMMob} to mezlist
                    /call AddToArray MezArray ${NMMob}
                }
            /next i
        }
        /varset MezAEClosest ${NearestSpawn[1,xtarhater radius ${MezRadius}].ID}
        /if (${DebugMez}) /delay 5
        DEBUGMEZ MezRadar: MezMobCount: ${MezMobCount} Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Do Mez Stuff
| -------------------------------------------------------------------------------------
    Sub DoMezStuff(string SentFrom)
        /if (${EventByPass}!=2) {
            /varset EventByPass 1
            /varset GoMByPass 1
            /while (1) {
                /varset EventFlag 0
                /doevents
                /if (!${EventFlag}) /break
            }
            |/doevents
            /varset MezBroke 0
            /varset EventByPass 0
            /varset GoMByPass 0
        }
        /varset GoMByPass 0
        /if (!${MezOn} || ${Me.Hovering} || (!${MyTargetID} && ${Spawn[=${MainAssist}].ID} && ${Spawn[=${MainAssist}].Type.NotEqual[Mercenary]}) || ${DMZ} && ${Me.InInstance}==FALSE) /return
        DEBUGMEZ DoMezStuff: Enter ${SentFrom}
        /declare i int local 0
        /declare j int local 0
        /declare skipFlag int local 0
        /declare mezCombat bool local False
        | Reset skip mez on health setting if tank dies to 1%
        /if (!${Spawn[=${MainAssist}].ID}) /varset MezStopHPs 1
        /if (${Me.Combat} || ${Me.CombatState.Equal[combat]} || ${DPSOn}) /varset mezCombat True
        /call MezRadar
        DEBUGMEZ DoMezStuff MezMobCount ${MezMobCount} ${mezCombat}
        /if (${MezMobCount}<2 && ${Spawn[=${MainAssist}].ID}) {
            DEBUGMEZ MezMobCount was ${MezMobCount} (so less than 2) & ${MainAssist} is alive so we return
            /varset MezMobDone 1
            /return
        }
        | Necros can't AE mez.
        | MezOn - 0=Off/1=Single & AE Mez/2=Single Mez only/3=AE Mez only.
        /if (${Select[${Me.Class.ShortName},BRD,ENC]}) {
            /if (${Select[${MezOn},1,3]} && ${MezAECount}>0 && ${MezMobAECount}>=${MezAECount} && ${MezAETimer}==0) {
                DEBUGMEZ I'm about to AEMez
                /varset MezMobAECount ${SpawnCount[npc xtarhater loc ${Spawn[id ${MezAEClosest}].X} ${Spawn[id ${MezAEClosest}].Y} radius ${Spell[${MezAESpell}].AERange}]}
                | Check if Mezzer out of group and using XTSlot to assist MainAssist. Add 1 mob to count.
                /if (${Me.XTarget[${XTSlot}].TargetType.NotEqual[Auto Hater]} && ${Me.XTarget[${XTSlot}].Type.NotEqual[pc]}) /varcalc MezMobAECount ${MezMobAECount}+1
                /if (${MezMobAECount}>=${SpawnCount[npc loc ${Spawn[id ${MezAEClosest}].X} ${Spawn[id ${MezAEClosest}].Y} radius ${Spell[${MezAESpell}].AERange}]}) /call MezMobsAE ${MezAEClosest}
            }
        }
        /for i 1 to ${XSlotTotal}
            /varset skipFlag 0
            | Every Mez test conditon is listed seperately for clarity
            DEBUGMEZ we are in the mezcondition loop i is: ${i} ${MezTimer${i}} ${Spawn[${MezArray[${i},1]}].ID}
            | Test -> Is my single mez spell ready
            /if (!${Me.SpellReady[${MezSpell}]} && !${IAmABard}) {
                /if (${MezOn}!=2) {
                    DEBUGMEZ ${MezSpell} not ready, return
                    /return
                }
                /while (!${Me.SpellReady[${MezSpell}]}) {
                    DOPARSE
                    /delay 2
                }
            }
            | Test -> array not empty,
            /if (${MezArray[${i},1].Equal[NULL]}) {
                DEBUGMEZ ${i}  MezArray[${i},1] was equal to NULL, Skip
                /continue
            }
            | Test -> Target is dead
            /if (!${Spawn[${MezArray[${i},1]}].ID} || ${Spawn[${MezArray[${i},1]}].Type.Equal[corpse]}) {
                DEBUGMEZ ${i}  ${MezArray[${i},3]} is dead, Skip
                /call RemoveFromArray MezArray ${i}
                /continue
            }
            | Is mob in MezRadius distance
            /if (${Spawn[${MezArray[${i},1]}].Distance}>=${MezRadius}) {
                DEBUGMEZ ${i}  Mob distance is greater than MezRadius: ${Spawn[${MezArray[${i},1]}].Distance} ${MezRadius} Skip
                /call RemoveFromArray MezArray ${i}
                /continue
            }
            | Test -> is target MA's current Target
            /if (${Spawn[${MezArray[${i},1]}].ID}==${MyTargetID} && ${Spawn[=${MainAssist}].ID}) {
                DEBUGMEZ ${Spawn[${MezArray[${i},1]}].Name} had the same ID(${Spawn[${MezArray[${i},1]}].ID}) as MyTargetID(${MyTargetID}) & ${MainAssist}(${Spawn[=${MainAssist}].ID}), Skip
                /call RemoveFromArray MezArray ${i}
                /continue
            }
            | Test -> is  MA's a merc skip 1st target so he attackes it.
            /if (${AggroTargetID} && !${MyTargetID} && ${Spawn[=${MainAssist}].ID} && ${Spawn[=${MainAssist}].Type.Equal[Mercenary]}) {
                DEBUGMEZ AggroTargetID = ${AggroTargetID}  MyTargetID = ${MyTargetID} & Spawn[=MainAssist].ID = ${Spawn[=${MainAssist}].ID} & Spawn[=MainAssist].Type.Equal[Mercenary] was true, Skip
                /call RemoveFromArray MezArray ${i}
                /continue
            }
            | Test -> is taget above mez hps threshold
            /if (${Spawn[${MezArray[${i},1]}].PctHPs}<${MezStopHPs}) {
                DEBUGMEZ Spawn[MezArray[${i},1]].PctHPs(${Spawn[${MezArray[${i},1]}].PctHPs}) was less than MezStopHPs(${MezStopHPs}), Skip
                /call RemoveFromArray MezArray ${i}
                /continue
            }
            | Test -> is target within levels defined in ini file
            /if (${MezArray[${i},2]}>${MezMaxLevel} || ${MezArray[${i},2]}<${MezMinLevel}) {
                DEBUGMEZ MezArray[${i},2](${MezArray[${i},2]}) was greater than MezMaxLevel(${MezMaxLevel}) OR less than MezMinLevel(${MezMinLevel}), Skip
                /call RemoveFromArray MezArray ${i}
                /continue
            }
            | Test -> is the target in line of sight
            /if (!${Spawn[${MezArray[${i},1]}].LineOfSight}) {
                DEBUGMEZ I dont have LineOfSight to Spawn[MezArray[${i},1]](${Spawn[${MezArray[${i},1]}].Name} ID:${Spawn[${MezArray[${i},1]}].ID}), Skip
                /continue
            }
            | Test -> I am a bard and if the player is dumb enough to have me tank then don't mez.
            /if (${IAmABard} && ${IAmMA} && ${MyTargetID} && ${AggroTargetID} && ${MezArray[${i},1].Equal[${MyTargetID}]}) {
                DEBUGMEZ im a bard and stuff, Skip
                /continue
            }
            | Test -> is target a giant unmezzable
            /if (${Spawn[${MezArray[${i},1]}].Body.Name.Equal[Giant]}) {
                DEBUGMEZ Spawn[MezArray[${i},1]] (${Spawn[${MezArray[${i},1]}].Name} ID:${MezArray[${i},1]}) is a giant, Skip
                /continue
            }
            | Test -> is target on my mez immune list
            /if (${Alert[4].Size}) {
                /for j 0 to ${Alert[4].Size}
                    /if (${Alert[4].List[${j}].Name.Equal[${MezArray[${i},3]}]} && ${Alert[4].List[${j}].Name.Length}) {
                        /if (${MMTimer${i}}==0) /call BroadCast g "MEZ Immune Detected -> ${MezArray[${i},3]} <- ID:${MezArray[${i},1]}"
                        /varset MMTimer${i} 1m
                        DEBUGMEZ ${Spawn[${MezArray[${i},1]}].Name} ID:${MezArray[${i},1]} is immune, Skip
                        /varset skipFlag 1
                        /break
                    }
                /next j
                /if (${skipFlag}) {
                    /call RemoveFromArray MezArray ${i}
                    /continue
                }
            }
            | Test -> Do i have enough mana to cast the spell
            /if (${Me.CurrentMana}<${Spell[${MezSpell}].Mana}) {
                DEBUGMEZ i didnt have enough mana to cast ${MezSpell}, Skip
                /continue
            }
            | Test -> Do i have a mez timer on the mob?
            /if (${MezTimer${i}} > 0) {
                DEBUGMEZ MezTimer${i}(${MezTimer${i}}) was greater than 0 , Skip
                /continue
            }
            | Stop mezzing last mob because pets and mercs won't attack it.
            /if (${MezMobCount}<=1 && ${Spawn[=${MainAssist}].ID} && (${Spawn[=${MainAssist}].Type.Equal[Mercenary]} || ${Spawn[=${MainAssist}].Type.Equal[Pet]})) {
                DEBUGMEZ MezMobCount(${MezMobCount}) was less or equal to 1 & ( MainAssist(${MainAssist}) was a Mercenary OR a Pet ), Skip
                /continue
            }
            /if (${Spawn[${MainAssist} ${MainAssistType} group].ID} && ${Select[${MezArray[${i},1]},XTCHECK]}==0) {
                DEBUGMEZ If tank is alive and mob not on xtarget Skip
                /continue
            }
            /if (${MezImmuneIDs.Find[|${MezArray[${i},1]}]}) {
                DEBUGMEZ Mez Immune Mob Detected: ${MezArray[${i},3]} Skip
                /call RemoveFromArray MezArray ${i}
                /continue
            }
            /if (${IAmABard}) {
                /attack off
            }
            DEBUGMEZ MezTimer ${i} ${MezTimer${i}} ${Spawn[${MezArray[${i},1]}].ID}
            /if (${Select[${MezOn},1,2]}) {
                /doevent AttackCalled
                /if (${CalledTargetID}) {
                     DEBUGMEZ "--> Checking target before turning on attack!! <--"
                    /call CombatTargetCheck 0
                    /if (${MyTargetID}==${MezArray[${i},1]}) /continue
                }
                /if (${Me.Invis}) /makemevisible

				DEBUGMEZ im gonna singlemez ${MezArray[${i},1]} ${Me.Invis}
				/if (${Me.Invis}) /makemevisible
                /call MezMobs ${MezArray[${i},1]} ${i}
                /varset MezMobDone 1
                /if (${Macro.Return.Equal[CAST_CANCELLED]} && ${MezAETimer}==0 && ${MezOn}==1) {
                    DEBUGMEZ I'm about to AEMez-
                    /call MezMobsAE ${MezAEClosest}
                    /varcalc i ${i}-1
                }
			}
        }
        /next i
        /if (!${Target.ID} || ${Target.ID}!=${MyTargetID}) {
            /if (${IAmABard} && ${CombatStart} && ${Bool[${Target.Mezzed}]}) {
               | /varset MyTargetID ${Int[${Me.GroupAssistTarget.ID}]}
                /call CombatTargetCheck1 1
              /if (${MedleyOn}) /squelch /medley
            } else {
                /if (${mezCombat}) /call CombatTargetCheck 1
            }	|-> Do we want to Charm stuff? if So call CharmStuff after mezzing *its safer that way*
        }
		/if (${CharmOn}==1 && !${Me.Pet.ID}) {
			/call DoCharmStuff DoMezStuff
		}
		DEBUGMEZ DoMezStuff: Leave
    /return
    
| -------------------------------------------------------------------------------------
| SUB: Mez Mobs AE
| -------------------------------------------------------------------------------------
    Sub MezMobsAE(int AEMezID)
        | Bard AE mez code
        /declare i int local 1
        /declare WasChasing int local 0
        /if (!${Me.AltAbility[${MezAESpell}]} && !${Me.Book[${MezAESpell}]}) {
            /echo Invalid AEMezSpell ${MezAESpell}. Check the spelling.
            /return
        }
        /if (${IAmABard}) {
            /if (${Target.ID}) /varset AEMezID ${Target.ID}
            /attack off
            /call CastWhat "${MezAESpell}" ${AEMezID} Mez 0 0
            /call BroadCast g "MEZZING-> AE Mezzing Now - ${MezAESpell} "
            /varset MezAETimer 300
        }
        | Enchanter AE mez code
        /if (${Me.Class.Name.Equal[Enchanter]}) {
            /if (${ChaseAssist}) {
                /varset ChaseAssist 0
                /varset WasChasing 1
                /squelch /stick off
                /moveto off
                /if (${PullMoveUse.Equal[nav]}) {
                    /if (${Navigation.Active}) /nav stop
                }
                /delay 30 !${Me.Moving}
            }
            /echo I AM AE MEZZING ${MezAESpell}
            /while (!${Me.SpellReady[${MezAESpell}]}) {
                /delay 2
            }
            /call BroadCast g "Casting AE Mez-> ${MezAESpell}"
            /call CastWhat "${MezAESpell}" ${AEMezID} Mez 0 0
            /if (${Select[${Macro.Return},CAST_SUCCESS,CAST_RESISTED,CAST_IMMUNE]}) {
                /echo I JUST CAST AE MEZ ${MezAESpell}
                /varset MezAETimer ${Spell[${MezAESpell}].Duration.TotalSeconds}s
                DEBUGMEZ TIMER SET ${MezAETimer}
                /call BroadCast g "AE MEZ Complete-> ${MezAESpell}"
            } else {
                /call BroadCast g "AE MEZ ${MezAESpell} Failed -> ${Macro.Return}"
                /varset MezAETimer ${Me.GemTimer[${MezAESpell}].TotalSeconds}s
            }
            /if (${WasChasing}) /varset ChaseAssist 1
        }
        | Reset all mez timers to 0 after AE Mez
        /for i 1 to 30
            /varset MezTimer${i} 0
        /next i
    /return
| -------------------------------------------------------------------------------------
| SUB: Mez Mobs
| -------------------------------------------------------------------------------------
    Sub MezMobs(int MobID, int TimerNum)
        DEBUGMEZ MezMobs Enter MobID:${MobID} Timer#:${TimerNum}
        /declare MezFail int local 0
        /declare MezTry int local 1
        /declare ReMez int local 0
        /declare wasCombat int local 0
        /if (${Me.Combat}) {
            /attack off
            /varset wasCombat 1
            /delay 25 !${Me.Combat}
        }
        | Added this check because of timing. Mob to mez was getting DOTed because of MeleeTwist.
        /if (${IAmABard} && ${MeleeTwistOn}) /call CastBardCheck 0
        /squelch /target id ${MobID}
        /delay 20 ${Target.ID}==${MobID} && ${Target.BuffsPopulated}
        /if (${Target.ID}==${MobID}) {
            /if (${Int[${Target.Mezzed.ID}]} && ${Target.Mezzed.Name.Equal[${MezSpell}]}) {
                DEBUGMEZ MezTimer${TimerNum} ${MezTimer${TimerNum}} Target Info: ${Target.Mezzed.ID} ${Target.Mezzed.Name} ${Target.BuffDuration[${Target.Mezzed.Name}].TotalSeconds} Line#: ${Macro.CurLine}
                /if (${Target.BuffDuration[${Target.Mezzed.Name}].TotalSeconds}>${Math.Calc[${Spell[${MezSpell}].MyDuration.TotalSeconds}*.10]}) {
                    /varcalc MezCount[${TimerNum}] 1
                    /varcalc MezTimer${TimerNum} (${Target.BuffDuration[${Target.Mezzed.Name}].TotalSeconds}*10)*.85
                    /return
                }
            }
            /if (${MezCount[${TimerNum}]}<1) {
                /call BroadCast g "MEZZING-> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
            } else {
                /call BroadCast g "ReMEZZING-> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                /varset ReMez 1
            }
            | Chanter mez code
            /if (${Me.Class.Name.Equal[Enchanter]}) {
                /while (1) {
                    /call CastWhat "${MezSpell}" ${MobID} MezMobs 0 0
                    /varcalc MezFail ${MezFail}+1
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /if (!${ReMez}) {
                            /call BroadCast g "JUST MEZZED -> ${MezSpell} on ${Spawn[${MobID}].CleanName}:${MobID}"
                        } else {
                            /call BroadCast g "JUST REMEZZED -> ${MezSpell} on ${Spawn[${MobID}].CleanName}:${MobID}"
                        }
                        /varcalc MezCount[${TimerNum}] ${MezCount[${TimerNum}]}+1
                        /varcalc MezTimer${TimerNum} (${Spell[${MezSpell}].MyDuration.TotalSeconds}*10)*.90
                        DEBUGMEZ MezTimer${TimerNum} ${MezTimer${TimerNum}}
                    } else /if (${Macro.Return.Equal[CAST_RESISTED]} && ${MezFail}<2) {
                        /call BroadCast g "MEZ Resisted -> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                        | Added for Debuffing Resistant Mobs while Mezzing.
                        /if (${MezDebuffOnResist} && !${Target.Tashed.ID}) {
                            /while (${Me.SpellInCooldown}) {
                                /delay 3
                            }
                            /call CastWhat "${MezDebuffSpell}" ${MobID} MezDebuff 0 0
                            /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                                /call BroadCast g "MEZ Debuffed -> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                                /while (${Me.SpellInCooldown}) {
                                    /delay 3
                                }
                            }
                        }
                        /continue
                    } else /if (${Macro.Return.Equal[CAST_IMMUNE]}) {
                        /if (!${MezImmuneIDs.Find[|${Target.ID}]}) /call AddMezImmune ${Target.ID}
                        /if (${AAMezImmune}) /call Bind_AddMezImmune ${Target.CleanName}
                    } else /if (${Macro.Return.Equal[CAST_CANCELLED]}) {
                        /if (${CastingInterruptOn}) /return CAST_CANCELLED
                    }
                    /break
                }
            | Bard mez code
            } else /if (${IAmABard}) {
                /while (1) {
                    /if (${Me.Combat}) {
                        /varset wasCombat 1
                        /attack off
                        |--- turn off attack you silly bard, or  you will break your mez. 
                        /delay 35 !${Me.Combat}
                        /call CastWhat "${MezSpell}" ${MobID} MezMobs 0 0
                    } else {
                        /call CastWhat "${MezSpell}" ${MobID} MezMobs 0 0
                    }
                    /varcalc MezFail ${MezFail}+1
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /if (!${ReMez}) {
                            /call BroadCast g "JUST MEZZED -> ${MezSpell} on ${Spawn[${MobID}].CleanName}:${MobID}"
                        } else {
                            /call BroadCast g "JUST REMEZZED -> ${MezSpell} on ${Spawn[${MobID}].CleanName}:${MobID}"
                        }
                        
                        /squelch /target clear
                        /varcalc MezCount[${TimerNum}] ${MezCount[${TimerNum}]}+1
                        /varcalc MezTimer${TimerNum} (${Spell[${MezSpell}].MyDuration.TotalSeconds}*10)*.90
                        DEBUGMEZ MezTimer${TimerNum} ${MezTimer${TimerNum}} ${Macro.Return}
                    } else /if (${Macro.Return.Equal[CAST_RESISTED]} && ${MezFail}<2) {
                        /call BroadCast g "MEZ Resisted -> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                        DEBUGMEZ Cast Resisted ${Macro.Return}
                        /continue
                    } else /if (${Macro.Return.Equal[CAST_IMMUNE]}) {
                        /if (!${MezImmuneIDs.Find[|${Target.ID}]}) /call AddMezImmune ${Target.ID}
                        /if (${AAMezImmune}) /call Bind_AddMezImmune ${Target.CleanName}
                        DEBUGMEZ Cast Immune ${Macro.Return}
                    } else /if (${Macro.Return.Equal[CAST_CANCELLED]}) {
                        DEBUGMEZ Cast Cancelled ${Macro.Return}
                        /if (${CastingInterruptOn}) /return CAST_CANCELLED
                    }
                    /if (${wasCombat}) {
                        /varset wasCombat 0
                       | /delay 15
                        /call CombatTargetCheck 0
                        |/attack on
                    }
                    /break
                }
            | Necro mez code
            } else /if (${Me.Class.Name.Equal[Necromancer]}) {
                /while (1) {
                    /call CastWhat "${MezSpell}" ${MobID} Mez 0 0
                    /varcalc MezFail ${MezFail}+1
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /varcalc MezCount[${TimerNum}] ${MezCount[${TimerNum}]}+1
                        /varcalc MezTimer${TimerNum} (${Spell[${MezSpell}].MyDuration.TotalSeconds}*10)*.95
                        DEBUGMEZ MezMobs MezTimer${TimerNum} ${MezTimer${TimerNum}}
                        | Necro flag to release from mez loop since spell has 6 sec refresh.
                    } else /if (${Macro.Return.Equal[CAST_RESISTED]} && ${MezFail}<2) {
                        /call BroadCast g "MEZ Resisted -> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                        /continue
                    } else /if (${Macro.Return.Equal[CAST_IMMUNE]}) {
                        /if (!${MezImmuneIDs.Find[|${Target.ID}]}) /call AddMezImmune ${Target.ID}
                        /if (${AAMezImmune}) /call Bind_AddMezImmune ${Target.CleanName}
                    }
                    /break
                }
            }
            /varset MezTry 0
        }
        
        DEBUGMEZ MezMobs Leave
    /return
| ----------------------------------------------------------------------------
| SUB: Create Timers Mez
| ----------------------------------------------------------------------------
    Sub CreateTimersMez
        | Declare timers for mezzing
        /if (${Select[${Me.Class.ShortName},BRD,ENC,NEC]}) {
            /declare l int local
            /for l 1 to 30
                /if (!${Defined[MezTimer${l}]}) {
                    /declare MezTimer${l} timer outer 0
                    /declare MMTimer${l} timer outer 0
                } else {
                    /varset MezTimer${l} 0
                    /varset MMTimer${l} 0
                }
            /next l
        }
    /return
