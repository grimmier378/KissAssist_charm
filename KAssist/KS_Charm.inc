|-------------------------------------------------------------------------------------
|   Sub CharmRadar
| -------------------------------------------------------------------------------------
   Sub CharmRadar
        /if ((${DMZ} && ${Me.InInstance}==FALSE && !${CharmOn})) /return
        DEBUGCHARM CharmRadar Enter
        /declare NCMob int local 0
        /declare i int local
        /varset CharmMobAECount 0
        /varset CharmAEClosest 0
        /varset CharmMobCount ${SpawnCount[xtarhater]}
        /varset CharmMobAECount ${SpawnCount[xtarhater radius ${CharmRadius}]}
        /if (${Me.XTarget[${XTSlot}].TargetType.NotEqual[Auto Hater]} && ${Me.XTarget[${XTSlot}].ID}) {
            /varcalc CharmMobCount ${CharmMobCount}+1
            /varcalc CharmMobAECount ${CharmMobAECount}+1
        }
        /if (${CharmMobAECount}) {
            /for i 1 to ${CharmMobAECount}
                /varset NCMob ${NearestSpawn[${i},xtarhater radius ${CharmRadius}].ID}
                DEBUGCHARM CharmRadar:  ${i} ${NCMob} ${Spawn[id ${NCMob}].Type}
                /if (${NCMob} && ${Select[${NCMob},${CharmArray[1,1]},${CharmArray[2,1]},${CharmArray[3,1]},${CharmArray[4,1]},${CharmArray[5,1]},${CharmArray[6,1]},${CharmArray[7,1]},${CharmArray[8,1]},${CharmArray[9,1]},${CharmArray[10,1]},${CharmArray[11,1]},${CharmArray[12,1]},${CharmArray[13,1]}]}==0) {
                    DEBUGCHARM CharmRadar: ADDING -> Name: ${Spawn[${NCMob}].Name} ID: ${NCMob} to charmlist
                    /call AddToArray CharmArray ${NCMob}
                }
            /next i
        }
        /varset CharmAEClosest ${NearestSpawn[1,xtarhater radius ${CharmRadius}].ID}
        /if (${DebugCharm}) /delay 5
        DEBUGCHARM CharmRadar: CharmMobCount: ${CharmMobCount} Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Do Charm Stuff
| This is mostly for Druids and Necros but any charming class with MezOn turned off can use.
| Mezzing Classes will perform a slightly faster calling this during the mez routine. 
| -------------------------------------------------------------------------------------
    Sub DoCharmStuff(string SentFrom)
		DEBUGN DoCharmStuff: Check for a pet? = ${Me.Pet.ID}
		/if (${Me.Pet.ID}>0 || !${CharmOn}) /return
		
		/varset PetActiveState 0
        /if (${EventByPass}!=2) {
            /varset EventByPass 1
            /varset GoMByPass 1
            /while (1) {
                /varset EventFlag 0
                /doevents
                /if (!${EventFlag}) /break
            }
            |/doevents
			/varset CharmBroke 0
            /varset EventByPass 0
            /varset GoMByPass 0
        }
        /varset GoMByPass 0
        /if (!${CharmOn} || ${Me.Hovering} || (!${MyTargetID} && ${Spawn[=${MainAssist}].ID} && ${Spawn[=${MainAssist}].Type.NotEqual[Mercenary]}) || ${Me.Pet.ID}>0 || ${DMZ} && ${Me.InInstance}==FALSE) /return
        DEBUGN DoCharmStuff: Enter ${SentFrom}
        /declare i int local 0
        /declare j int local 0
        /declare skipFlag int local 0
        /declare CharmCombat bool local False
		/varset CharmMobDone 0
        | Reset skip Charm on health setting if tank dies to 1%
        /if (${Me.Combat} || ${Me.CombatState.Equal[combat]} || ${DPSOn}) /varset CharmCombat True
        /call CharmRadar
        DEBUGN DoCharmStuff CharmMobCount ${CharmMobCount} ${CharmCombat}
