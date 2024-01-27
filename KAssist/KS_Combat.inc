| -------------------------------------------------------------------------------------
| SUB: Combat
| -------------------------------------------------------------------------------------
    Sub Combat
        /doevents
        DEBUGCOMBAT Role: ${Role} Camp Distance: ${Math.Distance[${CampYLoc},${CampXLoc}]} ${CampRadius} Pulling: ${Pulling}  ${MyTargetID} ${Target.ID} ${Target.LineOfSight} ${DPSPaused}
        /if (${Select[${Role},puller,pullertank,pullerpettank]} && ${Math.Distance[${CampYLoc},${CampXLoc}]}>=${CampRadius} && ${Pulling}) /return
        /if (!${MyTargetID} || !${Target.ID}) /return
        /if (${DMZ} && ${Me.InInstance}==FALSE) /return
        |/if (!${Target.LineOfSight}) /return
        /if (!${Target.LineOfSight} && !${Role.Find[hunter]}) /return

        /if (${DPSPaused}) /return
        | Check if mob mezzed
        /if (${Target.Mezzed.ID}) {
            /if (${Spawn[=${MainAssist}].ID} && !${IAmMA} && ${Spawn[${MyTargetID}].PctHPs}<=${AssistAt}) {
                /echo Mob is mezzed ->> ${Target.CleanName} <<- I am not going to wake it since I am on assist.
                /delay 10
                /return
            }
        }
        /if (${Select[${Role},tank,pullertank]}==0 && (${Me.Song[Rallying Call].ID} || ${Me.Song[Rallying Solo].ID})) /return
        DEBUGCOMBAT Combat Enter
        /declare CombatRadius int   local ${If[${Spawn[id ${MyTargetID}].MaxRangeTo}>${MeleeDistance},${Math.Calc[${Spawn[id ${MyTargetID}].MaxRangeTo}+5]},${MeleeDistance}]}
        /declare SpamTankWait int   local 1
        /declare navTimer     timer local 0
        /varcalc ZDist ${Math.Distance[${Target.Z}:${Me.Z}]}-(${Target.Height}-${Me.Height})
        /if (${CuresOn}) {
            /call CheckCures Combat
        } else /if (${HealsOn}) {
            /if (!${LastHealCheck}) {
                /call CheckHealth Combat1
                /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
            }
        }
        /if (${ChaseAssist}) /call DoWeChase
        /if (${MezOn}) {
            /call MezCheck Combat
        }
		/if (${CharmOn}) {
                /call CharmCheck Combat
        }
        | Check Target Hps and type NPC or NPC Pet
        DEBUGCOMBAT Combat ${Spawn[${MyTargetID}].Name} PctHPs = ${Spawn[${MyTargetID}].PctHPs} AssistAt=${AssistAt} Distance = ${Spawn[${MyTargetID}].Distance} CombatRadius = ${CombatRadius}
        /if (${DPSMeter}) {
            /if (!${DPSAdv.MyStatus}) /DPSAdv mystart
        }
        | PullerPetTank and PetTank send in pet if mob in range
        /if (${Spawn[${MyTargetID}].PctHPs}<=${PetAssistAt} && ${Spawn[${MyTargetID}].Distance}<${PetAttackRange}) /call CombatPet
        /if (${AggroOn} && !${IAmMA} && ${Select[${Role},Tank]} && ${Spawn[${MyTargetID}].Distance}>${CombatRadius}) /call AggroCheck
        /if (${Spawn[${MyTargetID}].Type.NotEqual[Corpse]} && !${Bool[${Target.Charmed}]} && ${Spawn[${MyTargetID}].PctHPs}<=${AssistAt} && (${Spawn[${MyTargetID}].Distance}<${CombatRadius} || (${Math.Distance[${CampYLoc},${CampXLoc}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}<=${CombatRadius} && ${Math.Distance[${Target.Y},${Target.X}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}<=${CampRadius}))) {
            /if (!${CombatStart}) {
                DEBUGCOMBAT Combat Setting CombatStart to 1
                /if (${Cursor.ID}) /call CheckCursor Combat 0
                /varset MercAssisting 0
                /varset CombatStart 1
                /echo  ATTACKING -> ${Spawn[${MyTargetID}].CleanName} <-
                /if (${IAmABard}) /call DoBardStuff
                /if (${Select[${Role},Tank,PullerTank,Hunter]}) /call BroadCast y "TANKING-> ${Spawn[${MyTargetID}].CleanName} <- ID:${MyTargetID}"
                /if (${Select[${Role},petTank,pullerpettank,hunterpettank]}) /call BroadCast y "${Me.Pet.CleanName} is TANKING-> ${Spawn[${MyTargetID}].CleanName} <- ID:${MyTargetID}"
                | Moloing as as puller turn off puller tag in group so tank merc attacks and doesn't wait for me to get 2 feet near them.
                /if (${Group}==1 && ${Select[${Role},puller]} && ${Group.Puller.Name.Equal[${Me}]} && ${Spawn[=${MainAssist}].Type.Equal[Mercenary]} && ${Math.Distance[${CampYLoc},${CampXLoc}]}<${CampRadius}) /call AssignGroupRole unset "${Me.CleanName}" 3
                /if (!${Spawn[${MyTargetID}].LineOfSight} && ${Role.Find[hunter]}) /call GetCombatPosition 1
            }
            /if (${FaceMobOn} && ${Target.ID} && (${Me.Standing} || ${Me.Mount.ID})) /face ${If[${FaceMobOn}==1,fast nolook,nolook]}
            /if (!${Me.FeetWet}) /look 0
            /if (!${Attacking}) {
                /if (${MeleeOn}) {
                    /varset Attacking 1
                    /if (${Me.Sitting}) /stand
                    /if (${AutoFireOn}!=1) {
                        DEBUGCOMBAT Combat Attack On
                        | Speed up tanks attacking inc mobs MQ2Melee /killthis can cause a delay
                        /if (${Select[${Role},tank,pullertank,hunter]}) {
                            /if (${Me.Skill[Taunt]} && ${Me.AbilityReady[Taunt]}) /doability Taunt
                        }
                        /if (!${Me.Combat} && ${BeforeArray[1].NotEqual[null]}) /call BeforeAttack ${MyTargetID} 1
                        /if (${PetCombatOn} && ${Me.Pet.ID}) {
                            /if (${Spawn[${MyTargetID}].PctHPs}<=${PetAssistAt} && !${Pet.Combat}) /call CombatPet
                        }
                        | Move to mob Attack on and Stick handled in CheckStick
                        /call CheckStick 0 1
                        | Move closer if Z distance more than 4 due to levitation
                        /call ZAxisCheck ${ZDist} 4.1
                    } else {
                        /if (!${Me.Combat} && ${BeforeArray[1].NotEqual[null]}) /call BeforeAttack ${MyTargetID} 1
                    }
                } else {
                    /if (${Stick.Active}) /stick off
                    /if (${PetCombatOn} && ${Me.Pet.ID}) {
                        /if (${Spawn[${MyTargetID}].PctHPs}<=${PetAssistAt} && !${Pet.Combat}) {
                            /call CombatPet
                            /varset Attacking 1
                        }
                    }
                }
            }
            /varset  MyExp ${Me.PctExp}
            /varset  MyAAExp ${Me.PctAAExp}
            | Turn on check for mez mob in camp that has been mem blurred if i am a tank type
            /if (${Select[${Role},tank,pullertank,pettank,pullerpettank]}) /varset MezMobFlag 1
            /while (1) {
                DEBUGCOMBAT Attack 1
                /while (1) {
                    /varset EventFlag 0
                    /doevents
                    /if (!${EventFlag}) /break
                }
                /if (${Role.Equal[offtank]} && !${Spawn[=${MainAssist}].ID}) /switchma ${Me} tank 0
                /if (${MercOn} && ${MercAssisting}!=${MyTargetID} && ${Mercenary.State.Equal[Active]}) /call MercsDoWhat
                /if (${AutoFireOn}!=1) {
                    /if (${MeleeOn}) {
                        /if (${Spawn[${MyTargetID}].Distance3D}>${Spawn[${MyTargetID}].MaxRangeTo} && !${Stick.Active}) /call CheckStick 0 ${If[${UseMQ2Melee},0,1]}
                    }
                } else {
                    | This is where we would add our call to the routine to position us for Autofire.
                    /if (${CombatStart} && !${Me.AutoFire}) {
                        /if (${Target.ID}==${MyTargetID} && !${Bool[${Target.Mezzed.ID}]}) {
                            /if (${Me.Sitting}) /stand
                            /autofire
                        }
                    }
                }
                /if (${MezOn}) {
                    /call MezCheck Combat
                }
				/if (${CharmOn}) {
					/call CharmCheck Combat
				}
                /if (${AEOn}) /call AECheck ${AEDisplayMobInfo}
                /if (${AggroOn}) /call AggroCheck
                /if (${CuresOn}) {
                    /call CheckCures Combat
                } else /if (${HealsOn}) {
                    /if (!${LastHealCheck}) {
                        /call CheckHealth Combat2
                        /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
                    }
                } else /if (${AutoRezOn}) {
                    /call RezCheck group
                }
                /if (!${NamedCheck} && (${BurnAllNamed} || ${GMailEvents.Find[named]})) /call NamedWatch 0
                /if (${Role.NotEqual[puller]} || !${ChainPull}) {
                    /if (${DebuffAllOn}) {
                        DEBUGCOMBAT Attack 2
                        | Debuff stuff
                        /call DoDebuffStuff ${MyTargetID}
                    }
                    |When returning the mob you were started on could be dead, so don't keep trying to kill a corpse.
                    /if (${Spawn[${MyTargetID}].Type.Equal[Corpse]} || ${Bool[${Target.Charmed}]} || !${Spawn[${MyTargetID}].ID} || ${DPSPaused}) {
                        /call CombatReset 0 combat
                        /break
                    }
                    /if (${DPSOn}) {
                        /if (!${MeleeOn} && !${AggroOffTimer} && ${Me.Invis}) /makemevisible
                        /call CombatCast
                        /if (!${MyTargetID}) {
                            /call CombatReset 0 combat
                            /break
                        }
                        /if (${Macro.Return.Equal[tcnc]}) /continue
                    } else {
                        /if (${MashArray.Size}>0 && ${MashArray[1].NotEqual[null]}) /call MashButtons
                    }
                }
                DEBUGCOMBAT Attack 3
                |moved heal checking from CastMana to before calling it.
                /if (${CuresOn}) {
                    /call CheckCures Combat
                } else /if (${HealsOn}) {
                    /if (!${LastHealCheck}) {
                        /call CheckHealth Combat2
                        /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
                    }
                } else /if (${AutoRezOn}) {
                    /call RezCheck group
                }
                /call CastMana combat
                /call WriteDebuffs
                /if (${IAmABard}) /call DoBardStuff
                /call CombatTargetCheck 1
                /if (${MyTargetID} && ${Spawn[${MyTargetID}].Type.NotEqual[Corpse]} && !${DPSPaused}) {
                    /if (${AutoFireOn}!=1) {
                        /if (${Attacking}) {
                            /if (${MeleeOn}) {
                                /if (${Target.PctHPs}<=${AssistAt} && ${Target.Distance}<${CombatRadius}) {
                                    /if (!${TargetSwitchingOn} && ${Target.ID}!=${MyTargetID}) {
                                        /squelch /target id ${MyTargetID}
                                        /delay 10 ${Target.ID}==${MyTargetID}
                                    }
                                    /if (!${Me.Combat}) {
                                        /if (${Me.Sitting}) /stand
                                        /call CheckStick 0 1
                                    } else {
                                        /if (!${Stick.Active}) /call CheckStick 1 0
                                    }
                                } else /if (!${Me.Combat} && ${IAmMA}) {
                                    /if (${DebugCombat}) {
                                        /beep
                                        /echo \atIm in the :Attack loop but attack is not on Attacking=${Attacking} MeleeOn=${MeleeOn} Target.PctHPs=${Target.PctHPs} Math.Calc[AssistAt-5]=${Math.Calc[${AssistAt}-5]} Target.Distance=${Target.Distance} CombatRadius=${CombatRadius} AutoFireOn=${AutoFireOn}
                                    }
                                    /if (${Target.ID} && ${Select[${Me.State},stand,mount]}) /squelch /attack on
                                }
                            }
                        }
                    } else /if (${AutoFireOn}==1) {
                        /if (${CombatStart} && !${Me.AutoFire}) {
                            /if (${Target.ID}==${MyTargetID} && !${Bool[${Target.Mezzed.ID}]}) {
                                /if (${Me.Sitting}) /stand
                                /autofire
                            }
                        }
                    }
                    /if (${PetCombatOn} && ${Me.Pet.ID}) {
                        /if (${Spawn[${MyTargetID}].PctHPs}<=${PetAssistAt} && !${Pet.Combat}) {
                            /call CombatPet
                        }
                    }
                    | Chain Pulling Check Here
                    /if (${Role.Equal[puller]} && ${ChainPull}) {
                        /call ChainPullNextMob ${SpamTankWait}
                        /if (${Macro.Return.Equal[1]}) {
                            /return
                        } else /if (${Macro.Return.Equal[W]}) {
                            /varset SpamTankWait 0
                        }
                        /if (${DebuffAllOn}) /call DebuffCast ${MyTargetID} 1
                        /if (${DPSOn}) {
                            /if (!${MeleeOn} && !${AggroOffTimer} && ${Me.Invis}) /makemevisible
                            /call CombatCast
                            /if (!${MyTargetID}) {
                                /call CombatReset 0 combat
                                /break
                            }
                            /if (${Macro.Return.Equal[tcnc]}) /continue
                        } else {
                            /if (${MashArray.Size}>0 && ${MashArray[1].NotEqual[null]}) /call MashButtons
                        }
                    } else {
                        DEBUGCHAINPULL Chain Pull Failed 0: ${Role} ${ChainPull}
                    }
                } else {
                    | We have a corpse targeted if we get here.
                    | or we lost our target and MyTargetID == 0
                    /if (${DPSPaused} || !${TargetSwitchingOn}) {
                        /call CombatReset 0 combat
                        /break
                    } else {
                        | Target Switching must be enabled to get here.
                        DEBUGCOMBAT Attack 4 ${MyTargetID} ${Target.ID} ${TargetSwitchingOn}
                        /if (${IAmMA}) {
                            /if (${Target.ID} && ${Target.ID}!=${MyTargetID}) {
                                /if (${MyTargetID}) /varset LastTargetID ${MyTargetID}
                                /varset MyTargetID 0
                                /call CombatTargetCheck 1
                                /if (!${MyTargetID}) {
                                    /varset MyTargetID ${LastTargetID}
                                    /call CombatReset 0 combat
                                    /break
                                }
                                /continue
                            } else {
                                /call CombatReset 0 combat
                                /break
                            }
                        } else {
                            /call CombatReset 0 combat
                            /break
                        }
                    }
                    /if (${Defined[ParseDPS]}) {
                        /if (!${ParseDPS}) /break
                    }
                }
                /if (${Me.Feigning} || ${Me.Invis}) {
                    /call FeignAggroCheck
                    /delay 5
                    /if (${Me.Feigning} || ${Me.Invis}) /break
                }
            }
        } else /if (${Spawn[${MyTargetID}].Distance}<${CombatRadius} || (${Math.Distance[${CampYLoc},${CampXLoc}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}<=${CombatRadius} && ${Math.Distance[${Target.Y},${Target.X}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}<=${CampRadius})) {
            /if (${DPSOn} || ${MeleeOn} || ${PetOn}) /call CombatTargetCheck 1
            /if ((${PetOn} && ${PetCombatOn}) || (${CharmOn} && ${Me.Pet.ID})) {
                /if (${MezOn}) /call MezCheck Combat
				/if (${CharmOn}) /call CharmCheck Combat
				/if (${Spawn[${MyTargetID}].PctHPs}<=${PetAssistAt} && !${Pet.Combat}) /call CombatPet
            } else {
                |/if (${CharmOn} && ${Me.Pet.ID}) /call DoPetStuff
				/if (${CharmOn}) /call CharmCheck Combat
                /if (${MezOn}) /call MezCheck Combat
                /if (${IAmABard} && ${MeleeTwistOn}==2 && !${DPSTwisting}) /call DoBardStuff
                /if (${DebuffAllOn}==2 && ${MyTargetID} && ${AggroTargetID}) /call DoDebuffStuff ${MyTargetID}
            }
            /if (!${Me.Combat} && ${BeforeArray[1].NotEqual[null]}) /call BeforeAttack ${MyTargetID} 2
        }
        DEBUGCOMBAT Combat Leave
    /return

| -------------------------------------------------------------------------------------
| SUB: CombatTargetCheck
| -------------------------------------------------------------------------------------
    Sub CombatTargetCheck(int SetTarget)
        | If mob is dead return
        DEBUGCOMBAT CombatTargetCheck1 ${MyTargetID} ${Spawn[${MyTargetID}].Type} ${CalledTargetID} ${TargetSwitchingOn}
        /if (${IAmMA}) {
            /if (${TargetSwitchingOn} && ${Target.ID} && (${Spawn[${Target.ID}].Type.Equal[Corpse]} || ${Bool[${Spawn[${Target.ID}].Type.Equal[Corpse]}]})) /return
        } else {
            /if (${TargetSwitchingOn}) /return
        }
        /if (${DPSPaused} && ${SetTarget}!=2) /return
        /if (${MyTargetID} && (${Spawn[${MyTargetID}].Type.Equal[Corpse]} || !${Spawn[${MyTargetID}].ID})) {
            /varset LastTargetID ${MyTargetID}
            /varset MyTargetID 0
            /return
        }
        /declare CMyTargetID int Local ${MyTargetID}
        | Check target matches MA if group mainassist assigned and MA is in group
        /if (${Spawn[=${MainAssist}].ID} && ${Group.MainAssist.ID} && ${Spawn[=${MainAssist}].ID}==${Group.MainAssist.ID}) {
            |/if (${Group.MainAssist.ID}!=${Me.ID}) {
            /if (!${IAmMA}) {
                /if (${Target.ID}!=${Me.GroupAssistTarget.ID}) {
                    /if (${MyTargetID}!=${Me.GroupAssistTarget.ID} && ${Spawn[id ${Me.GroupAssistTarget.ID} npc].ID}) {
                        /if (${MyTargetID}) /echo My target does not match MA's. Switching to new target.
                        /varset MyTargetID ${Me.GroupAssistTarget.ID}
                    }
                }
            } else {
                /if (${Target.ID} && ${Target.ID}!=${MyTargetID}) {
                    /if (${TargetSwitchingOn}) {
                        /if (!${Select[${Target.Type},pc,mercenary,pet]} || (${Target.Type.Equal[pet]} && ${Target.Master.Type.NotEqual[pc]})) {
                            /echo I am MA, and I switched to new Target.
                            /varset MyTargetID ${Target.ID}
                            /varset MyTargetName ${Target.CleanName}
                            /if (${Select[${Role},Tank,PullerTank,Hunter]}) {
                                /call BroadCast y "TANKING-> ${Spawn[${MyTargetID}].CleanName} <- ID:${MyTargetID}"
                            } else /if (${Select[${Role},petTank,pullerpettank,hunterpettank]}) {
                                /call BroadCast y "${Me.Pet.CleanName} is TANKING-> ${Spawn[${MyTargetID}].CleanName} <- ID:${MyTargetID}"
                            }
                        }
                    } else {
                        /target id ${MyTargetID}
                        /delay 10 ${Target.ID}==${MyTargetID}
                    }
                }
            }
            /varset CalledTargetID 0
            /doevents AttackCalled flush
        } else {
            /if (!${CalledTargetID}) {
                /while (1) {
                    /varset EventFlag 0
                    /doevents AttackCalled
                    /if (!${EventFlag}) /break
                }
            }
            /if (${CalledTargetID} && ${CalledTargetID}!=${MyTargetID}) {
                /varset MyTargetID ${CalledTargetID}
                /varset CalledTargetID 0
            }
        }
        /if (${CMyTargetID}!=${MyTargetID} && ${Target.ID}!=${MyTargetID} && ${Spawn[${MyTargetID}].ID}) {
            /if (${XTarAutoSet} && !${Group.Member[${MainAssist}].Index} && !${IAmMA}) {
                /if (${Spawn[${MyTargetID}].Type.NotEqual[PC]}) {
                    /squelch /target id ${MyTargetID}
                    /delay 10 ${Target.ID}==${MyTargetID}
                    /if (!${Me.XTarget[${XTSlot}].ID} || (${Me.XTarget[${XTSlot}].ID}!=${MyTargetID})) /xtarget set ${XTSlot} currenttarget
                }
            } else {
                /if (${SetTarget}) {
                    /squelch /target id ${MyTargetID}
                    /delay 10 ${Target.ID}==${MyTargetID}
                }
            }
            /varset MyTargetName ${Spawn[${MyTargetID}].CleanName}
            /varset LastTargetID ${MyTargetID}
        }
        DEBUGCOMBAT CombatTargetCheck2 ${MyTargetID} ${LastTargetID} ${Spawn[${MyTargetID}].Type} ${CalledTargetID}
    /return
| -------------------------------------------------------------------------------------
| SUB: CombatTargetCheck1
| -------------------------------------------------------------------------------------
    Sub CombatTargetCheck1(int SetTarget)
        | If mob is dead return
        /if (${MyTargetID} && (${Spawn[${MyTargetID}].Type.Equal[Corpse]} || !${Spawn[${MyTargetID}].ID})) /return
        /if (${DPSPaused}) /return
        /declare CMyTargetID int Local ${MyTargetID}
        /declare i_GATID int local ${Int[${Me.GroupAssistTarget.ID}]}
        /if (!${IAmMA}) {
            | Check target matches MA if group mainassist assigned and MA is in group
            /if (${Spawn[=${MainAssist}].ID} && ${Group.MainAssist.ID} && ${Spawn[=${MainAssist}].ID}==${Group.MainAssist.ID}) {
                |/if (${Target.ID}!=${Me.GroupAssistTarget.ID} && ${Group.MainAssist.ID}!=${Me.ID}) {
                |/if (${Group.MainAssist.ID}!=${Me.ID}) {
                    |/if (${Target.ID}!=${i_GATID}) {
                    |/if (${Target.ID}!=${Me.GroupAssistTarget.ID} && ${MyTargetID}!=${Me.GroupAssistTarget.ID} && ${Spawn[id ${Me.GroupAssistTarget.ID} npc].ID}) {
                    /if (${Target.ID}!=${i_GATID} && ${MyTargetID}!=${i_GATID}) {
                        /if (${Spawn[id ${i_GATID}].Type.Equal[npc]} || (${Spawn[id ${i_GATID}].Type.Equal[pet]} && ${Spawn[id ${i_GATID}].Master.Type.Equal[npc]})) {
                            /if (${MyTargetID}) /echo My target does not match MA's. Switching to new target.
                            /varset MyTargetID ${i_GATID}
                        }
                    }
                    |}
                |} else /if (${TargetSwitchingOn} && ${Group.MainAssist.ID}==${Me.ID} && ${Target.ID} && ${Target.ID}!=${MyTargetID} && !${Select[${Target.Type},pc,mercenary]} && (${Target.Type.Equal[pet]} && ${Target.Master.Type.NotEqual[pc]}))) {
                |} else /if (${TargetSwitchingOn} && ${Target.ID} && ${Target.ID}!=${MyTargetID} && (!${Select[${Target.Type},pc,mercenary,pet]} || (${Target.Type.Equal[pet]} && ${Target.Master.Type.NotEqual[pc]}))) {
            } else /if (${CalledTargetID} && ${CalledTargetID}!=${MyTargetID}) {
                /varset MyTargetID ${CalledTargetID}
                /varset CalledTargetID 0
            }
        } else {
        |    /if (${TargetSwitchingOn} && ${Target.ID} && ${Target.ID}!=${MyTargetID} && (!${Select[${Target.Type},pc,mercenary,pet]} || (${Target.Type.Equal[pet]} && ${Target.Master.Type.NotEqual[pc]}))) {
        |        /echo I am MA, and I switched to new Target.
        |        /varset MyTargetID ${Target.ID}
        |        /if (${Select[${Role},Tank,PullerTank,Hunter]}) {
        |            /call BroadCast y "TANKING-> ${Spawn[${MyTargetID}].CleanName} <- ID:${MyTargetID}"
        |        } else /if (${Select[${Role},petTank,pullerpettank,hunterpettank]}) {
        |            /call BroadCast y "${Me.Pet.CleanName} is TANKING-> ${Spawn[${MyTargetID}].CleanName} <- ID:${MyTargetID}"
        |        }
        |    }
        }
        /if (${CMyTargetID}!=${MyTargetID} && ${Target.ID}!=${MyTargetID} && ${Spawn[${MyTargetID}].ID}) {
            /if (${SetTarget}) {
                /squelch /target id ${MyTargetID}
                /delay 10 ${Target.ID}==${MyTargetID}
                /if (${Me.Pet.ID} && ${Me.Pet.Combat}) {
                    /pet attack
                    /delay 5
                    /pet swarm
                }
            }
            /varset MyTargetName ${Spawn[${MyTargetID}].CleanName}
            /if (${XTarAutoSet} && !${Group.Member[${MainAssist}].Index} && !${IAmMA} && ${Target.Type.NotEqual[PC]}) {
                /if (!${Me.XTarget[${XTSlot}].ID} || (${Me.XTarget[${XTSlot}].ID}!=${MyTargetID})) /xtarget set ${XTSlot} currenttarget
            }
        }
    /return
| -------------------------------------------------------------------------------------
| SUB: Combat Cast
| -------------------------------------------------------------------------------------
    Sub CombatCast
        DEBUGCOMBAT CombatCast Enter
        /declare i           int      local 0
        /declare DPSStart    int      local ${Math.Calc[${DebuffCount}+1]}
        /if (${DPSStart}>${DPS.Size}) /return
        /declare DPSAt       int      local ${AssistAt}
        /declare DPSTargetID int      local 0
        /declare DPSItem     string   local
        /declare DPSPart1    string   local
        /declare DPSPart2    string   local
        /declare DPSPart3    string   local
        /declare DPSPart4    string   local
        /declare DPSPart5    string   local
        /declare DPSStartID  int      local 0
        /declare WeaveCheck  int      local 0
        /declare CondNo      int      local 0
        /call WriteDebuffs
        | Note about being Invised in this routine:
        | Invis is not broken for Pure casters, that are pure DPS characters. Reason for this is Aggro checking.
        | Some classes Fade, Feign, ect... That after loosing Aggro also become invised by the action, to keep from regaining aggro.
        | If you need your pure DPS caster to break invis. You can add an entry using the mash tag. You will have to use
        | an AA, or an Item that will break the invis.
        /if (${Target.ID} && ${Target.ID}!=${MyTargetID}) {
            /call CombatTargetCheck 1
            /if (!${MyTargetID}) /return
        }
        /varset DPSStartID ${MyTargetID}
        /for i ${DPSStart} to ${DPS.Size}
            /if (${CharmOn}) /call CharmCheck CombatCast
			/if (${MezOn}) {
                /call MezCheck CombatCast
            }

            /while (1) {
                /varset EventFlag 0
                /doevents
                /if (!${EventFlag}) /break
            }
            /varset DPSTargetID ${MyTargetID}
            /if (${Spawn[${MyTargetID}].Type.Equal[Corpse]} || !${Spawn[${MyTargetID}].ID} || ${DPSPaused}) {
                DEBUGCOMBAT CombatCast: Return from CombatCast cause my target is a corpse or dead or DPSPaused ${DPSPaused}.
                /return
            }
            DEBUGCOMBAT CombatCast ${DPS[${i}].Arg[1,|]} ${Spell[${DPS[${i}].Arg[1,|]}].RankName} ${DPS[${i}].Arg[1,|]} !${Me.SpellReady[${Spell[${DPS[${i}].Arg[1,|]}].RankName}]} && !${Me.AltAbilityReady[${DPS[${i}].Arg[1,|]}]} && !${Me.CombatAbilityReady[${Spell[${DPS[${i}].Arg[1,|]}].RankName}]} && !${Me.AbilityReady[${DPS[${i}].Arg[1,|]}]}

            /while (1) {
                | This is the Top of the outer loop that will drop you out just above the /next i before going to the next DPS entry.
                /if (!${Bool[${DPS[${i}].Arg[1,|]}]}) {
                    DEBUGCOMBAT CombatCast: I'm skipping in CombatCast cause there is no spell: DPS${i}
                    /break
                }
                /if (${DPS[${i}].Find[|weave]} || ${DPS[${i}].Find[|mash]} || ${DPS[${i}].Find[|ambush]}) /break
                /if (!${DPS[${i}].Find[command:]} && (!${Me.SpellInCooldown} && !${IAmABard} && !${Me.SpellReady[${Spell[${DPS[${i}].Arg[1,|]}].RankName}]}) && !${Me.AltAbilityReady[${DPS[${i}].Arg[1,|]}]} && !${Me.CombatAbilityReady[${Spell[${DPS[${i}].Arg[1,|]}].RankName}]} && !${Me.AbilityReady[${DPS[${i}].Arg[1,|]}]} && !${Me.ItemReady[${DPS[${i}].Arg[1,|]}]}) /break
                /if (${Int[${DPS[${i}].Arg[2,|]}]} > 0) {
                    /varset DPSPart1 ${DPS[${i}].Arg[1,|]}
                    /varset DPSPart2 ${DPS[${i}].Arg[2,|]}
                    /if (${Bool[${DPS[${i}].Arg[3,|]}]}) {
                        /varset DPSPart3 ${DPS[${i}].Arg[3,|]}
                    } else {
                        /varset DPSPart3
                    }
                    /if (${Bool[${DPS[${i}].Arg[4,|]}]}) {
                        /varset DPSPart4 ${DPS[${i}].Arg[4,|]}
                    } else {
                        /varset DPSPart4
                    }
                    /if (${Bool[${DPS[${i}].Arg[5,|]}]}) {
                        /varset DPSPart5 ${DPS[${i}].Arg[5,|]}
                    } else {
                        /varset DPSPart5
                    }
                    /if (${DPSOn}==1) {
                        /if (!${IAmMA}) {
                            /varset DPSAt ${DPSPart2}
                        } else {
                            /varset DPSAt ${AssistAt}
                        }
                    } else {
                        /varset DPSAt ${AssistAt}
                    }
                } else {
                    /break
                }
                /if (${ConOn} && ${DPS[${i}].Find[|cond]}) {
                    /varset CondNo ${DPS[${i}].Mid[${Math.Calc[${DPS[${i}].Find[|cond]}+5]},3]}
                } else {
                    /varset CondNo 0
                }
                DEBUGCOMBAT DPS${i}:${DPSPart1} ConOn: ${ConOn}  CondNo: ${CondNo}  Parsed:
                /if (${Target.Mezzed.ID} && !${IAmMA} && ${Spell[${DPSPart1}].Category.NotEqual[Utility Detrimental]}) {
                    DEBUGCOMBAT Skipping spell in CombatCast because target is mezzed and I'm not mainassist and it will wake up the mob.
                    /break
                }
                /if (${Select[${DPSPart3},Me,MA,maonce]}==0 && ${DPSTimer${i}} && (${Me.CombatAbility[${DPSPart1}]} || ${Me.Skill[${DPSPart1}]})) /break
                /call CheckBeforeCast CombatCast
                DEBUGCOMBAT CombatCast 0:${DPS[${i}]} 1:${DPSPart1} 2:${DPSPart2} 3:${DPSPart3} 4:${DPSPart4} 5:${DPSPart5} 6:${DPSAt} 7:${Spawn[${MyTargetID}].PctHPs} 8:${DPSSkip} 9:${DPSAt}
                /if (!${AggroTargetID}) /return
                | Check if self combat buff is active
                /if (${DPSPart3.Equal[Me]} && (${Me.Buff[${DPSPart1}].ID} || ${Me.Song[${DPSPart1}].ID})) /break
                /if (${Me.Class.Name.Equal[Cleric]} && ${DPSPart1.Find[Hammer]} && ${Me.Pet.ID}) /break
                /if (${Me.Class.Name.Equal[Wizard]} && ${Me.Pet.ID} && (${DPSPart1.Find[sword]} || ${DPSPart1.Find[blade]})) /break
                /if (${ABTimer${i}} || ${DPSTimer${i}} || ${FDTimer${i}}) /break
                /if (${Spawn[${MyTargetID}].PctHPs}<=${DPSSkip} || (${DPSOn}==1 && ${Spawn[${MyTargetID}].PctHPs}>${DPSAt})) /break
                /if (${DPSPart3.Equal[MA]} || ${DPSPart3.Equal[maonce]}) {
                    /if (${Role.Find[pettank]}) {
                        /varset DPSTargetID ${Me.Pet.ID}
                    } else {
                        /varset DPSTargetID ${Spawn[=${MainAssist}].ID}
                    }
                } else /if (${DPSPart3.Equal[Me]} || ${DPSPart3.Equal[Feign]}) {
                    /varset DPSTargetID ${Me.ID}
                }
                DEBUGCOMBAT CombatCast  DPSTarget: ${DPSTargetID} ${Spawn[${DPSTargetID}]} End: ${DPSSkip} ${Spawn[${DPSTargetID}].PctHPs} Start: ${DPSAt} Target: ${Target.ID} Timer: ${DPSTimer${i}} DPSOn: ${DPSOn}
                /if (${Select[${DPSPart3},Me,MA]} && ${Me.Combat} && !${IAmMA}) {
                    /squelch /attack off
                    /delay 10 !${Me.Combat}
                }
                | Only cast spells with weave tag during global cooldown
                /if (${Me.SpellInCooldown} && !${IAmABard}) {
                    /if (${WeaveArray.Size}>0 && ${WeaveArray[1].NotEqual[null]}) {
                        /call CastWeave ${MyTargetID}
                        /varset WeaveCheck 1
                    }
                    /if (${Me.Gem[${DPSPart1}]} && ${Spawn[${MyTargetID}].ID}) {
                        DEBUGCOMBAT CombatCast Spell in Cool Down DPSOn: ${DPSOn}
                        /if (${DPSOn}==2) {
                            /while (${Me.SpellInCooldown}) {
                                /delay 2
                            }
                        } else {
                            /break
                        }
                    }
                }
                /if (${IAmMA} && ${TargetSwitchingOn} && ${Target.ID} && ${Target.ID}!=${MyTargetID}) {
                    /call CombatTargetCheck 1
                    /if (${DPSStartID}!=${MyTargetID} || !${MyTargetID}) /return
                }
                /call CastWhat "${DPSPart1}" ${DPSTargetID} DPS ${CondNo} 0
                DEBUGCOMBAT CombatCast MacroReturn: ${Macro.Return}
                /if (${MeleeOn} && !${Me.Combat} && ${Target.ID}==${MyTargetID}) /squelch /attack on
                /if (${Macro.Return.Equal[CAST_COND_FAILED]}) /break
                /varset MacroReturn ${Macro.Return}
                /varset LastDPSCast ${DPSPart1}
                /doevents
                /if (${MacroReturn.Equal[CAST_SUCCESS]}) {
                    /if (${Spawn[${MyTargetID}].ID}) /echo ** ${DPSPart1} on >> ${Spawn[${DPSTargetID}].CleanName} <<
                    | Set timer for spells labeled once 5m
                    /if (${DPSPart3.Length} && (${DPSPart3.Equal[once]} || ${DPSPart3.Equal[maonce]})) {
                        /varset DPSTimer${i} 5m
                    | Timers for items
                    } else /if (${FindItemCount[=${DPSPart1}]}) {
                        /varcalc DPSTimer${i} ${FindItem[=${DPSPart1}].Spell.Duration.TotalSeconds}*10
                    | FD to lose aggro
                    } else /if (${Select[${Me.Class.ShortName},BST,MNK,NEC,SHD]} && ${DPSPart3.Equal[Feign]}) {
                        /delay 30 ${Me.State.Equal[FEIGN]}
                        /varset FDTimer${i} 60s
                        /delay 10s ${Me.State.NotEqual[FEIGN]}
                        /if (${Me.State.Equal[FEIGN]} && !${Me.Sitting}) /stand
                    | Timers for spells
                    } else /if (${Me.Book[${DPSPart1}]}) {
                        | - Attack Buff Timers are seperate. DPS timers are cleared after every fight.
                        /if (${Select[${DPSTargetID},${Me.ID},${Spawn[=${MainAssist}].ID}]}) {
                            /varcalc ABTimer${i} ${Spell[${DPSPart1}].MyDuration.TotalSeconds}*10
                            DEBUGCOMBAT CombatCast Spell: ${DPSPart1} Timer: ABTimer${i} ${ABTimer${i}}
                        | - Custom timer for counterbias
                        } else /if (${Me.Class.Name.Equal[Shaman]} && ${DPSPart1.Find[counterbias]}) {
                            /varset DPSTimer${i} 1.5m
                        | Custom timer for Chanter suffocation
                        } else /if (${Me.Class.Name.Equal[enchanter]} && ${DPSPart1.Find[suffocation]}) {
                            /varset DPSTimer${i} 1m
                        | Custom timer for Beastlord Feralgia Timer
                        } else /if (${Me.Class.Name.Equal[Beastlord]} && ${DPSPart1.Find[feralgia]}) {
                            /varset DPSTimer${i} 1.5m
                        | Regular spells
                        } else /if (${Spell[${DPSPart1}].Duration.TotalSeconds}>0) {
                            /varcalc DPSTimer${i} ${Spell[${DPSPart1}].Duration.TotalSeconds}*10
                        | Spells with no timers assign DPS interval
                        } else {
                            /varset DPSTimer${i} ${DPSInterval}s
                        }
                    | AA timers
                    } else /if (${Me.AltAbility[${DPSPart1}]}) {
                        /if (${Me.AltAbility[${DPSPart1}].Spell.Duration.TotalSeconds}) {
                            /varcalc DPSTimer${i} ${Me.AltAbility[${DPSPart1}].Spell.Duration.TotalSeconds}*10
                        } else /if (${Me.AltAbility[${DPSPart1}].Spell.Trigger.Duration.TotalSeconds}) {
                            /varcalc DPSTimer${i} ${Me.AltAbility[${DPSPart1}].Spell.Trigger.Duration.TotalSeconds}*10
                        }
                    | Disc Timers
                    } else /if (${Spell[${DPSPart1}].Duration}>0 && ${Me.CombatAbility[${DPSPart1}]}) {
                        /varcalc DPSTimer${i} ${Spell[${DPSPart1}].Duration.TotalSeconds}*10
                    | AA and disc with no timers assign DPS interval
                    } else {
                        /varset DPSTimer${i} ${DPSInterval}s
                    }
                } else /if (${MacroReturn.Left[13].Equal[CAST_RESISTED]}) {
                    |/echo ** ${DPSPart1} on >> ${Spawn[${DPSTargetID}].CleanName} << - RESISTED
                    /echo ** ${Spawn[${DPSTargetID}].CleanName} - RESISTED - ${LastResisted}
                    /if (${DPSPart3.Length} && (${DPSPart3.Equal[once]} || ${DPSPart3.Equal[maonce]})) {
                        /varset DPSTimer${i} 5m
                    }
                } else /if (${MacroReturn.Equal[CAST_TAKEHOLD]}) {
                    /echo ** ${DPSPart1} on >> ${Spawn[${DPSTargetID}].CleanName} << - DID NOT TAKE HOLD
                    /varset DPSTimer${i} 5m
                }
                /break
            }
            | This is the Bottom of the Outer Loop. Add additional checks needed before /next i entry.
            DEBUGCOMBAT CombatCast ${DPS[${i}].Arg[1,|]} ${Macro.Return} DPSTimer${i} ${DPSTimer${i}}
            /call CombatTargetCheck 1
            /if (!${MyTargetID}) /return
            /if (${DPSStartID}!=${MyTargetID}) {
                /if (${IAmMA} && ${TargetSwitchingOn}) {
                    /return
                } else /if (${MeleeOn} && !${Me.Combat}) {
                    |Target Change Not in Combat
                    /return tcnc
                }
            }
            /if (${WeaveArray.Size}>0 && ${WeaveArray[1].NotEqual[null]} && (${Me.SpellInCooldown} || !${WeaveCheck})) /call CastWeave ${MyTargetID}
            /if (${MashArray.Size}>0 && ${MashArray[1].NotEqual[null]}) /call MashButtons
            /varset WeaveCheck 0
        /next i
        DEBUGCOMBAT CombatCast: Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: CheckStick
| -------------------------------------------------------------------------------------
    Sub CheckStick(int p_Flag1, int p_UseAttack)
        /declare _CampXLoc               int         local       ${CampXLoc}
        /declare _CampYLoc               int         local       ${CampYLoc}
        /declare _CampZLoc               int         local       ${CampZLoc}
        /declare navTimer timer local 0
        DEBUGCOMBAT CheckStick: Enter.
        DEBUGCOMBAT CheckStick0: 1:${Stick.Active} 2:!${IAmMA} 3:${MoveUtils.Aggro} 4:${StickHow} 5:${Me.FeetWet} 6:${ChaseAssist}

        /if (${ChaseAssist} || !${ReturnToCamp}) {
            /varset _CampXLoc ${Spawn[=${MainAssist}].X}
            /varset _CampYLoc ${Spawn[=${MainAssist}].Y}
            /varset _CampZLoc ${Spawn[=${MainAssist}].Z}
        }

        /if (${MeleeDistance}>${CampRadius}) {
            DEBUGCOMBAT CheckStick1: ${CampRadius} ${MeleeDistance} CFT: ${Math.Distance[${_CampYLoc},${_CampXLoc}:${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X}]} CFMA: ${Math.Distance[${_CampYLoc},${_CampXLoc}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]} TFMA: ${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}
            | Is Mob to far from camp?
            /if (${Math.Distance[${_CampYLoc},${_CampXLoc}:${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X}]}>${MeleeDistance}) {
                | Is MA/Tank to far from camp?
                /if (${Math.Distance[${_CampYLoc},${_CampXLoc}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}>${MeleeDistance}) {
                    /return
                } else {
                    | The Ma/Tank is close enough to engage, but is the Mob still to far from the tank?
                    /if (${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}>${CampRadius}) /return
                }
            } else {
                | Add check to see if you are so far away you need to abort this routine.
            }
            DEBUGCOMBAT CheckStick2: ${Spawn[id ${MyTargetID}].MaxRangeTo.Int} ${Spawn[id ${MyTargetID}].Distance}
            /if (${Spawn[id ${MyTargetID}].MaxRangeTo.Int}<${Spawn[id ${MyTargetID}].Distance}) {
                /if (${PullMoveUse.Equal[nav]}) {
                    /nav id ${MyTargetID}
                    /varset navTimer 30
                    /while (${navTimer}) {
                        /if (${Spawn[id ${MyTargetID}].MaxRangeTo.Int}>=${Spawn[id ${MyTargetID}].Distance}) {
                            /nav stop
                            /break
                        }
                    }
                    /if (${Navigation.Active}) /nav stop
                |}
                } else {
                    /moveto dist ${Spawn[id ${MyTargetID}].MaxRangeTo}
                }
            }
        }
        DEBUGCOMBAT CheckStick3: ${p_Flag1} ${p_UseAttack} ${Me.Combat} ${Me.State} ${UseMQ2Melee}
        /if (${p_UseAttack}) {
            /if (${UseMQ2Melee}) {
                /killthis
                /return
            } else {
                /if (!${Me.Combat} && ${Target.ID} && ${Select[${Me.State},stand,mount]}) /squelch /attack on
            }
        }

        /if (${MeleeDistance}>${CampRadius}) {
            /if (!${Select[${StickHow},0,I]}) {
                /if (!${p_Flag1}) {
                    /if (${Me.FeetWet}) {
                        /stick ${Math.Calc[${Spawn[id ${MyTargetID}].MaxRangeTo}-3]} id ${MyTargetID} ${StickHow} uw
                    } else {
                        /stick ${Spawn[id ${MyTargetID}].MaxRangeTo} id ${MyTargetID} ${StickHow}
                    }
                } else /if (${p_Flag1}==1) {
                    /if (${Target.ID} && ${Me.TargetOfTarget.ID}!=${Me.ID}) {
                        /if (${Me.FeetWet}) {
                            /stick id ${MyTargetID} ${StickHow} uw
                        } else {
                            /stick id ${MyTargetID} ${StickHow}
                        }
                    }
                }
            } else /if (${StickHow.NotEqual[I]}) {
                /if (!${p_Flag1}) {
                    /if (${Me.FeetWet}) {
                        /stick ${Math.Calc[${Spawn[id ${MyTargetID}].MaxRangeTo}-3]} id ${MyTargetID} uw
                    } else {
                        /stick ${Spawn[id ${MyTargetID}].MaxRangeTo} id ${MyTargetID}
                    }
                } else /if (${p_Flag1}==1) {
                    /if (${Target.ID} && ${Me.TargetOfTarget.ID}!=${Me.ID}) {
                        /if (${Me.FeetWet}) {
                            /stick id ${MyTargetID} uw
                        } else {
                            /stick id ${MyTargetID}
                        }
                    }
                }
            }
        }
        DEBUGCOMBAT CheckStick: Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Mash Buttons
| -------------------------------------------------------------------------------------
    Sub MashButtons(TarID)
        /if (${DPSOn}==0) /return
        /if (!${Select[${Me.State},stand,mount]}) /return
        /declare i int local
        /declare MashThis string local
        /declare MashThisArg3 string local
        /declare MashThisArg4 string local
        /if (${TargetSwitchingOn}) {
            /if (${IAmMA} && ${Target.ID} && ${Target.ID}!=${MyTargetID}) /call CombatTargetCheck 1
        } else /if (${Target.ID}!=${MyTargetID}) {
            /target id ${MyTargetID}
            /delay 10 ${Target.ID}==${MyTargetID}
        }
        /for i 1 to ${MashArray.Size}
            /varset MashThis ${MashArray[${i}].Arg[1,|]}
            /if (!${Bool[${MashThis}]} || !${Target.ID} || ${Target.Type.Equal[corpse]}) /return
            /if (!${ConOn} || !${MashArray[${i}].Find[|cond]} || ${If[${Cond[${MashArray[${i}].Mid[${Math.Calc[${MashArray[${i}].Find[|cond]}+5]},3]}]},1,0]}) {
                /if (${FindItem[=${MashThis}].ID} && ${Me.ItemReady[${MashThis}]}) {
                    /useitem "${MashThis}"
                    /delay 2
                    /echo ## Mashing >> ${MashThis} <<
                } else /if (${Me.AltAbility[${MashThis}]} && ${Me.AltAbilityReady[${MashThis}]} && ${Me.AltAbility[${MashThis}].Type}!=5 && ${MashThis.NotEqual[twincast]})  {
                    /alt act ${Me.AltAbility[${MashThis}].ID}
                    /delay 2
                    /if (!${Me.AltAbilityReady[${MashThis}]}) /echo ## Mashing >> ${MashThis} <<
                } else /if (${Me.CombatAbility[${MashThis}]} && !${Me.CombatAbilityTimer[${MashThis}]} && ${Me.CombatAbilityReady[${MashThis}]} && ${Spell[${MashThis}].EnduranceCost}<${Me.Endurance}) {
                    /disc ${Me.CombatAbility[${Me.CombatAbility[${MashThis}]}].ID}
                    /delay 2
                   /if (!${Me.CombatAbilityReady[${MashThis}]}) /echo ## Mashing >> ${MashThis} <<
                } else /if (${Me.Skill[${MashThis}]} && ${Me.AbilityReady[${MashThis}]}) {
                    /doability "${MashThis}"
                    /delay 2
                    /if (!${Me.AbilityReady[${MashThis}]}) /echo ## Mashing >> ${MashThis} <<
                }
            }
        /next i
    /return 0
| -------------------------------------------------------------------------------------
| SUB: BeforeAttack
| -------------------------------------------------------------------------------------
    Sub BeforeAttack(TarID, int CondCheck)
        DEBUGCOMBAT BeforeAttack: Enter
        /declare i int local
        /declare UseThis string local
        /declare UseThisArg3 string local
        /declare UseThisArg4 string local
        /for i 1 to ${BeforeArray.Size}
            /varset UseThis ${BeforeArray[${i}].Arg[1,|]}
            /if (!${Bool[${UseThis}]} || !${Target.ID}) /return
            /if (!${BeforeArray[${i}].Find[|cond]} && ${CondCheck}==2) /continue
            /if (!${ConOn} || !${BeforeArray[${i}].Find[|cond]} || ${If[${Cond[${BeforeArray[${i}].Mid[${Math.Calc[${BeforeArray[${i}].Find[|cond]}+5]},3]}]},1,0]}) {
                /if (${FindItem[=${UseThis}].ID} && ${Me.ItemReady[${UseThis}]}) {
                    /useitem "${UseThis}"
                    /echo ## Before Attack >> ${UseThis} <<
                } else /if (${Me.AltAbility[${UseThis}]} && ${Me.AltAbilityReady[${UseThis}]} && ${Me.AltAbility[${UseThis}].Type}!=5 && ${UseThis.NotEqual[twincast]})  {
                    /alt act ${Me.AltAbility[${UseThis}].ID}
                    /echo ## Before Attack >> ${UseThis} <<
                } else /if (${Me.CombatAbility[${UseThis}]} && !${Me.CombatAbilityTimer[${UseThis}]} && ${Me.CombatAbilityReady[${UseThis}]} && ${Spell[${UseThis}].EnduranceCost}<${Me.Endurance}) {
                    /disc ${Me.CombatAbility[${Me.CombatAbility[${UseThis}]}].ID}
                    /echo ## Before Attack >> ${UseThis} <<
                } else /if (${Me.Skill[${UseThis}]} && ${Me.AbilityReady[${UseThis}]}) {
                    /doability "${UseThis}"
                    /echo ## Before Attack >> ${UseThis} <<
                } else /if (${UseThis.Find[command:]}) {
                    /call CastCommand "${UseThis}" ${MyTargetID} 0
                }
                /delay 3
            }
        /next i
        DEBUGCOMBAT BeforeAttack: Leave
    /return 0
| -------------------------------------------------------------------------------------
| SUB: CheckForCombat Used to help with controlling combat
| -------------------------------------------------------------------------------------
    Sub CheckForCombat(int SkipCombat, string FromWhere, int WaitTime)
        /if (${ChaseAssist} && ${Me.Moving} && (!${IAmMA} || ${WhoToChase.NotEqual[${Me}]})) /return
        DEBUGCOMBAT CheckForCombat Enter SkipCombat: ${SkipCombat} ChainPull: ${ChainPull} Role: ${Role} ${FromWhere}
        /while (1) {
            /if (${SkipCombat}==0) {
                /if (${IAmDead} && ${CampZone}==${Zone.ID} && (${Me.Buff[Resurrection Sickness].ID} || ${SpawnCount[pccorpse ${Me}]}==0)) /varset IAmDead 0
                /call MobRadar los ${MeleeDistance} CheckForCombat
				/if (${CharmOn}==1) /call CharmRadar los ${MeleeDistance} CheckForCombat
                /if (${ChaseAssist}) /call DoWeChase
                /if (${DebugCombat}) /delay 5
                /if ((${DMZ} && ${Me.InInstance}==FALSE) || ${Me.Hovering} || (${IAmDead} && !${AggroTargetID}) || (!${MobCount} && !${AggroTargetID}) || (!${DPSOn} && !${MeleeOn})) /return
                DEBUGCOMBAT CheckForCombat: Assist/Combat Begin ${FromWhere}
                /if (${IAmABard} && ${MeleeTwistOn}==2 && !${DPSTwisting}) /call DoBardStuff
                /varset EngageWaitTimer ${WaitTime}
                /if (!${IAmMA}) {
                    |/varset EngageWaitTimer 200
                    /while (1) {
                        | If I am not off tank or the MainAssist is alive then just call assist.
                        /if (${Role.NotEqual[offtank]} || ${Spawn[=${MainAssist}].ID}) {
                            /call Assist
                            | if MA is down and I am offtank make me MA.
                            /if (${Macro.Return.Equal[switch]}) {
                                /switchma ${Me} tank 0
                                /break
                            }
                        } else {
                            | I am Off Tank and MainAssist is dead.
                            /switchma ${Me} tank 0
                            /break
                        }
                        /if (${HealsOn} && ${AggroTargetID}) /call CheckHealth CheckForCombat1
                        /if (${MyTargetID} || ${Role.NotEqual[assist]} || !${EngageWaitTimer} || !${AggroTargetID}) /break
                    }
                    /if (${LOSBeforeCombat} && ${MyTargetID} && ${AggroTargetID} && !${Spawn[${MyTargetID}].LineOfSight}) /call GetCombatPosition 0
                } else {
                    |I am MainAssist
                    |Need to call WaitForMob from here and remove it from the Pull routine.
                    /if (!${Select[${Role},pullertank,pullerpettank,hunter,hunterpettank]}) {
                        /if (${AggroTargetID}) {
                            /while (1) {
                                /if (!${AggroTargetID} || ${Spawn[${AggroTargetID}].Type.Equal[Corpse]}) /break
                                /if (${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]}>${MeleeDistance}) {
                                    /varset MobCount ${SpawnCount[xtarhater radius ${MeleeDistance} zradius ${MaxZRange}]}
                                } else {
                                    /varset MobCount ${SpawnCount[xtarhater loc ${CampXLoc} ${CampYLoc} radius ${MeleeDistance} zradius ${MaxZRange}]}
                                }
                                /if (!${MobCount} && ${EngageWaitTimer}) /continue
                                /break
                            }
                        }
                    }
                    /call GetCombatTarget
                }
                /call Combat
                | Check if you FD'd to loose aggro and never stood back up.
                /if (${Me.Feigning} || ${Me.Invis}) /call FeignAggroCheck
                DEBUGCOMBAT  CheckForCombat: Assist/Combat End ${FromWhere}
                /if (${ChainPull}==2) /return
            }
            /if (${MezOn}) {
                /call MezCheck CheckForCombat
            }
			/if (${CharmOn}) {
                /call CharmCheck CheckForCombat
            }
            | Check to see if you are a healer and DPS is off, or DPS is on but the mob is not in LOS
            /while (${SkipCombat}==1 || (${DPSOn} && ${HealsOn} && ${MyTargetID} && ${Spawn[${MyTargetID}].LineOfSight}==FALSE)) {
                DEBUGCOMBAT CheckForCombat ${SkipCombat} ${Spawn[${MyTargetID}].LineOfSight} ${Spawn[${MyTargetID}].ID} ${MyTargetID}
                /if (${IAmABard} && ${MeleeTwistOn}==2 && !${DPSTwisting}) /call DoBardStuff
                /while (1) {
                    /varset EventFlag 0
                    /doevents
                    /if (!${EventFlag}) /break
                }
                /if (${AggroTargetID} || ${CalledTargetID}) {
                    /call CombatTargetCheck ${If[${SkipCombat},0,1]}
                    /if (!${AggroTargetID}) /continue
                    /if (${AggroOn}) {
                        /call AggroCheck
                        | Check if you FD'd to loose aggro and never stood back up.
                        /if (${Me.Feigning} || ${Me.Invis}) /call FeignAggroCheck
                    }
                    /if (!${CombatStart} && ${BeforeArray[1].NotEqual[null]}) {
                        /call BeforeAttack ${MyTargetID} 1
                        /varset CombatStart 1
                    }
                    /if (${CuresOn}) {
                        /call CheckCures CheckForCombat
                    } else /if (${HealsOn}) {
                        /call CheckHealth CheckForCombat2
                    }
                    /if (${AutoRezOn}) /call RezCheck group
                    /if (!${NamedCheck} && (${BurnAllNamed} || ${GMailEvents.Find[named]})) /call NamedWatch ${SkipCombat}
                    /call CastMana CheckForCombat
                    /call WriteDebuffs
                    /if (${MyTargetID} && (${Spawn[${MyTargetID}].Type.Equal[Corpse]} || !${Spawn[${MyTargetID}].ID})) /call CombatReset 0 CheckForCombat0
                    /if (${WhoToChase.NotEqual[${Me}]} && ${ChaseAssist}) /call DoWeChase
                } else {
                    /if (${Role.NotEqual[manual]} && ${CombatStart}) /call CombatReset 0 CheckForCombat1
                    /return
                }
            }
            /call CheckForAdds CheckForCombat
            /if (${Select[${Role},tank,pullertank]}) {
                | If tank is assisting puller and in chase mode
                /if (${WhoToChase.NotEqual[${Me}]} && ${ChaseAssist}) /call DoWeChase
                /if (${ReturnToCamp} && ((!${MobCount} && ${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]}>15) || (${MobCount}==1 && ${AggroTargetID} && ${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]}>75))) /call DoWeMove 1 checkforcombat
                /if (${MobCount} && ${AggroTargetID}) /continue
                /if (${BuffsOn}) {
                    /for GlobalIndex 1 to ${BuffsSize}
                        /if (${Buffs[${GlobalIndex}].Arg[2,|].NotEqual[end]}) /continue
                        /if (${Me.PctEndurance}<=${Buffs[${GlobalIndex}].Arg[3,|]} && (${Me.CombatAbilityReady[${Buffs[${GlobalIndex}].Arg[1,|]}]} || ${Me.AltAbilityReady[${Buffs[${GlobalIndex}].Arg[1,|]}]})) /call CheckEndurance "${Buffs[${GlobalIndex}].Arg[1,|]}" ${Buffs[${GlobalIndex}].Arg[3,|]} ${Buffs[${GlobalIndex}].Arg[4,|]}
                    /next GlobalIndex
                }
                DEBUGCOMBAT CheckForCombat MobCount: ${MobCount} AggroTargetID: ${AggroTargetID} ChainPull: ${ChainPull} ${SkipCombat}
                /if (${TankTimer}) /continue
            } else /if (${Role.NotEqual[manual]} ) {
                /if (${MyTargetID} && (${Spawn[${MyTargetID}].Type.Equal[Corpse]} || !${Spawn[${MyTargetID}].ID})) /call CombatReset 0 CheckForCombat2
            }
            /call MobRadar los ${MeleeDistance} CheckForCombat
			/if (${CharmOn}==1) /call CharmRadar los ${MeleeDistance} CheckForCombat
			
            DEBUGCOMBAT CheckForCombat MobCount: ${MobCount} AggroTargetID: ${AggroTargetID} ChainPull: ${ChainPull} ${SkipCombat} ${MezMobFlag}
            /if ((!${AggroTargetID} && (!${MezMobFlag} || !${MobCount}) && !${ChainPull}) || (${MobCount}<2 && ${ChainPull})) /break
            | if I am in melee distance of camp and there are no mobs in melee distance then we need to return to camp.
            | even if there are mobs in XTarget that doesn't mean we should NOT return to camp.
            /if (${ReturnToCamp} && ${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]}>${CampRadius} && !${SpawnCount[xtarhater loc ${CampXLoc} ${CampYLoc} radius ${MeleeDistance}]}) /call DoWeMove 1 checkforcombat
        }
        /if (${MobCount}==1 && ${ChainPull}==2) /return
        /if (${ReturnToCamp} && ${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]} > 15) {
            /if (!${IAmMA}) /delay ${Math.Calc[${Math.Rand[1]}+1]}s
            /if (${MobCount}<=0) /call DoWeMove 0 checkforcombat
        } else /if (${Select[${Role},hunter,hunterpettank]} && ${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}>${Math.Calc[${MaxRadius}*.95]}) {
            DEBUGPULL PullHunter Returning hunter to camp
            /echo ${Role}: Reached edge of ${MaxRadius} hunting radius. Trying to return to camp.
            /call FindMobOnReturn
        }
        DEBUGCOMBAT CheckForCombat leave.
    /return
| -------------------------------------------------------------------------------------
| SUB: GetCombatPosition - move to LOS of Mob
| -------------------------------------------------------------------------------------
    Sub GetCombatPosition(int moveToTarget)
        DEBUGCOMBAT GetCombatPosition Enter - ${Spawn[${MyTargetID}].Distance3D} - ${MeleeDistance}
        /declare GMPTimer timer local 0
        /declare moveToAllow int local 1

        /if (${DPSPaused}) {
            /varset moveToAllow 0
        } else /if (!${moveToTarget}) {
            /if (${Spawn[${MyTargetID}].Distance3D}>${MeleeDistance}) {
                /varset moveToAllow 0
            } else /if (${Spawn[${MyTargetID}].PctHPs}>${AssistAt}) {
                /varset moveToAllow 0
            }
        }

        /if (${moveToAllow}) {
            /varset GMPTimer 50
            /if (${PullMoveUse.Equal[nav]}) {
                /nav id ${MyTargetID}
                /while (${GMPTimer}) {
                    /if (${Spawn[${MyTargetID}].Distance3D}<${MeleeDistance} && ${Spawn[${MyTargetID}].LineOfSight}) {
                        /nav stop
                        /break
                    }
                    /delay 1
                }
                /if (${Navigation.Active}) /nav stop
            } else {
                /moveto id ${MyTargetID}
                /while (${GMPTimer}) {
                    /if (${Spawn[${MyTargetID}].Distance3D}<${MeleeDistance} && ${Spawn[${MyTargetID}].LineOfSight}) {
                        /moveto off
                        /break
                    }
                    /delay 1
                }
                /if (${MoveTo.Moving}) /moveto off
            }
        } else {
            /if (${Spawn[${MyTargetID}].Distance3D}>${MeleeDistance} || ${Spawn[${MyTargetID}].PctHPs}>${AssistAt}) {
                /if (!${SpamTimer3}) {
                    /echo Get Combat Position Failed. Target is too far away or Not ready to attack.
                    /varset SpamTimer3 50
                }
            }
        }
        DEBUGCOMBAT GetCombatPosition Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Assist - Get Main Tank's target
| -------------------------------------------------------------------------------------
    Sub Assist(FromWhere)
        /if (${DMZ} && ${Me.InInstance}==FALSE) /return
        /if (!${MeleeOn} && !${DPSOn} && !${MezOn}) /return
        /if (${DPSPaused}) /return
        /if (${Me.Hovering}) /return
        /if (${Pulled} && ${MyTargetID} && ${IAmMA}) /return
        DEBUGCOMBAT Assist Enter
        /declare TempTargetID int local
        /doevents
        /call MobRadar los ${MeleeDistance} Assist
        | Normal assist check
        DEBUGCOMBAT Assist0: ${Me} ${MainAssist} ${IAmMA} ${Spawn[=${MainAssist}].Distance} ${MobCount} ${AggroTargetID} ${Target.ID} ${MyTargetID} ${Me.Combat}
        /if (${MobCount} || ${AggroTargetID}) {
            /if (${Spawn[=${MainAssist}].ID}) {
                /if (${Spawn[=${MainAssist}].Distance}<200) {
                    DEBUGCOMBAT Assist1: ${Group.MainAssist.ID} ${Spawn[=${MainAssist}].ID} ${Me.GroupAssistTarget.ID} ${Me.XTarget[${XTSlot}].ID} ${Me.XTarget[${XTSlot2}].ID}
                    /if (${Group.MainAssist.ID} && ${Group.MainAssist.ID}==${Spawn[=${MainAssist}].ID}) {
                        /if (${Bool[${Me.GroupAssistTarget.ID}]}==TRUE) {
                            /target id ${Me.GroupAssistTarget.ID}
                            /delay 10 ${Me.GroupAssistTarget.ID}==${Target.ID}
                        } else /if (${Target.ID}) {
                            /squelch /target clear
                            /delay 10 !${Target.ID}
                            /varset MyTargetID 0
                        }
                    } else {
                        /assist ${MainAssist}
                        /delay 5s ${Me.AssistComplete}==TRUE
                        /if (${XTarAutoSet} && !${Group.Member[${MainAssist}].Index} && !${IAmMA} && !${Me.XTarget[${XTSlot}].ID} && ${Target.Type.NotEqual[PC]}) /xtarget set ${XTSlot} currenttarget
                    }
                | If the tank is dead and aggro mob in camp target 1st mob on autohater in xtarget
                } else /if (${Role.Equal[offtank]}) {
                    /return switch
                } else /if (${AggroTargetID} && ${Spawn[${AggroTargetID}].Distance}<=${MeleeDistance}) {
                    /squelch /target id ${AggroTargetID}
                    /delay 10
                }
            }
        }
        DEBUGCOMBAT Assist: validatetarget ID:${Target.ID}
        /if (!${Target.ID}) {
            DEBUGCOMBAT Assist: Validate Target - No target return
            /return
        }
        /varset TempTargetID ${Target.ID}
        /call ValidateTarget
        | If target is not valid return
        DEBUGCOMBAT Assist Target:${Target.CleanName} ValidTarget:${ValidTarget} Marco Return:${Macro.Return}
        /if (${ValidTarget}==0) {
            /varset MyTargetID 0
            /varset MyTargetName
            /return
        } else {
            /if (${Target.ID}!=${TempTargetID}) {
               /squelch /target clear
               /delay 10 !${Target.ID}
               /target id ${TempTargetID}
               /delay 10 ${Target.ID}
            }
            /if (${XTarAutoSet} && !${Group.Member[${MainAssist}].Index} && !${IAmMA} && !${Me.XTarget[${XTSlot}].ID} && ${Target.Type.NotEqual[PC]}) /xtarget set ${XTSlot} currenttarget
            /varset MyTargetID ${Target.ID}
            /varset MyTargetName ${Target.CleanName}
            /varset LastTargetID ${MyTargetID}
        }
        DEBUGCOMBAT Assist MyTargetName:${MyTargetName} ID:${MyTargetID} !${IAmMA}
        DEBUGCOMBAT Assist Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: GetCombatTarget - Main Tank selects target to kill.
| -------------------------------------------------------------------------------------
    Sub GetCombatTarget
        /if (${DMZ} && ${Me.InInstance}==FALSE) /return
        /if (!${MeleeOn} && !${DPSOn} && !${MezOn}) /return
        /if (${DPSPaused}) /return
        /if (${Me.Hovering}) /return
        /if (${Pulled} && ${MyTargetID} && ${IAmMA}) /return
        DEBUGCOMBAT GetCombatTarget Enter
        /declare j int local
        /declare n int local
        /declare HighestMobID  int local 0
        /declare ClosestMobID  int local 0
        /declare MostHurtMobID int local 0
        /declare BlurredMobID  int local 0
        /declare TempTargetID  int local
        /doevents
        /if (${AggroTargetID2} && (!${Spawn[${AggroTargetID2}].ID} || ${Spawn[${AggroTargetID2}].Type.Equal[corpse]})) /varset AggroTargetID2 0
        /call MobRadar los ${MeleeDistance} GetCombatTarget
        | Normal assist check
        DEBUGCOMBAT GetCombatTarget0: ${Me} ${MainAssist} ${Spawn[=${MainAssist}].Distance} ${MobCount} ${AggroTargetID} ${Target.ID} ${MyTargetID} ${Me.Combat}
        /if (${IAmMA} || (!${Spawn[=${MainAssist}].ID} && ${Role.Equal[offtank]})) {
            DEBUGCOMBAT GetCombatTarget2: MainAssist.Equal[${Me}] is ${IAmMA} OR my role is ${Role} MobCount = ${MobCount}
            /if (${Target.CleanName.Equal[${Me}]} && (${MobCount} || ${AggroTargetID})) /squelch /target clear
            | I am tank with a single mob wait for it then validate and attack.
            /if (${MobCount}==1 && (!${XTSlot2} || (${XTSlot2} && !${Me.XTarget[${XTSlot2}].ID})) && ${AggroTargetID}) {
                DEBUGCOMBAT GetCombatTarget: ${MobCount}==1 && ${AggroTargetID}
                /squelch /target id ${AggroTargetID}
                /delay 10 ${Target.ID}==${AggroTargetID}
                DEBUGCOMBAT GetCombatTarget: Single target: ${AggroTargetID} ${Spawn[${AggroTargetID}].CleanName}
            } else /if ((${MobCount}>=2 || (${XTSlot2} && ${Me.XTarget[${XTSlot2}].ID})) && ${AggroTargetID}) {
                | More than one mob
                | Check if named incoming an target if true else assign closest target.
                /if (${Spawn[xtarhater named].ID}) {
                    /varset TempTargetID ${Spawn[xtarhater named].ID}
                    /squelch /target id ${TempTargetID}
                    /delay 10 ${Target.ID}==${TempTargetID}
                } else /if (${Alert[4].Size} && ${Spawn[xtarhater alert 4].ID} && ${Spawn[xtarhater alert 4].Type.NotEqual[Corpse]}) {
                    /varset TempTargetID ${NearestSpawn[1, xtarhater alert 4].ID}
                    /call BroadCast y "Grabbing MEZ Immune -> ${Spawn[${TempTargetID}].Name} <- ID:${Spawn[${TempTargetID}].ID} "
                    /squelch /target id ${TempTargetID}
                    /delay 10 ${Target.ID}==${TempTargetID}
                } else {
                    /varset j ${SpawnCount[xtarhater]}
                    /if (${j}) {
                        /varset TempTargetID ${NearestSpawn[1, xtarhater].ID}
                        /varset ClosestMobID ${TempTargetID}
                        /varset HighestMobID ${TempTargetID}
                        /varset MostHurtMobID ${TempTargetID}
                        /for n 1 to ${j}
                            /varset TempTargetID ${NearestSpawn[${n}, xtarhater].ID}
                            /if (${Spawn[${TempTargetID}].PctHPs}<${Spawn[${MostHurtMobID}].PctHPs}) /varset MostHurtMobID ${TempTargetID}
                            /if (${Spawn[${TempTargetID}].Level}>${Spawn[${HighestMobID}].Level}) /varset HighestMobID ${TempTargetID}
                        /next n
                        | Don't need to target mob on incoming and out of meleedistance'
                        /varset TempTargetID ${ClosestMobID}
                        /if (${ReturnToCamp}) {
                            /if (${MostHurtMobID} && ${ClosestMobID}!=${MostHurtMobID} && ${Math.Distance[${Spawn[${MostHurtMobID}].Y},${Spawn[${MostHurtMobID}].X}:${CampYLoc},${CampXLoc}]}<=${MeleeDistance}) /varset TempTargetID ${MostHurtMobID}
                            /if (${TempTargetID}!=${HighestMobID}) {
                                /if (${Spawn[${TempTargetID}].Level}<${Spawn[${HighestMobID}].Level} && ${Select[${Spawn[${TempTargetID}].Animation},26,32,71,72,110,111]} && ${Math.Distance[${Spawn[${HighestMobID}].Y},${Spawn[${HighestMobID}].X}:${CampYLoc},${CampXLoc}]}<=${MeleeDistance}) /varset TempTargetID ${HighestMobID}
                            }
                            /if (${Target.ID}!=${TempTargetID} && ${Math.Distance[${Spawn[${TempTargetID}].Y},${Spawn[${TempTargetID}].X}:${CampYLoc},${CampXLoc}]}<=${MeleeDistance}) {
                                /squelch /target id ${TempTargetID}
                                /delay 10 ${Target.ID}==${TempTargetID}
                            }
                        } else {
                            /if (${MostHurtMobID} && ${ClosestMobID}!=${MostHurtMobID} && ${Spawn[${MostHurtMobID}].Distance}<=${MeleeDistance}) /varset TempTargetID ${MostHurtMobID}
                            /if (${TempTargetID}!=${HighestMobID}) {
                                /if (${Spawn[${TempTargetID}].Level}<${Spawn[${HighestMobID}].Level} && ${Select[${Spawn[${TempTargetID}].Animation},26,32,71,72,110,111]} && ${Math.Distance[${Spawn[${HighestMobID}].Y},${Spawn[${HighestMobID}].X}:${CampYLoc},${CampXLoc}]}<=${MeleeDistance}) /varset TempTargetID ${HighestMobID}
                            }
                            /if (${Target.ID}!=${TempTargetID} && ${Spawn[${TempTargetID}].Distance}<=${MeleeDistance}) {
                                /squelch /target id ${TempTargetID}
                                /delay 10 ${Target.ID}==${TempTargetID}
                            }
                        }
                    }
                }
                DEBUGCOMBAT ClasestMobID: ${ClosestMobID} Distance: ${Math.Distance[${Spawn[${ClosestMobID}].Y},${Spawn[${ClosestMobID}].X}:${CampYLoc},${CampXLoc}]} XTarget[${XTSlot2}] ID: ${Me.XTarget[${XTSlot2}].ID} Distance: ${Math.Distance[${Me.XTarget[${XTSlot2}].Y},${Me.XTarget[${XTSlot2}].X}:${CampYLoc},${CampXLoc}]} AggroTargetID: ${AggroTargetID} MostHurtID: ${MostHurtMobID}
            } else /if (!${AggroTargetID} && ${MobCount} && ${MezMobFlag}==1) {
                | Check for memblurred mobs in camp when nothing left on xtarget. Check only for 5 secs after last mob.
                DEBUGCOMBAT GetCombatTarget3: ${AggroTargetID} ${MobCount} ${MezMobFlag}
                /varset BlurredMobID ${NearestSpawn[1,npc targetable los radius ${MeleeDistance} zradius 50 noalert 3].ID}
                DEBUGCOMBAT GetCombatTarget4: ${BlurredMobID} ${Math.Distance[${Spawn[${BlurredMobID}].Y},${Spawn[${BlurredMobID}].X}:${CampYLoc},${CampXLoc}]} ${MeleeDistance}
                /if (${BlurredMobID} && ${Math.Distance[${Spawn[${BlurredMobID}].Y},${Spawn[${BlurredMobID}].X}:${CampYLoc},${CampXLoc}]}<${MeleeDistance}) {
                    /squelch /target ID ${BlurredMobID}
                    /varset MezMobFlag 0
                    /delay 30 ${Target.ID}==${BlurredMobID} && ${Target.BuffsPopulated}
                    /if (${Target.Mezzed.ID}) {
                        /call BroadCast y "Found Mezzed Mob: ${Spawn[${BlurredMobID}].CleanName} <- ID:${BlurredMobID}"
                        /echo MEZ >> Found a Mezzed mob in Camp. <<
                        /varset AggroTargetID2 ${BlurredMobID}
                        /varset MyTargetID ${Target.ID}
                        /varset MyTargetName ${Target.CleanName}
                        /varset LastTargetID ${MyTargetID}
                    } else {
                        /if (${Target.ID}) /squelch /alert add 3 id ${Target.ID}
                    }
                    /return
                }
                /varset MezMobFlag 0
            }
        }
        DEBUGCOMBAT GetCombatTarget: validatetarget ID: ${Target.ID}
        /if (!${Target.ID}) {
            DEBUGCOMBAT GetCombatTarget: Validate Target - No target return
            /return
        }
        /varset TempTargetID ${Target.ID}
        /call ValidateTarget
        | If target is not valid return
        DEBUGCOMBAT GetCombatTarget Target:${Target.CleanName} ValidTarget:${ValidTarget} Marco Return:${Macro.Return}
        /if (${ValidTarget}==0) {
            /varset MyTargetID 0
            /varset MyTargetName
            /return
        } else {
            /if (${Target.ID}!=${TempTargetID}) {
               /squelch /target clear
               /delay 10 !${Target.ID}
               /target id ${TempTargetID}
               /delay 10 ${Target.ID}
            }
           /varset MyTargetID ${Target.ID}
           /varset MyTargetName ${Target.CleanName}
           /varset LastTargetID ${MyTargetID}
        }
        DEBUGCOMBAT GetCombatTarget MyTargetName:${MyTargetName} ID:${MyTargetID}
        DEBUGCOMBAT GetCombatTarget Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Validate Target Combined Spawn and Target
| -------------------------------------------------------------------------------------
    Sub ValidateTarget(int SpawnID)
        DEBUGCOMBAT ValidateTarget Enter Target Name:${Target.CleanName} ID:${Target.ID} Spawn Name: ${Spawn[${SpawnID}].CleanName} ID:${SpawnID}
        /declare j int local
        /declare PullValid int local 0
        /declare MobID int local 0
        /declare MobName string local
        /declare MobNameCheck string local
        /declare MobType string local
        /declare MeleeDistanceCheck int local ${MeleeDistance}
        /varset ValidTarget 0
        /if (${SpawnID}) {
           /varset MobID ${Spawn[${SpawnID}].ID}
           /varset MobName ${Spawn[${SpawnID}].CleanName}
           /varset MobType ${Spawn[${SpawnID}].Type}
        } else {
           /varset MobID ${Target.ID}
           /varset MobName ${Target.CleanName}
           /varset MobType ${Target.Type}
        }
        /if (!${MobID}) /return NoTarget
        DEBUGCOMBAT ValidateTarget Validate 1
        /if (${Select[${MobType},AURA,BANNER,CAMPFIRE,CORPSE,CHEST,ITEM,TRIGGER,TRAP,TIMER,MOUNT,Mercenary]}) /return BadTargetType
        DEBUGCOMBAT ValidateTarget Validate 2
        | If mob found on ignore and not on xtarget skip
        /if (${MobsToIgnore.Find[${MobName}]}) {
            /if (${Select[${MobID},XTCHECK]}==0) /return MobOnIgnoreList
        }
        /if (${MobsToIgnoreByID.Find[${MobID}|]}) /return MobOnIgnoreListByID
        | Check if target is somewhere on my extended target.
        /if (${Select[${Role},tank]} && ${MobCount}<=${XSlotTotal} && ${Spawn[${MainAssist} ${MainAssistType} group].ID}) {
            /if (${SpawnCount[id ${MobID} xtarhater]}==0) /return NotOnXTarget
        }
        /if (${Spawn[id ${MobID}].MaxRangeTo}>${MeleeDistance}) /varcalc MeleeDistanceCheck ${Spawn[id ${MobID}].MaxRangeTo}+5
        /if (${ReturnToCamp} && ${Math.Distance[${Spawn[${MobID}].Y},${Spawn[${MobID}].X}:${CampYLoc},${CampXLoc}]}>${MeleeDistanceCheck} && !${Pulling} && ${Select[${Role},tank,pullertank,pettank,pullerpettank]}) /return OutofCampRadius
        DEBUGCOMBAT ValidateTarget Validate 3
        /if (${MobName.Find[eye of]} && ${SpawnCount[pc ${MobName.Right[${Math.Calc[${MobName.Length}-7]}]}]}) /return Spell-Eye-PC
        /if (${MobType.Equal[Pet]} && ${Spawn[${MobID}].Master.Type.Equal[PC]}) /return PET-PC
        /if (${Target.ID}==${MobID} && ${Target.Charmed.ID}) /return Target-Charmed
        /if (!${Select[${EverQuest.Server},zek]} && ${Select[${MobType},PC]}) /return PC
        | This prevents characters on the Zek server from attacking PC targets in their group
        /if (${Select[${EverQuest.Server},zek]} && ${Select[${MobType},PC]}) {
            /if (${Spawn[id ${MobID} group].ID}>0) /return GroupMember
            /if (${MobID}==${Spawn[=${MainAssist}].ID}) /return MA
        }
        DEBUGCOMBAT ValidateTarget Validate 4
        | Only valid when pulling
        /if (${Pulling} && !${Pulled}) {
            /if (${PullIgnore1.Find[${MobID}|]}) /return MobOnPullIgnoreListByID
            /if (${Select[${PullMoveUse},los,nav]}>0) {
                /if (${Math.Distance[${Spawn[${MobID}].Y},${Spawn[${MobID}].X}:${CampYLoc},${CampXLoc}]}>${MaxRadius} && ${Spawn[${MobID}].Distance3D}>${PullRange}) /return OutofRadius
                /if (${PullMoveUse.Equal[los]} && ${Select[${Role},puller,pullertank,pullerpettank]} && !${Spawn[${MobID}].LineOfSight}) /return NoLOS
            }
            /if (${Spawn[${MobID}].PctHPs}<=99) /return PullNotFullHealth
            /if (${SpawnCount[notid ${Me.ID} loc ${Spawn[${MobID}].X} ${Spawn[${MobID}].Y} radius 30 pc nogroup]}>0) /return PCNear
            /if (${Spawn[${MobID}].Level}<${PullMin} || ${Spawn[${MobID}].Level}>${PullMax}) /return BadLevel
            /if (${Window[TargetOfTargetWindow].Open} && ${Target.ID}==${MobID}) {
            | - If mob I'm pulling is aggroed already by non damaging spell by someone else.
               /delay 10 ${Target.ID}==${MobID}
               /if (${Me.TargetOfTarget.Type.Equal[PC]} && ${Me.TargetOfTarget.Name.NotEqual[${Me}]} && !${Group.Member[${Me.TargetOfTarget.Name}].Index}) /return PullToTTNotPuller
               /if (${Me.TargetOfTarget.Type.Equal[PET]} && ${Me.TargetOfTarget.Name.NotEqual[${Me.Pet.CleanName}]}) /return PullToTTNotMyPet
               /if (${Me.TargetOfTarget.Type.Equal[mercenary]} && !${Group.Member[${Me.TargetOfTarget.Name}].Index}) /return PullToTNotMyMerc
            }
            /if (${DebugPull} && ${MobsToPull.NotEqual[all]}) /echo ValidateTarget ${MobsToPull}
            /if (${MobsToPull.Equal[all]}) {
                /varset PullValid 1
            } else {
                /for j 1 to 25
                    /varset MobNameCheck ${MobsToPull.Arg[${j},,]}
                    /if (${MobNameCheck.Find[*]}) {
                        /varset MobNameCheck ${MobNameCheck.Replace[*,]}
                    } else /if (${MobNameCheck.Find[#]}) {
                        /varset MobNameCheck ${MobNameCheck.Replace[#,]}
                    }
                    DEBUGPULL ValidateTarget ${j} ${MobsToPull.Arg[${j},,]} ${MobName.Find[${MobNameCheck}]}
                    /if (${MobName.Find[${MobNameCheck}]}) {
                        /varset PullValid 1
                        /break
                    }
                /next j
            }
            /if (!${PullValid}) /return PullMobNotonList
        }
        /varset ValidTarget 1
        DEBUGCOMBAT ValidateTarget Leave
    /return ${ValidTarget}

| -------------------------------------------------------------------------------------
| SUB: Check Before Cast
| -------------------------------------------------------------------------------------
    Sub CheckBeforeCast(SentFrom)
        /if (${GroupEscapeOn} && ${Select[${Me.Class.ShortName},WIZ,DRU]} && ${Raid.Members}==0 && ${SentFrom.Equal[CombatCast]}) /call GroupEscape
        /if (${MezOn} && ${SentFrom.Equal[CombatCast]}) {
            /call MezCheck CheckBeforeCombat

        }
	    /if (${CharmOn} && ${SentFrom.Equal[CombatCast]}) {
            /call CharmCheck CheckBeforeCombat

        }
        /if (${CuresOn}) {
            /call CheckCures CheckBeforeCast
        } else /if (${HealsOn}) {
            /if (!${LastHealCheck}) {
                /call CheckHealth CheckBeforeCast
                /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
            }
        } else /if (${AutoRezOn}) {
            /call RezCheck
        }
        /if (${AggroTargetID}) {
            /if (${Cursor.ID}) /call CheckCursor CombatCast 0
            /if (${MercOn} && ${MercAssisting}!=${MyTargetID} && ${Mercenary.State.Equal[Active]}) /call MercsDoWhat
            /if (${AEOn}) /call AECheck ${AEDisplayMobInfo}
            /if (${AggroOn}) /call AggroCheck
            /if (${Spawn[${MyTargetID}].PctHPs}<=${PetAssistAt} && !${Pet.Combat}) /call CombatPet
        } else {
            /doevents
        }
    /return
| -------------------------------------------------------------------------------------
| SUB: CombatReset
| -------------------------------------------------------------------------------------
    Sub CombatReset(int SFlag, string CalledFrom)
        DEBUGCOMBAT CombatReset: Enter ${SFlag} ${CalledFrom}
        /declare DPSCalc string local
        /declare DPSTCalc string local
        /declare DPSMyDam string local
        /declare DPSPCalc string local
        /declare DPSPTCalc string local
        /declare DPSMyPetDam string local
        /declare DPSMeAndPetDam string local
        /declare DPSMPCalc string local
        /declare DPSMPTCalc string local
        /declare i int local
        /declare j int local
        /if (!${MyTargetID} && ${LastTargetID}) /varset MyTargetID ${LastTargetID}
        /if (${MyTargetID} && ${CombatStart} && ${DPSMeter} && !${SFlag}) {
            /echo SLAIN >> ${MyTargetName} <<
            /echo EXP: ${If[${Me.PctExp}<${MyExp},${Math.Calc[100-${MyExp}+${Me.PctExp}]},${Math.Calc[${Me.PctExp}-${MyExp}]}]}% AAEXP: ${If[${Me.PctAAExp}<${MyAAExp},${Math.Calc[100-${MyAAExp}+${Me.PctAAExp}]},${Math.Calc[${Me.PctAAExp}-${MyAAExp}]}]}%
            /popup SLAIN >> ${MyTargetName} << EXP: ${If[${Me.PctExp}<${MyExp},${Math.Calc[100-${MyExp}+${Me.PctExp}]},${Math.Calc[${Me.PctExp}-${MyExp}]}]}% AAEXP: ${If[${Me.PctAAExp}<${MyAAExp},${Math.Calc[100-${MyAAExp}+${Me.PctAAExp}]},${Math.Calc[${Me.PctAAExp}-${MyAAExp}]}]}%
            /if (!${DPSAdv.TimeElapsed} || ${DPSAdv.MyStatus}) {
                /echo DPS Meter: Was a timing error skipping DPS Meter
                /DPSAdv myreset
            } else {
                /echo \aw DPS Meter - \ay Your Total Damage: \at ${DPSAdv.MyDamage.Prettify} - \ay Fight Time: \aw ${DPSAdv.TimeElapsed}s - \ay DPS: \ag ${DPSAdv.MyDPS.Prettify}
                /if (!${DPSAdv.PetDamage}) {
                    /call BroadCast r "${Me}'s DPS: ${DPSAdv.MyDPS.Prettify}"
                } else {
                    /echo \aw DPS Meter - \ay Pet Total Damage: \at ${DPSAdv.PetDamage.Prettify} \ay - Pet DPS: \ag ${DPSAdv.PetDPS.Prettify}
                    /call BroadCast r "${Me}'s DPS: ${DPSAdv.MyDPS.Prettify} + Pet = Total: ${DPSAdv.TotalDPS.Prettify}"
                    /echo \aw DPS Meter - \ay You + Pet Total Damage: \at ${DPSAdv.TotalDamage.Prettify} \ay - DPS: \ag ${DPSAdv.TotalDPS.Prettify}
                }
                /popup Total Damage: ${DPSAdv.TotalDamage.Prettify} - Fight Time: ${DPSAdv.TimeElapsed}s - DPS: ${DPSAdv.TotalDPS.Prettify}
                /if (${Defined[ParseDPS]} || ${DPSWriteOn}) {
                    /ini "Kissassist_DPS" "${MyTargetName}-${MyTargetID} - ${Time.Month}/${Time.Day}/${Time.Year}-${Time.Time12}" "DPS-${Me.Class}-${Me.Level}" "Total Damage: ${DPSAdv.MyDamage.Prettify} - Fight Time: ${DPSAdv.TimeElapsed}s - DPS: ${DPSAdv.MyDPS.Prettify}"
                    /if (${DPSAdv.PetDamage}) {
                        /ini "Kissassist_DPS" "${MyTargetName}-${MyTargetID} - ${Time.Month}/${Time.Day}/${Time.Year}-${Time.Time12}" "DPS-MyPet" "My Pet Total Damage: ${DPSAdv.PetDamage.Prettify} - Pet DPS: ${DPSAdv.PetDPS.Prettify}"
                        /ini "Kissassist_DPS" "${MyTargetName}-${MyTargetID} - ${Time.Month}/${Time.Day}/${Time.Year}-${Time.Time12}" "DPS-Combined" "Me + Pet Total Damage: ${DPSAdv.TotalDamage.Prettify} - Combined DPS: ${DPSAdv.TotalDPS.Prettify}"
                    }
                }
            }
        }
        /call MobRadar los ${MeleeDistance} CombatReset
        |/for j 1 to 50
        |    /if (${AddsArray[${j},1].Equal[${MyTargetID}]}) /call RemoveFromArray AddsArray ${j}
        |    /if (${MobCount}==0) /call RemoveFromArray AddsArray ${j}
        |    /if (${MezOn} && ${j}<=13) {
        |        /if (${MezArray[${j},1].Equal[${MyTargetID}]}) {
        |            /call RemoveFromArray MezArray ${j}
        |        } else /if (${MezArray[${j},1].NotEqual[NULL]}) {
        |            /if (!${Spawn[${MezArray[${j},1]}].ID} || ${Spawn[${MezArray[${j},1]}].Type.Equal[Corpse]}) /call RemoveFromArray MezArray ${j}
        |        }
        |    }
        |/next j
		/if (${CharmOn}) {
            /for j 1 to ${XSlotTotal}
                /if (${CharmArray[${j},1].Equal[${MyTargetID}]}) {
                    /call RemoveFromArray CharmArray ${j}
                } else /if (${CharmArray[${j},1].NotEqual[NULL]}) {
                    /if (!${Spawn[${CharmArray[${j},1]}].ID} || ${Spawn[${CharmArray[${j},1]}].Type.Equal[Corpse]}) /call RemoveFromArray CharmArray ${j}
                }
            /next j

            /if (${CharmImmuneIDs.Find[|${MyTargetID}]}) /varset CharmImmuneIDs ${CharmImmuneIDs.Replace[|${MyTargetID},]}
            /varset j 1
            /while (${Bool[${CharmImmuneIDs.Arg[${j},|].Length}]}) {
                /if (!${Bool[${Spawn[${CharmImmuneIDs.Arg[${j},|]}].ID}]} || ${Spawn[${CharmImmuneIDs.Arg[${j},|]}].Type.Equal[corpse]} ) {
                    /varset CharmImmuneIDs ${CharmImmuneIDs.Replace[|${CharmImmuneIDs.Arg[${j},|]},]}
                } else {
                    /varcalc j ${j}+1
                }
            }
            /varset CharmMobDone 0
        }
        /if (${MezOn}) {
            /for j 1 to ${XSlotTotal}
                /if (${MezArray[${j},1].Equal[${MyTargetID}]}) {
                    /call RemoveFromArray MezArray ${j}
                } else /if (${MezArray[${j},1].NotEqual[NULL]}) {
                    /if (!${Spawn[${MezArray[${j},1]}].ID} || ${Spawn[${MezArray[${j},1]}].Type.Equal[Corpse]}) /call RemoveFromArray MezArray ${j}
                }
            /next j

            /if (${MezImmuneIDs.Find[|${MyTargetID}]}) /varset MezImmuneIDs ${MezImmuneIDs.Replace[|${MyTargetID},]}
            /varset j 1
            /while (${Bool[${MezImmuneIDs.Arg[${j},|].Length}]}) {
                /if (!${Bool[${Spawn[${MezImmuneIDs.Arg[${j},|]}].ID}]} || ${Spawn[${MezImmuneIDs.Arg[${j},|]}].Type.Equal[corpse]} ) {
                    /varset MezImmuneIDs ${MezImmuneIDs.Replace[|${MezImmuneIDs.Arg[${j},|]},]}
                } else {
                    /varcalc j ${j}+1
                }
            }
            /varset MezMobDone 0
        }
        /if (${MobsToIgnoreByID.NotEqual[null]}) {
            /varset j 1
            /while (${Bool[${MobsToIgnoreByID.Arg[${j},|].Length}]} && ${MobsToIgnoreByID.Arg[${j},|].NotEqual[null]}) {
                /if (!${Bool[${Spawn[${MobsToIgnoreByID.Arg[${j},|]}].ID}]} || ${Spawn[${MobsToIgnoreByID.Arg[${j},|]}].Type.Equal[corpse]} ) {
                    /varset MobsToIgnoreByID ${MobsToIgnoreByID.Replace[|${MobsToIgnoreByID.Arg[${j},|]},]}
                } else {
                    /varcalc j ${j}+1
                }
            }
        }
        /varset CalledTargetID 0
        /varset AggroTargetID2 0
        /if (${Defined[DPSTimeStart${MyTargetID}]}) /deletevar DPSTimeStart${MyTargetID}
        /varset MyTargetID 0
        /varset MyTargetName
        /varset LastTargetID 0
        /varset ValidTarget 0
        /varset MercAssisting 0
        /varset CombatStart 0
        /varset Pulled 0
        /varset NamedCheck 0
        /if (${Role.Find[puller]}) {
            /call PullReset
        } else {
            /moveto dist 10
            /if (${MoveTo.Moving}) /moveto off
            /if (${PullMoveUse.Equal[nav]}) {
                /if (${Navigation.Active}) /nav stop
            }
        }
        /if (${UseMQ2Melee}) /squelch /melee melee=0
        /squelch /attack off
        /if (${Me.AutoFire}) /autofire
        /if (${AutoFireOn}==2) /varset AutoFireOn 1
        /squelch /target clear
        DEBUGCOMBAT CombatReset In Group Check: ${Group.Member[${MainAssist}].Index} !${IAmMA} ${XTSlot}
        /if (${XTarAutoSet} && !${Me.Hovering}) {
            /if (!${Group.Member[${MainAssist}].Index} && !${IAmMA}) /xtarget set ${XTSlot} autohater
        }
        /if (${DPSOn}) {
            /for i 1 to ${DPS.Size}
                /if (${Int[${DPS[${i}].Arg[2,|]}]}<101) {
                    /varset DPSTimer${i} 0
                    /varset FDTimer${i}  0
                }
            /next i
        }
        /if (${Me.Pet.ID}) {
            /varset PetAttack 0
            /pet back off
            /if (${PetHoldOn}) /pet ${PetHold} on
        }
        /if (${DebuffAllOn} && ${DebuffCount}) {
            /varset j 0
            /for i 1 to ${XSlotTotal}
                /if (${Me.XTarget[${i}].ID} && ${Me.XTarget[${i}].TargetType.Equal[Auto Hater]} && ${Me.XTarget[${i}].Type.NotEqual[Corpse]} && ${Me.XTarget[${i}].Distance}<=${MeleeDistance}) /varcalc j ${j}+1
            /next i
            /if (${j}==0) {
                /for i 1 to ${DebuffCount}
                    /varset DBOTimer${i} 0
                    /varset DBOList${i}
                /next i
            }
        }
        /varset Attacking 0
        /varset BurnActive 0
        /varset DPSTarget 0
        /if (${BurnID} && (!${Spawn[id ${BurnID}].ID} || ${Spawn[id ${BurnID}].Type.Equal[corpse]})) {
            /varset BurnCalled 0
            /varset BurnID 0
            /echo Burn Target is Dead. Turning Burn off.
        }
        |/if (!${TwistOn} && ${MeleeTwistOn}) /varset DPSTwisting 0
        /if (${LootOn} && !${SFlag}) {
            /delay 10
            /call LootStuff
            /if (${IAmABard}) {
                /call CastBardCheck
            }
        }
        /if (${IAmABard}) {
            /varset DPSTwisting 0
            /call CastBardCheck
            /call DoBardStuff
        }
        /if (!${IAmMA} && ${TargetSwitchingOn}) /varset TargetSwitchingOn 0
        /varset TankTimer 30s
        DEBUGCOMBAT CombatReset: Leave
        /varset PetFollowTimer 60s
        /delay 2s ${AggroOffTimer}==0
        /while (1) {
            /varset EventFlag 0
            /doevents Timer
            /if (!${EventFlag}) /break
        }
        /while (1) {
            /varset EventFlag 0
            /doevents
            /if (!${EventFlag}) /break
        }
        /if (${Stick.Active} && ${Stick.StickTarget}!=${If[${Spawn[=${WhoToChase}].Type.Equal[Pet]},${Spawn[=${WhoToChase}].Master.ID},${Spawn[=${WhoToChase}].ID}]}) /stick off
        /if (${UseMQ2Melee}) /squelch /melee melee=1
        /if (${Select[${Me.Class.ShortName},ROG]}) /call Roguestuff
    /return
| -------------------------------------------------------------------------------------
| SUB: Check for Adds
| -------------------------------------------------------------------------------------
    Sub CheckForAdds(CalledFrom)
        DEBUGN  CheckForAdds: Enter from ${CalledFrom}
        /call MobRadar los ${MeleeDistance} CheckForAdds
        DEBUGN MobCount:${MobCount}
        | If no mobs, in a DMZone or pulling Leave
        /if (${MobCount}<=1)  /return
        /if (${DMZ} && ${Me.InInstance}==FALSE) /return
        /if (${Pulling}) /return
        /if (!${DPSOn} && !${MeleeOn}) /return
        /if (${Select[${Role},puller,pullertank,pullerpettank]} && ${Math.Distance[${CampYLoc},${CampXLoc}]}>=${CampRadius}) /return
        /if (${IAmDead}) /return
        /if (${ChainPull}==2 || ${DPSPaused}) /return
        | If I have a valid living target in camp radius Leave
        /if (!${Target.ID} && ${MyTargetID} && ${Spawn[${MyTargetID}].ID} && ${Spawn[${MyTargetID}].Distance}<${CampRadius}) {
            /squelch /target id ${MyTargetID}
            /return
        }
        | Spam Adds in camp
        /if (${AggroTargetID} && ${Spawn[${AggroTargetID}].Distance}<=${CampRadius} && !${MyTargetID} && !${AddSpam}) {
            /popup Add(s) in camp detected
            /if (${IAmMA} || ${Select[${Role},tank,pullertank,pettank,pullerpettank]}) /call BroadCast r "Add(s) in camp detected"
            /if (${Select[${Role},pullertank,pullerpettank]}) /varset Pulled 0
            /varset AddSpam 5s
        }
        | Prevent puller from stalling if adds in camp and still returning from pulling mob.
        /if (${Select[${Role},puller,pullertank,pullerpettank]} && ${Pulled} && ${Math.Distance[${CampYLoc},${CampXLoc}]}>=15) /return
        /if (!${Target.ID} && ${Select[${Role},tank,pullertank,pettank,pullerpettank,hunter,hunterpettank]}) /squelch /target ID ${Spawn[${AggroTargetID}].ID}
        /if (${Target.Type.NotEqual[npc]} && ${MyTargetID}) {
            /if (!${Spawn[${MyTargetID}].ID} || ${Spawn[${MyTargetID}].Type.Equal[corpse]}) {
                /varset LastTargetID ${MyTargetID}
                /varset MyTargetID 0
            }
            /squelch /target clear
            /return
        }
        DEBUGN CheckForAdds: Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: AggroCheck
| -------------------------------------------------------------------------------------
    Sub AggroCheck
        /if (!${Me.HaveExpansion[Rain of Fear]} || ${Target.Type.Equal[corpse]} || ${Defined[ParseDPS]}) /return
        DEBUGCOMBAT AggroCheck Enter ${MyTargetID}
        /declare i            int    local
        /declare AggroSpell   string local
        /declare AggroGLT     string local
        /declare AggroPCT     int    local
        /declare AggroTarget  string local
        /declare AggroTID     int    local
        /if (${IAmMA} && ${TargetSwitchingOn} && ${Target.ID} && ${Target.ID}!=${MyTargetID} && ${CombatStart}) /call CombatTargetCheck 1
        /if (!${MyTargetID}) /return
        /for i 1 to ${Aggro.Size}
            /if (${Target.Type.Equal[corpse]} || !${Target.ID}) /break
            /varset AggroSpell ${Aggro[${i}].Arg[1,|]}
            /varset AggroPCT ${Aggro[${i}].Arg[2,|]}
            /varset AggroGLT ${Aggro[${i}].Arg[3,|]}
            /varset AggroTarget ${Aggro[${i}].Arg[4,|]}
            /if (${AggroTarget.Left[4].Equal[cond]}) /varset AggroTarget null
            /if (${AggroGLT.Equal[<<]}) /varcalc AggroPCT ${AggroPCT}-100
            DEBUGCOMBAT AggroCheck ${Me.PctAggro} ${AggroGLT} ${AggroPCT} ${AggroSpell} ${Target.SecondaryPctAggro} ${AggroGLT.Equal[<<]}
            /if (${ConOn} && ${Aggro[${i}].Find[|cond]} && ${If[${Cond[${Aggro[${i}].Mid[${Math.Calc[${Aggro[${i}].Find[|cond]}+5]},3]}]},0,1]}) /continue
            /if (${Me.SpellReady[${AggroSpell}]} || ${Me.AltAbilityReady[${AggroSpell}]} || ${Me.AbilityReady[${AggroSpell}]} || ${Me.CombatAbilityReady[${AggroSpell}]}) {
                | If I need to Gain more Aggro. My Aggro Check.
                /if (${AggroGLT.Equal[<]}) {
                    /if (${AggroPCT}<=${Me.PctAggro}) /continue
                | If I need to Gain more Aggro. Secondary Aggro Check.
                } else /if (${AggroGLT.Equal[<<]}) {
                    /if (!${Target.SecondaryPctAggro} || ${Target.SecondaryPctAggro}<${AggroPCT}) /continue
                | If I need to Loose Aggro
                } else /if (${AggroGLT.Equal[>]}) {
                    /if (${AggroPCT}>${Me.PctAggro}) {
                        /continue
                    }
                } else {
                    /continue
                }
                /if (${Me.CombatAbility[${AggroSpell}]} && (${Spell[${AggroSpell}].Duration} && ${Spell[${AggroSpell}].TargetType.Equal[self]} && ${Me.ActiveDisc.ID})) /continue
                DEBUGCOMBAT Trying to control Aggro: ${Me.PctAggro} ${AggroGLT} ${AggroPCT} ${AggroSpell}
                /if (${Select[${AggroTarget},null,Mob]} || (${AggroTarget.Equal[INC]} && ${Spawn[${MyTargetID}].Distance}>${MeleeDistance})) {
                    /varset AggroTID ${MyTargetID}
                } else /if (${AggroTarget.Equal[Me]}) {
                    /varset AggroTID ${Me.ID}
                } else /if (${AggroTarget.Equal[MA]}) {
                    /varset AggroTID ${Spawn[=${MainAssist}].ID}
                } else /if (${AggroTarget.Equal[Pet]}) {
                    /varset AggroTID ${Me.Pet.ID}
                }
                /if (${AggroTarget.Equal[INC]} && ${Spawn[${MyTargetID}].Distance}<${MeleeDistance}) /continue
                /if (!${AggroTID}) /continue
                /call CastWhat "${AggroSpell}" ${AggroTID} Aggro 0 0
                /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                    /echo Casting >> ${AggroSpell} << to control AGGRO(${AggroGLT}) on ${Spawn[${AggroTID}].CleanName}.
                    /if (${AggroGLT.Equal[>]}) {
                        /if (!${AggroOffTimer}) {
                           /if (${Me.Feigning} || ${Me.Invis}) /varset AggroOffTimer 20
                        }
                    }
                    /break
                }
                /if (${AggroGLT.Equal[>]} && ${AggroPCT}>${Me.PctAggro}) /break
                /if (${AggroGLT.Equal[<<]} && ${Target.SecondaryPctAggro}<${AggroPCT}) /break
                /if (${AggroGLT.Equal[<]} && ${AggroPCT}<${Me.PctAggro}) /break
            }
        /next i
        DEBUGCOMBAT AggroCheck Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Do Debuff Stuff
| -------------------------------------------------------------------------------------
    Sub DoDebuffStuff(int FirstMobID)
        /doevents
        /if (!${DebuffAllOn} || !${DebuffCount} || ${Window[RespawnWnd].Open} || (${MezOn} && !${MezMobDone})) /return
        /if ((${DMZ} && ${Me.InInstance}==FALSE) || (!${MyTargetID} && ${Spawn[=${MainAssist}].ID} && ${Spawn[=${MainAssist}].Type.NotEqual[Mercenary]})) /return
        /if (${DPSPaused}) {
            DEBUGCOMBAT DoDebuffStuff: Return from CombatCast cause DPSPaused is ${DPSPaused}
            /return
        }
        DEBUGCOMBAT DoDebuffStuff: Enter
        /declare i               int      local 0
        /declare j               int      local 0
        /declare k               int      local 0
        /declare DebuffTargetID  int      local 0
        /declare DebuffText      string   local
        /declare WasAttacking    int      local 0
        /if (${IAmABard} && ${IAmMA} && ${MyTargetID} && ${AggroTargetID}) {
            DEBUGCOMBAT im a bard and stuff so im getting out of here. Returning
            /return
        }
        /call MobRadar los ${MeleeDistance} DoDebuffStuff
        /for i 1 to ${DebuffCount}
            /if (${DBOList${i}.Length}) {
                /varset k 1
                /if (!${DBOTimer${i}}) {
                    | Reset the list with only the current DPS mob ID. So you won't try and debuff the mob again.
                    /varset DBOList${i} |${MyTargetID}
                } else {
                    |Remove mobs from list that are dead or over 200 units away.
                    /while (${DBOList${i}.Arg[${k},|].Length}) {
                        /if (!${Spawn[${DBOList${i}.Arg[${k},|]}].ID} || ${Spawn[${DBOList${i}.Arg[${k},|]}].Distance}>200 || ${Spawn[${DBOList${i}.Arg[${k},|]}].Type.Equal[Corpse]}) {
                            /varset DBOList${i} ${DBOList${i}.Replace[|${DBOList${i}.Arg[${k},|]},]}
                        } else {
                            /varcalc k ${k}+1
                        }
                    }
                }
            }
        /next i
        | For Mobs
        DEBUGCOMBAT MobCount was ${MobCount} MainAssist: ${MainAssist} FirstMobID: ${FirstMobID} MyTargetID: ${MyTargetID}
        /if (${Spawn[${FirstMobID}].Type.Equal[npc]} || (${Spawn[${FirstMobID}].Type.Equal[pet]} && ${Spawn[${FirstMobID}].Master.Type.NotEqual[pc]})) /call DebuffCast "${FirstMobID}" 1
        /for j 1 to ${XSlotTotal}
            |--- only call code for GroupEscape kick off.
            /varset DebuffTargetID ${MyTargetID}
            /if (${GroupEscapeOn} && ${Select[${Me.Class.ShortName},WIZ,DRU]} && (${Raid.Members}==0)) /call GroupEscape
            /if (${DPSPaused}) {
                DEBUGCOMBAT DoDebuffStuff: Return from CombatCast cause DPSPaused is ${DPSPaused}
                /return
            }
            /if (!${Me.XTarget[${j}].ID} || (${Me.XTarget[${j}].ID}==${FirstMobID})) /continue
            | Every Debuff test conditon is listed seperately because Maskoi forgets why he does anything.
            DEBUGCOMBAT DoDebuffStuff: we are in the debuffcondition loop j is: ${j}
            | Is Xtarget type not of type auto hater
            /if (${Me.XTarget[${j}].TargetType.NotEqual[Auto Hater]} || ${Spawn[${Me.XTarget[${j}].ID}].Type.Equal[Corpse]}) {
                DEBUGCOMBAT XTarget Type: ${Me.XTarget[${j}].TargetType} Spawn Type: ${Spawn[${Me.XTarget[${j}].ID}].Type}
                /continue
            }
            | Is mob in spell Radius distance
            /if (${Spawn[${Me.XTarget[${j}].ID}].Distance}>=${MeleeDistance}) {
                DEBUGCOMBAT DoDebuffStuff: ${j}  Mob distance is greater than MeleeDistance: ${Spawn[${Me.XTarget[${j}].ID}].Distance}
                /continue
            }
            | Test -> is the target in line of sight
            /if (!${Spawn[${Me.XTarget[${j}].ID}].LineOfSight}) {
                DEBUGCOMBAT DoDebuffStuff: I dont have LineOfSight to Spawn[${Me.XTarget[${j}].ID}] ${Spawn[${Me.XTarget[${j}].ID}].Name} so im going to Skip Mob
                /continue
            }
            /if (${Spawn[${Me.XTarget[${j}].ID}].Type.Equal[pc]} || (${Spawn[${Me.XTarget[${j}].ID}].Type.Equal[pet]} && ${Spawn[${Me.XTarget[${j}].ID}].Master.Type.Equal[pc]})) {
                DEBUGCOMBAT DoDebuffStuff: Target is a PC or Pet of a PC. Skipping.
                /continue
            }
            /varset DebuffTargetID ${Me.XTarget[${j}].ID}
            DEBUGCOMBAT DoDebuffStuff Target: ${DebuffTargetID}=${Target.ID} DebuffAllOn: ${DebuffAllOn} J: ${j}
            /if (${MeleeOn} && ${Me.Combat} && (!${IAmMA} || ${DebuffTargetID}!=${Target.ID})) {
               /squelch /attack off
                /delay 10 !${Me.Combat}
                /varset WasAttacking 1
            }
            | ****************  For Spells  *****************************
            /if (${DebuffAllOn}==2 && !${BurnCalled}) {
                /call DebuffCast "${DebuffTargetID}" 1
            } else {
                /call DebuffCast "${DebuffTargetID}" 0
            }
            DEBUGCOMBAT DoDebuffStuff 4.0
            | Reset values if 3rd value in dps for All
            | ************ Next Mob ***************
        /next j
        /varset DebuffTargetID ${MyTargetID}
        /if (${Target.ID}!=${MyTargetID}) {
            /if (${Spawn[${MyTargetID}].Type.NotEqual[Corpse]}) {
                /target id ${MyTargetID}
                /delay 1s ${Target.ID}==${MyTargetID}
                /if (${WasAttacking}) /squelch /attack on
            }
        }
        DEBUGCOMBAT DoDebuffStuff: Leave
    /return
|--------------------------------------------------------------------------------
| SUB: DebuffCast
| -------------------------------------------------------------------------------------
    Sub DebuffCast(int DebuffTargetID, int FWait)
        /declare ActionReady     int      local 0
        /declare c               int      local 0
        /declare CastCount       int      local 0
        /declare CastRange       int      local 0
        /declare DebuffText      string   local
        /declare echoTimer1      timer    local 0
        /declare echoTimer2      timer    local 0
        /declare echoTimer3      timer    local 0
        /declare f               int      local 0
        /declare g               int      local 0
        /declare i               int      local 0
		/declare CharmCheckFlag    int      local 0
        /declare MezCheckFlag    int      local 0
        /declare Tag1            string   local
        /declare Tag2            string   local
        /declare TempTimer       timer    local 0
        /declare WaitTimerDC     timer    local 0
        | ****************  For Spells  *****************************
        DEBUGCOMBAT DebuffCast Enter.
        | g  - is control variable for remaining in loop while checking for mobs to debuff and spell is ready to cast.
        | c  - is to tell if the current mob is NOT on the Debuffed List or the debufftimer has runout. c=1 mob needs debuff, c=0 Mob can be skipped
        | f  - is a flag that is changed if we find a mob to debuff, but the current Spell or AA is not ready for casting, f=1 Spells/AA's were ready, f=0 Spell/AA not ready.
        | i  - is the index of the debuff to cast.
        | !${c} This mob has all debuffs, !${f} There is a debuff that was not ready for this mob.
        /while (1) {
            /varset TempTimer 70
            /varset g 1
            /while (${g} && ${TempTimer}) {
                /varset c 0
                /varset f 1
                /if (${i}==${DebuffCount}) {
                    | We should wait for spell ready. Debuffing is more important.
                    /if (${FWait}) {
                        /varset i 0
                        | We have tried every debuff even after waiting, so we need to leave.
                        /if (${CastCount}>=${DebuffCount}) /varset f 0
                    } else {
                        | We checked every debuff, and I am not waiting any longer. DPS is more important
                        /varset f 0
                    }
                }
                /while (${f} && ${i}<${DebuffCount}) {
                    /varset ActionReady 0
                    /varcalc i ${i}+1
                    /varset DebuffText ${DPS[${i}].Arg[1,|]}
                    /if (${Int[${Spell[${DebuffText}].Range}]}>=${Int[${Spell[${DebuffText}].AERange}]}) {
                        /varset CastRange ${Spell[${DebuffText}].Range}
                    } else {
                        /varset CastRange ${Spell[${DebuffText}].AERange}
                    }
                    DEBUGCOMBAT ${DebuffText} ${DPS[${i}].Length} ${DebuffText.NotEqual[null]} ${DBOList${i}.Find[|${DebuffTargetID}]} ${DBOTimer${i}} ${i} ${DebuffCount} ${TempTimer}
                    |If mob not on list or debuff timer expired then increment counter of mobs needing debuff.
                    /if (${DPS[${i}].Length} && ${DebuffText.NotEqual[null]} && (!${DBOList${i}.Find[|${DebuffTargetID}]} || ${DBOTimer${i}}==0)) {
                        /varset c 1
                        | is spell or AA ready to cast? If ready set flag to drop out of loop.
                        /if (${Select[TRUE,${Me.SpellReady[${DebuffText}]},${Me.AltAbilityReady[${DebuffText}]},${Me.ItemReady[=${DebuffText}]}]}) {
                            /varset f 0
                        } else {
                            /if (${FWait}) {
                                /varset ActionReady ${Select[TRUE,${Bool[${Me.Book[${DebuffText}]}]},${Bool[${Me.AltAbility[${DebuffText}]}]},${Bool[${FindItem[=${DebuffText}].ID}]}]}
                                | This is a spell.
                                /if (${ActionReady}==1) {
                                    /if (${Me.SpellInCooldown}) {
                                        /if (${Int[${Me.Gem[${DebuffText}]}]}) {
                                            /if (!${Me.GemTimer[${DebuffText}]}) {
                                                /varset f 0
                                                /break
                                            }
                                        }
                                    } else /if (${Int[${Me.Gem[${DebuffText}]}]} && ${Me.GemTimer[${DebuffText}]} && !${echoTimer1}) {
                                        /echo Waiting on Spell: ${DebuffText} to Refresh. Time Remaining: ${Me.GemTimer[${DebuffText}].TotalSeconds} Seconds.
                                        /varset echoTimer1 ${Me.GemTimer[${DebuffText}]}
                                        /if (${Me.GemTimer[${DebuffText}]}<36) {
                                            /varset f 0
                                            /varset WaitTimerDC ${Me.GemTimer[${DebuffText}]}
                                            /break
                                        }
                                    }
                                | This is an AltAbility
                                } else /if (${ActionReady}==2) {
                                    /if (${Math.Calc[${Me.AltAbilityTimer[${DebuffText}].TotalSeconds}*10]}<=${TempTimer}) {
                                        /if (!${echoTimer2}) {
                                            /echo Waiting on AltAbility To Cast ${DebuffText}
                                            /varset echoTimer2 ${TempTimer}
                                        }
                                        /if (${Math.Calc[${Me.AltAbilityTimer[${DebuffText}].TotalSeconds}*10]}<36) {
                                            /varset f 0
                                            /varset WaitTimerDC ${Math.Calc[${Me.AltAbilityTimer[${DebuffText}].TotalSeconds}*10]}
                                            /break
                                        }
                                    } else {
                                        /echo Skipping AltAbility. Wait time is to long for ${DebuffText}
                                        /if (${DBOTimer${i}}==0) /varcalc DBOTimer${i} ${Me.AltAbilityTimer[${DebuffText}].TotalSeconds}*10
                                    }
                                | This is an Item
                                } else /if (${ActionReady}==3) {
                                    /if (${Math.Calc[${FindItem[=${DebuffText}].TimerReady}*10]}<=${TempTimer}) {
                                        /if (!${echoTimer3}) {
                                            /echo Waiting on Item To Cast ${DebuffText}
                                            /varset echoTimer3 ${TempTimer}
                                        }
                                        /if (${Math.Calc[${FindItem[=${DebuffText}].TimerReady}*10]}<36) {
                                            /varset f 0
                                            /varset WaitTimerDC ${Math.Calc[${FindItem[=${DebuffText}].TimerReady}*10]}
                                            /break
                                        }
                                    } else {
                                        /echo Skipping Item. Wait time is to long for ${DebuffText}
                                        /if (${DBOTimer${i}}==0) /varcalc DBOTimer${i} ${FindItem[=${DebuffText}].TimerReady}*10
                                    }
                                }
                            }
                            /if (${HealsOn}) {
                                /if (!${LastHealCheck}) {
                                    /call CheckHealth DebuffCast1
                                    /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
                                }
                            }
                        }
                    }
                }
                DEBUGCOMBAT ${c} ${f} ${FWait}
                /if (!${c} || !${f} || (${f} && !${FWait})) /varset g 0
            }
            DEBUGCOMBAT DebuffCast 0 DebuffTargetID: ${DebuffTargetID} ${i} C: ${c} F: ${f} FWait: ${FWait}
            /if (${MezOn}) /varset MezBroke 0
			/if (${CharmOn}) /varset CharmBroke 0
            /varset GoMByPass 1
            /doevents
            /varset GoMByPass 0
            /if (${HealsOn}) {
                /if (!${LastHealCheck}) {
                    /call CheckHealth DebuffCast1
                    /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
                }
            }
            /if (${DPSPaused}) {
                DEBUGCOMBAT DebuffCast: Return from DebuffCast cause DPSPaused is ${DPSPaused}
                /return
            }
            /if (!${c} || ${g} || (${f} && !${FWait})) {
                DEBUGCOMBAT DebuffCast Exit.
                /return
            }
            DEBUGCOMBAT DebuffCast 1 DebuffTargetID: ${DebuffTargetID} ${i} Find: ${DBOList${i}.Find[|${DebuffTargetID}]} Timer: ${DBOTimer${i}} List: ${DBOList${i}}
            /if (${GroupEscapeOn} && ${Select[${Me.Class.ShortName},WIZ,DRU]} && (${Raid.Members}==0)) /call GroupEscape
            /if (${MezOn} && !${MezBroke}) {
                /call DoMezStuff DebuffCast
                /varset MezCheckFlag 1
            }
            /if (${CharmOn} && !${CharmBroke}) {
                /call DoCharmStuff DebuffCast
                /varset CharmCheckFlag 1
            }
            /varset TempTimer 0
            /varset Tag1 ${DPS[${i}].Arg[4,|]}
            /varset Tag2 ${DPS[${i}].Arg[5,|]}
            /if (${Tag1.Equal[always]}) /varset Tag2 always
            | Test -> Do i have enough mana to cast the spell
            /if (${Me.CurrentMana}<${Spell[${DebuffText}].Mana}) {
                DEBUGCOMBAT DebuffCast: i didnt have enough mana to cast ${DebuffText} so im going to Skip Cast
                /varcalc CastCount ${CastCount}+1
                /continue
            }
            /if (${Spawn[${DebuffTargetID}].Type.Equal[Corpse]} || ${Int[${Spawn[${DebuffTargetID}].ID}]}==0) /return
            /if (${Target.ID}!=${DebuffTargetID}) {
                /squelch /target clear
                /delay 10 !${Target.ID}
                /squelch /target id ${DebuffTargetID}
                /delay 20 ${Target.ID}==${DebuffTargetID} && ${Target.BuffsPopulated}
            }
            DEBUGCOMBAT DebuffCast: Debuff: ${DebuffText} TargetID: ${Target.ID} DebuffTargetID: ${DebuffTargetID} : ${Target.Buff[${DebuffText}].ID} ${Target.Buff[${DebuffText}].Caster} ${Me.SpellInCooldown} ${Tag1} ${Tag2} ${Target.BuffsPopulated}
            /while (1) {
                | Check Target for Buffs
                /if (${Target.BuffCount}) {
                    | Does the Target have the debuff I am about to cast
                    /if (${Target.Buff[${DebuffText}].ID}) {
                        /if (${Target.Buff[${DebuffText}].Caster.Equal[${Me.CleanName}]} || (${Target.Buff[${DebuffText}].Caster.NotEqual[${Me.CleanName}]} && ${Tag2.NotEqual[always]})) {
                            /varcalc TempTimer ${Target.BuffDuration[${DebuffText}].TotalSeconds}*10
                        }
                    }
                    | If timer didn't get set above then check this
                    /if (${TempTimer}==0) {
                        | Does the mob have one of the following debuffs if so lets skip casting
                        /if (${Tag1.Equal[strip]} && (${Int[${Target.Beneficial.ID}]}==0 || ${Select[${Int[${Target.Beneficial.ID}]},38728,38727,10080]})) {
                            /varset TempTimer 70
                        } else /if (${Tag2.NotEqual[always]}) {
                            /if (${Tag1.Equal[slow]} && ${Target.Slowed.ID}) {
                                |/if (${Target.Slowed.SlowPct}>=${Spell[${DebuffText}].SlowPct}) {
                                |    /varcalc TempTimer ${Target.BuffDuration[${Target.Slowed.Name}].TotalSeconds}*10
                                |}
                                /varcalc TempTimer ${Target.BuffDuration[${Target.Slowed.Name}].TotalSeconds}*10
                            } else /if (${Tag1.Equal[tash]} && ${Target.Tashed.ID}) {
                                /varcalc TempTimer ${Target.BuffDuration[${Target.Tashed.Name}].TotalSeconds}*10
                            } else /if (${Tag1.Equal[malo]} && ${Target.Maloed.ID}) {
                                /varcalc TempTimer ${Target.BuffDuration[${Target.Maloed.Name}].TotalSeconds}*10
                            } else /if (${Tag1.Equal[crip]} && ${Target.Crippled.ID}) {
                                /varcalc TempTimer ${Target.BuffDuration[${Target.Crippled.Name}].TotalSeconds}*10
                            } else /if (${Tag1.Equal[snare]} && ${Target.Snared.ID}) {
                                /varcalc TempTimer ${Target.BuffDuration[${Target.Snared.Name}].TotalSeconds}*10
                            } else /if (${Tag1.Equal[root]} && ${Target.Rooted.ID}) {
                                /varcalc TempTimer ${Target.BuffDuration[${Target.Rooted.Name}].TotalSeconds}*10
                            }
                        }
                    }
                    /if (${TempTimer}) {
                        /if (!${DBOList${i}.Find[|${DebuffTargetID}]}) /varset DBOList${i} ${DBOList${i}}|${DebuffTargetID}
                        /break
                    }
                    DEBUGCOMBAT DebuffCast: DebuffTargetID: ${DebuffTargetID} Slowed: ${Target.Slowed.ID} Tashed: ${Target.Tashed.ID} Maloed: ${Target.Maloed.ID} Crippled: ${Target.Crippled.ID} Tag1: ${Tag1} Tag2: ${Tag2} TempTimer: ${TempTimer}
                }
                /if (${HealsOn}) {
                    /if (!${LastHealCheck}) {
                        /call CheckHealth DebuffCast3
                        /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
                        /if (${Target.ID}!=${DebuffTargetID}) {
                            /if (${Spawn[${DebuffTargetID}].Type.Equal[Corpse]} || ${Int[${Spawn[${DebuffTargetID}].ID}]}==0) /return
                            /squelch /target clear
                            /delay 10 !${Target.ID}
                            /squelch /target id ${DebuffTargetID}
                            /delay 10 ${Target.BuffsPopulated}
                        }
                    }
                }
                | When you get here you won't want to cast Eradicate Magic if the mob has NO Beneficial Buffs
                /if (${Tag1.Equal[strip]} && ${Int[${Target.Beneficial.ID}]}==0) {
                    /if (!${DBOList${i}.Find[|${DebuffTargetID}]}) /varset DBOList${i} ${DBOList${i}}|${DebuffTargetID}
                    /varset TempTimer 70
                    /break
                }
                | Wait here for a sec, your spell bar may still be in global cool down.
                /if (${Me.SpellInCooldown} || ${WaitTimerDC}) {
                    /delay 5
                }
                | Conditional Check Added.
                /if (${ConOn} && ${DPS[${i}].Find[|cond]} && ${If[${Cond[${DPS[${i}].Mid[${Math.Calc[${DPS[${i}].Find[|cond]}+5]},3]}]},0,1]}) /break
                | Everything is ready, lets debuff this mob
                /call CastWhat "${DebuffText}" ${DebuffTargetID} DebuffCast 0 0
                DEBUGCOMBAT DebuffCast: Return From CastWhat: ${Macro.Return}
                /if (${Macro.Return.Equal[CAST_RESISTED]}) {
                    /call BroadCast r "** ${DebuffText} on >> ${Spawn[${DebuffTargetID}].CleanName} << - RESISTED"
                } else /if (${Macro.Return.Equal[CAST_TAKEHOLD]}) {
                    /call BroadCast r "** ${DebuffText} on >> ${Spawn[${DebuffTargetID}].CleanName} << - DID NOT TAKE HOLD"
                    /varset TempTimer 3m
                } else /if (${Macro.Return.Equal[CAST_IMMUNE]}) {
                    /call BroadCast g "** ${Spawn[${DebuffTargetID}].CleanName} is IMMUNE to - ${DebuffText}"
                    /if (!${DBOList${i}.Find[|${DebuffTargetID}]}) /varset DBOList${i} ${DBOList${i}}|${DebuffTargetID}
                    /varset TempTimer 3m
                } else /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                    /call BroadCast o "** Debuffing: ==> ${DebuffText} on >> ${Spawn[${DebuffTargetID}].CleanName}"
                    /if (!${DBOList${i}.Find[|${DebuffTargetID}]}) /varset DBOList${i} ${DBOList${i}}|${DebuffTargetID}
                    | Timers for items
                    /if (${FindItemCount[=${DebuffText}]}) {
                        /varcalc TempTimer ${FindItem[=${DebuffText}].Spell.Duration.TotalSeconds}*10
                        | Timers for AltAbilities
                    } else /if (${Me.AltAbility[${DebuffText}]}) {
                        /if (${Me.AltAbility[${DebuffText}].Spell.Duration.TotalSeconds}) {
                            /varcalc TempTimer ${Me.AltAbility[${DebuffText}].Spell.Duration.TotalSeconds}*10
                        } else /if (${Me.AltAbility[${DebuffText}].Spell.Trigger.Duration.TotalSeconds}) {
                            /varcalc TempTimer ${Me.AltAbility[${DebuffText}].Spell.Trigger.Duration.TotalSeconds}*10
                        }
                        | Timers for spells
                    } else /if (${Me.Book[${DebuffText}]}) {
                        | - Custom timer for counterbias
                        /if (${Me.Class.Name.Equal[Shaman]} && ${DebuffText.Find[counterbias]}) {
                            /varset TempTimer 1.5m
                            | Custom timer for Chanter suffocation
                        } else /if (${Me.Class.Name.Equal[enchanter]} && ${DebuffText.Find[suffocation]}) {
                            /varset TempTimer 1m
                            | Custom timer for Beastlord Feralgia Timer
                        } else /if (${Me.Class.Name.Equal[Beastlord]} && ${DebuffText.Find[feralgia]}) {
                            /varset TempTimer 1.5m
                            | Regular spells
                        } else /if (${Spell[${DebuffText}].Duration}>0) {
                            /varcalc TempTimer ${Spell[${DebuffText}].Duration.TotalSeconds}*10
                            | Spells with no timers assign DPS interval
                        } else {
                            /varset TempTimer ${DPSInterval}s
                        }
                        | AA and disc timers
                    } else /if (${Spell[${DebuffText}].Duration}>0) {
                        /varcalc TempTimer ${Spell[${DebuffText}].Duration.TotalSeconds}*10
                        | AA and disc with no timers assign DPS interval
                    } else {
                        /varset TempTimer ${DPSInterval}s
                    }
                }
                /if (${MezOn} && !${MezCheckFlag} && ${Me.XTarget[${XTSlot2}].ID}) {
                    /call DoMezStuff DebuffCast
                    /varset MezCheckFlag 1
                }
                /break
            }
            | Check Debuff Timer not set and the Temp Timer has been set.
            | You only want to set the DBOTimer for the first mob you get the Debuff to land on. Don't need a timer for every mob.
            /if (!${DBOTimer${i}} && ${TempTimer}) {
                /varcalc DBOTimer${i} ${TempTimer}*.95
            }
            | ************ Next Spell ***********
            /varcalc CastCount ${CastCount}+1
            DEBUGCOMBAT DebuffCast 2 DebuffTargetID: ${DebuffTargetID} ${i} Find: ${DBOList${i}.Find[|${DebuffTargetID}]} Timer: ${DBOTimer${i}} List: ${DBOList${i}} ${CastCount}
        }
        DEBUGCOMBAT DebuffCast leave.
    /return
| -------------------------------------------------------------------------------------
| SUB: PullRangeSet
| -------------------------------------------------------------------------------------
    Sub PullRangeSet
        DEBUGPULL PullRange Enter
        /declare PullItemRange int local
        /if (${FindItemCount[=${PullWith.Arg[1,|]}]} && ${Redguides}) {
            /if (${Select[${FindItem[=${PullWith.Arg[1,|]}].Type},Archery,Throwing,Throwingv1,Throwingv2,ammo]}) {
                /varset PullItem ${PullWith.Arg[1,|]}
                /varset PullItemRange ${FindItem[=${PullWith.Arg[1,|]}].Range}
                /if (!${PullItemRange}) /varset PullItemRange 50
                /if (${FindItem[=${PullWith.Arg[2,|]}].WornSlot[ammo]} && ${FindItemCount[=${PullWith.Arg[2,|]}]}) {
                    /varset PullAmmo ${PullWith.Arg[2,|]}
                    | If pullitem is a bow add arrow range to bow for total range
                    /if (${FindItem[=${PullWith.Arg[1,|]}].Type.Equal[Archery]}) {
                        /varcalc PullItemRange ${PullItemRange}+${FindItem[=${PullWith.Arg[2,|]}].Range}
                    }
                    /call PullVars ${Math.Calc[${PullItemRange}*(${PullRadiusToUse}/100)]} Ranged ${PullItemRange}
                } else {
                    /echo I can't find any ammo defaulting to Melee for PullWith
                    /varset PullWith Melee
                    /call PullVars 15 Melee 15
                }
                DEBUGPULL PullWith=(${PullWith}) PullItem=(${PullItem}) PullItemRange=(${PullItemRange}) PullItemCount=(${FindItemCount[=${PullWith.Arg[1,|]}]}) PullAmmo=(${PullAmmo})
            } else /if (${FindItemCount[=${PullWith}]} && !${PullWith.Arg[2,|].Length}) {
                /varset PullItemRange ${FindItem[=${PullWith}].Spell.Range}
                /call PullVars ${Math.Calc[${PullItemRange}*(${PullRadiusToUse}/100)]} "${PullWith}" ${PullItemRange}
            }
        } else /if (${Me.CombatAbility[${PullWith}]} || ${Me.Book[${PullWith}]} || ${Me.AltAbility[${PullWith}]}) {
            /if (${Me.AltAbility[${PullWith}]}) {
                DEBUGPULL ${Math.Calc[${Me.AltAbility[${PullWith}].Spell.Range}*(${PullRadiusToUse}/100)]} "${PullWith}" ${Me.AltAbility[${PullWith}].Spell.Range}
                /call PullVars ${Math.Calc[${Me.AltAbility[${PullWith}].Spell.Range}*(${PullRadiusToUse}/100)]} "${PullWith}" ${Me.AltAbility[${PullWith}].Spell.Range}
            } else {
                DEBUGPULL ${Math.Calc[${Spell[${Spell[${PullWith}].ID}].MyRange}*(${PullRadiusToUse}/100)]} "${PullWith}" ${Spell[${PullWith}].MyRange}
                /call PullVars ${Math.Calc[${Spell[${PullWith}].MyRange}*(${PullRadiusToUse}/100)]} "${PullWith}" ${Spell[${PullWith}].MyRange}
            }
        } else /if (${PullWith.Equal[Pet]}) {
            /if (${Role.Equal[hunterpettank]}) {
                /call PullVars ${Math.Calc[${PetAttackRange}*.80]} Pet ${Math.Calc[${PetAttackRange}*.80]}
            } else {
                /call PullVars 185 Pet 185
            }
        } else /if (${PullWith.Equal[Melee]}) {
            /if (${PullPathWpCount} && ${PullMoveUse.Equal[advpath]}) {
                /beep
                /beep
                /beep
                /popup You can't pull with MQ2Advpath and Melee. Please change PullWith to a Spell/AA/Disc/Ranged Weapon.
                DEBUGPULL ${PullPathWpCount} && ${PullMoveUse.Equal[advpath]}
                /echo You can't pull with MQ2Advpath and Melee. Please change PullWith to a Spell/AA/Disc/Ranged Weapon.
                /endmac
            } else {
                /call PullVars 15 Melee 15
            }
        } else /if (!${Select[${PullWith},Ranged]}) {
            /echo Unknown PullWith setting. Please see Kiss Instructions.
        }
        DEBUGPULL PullRange PullWith: ${PullWith} - CA:${Me.CombatAbility[${PullWith}]}/SPELL:${Me.Book[${PullWith}]}/ AA: ${Me.AltAbility[${PullWith}]}
        DEBUGPULL PullRange Leave
    /return
| ----------------------------------------------------------------------------
| SUB: BuffGroup
| ----------------------------------------------------------------------------
    Sub Bind_BuffGroup(int Flag1)
        DEBUGBUFF Bind_BuffGroup: Enter ${Flag1}
        /if (!${Flag1}) {
            /if (${EQBCOn} && ${Macro.IsTLO[EQBC]}) {
                /bcg //buffgroup 1
            } else /if (${DanNetOn} && ${Macro.IsTLO[DanNet]}) {
                /dgge /buffgroup 1
            } else {
                /echo BuffGroup Command requires MQ2EQBC/MQ2DanNet.
            }
        }
        /if (${BuffsOn}) {
            /varset ReadBuffsTimer 0
            /varset IniNextTimer 0
            /call CheckBuffs 1
        }
        DEBUGBUFF Bind_BuffGroup: Leave
    /return
| ----------------------------------------------------------------------------
| SUB: Burn Section for new bind
| ----------------------------------------------------------------------------
    Sub Bind_Burn(string s_BurnWhat, string s_State)
        /declare BurnWhat int local ${Int[${s_BurnWhat}]}
        /if (${s_BurnWhat.Equal[on]} || ${s_State.Equal[on]}) {
            /varset BurnOn 1
            /varset CampZone ${Zone.ID}
            /echo Turning Burn On. ${s_BurnWhat} - ${s_State}
        } else /if (${s_BurnWhat.Equal[off]} || ${s_State.Equal[off]}) {
            /varset BurnOn 0
            /varset BurnActive 0
            /varset BurnCalled 0
            /varset BurnID 0
            /echo Turning Burn Off.
        }
        /if (!${BurnOn}) /return
        /if (${s_BurnWhat.Equal[on]} && ${s_State.NotEqual[doburn]}) /return
        /varset BurnCalled 1
        /if (${BurnWhat}) {
            /varset BurnID ${BurnWhat}
        } else /if (${MyTargetID}) {
            /varset BurnID ${MyTargetID}
        } else /if (${Target.ID} && !${Select[${Target.Type},pc,pet,mercenary,corpse]}) {
            /varset BurnID ${Target.ID}
        }
        /call Burn ${BurnID}
    /return
| ----------------------------------------------------------------------------
| SUB: Burn Section
| ----------------------------------------------------------------------------
    Sub Burn(int EBurnWhat)
        /if (${IAmDead} || ${Me.Hovering} || ${CampZone}!=${Zone.ID} || !${BurnOn}) {
            /if (!${BurnOn}) /echo Leaving Burn. Burn is turned Off.
            /return
        }
        /declare i            int local
        /declare BurnSpell    string local
        /declare BurnTarget   string local
        /declare BurnTargetID int local
        /declare BurnArg3     string local
        /declare CondNo       int    local 0
        /declare AbortFlag    int    local 0
        /if (!${BurnActive}) /call BroadCast r "BURN ACTIVATED => Autobots Transform <="
        /if (${UseTribute} && !${Me.TributeActive}) {
           /squelch /tribute personal on
           /squelch /trophy personal on
           /varset TributeTimer 570s
        }
        /for i 1 to ${Burn.Size}
            /varset BurnSpell ${Burn[${i}].Arg[1,|]}
            /varset BurnTarget ${Burn[${i}].Arg[2,|]}
            /varset BurnArg3 ${Burn[${i}].Arg[3,|]}
            /varset AbortFlag 0
            /if (${ConOn} && ${Burn[${i}].Find[|cond]}) {
                /varset CondNo ${Burn[${i}].Mid[${Math.Calc[${Burn[${i}].Find[|cond]}+5]},3]}
            } else {
                /varset CondNo 0
            }
            /if (${BurnSpell.Equal[null]} || ${Me.Hovering}) /continue
            /if (${BurnTarget.Equal[Mob]}) {
                /varset BurnTargetID ${MyTargetID}
            } else /if (${BurnTarget.Equal[Me]}) {
                /varset BurnTargetID ${Me.ID}
            } else /if (${BurnTarget.Equal[MA]}) {
                /varset BurnTargetID ${Spawn[=${MainAssist}].ID}
            } else /if (${BurnTarget.Equal[Pet]}) {
                /varset BurnTargetID ${Me.Pet.ID}
            } else /if (${BurnTarget.Equal[abort]}) {
                /varset BurnTargetID ${MyTargetID}
                /varset BurnArg3 abort
            } else /if (${MyTargetID}) {
                /varset BurnTargetID ${MyTargetID}
            }
            /if (${BurnArg3.Equal[abort]}) {
                /varset AbortFlag 1
            }
            /call CastWhat "${BurnSpell}" ${BurnTargetID} Burn ${CondNo} ${AbortFlag}
            /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                /echo Casting >> BURN${i}:${BurnSpell}
                /if (!${IAmABard}) {
                    /while (${Me.Casting.ID} && ${Window[CastingWindow].Open}) {
                        /delay 5
                    }
                }
            }
        /next i
        /varset BurnActive 1
    /return
| ----------------------------------------------------------------------------
| SUB: Create Timers DPS
| ----------------------------------------------------------------------------
    Sub CreateTimersDPS
        | Declare timers for DPS duration spells and Attack buffs
        /declare i int local
        /varset DebuffCount 0
        /for i 1 to ${DPS.Size}
            /if (${Int[${DPS[${i}].Arg[2,|]}]}<101) {
                /if (!${Defined[DPSTimer${i}]}) {
                    /declare DPSTimer${i}       timer         outer 5
                    /declare ABTimer${i}        timer         outer 0
                    /declare FDTimer${i}        timer         outer 0
                } else {
                    /varset DPSTimer${i} 0
                    /varset ABTimer${i} 0
                    /varset FDTimer${i} 0
                }
            } else {
                /if (!${Defined[DBOTimer${i}]}) {
                    /declare DBOTimer${i}       timer         outer 0
                    /declare DBOList${i}        string        outer
                    /varcalc DebuffCount ${DebuffCount}+1
                } else {
                    /varset DBOTimer${i} 0
                    /varset DBOList${i}
                }
            }
        /next i
    /return