|        /if ((${CharmMobCount}<2) && (${CharmKeep}==0)) {
|            DEBUGN CharmMobCount was ${CharmMobCount} (so less than 2) & ${MainAssist} is alive so we return
|            /varset CharmMobDone 1
|            /return
|        }
		/if (${CharmKeep}==0) /varset CharmPet 0
        | CharmOn - 0=Off/1=On
        /for i 1 to ${XSlotTotal}
            /varset skipFlag 0
            | Every Charm test conditon is listed seperately for clarity
            DEBUGN we are in the Charm condition loop i is: ${i} ${Spawn[${CharmArray[${i},1]}].ID}
            | Test -> Is my Charm spell ready
            /while (${Me.SpellInCooldown}) {
                /delay 3
            }
			/if ((!(${Me.SpellReady[${CharmSpell}]} || ${Me.AltAbilityReady[${CharmSpell}]})) && !${IAmABard}) {
               /if (${CharmOn}) {
                   DEBUGN ${CharmSpell} state ${Me.SpellReady[${CharmSpell}]} ${Me.AltAbilityReady[${CharmSpell}]} not ready, return
                   /return
              }
               /while (!(${Me.SpellReady[${CharmSpell}]} || ${Me.AltAbilityReady[${CharmSpell}]})) {
                   DOPARSE
                  /delay 3
				  
              }
			} 
			/if (!${Me.Gem[${CharmSpell}]}) {
               /if (${CharmOn}) {
                    /MemSpell ${MiscGem} "${CharmSpell}"
                    /delay 150 ${Me.Gem[${MiscGem}].Name.Equal[${CharmSpell}]}
                   DEBUGN ${CharmSpell} state ${Me.SpellReady[${CharmSpell}]} ${Me.AltAbilityReady[${CharmSpell}]} not ready, return
                   /return
              }
         |--     /while (!${Me.Gem[${CharmSpell}]}) {
         |--         DOPARSE
         |--        /delay 3
                DEBUGN ${CharmSpell} state ${Me.SpellReady[${CharmSpell}]} ${Me.AltAbilityReady[${CharmSpell}]} not ready, return
				  /return
              }
			} 

            | Test -> array not empty,
            /if (${CharmArray[${i},1].Equal[NULL]}) {
                DEBUGN ${i}  CharmArray[${i},1] was equal to NULL, Skip
                /continue
            }
            | Test -> Target is dead
            /if (!${Spawn[${CharmArray[${i},1]}].ID} || ${Spawn[${CharmArray[${i},1]}].Type.Equal[corpse]}) {
                DEBUGN ${i}  ${CharmArray[${i},3]} is dead, Skip
                /call RemoveFromArray CharmArray ${i}
                /continue
            }
			| Test -> Charm Keep
            /if ((${Spawn[${CharmArray[${i},1]}].ID}!=${CharmPet}) && (${CharmKeep}==1 && (${CharmPet}))) {
                DEBUGN ${i}  ${CharmArray[${i},3]} is not the pet we want to ReCharm, Skip
                /call RemoveFromArray CharmArray ${i}
                /continue
            }
            | Test -> is target MA's current Target
            /if (${Spawn[${CharmArray[${i},1]}].ID}==${MyTargetID} && ${Spawn[=${MainAssist}].ID} && (${CharmKeep}==0 && !${CharmPet})) {
                DEBUGN ${Spawn[${CharmArray[${i},1]}].Name} had the same ID(${Spawn[${CharmArray[${i},1]}].ID}) as MyTargetID(${MyTargetID}) & ${MainAssist}(${Spawn[=${MainAssist}].ID}), Skip
                /call RemoveFromArray CharmArray ${i}
                /continue
            }

            | Test -> is target within levels defined in ini file
            /if (${CharmArray[${i},2]}>${CharmMaxLevel} || ${CharmArray[${i},2]}<${CharmMinLevel}) {
                DEBUGN CharmArray[${i},1](${CharmArray[${i},2]}) was greater than CharmMaxLevel(${CharmMaxLevel}) OR less than CharmMinLevel(${CharmMinLevel}), Skip
                /call RemoveFromArray CharmArray ${i}
                /continue
            }
			|Test -> mob type for necros
			/if (${Me.Class.ShortName.Equal[NEC]} && ${Spawn[${CharmArray[${i},1]}].Body.Name.NotEqual[Undead]}) {
				DEBUGN I'm a Necro CharmArray[${i},1](${CharmArray[${i},1]}) is NOT UNDEAD, Skip
				/call RemoveFromArray CharmArray ${i}
				/continue
			}
			|Test -> mob type for Druids
			/if (${Me.Class.ShortName.Equal[DRU]} && ${Spawn[${CharmArray[${i},1]}].Body.Name.NotEqual[Animal]}) {
				DEBUGN I'm a Druid CharmArray[${i},1](${CharmArray[${i},1]}) is NOT AN ANIMAL, Skip
				/call RemoveFromArray CharmArray ${i}
				/continue				
			}
            | Test -> is the target in line of sight
            /if (!${Spawn[${CharmArray[${i},1]}].LineOfSight}) {
                DEBUGN I dont have LineOfSight to Spawn[CharmArray[${i},1]](${Spawn[${CharmArray[${i},1]}].Name} ID:${Spawn[${CharmArray[${i},1]}].ID}), Skip
                /continue
            }
            | Test -> I am a bard and if the player is dumb enough to have me tank then don't Charm.
            /if (${IAmABard} && ${IAmMA} && ${MyTargetID} && ${AggroTargetID} && ${CharmArray[${i},1].Equal[${MyTargetID}]}) {
                DEBUGN im a bard and stuff, Skip
                /continue
            }
            | Test -> is target on my Charm immune list
            /if (${Alert[4].Size}) {
                /for j 0 to ${Alert[4].Size}
                    /if (${Alert[4].List[${j}].Name.Equal[${CharmArray[${i},3]}]} && ${Alert[4].List[${j}].Name.Length}) {
                        /if (${CMTimer${i}}==0) /call BroadCast g "Charm Immune Detected -> ${CharmArray[${i},3]} <- ID:${CharmArray[${i},1]}"
                        /varset CMTimer${i} 1m
                        DEBUGN ${Spawn[${CharmArray[${i},1]}].Name} ID:${CharmArray[${i},1]} is immune, Skip
                        /varset skipFlag 1
                        /break
                    }
                /next j
                /if (${skipFlag}) {
                    /call RemoveFromArray CharmArray ${i}
                    /continue
                }
            }
			|Test -> Is old pet in range?
			/if ((${Spawn[${CharmArray[${i},1]}].Distance}>${CharmRadius}) && (${Spawn[${CharmArray[${i},1]}].ID}==${CharmPet})) {
				DEBUGN OldPet Distance: ${Spawn[${CharmArray[${i},1]}].Distance} outside CharmRadius: ${CharmRadius}, Skip
                /continue
			}
			|
            | Test -> Do i have enough mana to cast the spell
            /if (${Me.CurrentMana}<${Spell[${CharmSpell}].Mana}) {
                DEBUGN i didnt have enough mana to cast ${CharmSpell}, Skip
                /continue
            }
            /if (${Spawn[${MainAssist} ${MainAssistType} group].ID} && ${Select[${CharmArray[${i},1]},XTCHECK]}==0) {
                DEBUGN If tank is alive and mob not on xtarget Skip
                /return
            }
            /if (${CharmImmuneIDs.Find[|${CharmArray[${i},1]}]}) {
                DEBUGN Charm Charm Immune Mob Detected: ${CharmArray[${i},3]} Skip
                /call RemoveFromArray CharmArray ${i}
                /continue
            }
            /if (${IAmABard}) {
                /squelch /attack off
            }
            /if (!${Defined[CharmTimer1]}) /call CreateTimersCharm
            DEBUGN CharmTimer ${i} ${CharmTimer${i}} ${Spawn[${CharmArray[${i},1]}].ID}
            /if (${Select[${CharmOn},1]}) {
                /doevent AttackCalled
                /if (${CalledTargetID}) {
                    /call CombatTargetCheck 0
					/target ${CharmArray[${i},1]}
                    /if (${MyTargetID}==${CharmArray[${i},1]}) /continue
                }
                /if (${Me.Invis}) /makemevisible
                |/echo ${CharmKeep} ${CharmPet}
				|DEBUGN im gonna Charm ${CharmArray[${i},1]}				
				DEBUGN ${Spawn[${CharmArray[${i},1]}].CleanName} Meets the Charming Chriteria - Attempting To Charm, Skip
				/call CharmMobs ${CharmArray[${i},1]} ${i}
				/call PetStateCheck

				/if (${Me.Pet.ID}) {
                    /squelch /pet back off
					/varset PetActiveState 1
                    /if (${IAmABard}) /varset PetOn 1
                    /if (${IAmABard}) /varset PetCombatOn 1
					/call BroadCast g "*** Charm Sucessfull ***"						
					/varset CharmMobDone 1
					/squelch /pet back off
					/if (${CharmKeep}) {
                        /varset CharmPet ${Me.Pet.ID}
                        /echo "Charm Pet ID: ${CharmPet}"
                    }
                    /if (${IAmABard} && ${CombatStart} && ${Bool[${Target.Charmed}]}) {
                        | /varset MyTargetID ${Int[${Me.GroupAssistTarget.ID}]}
                        /call CombatTargetCheck1 1
                         |/if (${MedleyOn}) /squelch /medley
                    } else {
                        /call CombatTargetCheck 1
                    }
					/return
				}	
				/continue	
            } else {
                |--we're not charming leave
                /return
            }
			/if (!${Me.Pet.ID}) {
				/varset PetActiveState 0
				/call BroadCast g "** CHARM FAIL **"
				/if (${CharmKeep} && ${i}==${XSlotTotal}) {
					/call BroadCast p "Old pet is Not in Range!"
				|--	/varset CharmPet 0
				|--	/echo "CharmPetID: ${CharmPet}"
				}               
            }
            /next i
		DEBUGN DoCharmStuff: Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Charm Mobs
| -------------------------------------------------------------------------------------
    Sub CharmMobs(int MobID, int TimerNum)
        DEBUGN CharmMobs Enter MobID:${MobID} Timer#:${TimerNum}
        /declare CharmFail int local 0
        /declare CharmTry int local 1
        /declare ReCharm int local 0
        /if (${Me.Combat}) {
            /squelch /attack off
            /delay 25 !${Me.Combat}
        }
        /squelch /target id ${MobID}
        /delay 20 ${Target.ID}==${MobID} && ${Target.BuffsPopulated}
        /if (${Target.ID}==${MobID}) {
            /if (${Int[${Target.Charmed.ID}]} && ${Target.Charmed.Name.Equal[${CharmSpell}]}) {
                DEBUGCHARM CharmTimer${TimerNum} ${CharmTimer${TimerNum}} Target Info: ${Target.Charmed.ID} ${Target.Charmed.Name} ${Target.BuffDuration[${Target.Charmed.Name}].TotalSeconds} Line#: ${Macro.CurLine}
                /if (${Target.BuffDuration[${Target.Charmed.Name}].TotalSeconds}>${Math.Calc[${Spell[${CharmSpell}].MyDuration.TotalSeconds}*.10]}) {
                    /varcalc CharmCount[${TimerNum}] 1
                    /varcalc CharmTimer${TimerNum} (${Target.BuffDuration[${Target.Charmed.Name}].TotalSeconds}*10)*.99
                    /return
                }
            }
            /if ((${CharmCount[${TimerNum}]}<1)) {
                /call BroadCast g "CHARMING-> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
				
            } else {
                /call BroadCast g "ReCHARMING-> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                /varset ReCharm 1
            }
			| Charm Code
                /while (1) {
                    /call CastWhat "${CharmSpell}" ${MobID} CharmMobs 0 0
                    /varcalc CharmFail ${CharmFail}+1
                    /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                        /if (!${ReCharm}) {
                            /call BroadCast g "JUST CHARMED -> ${CharmSpell} on ${Spawn[${MobID}].CleanName}:${MobID}"
                        } else {
                            /call BroadCast g "JUST RECHARMED -> ${CharmSpell} on ${Spawn[${MobID}].CleanName}:${MobID}"
                        }
						/call PetStateCheck
                        /varcalc CharmCount[${TimerNum}] ${CharmCount[${TimerNum}]}+1
						/varcalc CharmTimer${TimerNum} (${Spell[${CharmSpell}].MyDuration.TotalSeconds}*10)*.90
												
                       DEBUGN CharmTimer${TimerNum} ${CharmTimer${TimerNum}}
                    } else /if (${Macro.Return.Equal[CAST_RESISTED]} && ${CharmFail}<2) {
                        /call BroadCast g "CHARM Resisted -> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                        /if (${MezDebuffOnResist} && !${Target.Tashed.ID}) {
                            /while (${Me.SpellInCooldown}) {
                                /delay 3
                            }
                            /call CastWhat "${MezDebuffSpell}" ${MobID} MezDebuff 0 0
                            /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                                /call BroadCast g "Magic Debuffed -> ${Spawn[${MobID}].CleanName} <- ID:${MobID} : Trying again!"
                                /while (${Me.SpellInCooldown}) {
                                    /delay 3
                                }
                            }
                        }
                        /continue
                    } else /if (${Macro.Return.Equal[CAST_IMMUNE]}) {
						/call BroadCast g "CHARM IMMUNE -> ${Spawn[${MobID}].CleanName} <- ID:${MobID}"
                        /if (!${CharmImmuneIDs.Find[|${Target.ID}]}) /call AddCharmImmune ${Target.ID}
                        /if (${AACharmImmune}) /call Bind_AddCharmImmune ${Target.CleanName}
                    } else /if (${Macro.Return.Equal[CAST_CANCELLED]}) {
                        /if (${CastingInterruptOn}) /return CAST_CANCELLED
                    }
                    /break
                }
            
            /varset CharmTry 0
        }
        DEBUGN CharmMobs Leave
    /return
| ----------------------------------------------------------------------------
| SUB: Create Timers Charm
| ----------------------------------------------------------------------------
    Sub CreateTimersCharm
        | Declare timers for Charming
        /if (${Select[${Me.Class.ShortName},DRU,ENC,NEC,BRD]}) {
            /declare ll int local
            /for ll 1 to 30
                /if (!${Defined[CharmTimer${ll}]}) {
                    /declare CharmTimer${ll} timer outer 0
                    /declare CMTimer${ll} timer outer 0
                } else {
                    /varset CharmTimer${ll} 0
                    /varset CMTimer${ll} 0
                }
            /next ll
        }
    /return
