| -------------------------------------------------------------------------------------
| SUB: Event MezCheck
| -------------------------------------------------------------------------------------
    Sub MezCheck(sentFrom)
        DEBUGMEZ MezCheck Enter - ${sentFrom} ${MyTargetID}
        /varset EventByPass 1
        /varset MezBroke 0
        /while (1) {
            /varset EventFlag 0
            /doevents MezBroke
            /if (!${EventFlag}) /break
        }
        /varset EventByPass 0
        /varset GoMByPass 1
        /call CombatTargetCheck 1
        /call DoMezStuff MezCheck
        /varset GoMByPass 0
        DEBUGMEZ MezCheck Leave - ${MyTargetID}
    /return
| -------------------------------------------------------------------------------------
| SUB: Event CharmCheck
| -------------------------------------------------------------------------------------
    Sub CharmCheck(sentFrom)
        DEBUGMEZ CharmCheck Enter - ${sentFrom} ${MyTargetID}
		/varset EventByPass 1
        /varset CharmBroke 0
		/call PetStateCheck
        /while (1) {
            /varset EventFlag 0
            /doevents CharmBroke
            /if (!${EventFlag}) /break
        }
		/varset EventByPass 0
        /varset GoMByPass 1
        /call CombatTargetCheck 1
		/call DoCharmStuff CharmCheck
        /varset GoMByPass 0
        DEBUGMEZ CharmCheck Leave - ${MyTargetID}
    /return
| -------------------------------------------------------------------------------------
| SUB: Event CharmBroke
| -------------------------------------------------------------------------------------
    Sub Event_CharmBroke(charmtext,charmspell,charmmob)
        /declare i int local
        /varset EventFlag 1
        DEBUGMEZ event charmbroke Enter - ${charmspell} ${charmmob} ${MyTargetID}
                /call BroadCast g ">> CharmBreak - ${Spawn[=${charmmob}].CleanName} << has awakened from Charm -> ${charmspell}<- ${MyTargetName}"
        /if (!${CharmOn}) /return
        | Get out of here. DoCharmStuff triggered this call after being called from here. See Below.
        /if (${EventByPass}==2) /return
            /if (${Target.ID}!=${MyTargetID}) {
                /varset MyTargetID ${Target.ID}
                /varset MyTargetName ${Target.CleanName}
            } else {
                /call BroadCast g ">> CharmBreak - ${Spawn[=${charmmob}].CleanName} << has awakened -> ${charmspell}<- ${MyTargetName}"
                /for i 1 to 30
                    DEBUGMEZ ${CharmArray[${i},3]} - ${CharmArray[${i},1]} - ${CharmArray[${i},2]}
                    /if (${CharmArray[${i},3].Equal[${charmmob}]} && ${CharmArray[${i},1].NotEqual[null]} && ${CharmArray[${i},2]}!=${MyTargetID}) {
                        /echo Resetting Charm Timer ${charmmob} ID: ${CharmArray[${i},1]}
                        /varset CharmTimer${i} 0
                    }
                /next i
            }
            /if (${XTarAutoSet} && !${Group.Member[${MainAssist}].Index} && !${IAmMA} && ${Me.XTarget[${XTSlot}].ID}!=${Target.ID} && ${Target.Type.NotEqual[PC]}) /xtarget set ${XTSlot} currenttarget
            /return
        
       
        /for i 1 to 30
            DEBUGMEZ ${CharmArray[${i},3]} - ${CharmArray[${i},1]}
            /if (${CharmArray[${i},3].Equal[${charmmob}]} && ${CharmArray[${i},1].NotEqual[null]}) {
                /echo Resetting Charm Timer ${charmmob} ID: ${CharmArray[${i},1]}
                /varset CharmTimer${i} 0
            }
        /next i
        /doevents flush CharmBroke
        /varset EventFlag 0
        | Get Out of here and return back to DoCharmStuff. This event was triggered by DoCharmStuff.
        /if (${EventByPass}==1) /return
        /varset GoMByPass 1
        /varset EventByPass 2
        /call CombatTargetCheck 1
        /call DoCharmStuff Event_CharmBroke
        /varset EventByPass 0
        /varset GoMByPass 0
        | Set CharmOn = 2 to let us know that the event was triggered. Will be set back to 1 in other location
        /varset CharmBroke 1
        DEBUGMEZ event charmbroke Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Event MezBroke
| -------------------------------------------------------------------------------------
    Sub Event_MezBroke(meztext,mezmob,mezbreaker)
        /declare i int local
        /varset EventFlag 1
        DEBUGMEZ event mezbroke Enter - ${mezmob} ${mezbreaker} ${MyTargetID}
        /if (!${MezOn}) /return
        | Get out of here. DoMezStuff triggered this call after being called from here. See Below.
        /if (${EventByPass}==2) /return
        /if (${Spawn[${mezbreaker}].CleanName.Equal[${MainAssist}]}) {
            /if (${MainAssist.Equal[${Me}]}) /return
            /assist ${MainAssist}
            /delay 1s ${Me.AssistComplete}==TRUE
            /if (${Target.ID}!=${MyTargetID}) {
                /varset MyTargetID ${Target.ID}
                /varset MyTargetName ${Target.CleanName}
            } else {
                /call BroadCast g ">> MainAssist - ${Spawn[=${mezbreaker}].CleanName} << has awakened -> ${mezmob}<- ${MyTargetName}"
                /for i 1 to 30
                    DEBUGMEZ ${MezArray[${i},3]} - ${MezArray[${i},1]} - ${MezArray[${i},2]}
                    /if (${MezArray[${i},3].Equal[${mezmob}]} && ${MezArray[${i},1].NotEqual[null]} && ${MezArray[${i},1]}!=${MyTargetID}) {
                        /echo Resetting Mez Timer-1${mezmob} ID: ${MezArray[${i},1]} - ${MyTargetID}
                        /varset MezTimer${i} 0
                        /varset MezBroke 1
                    }
                /next i
            }
            /if (${XTarAutoSet} && !${Group.Member[${MainAssist}].Index} && !${IAmMA} && ${Me.XTarget[${XTSlot}].ID}!=${Target.ID} && ${Target.Type.NotEqual[PC]}) /xtarget set ${XTSlot} currenttarget
            /return
        }
        /if (${Spawn[${MainAssist}].Type.Equal[pet]} && ${Spawn[${MainAssist}].Master.CleanName.Equal[${mezbreaker}]}) /return
        /call BroadCast g ">> ${Spawn[=${mezbreaker}].CleanName} << has awakened -> ${mezmob}<-"
        /for i 1 to 30
            DEBUGMEZ ${MezArray[${i},3]} - ${MezArray[${i},1]}
            /if (${MezArray[${i},3].Equal[${mezmob}]} && ${MezArray[${i},1].NotEqual[null]} && ${MezArray[${i},1]}!=${MyTargetID}) {
                /echo Resetting Mez Timer-2 ${mezmob} ID: ${MezArray[${i},1]} - ${MyTargetID}
                /varset MezTimer${i} 0
                /varset MezBroke 1
            }
        /next i
        | Get Out of here and return back to DoMezStuff. This event was triggered by DoMezStuff.
        /if (${EventByPass}==1) /return
        /varset GoMByPass 1
        /varset EventByPass 2
        /call CombatTargetCheck 1
        /call DoMezStuff Event_MezBroke
        /varset EventByPass 0
        /varset GoMByPass 0
        | Set MezOn = 2 to let us know that the event was triggered. Will be set back to 1 in other location
        DEBUGMEZ event mezbroke Leave
    /return

| -------------------------------------------------------------------------------------
| SUB: Event MezImmune
| -------------------------------------------------------------------------------------
    Sub Event_MezImmune(int MezID)
        /varset EventFlag 1
        /if (${Select[${Me.Class.ShortName},Brd,Enc,Nec]}==0) /return
        | Assign temp var MezImmune list
        /declare ImmuneAdd string local ${MezImmune}
        | If mezimmune default text with the word null in it assign var spawn clean name
        /if (${ImmuneAdd.Find[null]}) {
            /varset ImmuneAdd ${Spawn[${MezID}].CleanName}
        } else {
            /varset ImmuneAdd ${ImmuneAdd},${Spawn[${MezID}].CleanName}
        }
        /if (!${MezImmune.Find[${Spawn[${MezID}].CleanName}]}) /ini "${InfoFileName}" "${Zone}${If[${Me.InInstance},_I,]}" "MezImmune" "${ImmuneAdd}"
        /call BroadCast g "MEZ Immune -> ${Spawn[${MezID}].CleanName} <- ID:${MezID} Adding to MezImmune list."
        | Reassign mezimmune var the new list
        /varset MezImmune ${ImmuneAdd}
    /return
| -------------------------------------------------------------------------------------
| SUB: Event CharmImmune
| -------------------------------------------------------------------------------------
    Sub Event_CharmImmune(int CharmID)
        /varset EventFlag 1
        /if (${Select[${Me.Class.ShortName},Brd,Enc,Nec]}==0) /return
        | Assign temp var CharmImmune list
        /declare ImmuneAdd string local ${CharmImmune}
        | If mezimmune default text with the word null in it assign var spawn clean name
        /if (${ImmuneAdd.Find[null]}) {
            /varset ImmuneAdd ${Spawn[${CharmID}].CleanName}
        } else {
            /varset ImmuneAdd ${ImmuneAdd},${Spawn[${CharmID}].CleanName}
        }
        /if (!${CharmImmune.Find[${Spawn[${CharmID}].CleanName}]}) /ini "${InfoFileName}" "${Zone}${If[${Me.InInstance},_I,]}" "CharmImmune" "${ImmuneAdd}"
        /call BroadCast g "Charm Immune -> ${Spawn[${CharmID}].CleanName} <- ID:${CharmID} Adding to CharmImmune list."
        | Reassign mezimmune var the new list
        /varset CharmImmune ${ImmuneAdd}
    /return
| -------------------------------------------------------------------------------------
| SUB: AddMezImmune
| -------------------------------------------------------------------------------------
    Sub AddMezImmune(int MezID)
        /if (${Select[${Me.Class.ShortName},Brd,Enc,Nec]}==0) /return
        /if (${MezImmuneIDs.Find[|${MezID}]}) /return
        /varset MezImmuneIDs ${MezImmuneIDs}|${MezID}
        /call BroadCast g "MEZ Immune -> ${Spawn[${MezID}].CleanName} <- ID:${MezID} Skipping."
    /return
| -------------------------------------------------------------------------------------
| SUB: Bind AddMezImmune
| -------------------------------------------------------------------------------------
    Sub Bind_AddMezImmune(MTImmune)
        /if (!${MTImmune.Length} || ${MTImmune.Find[null]} || ${Spawn[${MTImmune}].ID}==${Me.ID} ) {
            /echo No NPCs detected. Nothing added to list.
            /return
        }
        | Assign temp var MezImmune list
        /declare MezImmuneAdd string local ${MezImmune}
        /declare NameToUse string local
        /declare s_SpawnName string local ${Spawn[${MTImmune}].CleanName}
        /if (${s_SpawnName.Equal[null]}) {
            /echo ERROR: No Mob with ${MTImmune} in Name detected.
            /return
        }
        | If MobsToPull default text with the word null in it assign var spawn clean name
        /if (${Spawn[${MTImmune}].Name.Find[#]}) {
            /varset NameToUse #${s_SpawnName}
        } else /if (${s_SpawnName.Right[-10].Find[corpse]}) {
            /varset NameToUse ${s_SpawnName.Right[-8]}
        } else {
            /varset NameToUse ${s_SpawnName}
        }
        | If mob is on the list no need to add it again.
        /if (${MezImmune.Find[${NameToUse}]}) {
            /echo >> ${NameToUse} << already on Mez Immune List.
            /return
        }
        | If MezImmune default text with the word null in it assign var spawn clean name
        /if (${MezImmuneAdd.Find[null]} && ${MezImmune.Find[null]}) {
            /varset MezImmuneAdd ${NameToUse}
        } else {
            /varset MezImmuneAdd ${MezImmune},${NameToUse}
        }
        /ini "${InfoFileName}" "${ZoneName}" "MezImmune" "${MezImmuneAdd}"
        /echo MezImmune -> ${NameToUse} <- Adding to Mez Immune list.
        | Reassign MezImmune var the new list
        /varset MezImmune ${MezImmuneAdd}
    /return
| -------------------------------------------------------------------------------------
| SUB: AddCharmImmune
| -------------------------------------------------------------------------------------
    Sub AddCharmImmune(int CharmID)
        /if (${Select[${Me.Class.ShortName},Brd,Enc,Nec]}==0) /return
        /if (${CharmImmuneIDs.Find[|${CharmID}]}) /return
        /varset CharmImmuneIDs ${CharmImmuneIDs}|${CharmID}
        /call BroadCast g "CHARM Immune -> ${Spawn[${CharmID}].CleanName} <- ID:${CharmID} Skipping."
    /return
| -------------------------------------------------------------------------------------
| SUB: Bind AddCharmImmune
| -------------------------------------------------------------------------------------
    Sub Bind_AddCharmImmune(CTImmune)
        /if (!${CTImmune.Length} || ${CTImmune.Find[null]} || ${Spawn[${CTImmune}].ID}==${Me.ID} ) {
            /echo No NPCs detected. Nothing added to list.
            /return
        }
        | Assign temp var CharmImmune list
        /declare CharmImmuneAdd string local ${CharmImmune}
        /declare NameToUse string local
        /declare s_SpawnName string local ${Spawn[${CTImmune}].CleanName}
        /if (${s_SpawnName.Equal[null]}) {
            /echo ERROR: No Mob with ${CTImmune} in Name detected.
            /return
        }
        | If MobsToPull default text with the word null in it assign var spawn clean name
        /if (${Spawn[${CTImmune}].Name.Find[#]}) {
            /varset NameToUse #${s_SpawnName}
        } else /if (${s_SpawnName.Right[-10].Find[corpse]}) {
            /varset NameToUse ${s_SpawnName.Right[-8]}
        } else {
            /varset NameToUse ${s_SpawnName}
        }
        | If mob is on the list no need to add it again.
        /if (${CharmImmune.Find[${NameToUse}]}) {
            /echo >> ${NameToUse} << already on Charm Immune List.
            /return
        }
        | If CharmImmune default text with the word null in it assign var spawn clean name
        /if (${CharmImmuneAdd.Find[null]} && ${CharmImmune.Find[null]}) {
            /varset CharmImmuneAdd ${NameToUse}
        } else {
            /varset CharmImmuneAdd ${CharmImmune},${NameToUse}
        }
        /ini "${InfoFileName}" "${ZoneName}" "CharmImmune" "${CharmImmuneAdd}"
        /echo CharmImmune -> ${NameToUse} <- Adding to Charm Immune list.
        | Reassign CharmImmune var the new list
        /varset CharmImmune ${CharmImmuneAdd}
    /return

| -------------------------------------------------------------------------------------
| SUB: Event CantSee
| -------------------------------------------------------------------------------------
    Sub Event_CantSee
        |/varset EventFlag 1
        DEBUGN Event_CantSee
        | Reset position via stick
        /if (${Pulling}) {
           /varset CantSee 1
           /return
        }
        | need to add local variables or more Outer to figure stick command parts.
        |Start Here
        /if (${UseMQ2Melee}) {
            DEBUGN CantSee ${Attacking} ${Target.ID} ${Target.Distance} ${UseMQ2Melee} ${Stick.Active} ${Me.Moving} ${Melee} ${Melee.Status}
        } else {
            DEBUGN CantSee ${Attacking} ${Target.ID} ${Target.Distance} ${UseMQ2Melee} ${Stick.Active} ${Me.Moving}
        }
        /if (${Attacking}) {
            DEBUGN Can't See my target. Moving Closer.
            /if (${Me.Sitting}) /stand
            /if (${FaceMobOn} && ${Target.ID}) /squelch /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
            /look 0
            /if (!${UseMQ2Melee}) {
                /if (!${Select[${StickHow},0,I]}) {
                    /if (${Me.FeetWet}) {
                        /if (${Stick.Active}) /Stick off
                        /if (${FaceMobOn} && ${Target.ID}) /face
                        /moveto ${Target.MaxRangeTo} id ${Target.ID} uw
                        /delay 10
                        /if (${IAmMA}) {
                            /stick ${StickDistUW} id ${MyTargetID} moveback uw
                        } else {
                            /stick ${StickDistUW} id ${MyTargetID} behindonce moveback uw
                        }
                    } else {
                        /moveto ${Target.MaxRangeTo} id ${Target.ID}
                        /delay 10
                        /if (${IAmMA}) {
                            /stick ${StickDist} id ${MyTargetID} moveback
                        } else {
                            /stick ${StickDist} id ${MyTargetID} behindonce moveback
                        }
                    }
                } else /if (${StickHow.NotEqual[I]}) {
                    /if (${MeleeOn}) {
                        /if (${Me.FeetWet}) {
                            /stick ${StickDistUW} id ${MyTargetID} moveback uw
                        } else {
                            /if (${IAmMA}) {
                                /stick ${StickDist} id ${MyTargetID} moveback
                            } else {
                                /stick ${StickDist} id ${MyTargetID} !front moveback
                            }
                        }
                    }
                }
            }
        } else {
            /if (${FaceMobOn} && ${Target.ID}) /squelch /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
        }
        DEBUGN Leave Event_CantSee
        /doevents flush CantSee
    /return
| -------------------------------------------------------------------------------------
| SUB: Event CantHit
| -------------------------------------------------------------------------------------
    Sub Event_CantHit
        |/varset EventFlag 1
        DEBUGN Event_CantHit
        | Reset position via stick
        /if (${Pulling}) {
            /varset CantHit 1
            /if (${PullWith.Equal[Ranged]} && ${Target.Distance}<=30 && ${Target.LineOfSight}) /varset ToClose 1
        }
        DEBUGN Leave Event_CantHit
    /return
| -------------------------------------------------------------------------------------
| SUB: Event Missing
| -------------------------------------------------------------------------------------
    Sub Event_Missing
        /varset EventFlag 1
        /varset MissingComponent 1
    /return 1
| -------------------------------------------------------------------------------------
| SUB: Event Mlog Off
| -------------------------------------------------------------------------------------
    Sub Event_MLogOff
        /varset EventFlag 1
        /if (${DebugLogging}) {
            /mlog off
            /varset DebugLogging 0
        }
    /return
| ----------------------------------------------------------------------------
| SUB: Event AskForBuffs
| ----------------------------------------------------------------------------
    Sub Event_AskForBuffs(String Line, AFBCaller)
        /varset EventFlag 1
        DEBUGBUFF AskForBuffs: Enter
        /declare WorkID int local ${Int[${Spawn[${AFBCaller}].ID}]}
        /if (!${AFBCaller.Length} || ${AFBCaller.Equal[null]} || !${WorkID} || ${WorkID}==${Me.ID}) /return
        /if (${Group.Member[${AFBCaller}].Index} || ${Spawn[id ${WorkID}].Type.NotEqual[PC]}) /return
        /if (!${Raid.Member[${AFBCaller}].Group}) {
            /if (!${Me.Fellowship.Member[${AFBCaller}].Level}) {
                /if (!${Me.Guild.Equal[${Spawn[${AFBCaller}].Guild}]}) {
                    /if (!${Friends.Friend[${AFBCaller}]}) /return
                }
            }
        }
        /call Bind_TooBuffList add ${WorkID}
        DEBUGBUFF AskForBuffs: Exit
    /return
| ----------------------------------------------------------------------------
| SUB: Event AttackCalled
| ----------------------------------------------------------------------------
    Sub Event_AttackCalled(String Line, ACCaller, ACMobID)
        /varset EventFlag 1
        /echo ==> Attack Called: ${ACCaller} ${ACMobID}
        /if (!${ACMobID.Length} || ${ACMobID.Equal[null]}) {
            /varset CalledTargetID 0
            /return
        }
        /if (${IAmMA}) /return
        /if (${ACCaller.Equal[${MainAssist}]}) /varset CalledTargetID ${ACMobID}
    /return
| ----------------------------------------------------------------------------
| SUB: Event GotHit - Written by TreeHuginDruid for RedGuides
| ----------------------------------------------------------------------------
    Sub Event_GotHit(string Line, string AttackingMob)
        /varset EventFlag 1
        /declare GotHitDistChk int local 10
        /varset SitToMedTimer ${SitToMedTimer.OriginalValue}
        /varset GotHitToggle 1
        /declare AttackingMobID int local
        /if (${AggroTargetID} && !${Target.ID} && !${ReturnToCamp}) {
            /doevents flush GotHit
            /if (${DPSOn} || ${MeleeOn}) {
                /call CheckForCombat 0 Event_GotHit1 0
            } else {
                /call CheckForCombat 1 Event_GotHit2 0
            }
        }
        /if (${Select[${Role},pullertank,pettank,pullerpettank,hunterpettank]}) {
            /if (${Pulled} && ${WaitTimer} && ${Select[${Role},pullertank,pullerpettank]} && ${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}<=${CampRadius}) {
                /if (${NearestSpawn[npc los radius ${CampRadius} targetable].ID}!=${MyTargetID}) {
                    /varset MyTargetID 0
                    /varset MyTargetName
                    /if (${Target.ID}) /squelch /target clear
                }
                /varset WaitTimer 0s
                /varset AttackingMobID ${NearestSpawn[npc radius 39 targetable "${AttackingMob}"].ID}
                /echo I got hit by >>${AttackingMob}<< ID: ${AttackingMobID} while waiting for mob.
            }
            /if (${Select[${Role},pettank,pullerpettank,hunterpettank]}) {
                /doevents flush GotHit
                /varset AttackingMobID ${NearestSpawn[npc radius 39 targetable "${AttackingMob}"].ID}
                /squelch /target id ${Spawn[${AttackingMobID}].ID}
                /delay 10 ${Target.ID}==${AttackingMobID}
                |- If mob that hit me still too close, move to camp or away from mob.
                /if (${GotHitToggle} && ${AttackingMobID}>0) {
                    /echo I got hit by >>${AttackingMob}<< ID:${AttackingMobID}!
                    /moveto dist 10
                    /if (${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}>${CampRadius} && !${Select[${Role},hunterpettank]} && ${ReturnToCamp}) {
                        |- Only move away from mob if MoveWhenHit enabled (1)
                        /if (${MoveWhenHit}) {
                            /echo Moving back to camp!
                            /moveto loc ${CampYLoc} ${CampXLoc}
                        }
                    } else {
                        |- If mob is in pet attack range from camp, send pet in.
                        /if (${Me.Pet.ID} && ${Math.Distance[${Target.Y},${Target.X}:${Me.Pet.Y},${Me.Pet.X}]}<=${PetAttackRange}) {
                            /pet attack
                            /if (!${Pet.Taunt}) /pet taunt on
                        }
                        |- Only move away from mob if MoveWhenHit enabled (1)
                        /if (${MoveWhenHit}) {
                            /echo Trying to get away from >>${AttackingMob}<<.
                            /if (${Math.Distance[${Me.Y},${Me.X}:${Target.Y},${Target.X}]}<=${GotHitDistChk}) /moveto loc ${Target.Y} ${Math.Calc[${Target.X}-${GotHitDistChk}]}
                        }
                    }
                }
                /if (${MoveWhenHit}) /delay 15 ${Math.Distance[${Me.Y},${Me.X}:${Target.Y},${Target.X}]}>${GotHitDistChk}
                /if (${FaceMobOn} && ${Target.ID}) /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
                /look 0
            }
        }
        /varset GotHitToggle 0
        /doevents flush GotHit
    /return
| ----------------------------------------------------------------------------
| SUB: Event Zoned
| ----------------------------------------------------------------------------
    Sub Event_Zoned(Message)
        /varset EventFlag 1
        /if (${Message.Find[Drunken Monkey]} || ${Message.Find[effects]}) /return
        /call PauseWhileZoning Event_Zoned
        /if (!${JustZoned}) /echo Just zoned
        /call CombatReset 0 zoned
        /varset JustZoned 200
        /varset CampOnDeathTimer 10m
        /if (${LastZone}!=${Zone.ID}) {
            /if (${Zone.Name.Find[,]} || ${Zone.Name.Find[']}) {
                /varset ZoneName ${Zone.ShortName}${If[${Me.InInstance},_I,]}
            } else {
                /varset ZoneName ${Zone}${If[${Me.InInstance},_I,]}
            }
            /if (${UseSpawnMaster}) /call LoadSpawnMaster
        }
        /varset LastZone ${Zone.ID}
        | Shut down combat and mez in GH, GL, PoK, PoT, and Abysmal
        /if (${Select[${Zone.ID},345,344,202,203,279]}) {
            /varset DMZ 1
        } else {
            /varset DMZ 0
        }
        /if (${CampZone}!=${Zone.ID}) {
            /if (${ReturnToCamp}) {
                /varset ReturnToCamp 0
                /varset RememberCamp 1
            }
        } else {
            /if (${RememberCamp} && ${Math.Distance[${CampYLoc},${CampXLoc}]}<=150) {
                /varset ReturnToCamp 1
                /varset RememberCamp 0
            }
            /if (${IAmDead}) /varset IAmDead 0
        }
        /call WinTitle
    /return
| ----------------------------------------------------------------------------
| SUB: Event Joined 20s switch to prevent heals from firing off
| ----------------------------------------------------------------------------
    Sub Event_Joined(string Jmessage,string Joinee)
        DEBUGBUFF Event_Joined Enter ${Joinee}
        /declare i int local
        /declare j int local
        /varset EventFlag 1
        /echo ${Joinee} has joined the party.
        /varset JoinedParty 200
        /varset j ${Group.Member[${Joinee}].Index}
        /for i 1 to ${Buffs.Size}
            /varset Buff${i}GM${j} 0
            /if (${Joinee.Equal[${MainAssist}]}) /varset Buff${i}GM7 0
        /next i
        /if (${ReadBuffsTimer}) /varset ReadBuffsTimer 0
        /varset ForceBuffs 2
        DEBUGBUFF Event_Joined Exit
    /return
| ----------------------------------------------------------------------------
| SUB: Event Too Close
| ----------------------------------------------------------------------------
    Sub Event_TooClose
        |/varset EventFlag 1
        /if (${MyTargetID} && ${AutoFireOn}==1 && ${CombatStart}) {
            /if (${Me.AutoFire}) /autofire
            /varset AutoFireOn 2
            /if (${UseMQ2Melee}) /killthis
        }
        /if (${Pulling} && ${PullWithAlt.Equal[Melee]}) {
           /echo Mob Too Close for ${PullWith}... Switching to Melee.
           /varset ToClose 1
        }
        /doevents flush TooClose
    /return
| ----------------------------------------------------------------------------
| SUB: Event Camping - end macro
| ----------------------------------------------------------------------------
    Sub Event_Camping
        /varset EventFlag 1
        /if (${IAmABard}) {
            /if (${Medley.Active}) /squelch /medley off
        }
        /endmacro
    /return
| ----------------------------------------------------------------------------
| SUB: Event Too Far
| ----------------------------------------------------------------------------
    Sub Event_TooFar
        |/varset EventFlag 1
        /if (${CombatStart} && ((!${UseMQ2Melee} && ${StickHow.Equal[I]}) || (${UseMQ2Melee} && ${MeleeDistance}==${CampRadius}))) /return
        /varcalc ZDist ${Math.Distance[${Target.Z}:${Me.Z}]}-(${Target.Height}-${Me.Height})
        /if (${Math.Distance[${Target.Y},${Target.X}:${CampYLoc},${CampXLoc}]}<=${MeleeDistance} && ${MeleeOn} && ${CombatStart}) {
            /if (!${UseMQ2Melee}) {
                /if (${Me.FeetWet}) {
                    /if (${FaceMobOn}) /face
                    /if (!${Me.Moving}) /moveto id ${MyTargetID} mdist ${If[${Target.Distance}>5,${Math.Calc[${MoveTo.ArrivalDist}-5]},1]} uw
                    /delay 10
                    /if (${IAmMA}) {
                        /squelch /stick ${MoveTo.ArrivalDist} id ${MyTargetID} uw
                    } else {
                        /squelch /stick ${MoveTo.ArrivalDist} id ${MyTargetID} !front moveback uw
                    }
                } else {
                    /if (!${Me.Moving}) /moveto id ${MyTargetID} mdist ${If[${Target.Distance}>5,${Math.Calc[${MoveTo.ArrivalDist}*.90]},1]}
                    /squelch /stick ${MoveTo.ArrivalDist} id ${MyTargetID} ${StickHow}
                }
            } else {
                /if (${FaceMobOn}) /face
                /if (!${Me.Moving}) /moveto id ${MyTargetID} mdist ${If[${Target.Distance}>5,${Math.Calc[${MoveTo.ArrivalDist}-5]},1]} uw
                /delay 10
            }
        }
        /call ZAxisCheck ${ZDist} 4.1
        /if (${Select[${Role},pullertank,pullerpettank,puller]} && ${Pulling}) {
            /varset PullTooFar 1
        }
        /doevents flush TooFar
    /return
| ----------------------------------------------------------------------------
| SUB: Event Task Update - EQBC message
| ----------------------------------------------------------------------------
    Sub Event_TaskUpdate(Line,name)
        /varset EventFlag 1
        /call BroadCast t "Task updated...(${name})"
    /return
| ----------------------------------------------------------------------------
| SUB: Event Gain Something - EQBC message
| ----------------------------------------------------------------------------
    Sub Event_GainSomething(string Line,string text)
        /varset EventFlag 1
        /if (!${EQBCOn} && !${IRCOn} && !${DanNetOn}) /return
        /if (${Line.Find["ABILITY POINT!"]}) {
            /call BroadCast w "${Me.Name} gained an AA, now has ${Me.AAPoints} unspent"
        } else /if (${Line.Find[LEVEL]}) {
            /call BroadCast w "${Me.Name} gained a level, now is Level ${Me.Level}"
            |/if (${GMailEvents.Find[level]}) /call GmailSend "${Me.Name} gained a level, I am Level ${Me.Level}"
            | Recalculate pull when level gained
            /if (${PullLevel.Equal[auto]}) {
               /varcalc PullMin ${Me.Level}-5
               /varcalc PullMax ${Me.Level}+2
            } else /if (${PullLevel.Equal[linked]}) {
               /varcalc PullMin ${PullMin}+1
               /varcalc PullMax ${PullMax}+1
            }
            | need to add a new flag and set for memming new tombs and spells if in inventory.
        }
    /return
| ----------------------------------------------------------------------------
| SUB: Event Invised Mainly to turn off bard twisting while invis.
| ----------------------------------------------------------------------------
    Sub Event_Invised
        /varset EventFlag 1
        /if (${IAmABard}) {
            /if (${Medley.Active}) /call CastBardCheck
        }
    /return
| ----------------------------------------------------------------------------
| SUB: Event ImDead
| ----------------------------------------------------------------------------
    Sub Event_ImDead(IDMessage)
        /varset EventFlag 1
        /if (${IAmDead}) /return
        /echo I have died and the Angels wept.
        |/if (${GMailEvents.Find[dead]}) /call GmailSend "I have died. EQ Said ${IDMessage} Loc ${Me.X} ${Me.Y}"
        /varset IAmDead 1
        /doevents flush ImDead
        /if (${IAmABard}) {
            /call CastBardCheck
            |/varset BardStartTwist 0
        }
        /delay 20
        /if (${Me.Hovering}) /call PauseWhileHovering Event_ImDead
        /call CombatReset 0 ImDead
        /while (!${Zone.ID} && !${Me.ID}) {
            /delay 5
        }
        /if (${CampZone}==${Zone.ID}) /varset IAmDead 0
    /return
| ----------------------------------------------------------------------------
| SUB: Event GoM On
| ----------------------------------------------------------------------------
    Sub Event_GoMOn
        /varset EventFlag 1
        /if (${Select[${Me.Class.ShortName},BRD,BER,MNK,ROG,WAR]} || ${GoMByPass}) {
            /varset GoMByPass 0
            /doevents flush GoMOn
            /doevents flush GoMOff
            /return
        }
        /if (!${CombatStart} || ${GoMTimer}) /return
        | Release timer after trying to cast for 10 secs
        /varset GoMTimer 3s
        /declare i int local
        /declare GoMCast string local
        /declare GoMTarget string  local
        /declare GoMCastStat string ${CastResult}
        /declare CondNo local int 0
        /for i 1 to ${GoMSpell.Size}
            /if (${ConOn} && ${GoMSpell[${i}].Find[|cond]}) {
                /varset CondNo ${GoMSpell[${i}].Mid[${Math.Calc[${GoMSpell[${i}].Find[|cond]}+5]},3]}
            } else {
                /varset CondNo 0
            }
            /varset GoMCast ${GoMSpell[${i}].Arg[1,|]}
            /varset GoMTarget ${GoMSpell[${i}].Arg[2,|]}
            /if (${GoMCast.Equal[null]} || ${GoMCast.Find[spell]} || ${GoMSpellTimer${i}}) /continue
            /echo Gift of Mana detected! Trying to cast ${GoMCast}
            /doevents flush GoMOn
            /if (${GoMTarget.Equal[Me]} && ${Me.ID}) {
                /varset GoMTarget ${Me.ID}
            } else /if (${GoMTarget.Equal[MA]} && ${Spawn[=${MainAssist}].ID}) {
                /varset GoMTarget ${Spawn[=${MainAssist}].ID}
            } else /if (${MyTargetID}) {
                /varset GoMTarget ${MyTargetID}
            } else {
                /continue
            }
            /varset GoMActive 1
            /if (${GoMTarget.Equal[${MyTargetID}]} && (${Target.Type.Equal[Corpse]} || !${Spawn[id ${MyTargetID}].ID})) {
                DEBUGN Event_GoMOn [GoM] skipping because mob is dead or a corpse.
                /echo [GoM] being skipped, because target (${GoMTarget}) is a corpse.
                /varset GoMTimer 0
                /delay 10
                /doevents GoMOff
                /return
            }
            | Wait for global cooldown if active
            /while (${Me.SpellInCooldown}) {
                /if (${MeleeOn}) {
                    /if (${WeaveArray.Size}>0 && ${WeaveArray[1].NotEqual[null]}) /call CastWeave ${MyTargetID}
                    /if (${MashArray.Size}>0 && ${MashArray[1].NotEqual[null]}) /call MashButtons
                } else {
                    /delay 2
                }
            }
            /if (${Me.SpellReady[${GoMCast}]}) {
                /call CastWhat "${GoMCast}" ${GoMTarget} GoM ${CondNo} 0
                /if (${Macro.Return.Equal[CAST_COND_FAILED]}) /continue
                /varset LastDPSCast ${GoMCast}
            } else {
                /echo "${GoMCast}" is not ready!
                /delay 10
            }
            DEBUGN Return: ${Macro.Return}
            /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                /echo Gift of Mana Casting >> ${GoMCast} <<
                /varset GoMTimer 0
                /varset GoMSpellTimer${i} ${Spell[${GoMCast}].MyDuration.TotalSeconds}*10
                /doevents flush GoMOn
                /delay 10
                /doevents GoMOff
                /varset CastResult ${GoMCastStat}
                /return
            } else /if (${Macro.Return.Equal[CAST_CANCELLED]}) {
                /varset CastResult CAST_CANCELLED
                /return
            }
        /next i
        /varset CastResult ${GoMCastStat}
    /return
| ----------------------------------------------------------------------------
| SUB: Event GoM Off
| ----------------------------------------------------------------------------
    Sub Event_GoMOff
        /varset EventFlag 1
        /varset GoMActive 0
    /return
| ----------------------------------------------------------------------------
| SUB: Event GUEQBC - Echo guild meassages in EQBC
| ----------------------------------------------------------------------------
    Sub Event_GUEQBC(Message,GUName,GUText)
        /varset EventFlag 1
        /if (${GUName.Equal[${Me.CleanName}]}) /return
        /if (${Macro.IsTLO[EQBC]}) {
            /if (${EQBC.Connected} && ${Select[${EQBCOn},2,4]}) /call BroadCast p "${GUName} tells Guild: ${GUText} "
        }
    /return
| ----------------------------------------------------------------------------
| SUB: Event GUEQBC - Echo fellowship meassages in EQBC
| ----------------------------------------------------------------------------
    Sub Event_FSEQBC(Message,FSName,FSText)
        /varset EventFlag 1
        /if (${FSName.Equal[${Me.CleanName}]}) /return
        /if (${Macro.IsTLO[EQBC]}) {
            /if (${EQBC.Connected} && ${Select[${EQBCOn},3,4]}) /call BroadCast p "${FSName} tells Fellowship: ${FSText} "
        }
    /return
| ----------------------------------------------------------------------------
| SUB: Event KABegCheck
| ----------------------------------------------------------------------------
    Sub Event_KABegCheck(Message,BCWho,BCWhat,KBCExtra)
        /varset EventFlag 1
        /declare BCx int local 0
        /declare BCItem string local
        /declare BCCast string local

        /if (${Me.CleanName.Equal[${BCWho}]}) /return
        |/echo ${BCWho} ${BCWhat}
        DEBUGBUFF KABegCheck: Enter ${BCWho} ${BCWhat}
        /if (${KABegForList.Length}>2000) {
            /echo Begged For List Exceeded Max Length. May have to beg again.
            /return
        }
        /if (!${BuffsOn}) /return
        /if (!${Select[${BCWhat},BEGFORITEMS,BEGFORBUFFS]}) /return
        /declare WorkID int local ${Int[${Spawn[PC ${BCWho}].ID}]}
        /if (!${BCWho.Length} || ${BCWho.Equal[null]} || !${WorkID} || ${WorkID}==${Me.ID}) /return
        /if (${Spawn[id ${WorkID}].Type.NotEqual[PC]}) /return
        /if (!${Raid.Member[${BCWho}].Group}) {
            /if (!${Me.Fellowship.Member[${BCWho}].Level}) {
                /if (!${Me.Guild.Equal[${Spawn[${BCWho}].Guild}]}) {
                    /if (!${Friends.Friend[${BCWho}]}) /return
                }
            }
        }
        /for BCx 1 to ${Buffs.Size}
            /if (${Buffs[${BCx}].Find[|alias|${BCWhat}]}) {
                /varset BCCast ${Buffs[${BCx}].Arg[1,|]}
                /if (${Buffs[${BCx}].Arg[2,|].Equal[summon]}) {
                    /if (${Buffs[${BCx}].Arg[6,|].NotEqual[${BCWhat}]}) /continue
                    /varset BCItem ${Buffs[${BCx}].Arg[3,|]}
                } else {
                    /varset BCItem ${Buffs[${BCx}].Arg[1,|]}
                }
                /if (!${Me.Book[${BCCast}]} && !${AltAbility[${BCCast}]}) /continue
                /varset KABegActive 1
                /if (${KABegForList.Length}) {
                    /varset KABegForList ${KABegForList}|${BCWhat}:${BCWho}:${BCx}
                } else {
                    /varset KABegForList ${BCWhat}:${BCWho}:${BCx}
                }
                /break
            }
        /next BCx
        DEBUGBUFF KABegCheck: Leave ${KABegActive}
    /return
| ----------------------------------------------------------------------------
| SUB: Event EQBCIRC - Pick up commands from MQ2IRC or MQ2EQBC(todo)
| ----------------------------------------------------------------------------
    Sub Event_EQBCIRC(EIMessage,EISender,EICommand)
        /varset EventFlag 1
        /if (${EICommand.Left[1].Equal[/]} && ${EICommand.Left[2].NotEqual[/]}) /docommand ${EICommand}
        /doevents flush EQBCIRC
    /return
| ----------------------------------------------------------------------------
| SUB: Event CharmBackO - Charmed a mob Back off and reset combat
| ----------------------------------------------------------------------------
    Sub Event_CharmBackO(string Line)
        /varset EventFlag 1
		/if (${IAmMA}) {
			/call Bind_BackOff 1 10
			/call Bind_BackOff 0 10
			/call BroadCast g "Reset combat for Charmed Pet"
		}
        /doevents flush CharmBackO
    /return
| ----------------------------------------------------------------------------
| SUB: Event_WornOff - secondary check to rebuff
| ----------------------------------------------------------------------------
    Sub Event_WornOff(string Line, string wospell, string wotarget)
        /varset EventFlag 1
        /if (!${RebuffOn}) /return
        /if (${wotarget.Equal[${Me}]}) /return
        /if (${AggroTargetID}) /return
        /if (${Me.Class.Name.Find[bard]}) /return
        /if (${wospell.Find[promised]}) /return
        DEBUGBUFF Event_WornOff Enter ${wospell} ${wotarget}
        /declare i int local
        /declare j int local
        /declare WOSpellTT string local 0
        /declare WOTries int local 0
        /declare WOMessage int local 0
		|/call BroadCast o "Charm On ${CharmOn} Charm SPELL ${CharmSpell} and wornoff spell ${wospell}"
		|/if (${CharmOn} && ${Spell[${wospell}]}==${Spell[${CharmSpell}]}) {
		|/call BroadCast y "Charm SPELL ${CharmSpell} and wornoff spell ${wospell}"
		|/varset EventFlag 1
		|/doevents CharmBroke ${Line} ${wospell} ${wotarget} Event_WornOff		
		|} 
        /if (${Me.Book[${wospell}]}) {
            /varset WOSpellTT ${Spell[${Me.Book[${Me.Book[${wospell}]}].ID}].TargetType}
        } else {
            /varset WOSpellTT ${Spell[${wospell}].TargetType}
        }
        /if (${WOSpellTT.Left[5].Equal[group]}) /varset WOSpellTT group
        DEBUGBUFF Event_WornOff ${WOSpellTT}
        /if (${Group.Member[${wotarget}].Index}) {
            /varset j ${Group.Member[${wotarget}].Index}
            /for i 1 to ${Buffs.Size}
                /if (${Buffs[${i}].Find[${wospell}]}) {
                    /varset Buff${i}GM${j} 0
                    /if (${wotarget.Equal[${MainAssist}]}) /varset Buff${i}GM7 0
                    /if (!${WOMessage}) {
                        /echo ${wotarget} needs ${wospell} because it wore off.
                        /varset WOMessage 1
                    }
                    /if (${Select[${WOSpellTT},single,group]} && ${Me.Buff[${wospell}].ID}) {
                        /if (${Select[${EverQuest.Server},fippy,vulak,ragefire,lockjaw]}==0) {
                            /while (${WOTries}<6) {
                                /removebuff "${wospell}"
                                /delay 5
                                /varcalc WOTries ${WOTries}+1
                                /if (${Me.Buff[${wospell}].ID}) {
                                    DEBUGBUFF Spell: ${WOSpellTT} Slot: ${Me.Buff[${wospell}].ID} Buff: ${i} GM: ${j}
                                } else {
                                    /break
                                }
                            }
                        }
                    }
                    | reset spell timer on caster
                    /varset Buff${i}GM0 0
                    /if (${ReadBuffsTimer}) /varset ReadBuffsTimer 0
                }
            /next i
        } else {
            /for i 1 to ${Buffs.Size}
                /if (${Buffs[${i}].Find[${wospell}]}) {
                    /if (${wotarget.Equal[${MainAssist}]}) /varset Buff${i}GM7 0
                    /if (!${WOMessage}) {
                        /echo ${wotarget} needs ${wospell} because it wore off.
                        /varset WOMessage 1
                    }
                    /if (${ReadBuffsTimer}) /varset ReadBuffsTimer 0
                }
            /next i
            /if (${IniNextTimer}) /varset IniNextTimer 0
        }
        DEBUGBUFF Event_WornOff Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Event PetToysPlease
| -------------------------------------------------------------------------------------
    Sub Event_PetToysPlease(Message,string MTPet)
        /varset EventFlag 1
        /if (!${MTPet.Length}) /return
        /if (${MTPet.Find[null]}) /return
        /if (!${Spawn[pet ${MTPet}].ID} && ${MTPet.NotEqual[group]}) /return
        /declare PTPPetID int local
        /declare GMemID int local
        /declare GMemShortName string local
        /declare GMemPetName string local
        /declare i int local 0
        DEBUGPET PetToysPlease: Enter
        | If group command give all pets in group toys
        /if (${MTPet.Find[GROUP]}) {
            /echo I am giving pet toys to every Pet in Group except mine.
            | Start at 1 my pet doesn't need toys
            /for i 1 to 5
            /varset GMemID ${Group.Member[${i}].ID};
            /varset PTPPetID ${Group.Member[${i}].Pet.ID}
            /varset GMemShortName ${Group.Member[${i}].Class.ShortName}
            /varset GMemPetName ${Group.Member[${i}].Pet.Name}
            DEBUGPET PetToysPlease: ${GMemID} && ${PTPPetID} && ${Select[${GMemShortName},shm,nec,mag,bst,dru,enc,shd]} && ${Spawn[${GMemPetName}].Type.Equal[Pet]}
            /if (${GMemID} && ${PTPPetID} && ${Select[${GMemShortName},shm,nec,mag,bst,dru,enc,shd]} && ${Spawn[${GMemPetName}].Type.Equal[Pet]}) {
                /if (${PetToysOn}) /call pettoys ${GMemPetName}
            }
            /next i
        } else {
            | Individual Pet.
            /if (${PetToysOn}) {
                /echo Giving pet toys to (${MTPet}).
                /call pettoys ${MTPet}
            }
        }
        DEBUGPET PetToysPlease: Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Event PetToysPlease New
| -------------------------------------------------------------------------------------
    Sub Event_PetToysPlease_1(Message,string MTPet)
        /varset EventFlag 1
        | this version will create a list of pets to give toys too.
        | the routine to give items to pets will be called from the main loop.
        |
        /if (!${MTPet.Length}) /return
        /if (${MTPet.Find[null]}) /return
        /if (!${Spawn[pet ${MTPet}].ID} && ${MTPet.NotEqual[group]}) /return
        /if (!${PetToysOn}) /return
        DEBUGPET PetToysPlease: Enter AddToList
        | If group command give all pets in group toys
        /if (${MTPet.Find[GROUP]}) {
            /echo Adding Group entry to pet toys list.
            | Start at 1 my pet doesn't need toys
            /if (!${KABegForPetList.Length} || ${KABegForPetList.Equal[null]}) {
                /varset KABegForPetList group
            } else {
                /varset KABegForPetList ${KABegForPetList}|group
            }
            /varset KAPetBegActive 1
        } else {
            | Individual Pet.
            /if (!${KABegForPetList.Length} || ${KABegForPetList.Equal[null]}) {
                /varset KABegForPetList ${MTPet}
            } else {
                /varset KABegForPetList ${KABegForPetList}|${MTPet}
            }
            /varset KAPetBegActive 1
        }
        DEBUGPET PetToysPlease: Leave AddToList
    /return
| -------------------------------------------------------------------------------------
| SUB: Event_Timer
| ------------------------------------ -------------------------------------------------
    Sub Event_Timer(Timer, OriginalValue)
        /varset EventFlag 1
        /if (${Select[${Timer},TributeTimer,AggroOffTimer,TellTimer,SitToMedTimer,CursorIDTimer]}==0) /return
        DEBUGN Event_Timer Enter ${Timer}
        /if (${Timer.Equal[TributeTimer]}) {
            /if (${UseTribute} && ${Me.TributeActive}) {
                /call IsSpawnNamed ${Target.ID}
                |/if (!${AggroTargetID} || (${Target.ID} && !${Target.Named})) {
                /if (!${AggroTargetID} || !${Macro.Return}) {
                    /squelch /tribute personal off
                    /squelch /trophy personal off
                } else {
                    |/if (${Target.ID} && ${Target.Named}) /varset ${Timer} 580s
                    /if (${Macro.Return}) /varset ${Timer} 580s
                }
            }
        } else /if (${Timer.Equal[AggroOffTimer]}) {
            /if (${Me.Feigning}) /stand
            /if (${Me.Invis}) /makemevisible
        } else /if (${Timer.Equal[CursorIDTimer]}) {
            /if (${Cursor.ID}) /call CheckCursor Event_Timer 1
            /varset CursorID 0
        } else /if (${Timer.Equal[SitToMedTimer]} && !${Me.Moving}) {
            /if (!${MeleeOn} && ${Me.Standing} && ${MedCombat}==2 && !${Me.Mount.ID} && !${Me.Casting.ID} && (${Me.TargetOfTarget.ID}!=${Me.ID} || ${Target.ID}==${Me.ID})) /sit
        }
        DEBUGN Event_Timer leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Event LeftGroup
| -------------------------------------------------------------------------------------
    Sub Event_LeftGroup(Line,LGName)
        /varset EventFlag 1
        |/if (${GMailEvents.Find[leftgroup]}) /call GmailSend "${LGName} has left the group."
        /doevents flush LeftGroup
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_BEGIN
| ----------------------------------------------------------------------------
    Sub Event_CAST_BEGIN(line,CBItem)
        DEBUGCAST Event_CAST_BEGIN \awEnter ${line}
        /varset castReturn CAST_SUCCESS

        DEBUGCAST Event_CAST_BEGIN \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_CANNOTSEE
| ----------------------------------------------------------------------------
    Sub Event_CAST_CANNOTSEE
        DEBUGCAST Event_CAST_CANNOTSEE \awEnter
        /varset castReturn CAST_CANNOTSEE
        DEBUGCAST Event_CAST_CANNOTSEE \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_COLLAPSE
| ----------------------------------------------------------------------------
    Sub Event_CAST_COLLAPSE
        DEBUGCAST Event_CAST_COLLAPSE \awEnter
        /varset castReturn CAST_COLLAPSE
        DEBUGCAST Event_CAST_COLLAPSE \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_COMPONENTS
| ----------------------------------------------------------------------------
    Sub Event_CAST_COMPONENTS
        DEBUGCAST Event_CAST_COMPONENTS \awEnter
        /varset castReturn CAST_COMPONENTS
        /doevents flush CAST_COMPONENTS
        DEBUGCAST Event_CAST_COMPONENTS \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_DISTRACTED
| ----------------------------------------------------------------------------
    Sub Event_CAST_DISTRACTED
        DEBUGCAST Event_CAST_DISTRACTED \awEnter
        /varset castReturn CAST_DISTRACTED
        DEBUGCAST Event_CAST_DISTRACTED \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_FAILED
| ----------------------------------------------------------------------------
    Sub Event_CAST_FAILED(line,name)
        DEBUGCAST Event_CAST_FAILED \awEnter
        /varset castReturn CAST_FAILED
        DEBUGCAST Event_CAST_FAILED \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_FDFAIL
| ----------------------------------------------------------------------------
    Sub Event_CAST_FDFAIL(line,name)
        DEBUGCAST Event_CAST_FDFAIL \awEnter
        /if (${name.Equal[${Me.Name}]} && ${Defined[castReturn]}) {
            /if (${Me.Sitting}) /stand
            /varset castReturn CAST_RESTART
        }
        DEBUGCAST Event_CAST_FDFAIL \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_FIZZLE
| ----------------------------------------------------------------------------
    Sub Event_CAST_FIZZLE
        DEBUGCAST Event_CAST_FIZZLE \awEnter
        /varset castReturn CAST_FIZZLE
        DEBUGCAST Event_CAST_FIZZLE \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_IMMUNE
| ----------------------------------------------------------------------------
    Sub Event_CAST_IMMUNE(line,name)
        DEBUGCAST Event_CAST_IMMUNE \awEnter
        DEBUGCAST Event_CAST_IMMUNE \aw${line}
        /varset castReturn CAST_IMMUNE
        DEBUGCAST Event_CAST_IMMUNE \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_INTERRUPT
| ----------------------------------------------------------------------------
    Sub Event_CAST_INTERRUPTED
        DEBUGCAST Event_CAST_INTERRUPT \awEnter
        /varset castReturn CAST_INTERRUPTED
        DEBUGCAST Event_CAST_INTERRUPT \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_NOMOUNT
| ----------------------------------------------------------------------------
    Sub Event_CAST_NOMOUNT
        DEBUGCAST Event_CAST_NOMOUNT \awEnter
        /varset castReturn CAST_NOMOUNT
        DEBUGCAST Event_CAST_NOMOUNT \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_NOTARGET
| ----------------------------------------------------------------------------
    Sub Event_CAST_NOTARGET
        DEBUGCAST Event_CAST_NOTARGET \awEnter
        /varset castReturn CAST_NOTARGET
        DEBUGCAST Event_CAST_NOTARGET \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_NOTREADY
| ----------------------------------------------------------------------------
    Sub Event_CAST_NOTREADY
        DEBUGCAST Event_CAST_NOTREADY \awEnter
        /varset castReturn CAST_NOTREADY
        DEBUGCAST Event_CAST_NOTREADY \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_OUTDOORS
| ----------------------------------------------------------------------------
    Sub Event_CAST_OUTDOORS
        DEBUGCAST Event_CAST_OUTDOORS \awEnter
        /varset castReturn CAST_OUTOFMANA
        DEBUGCAST Event_CAST_OUTDOORS \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_OUTOFMANA
| ----------------------------------------------------------------------------
    Sub Event_CAST_OUTOFMANA
        DEBUGCAST Event_CAST_OUTOFMANA \awEnter
        /varset castReturn CAST_OUTOFMANA
        DEBUGCAST Event_CAST_OUTOFMANA \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_OUTOFRANGE
| ----------------------------------------------------------------------------
    Sub Event_CAST_OUTOFRANGE
        DEBUGCAST Event_CAST_OUTOFRANGE \awEnter
        /varset castReturn CAST_OUTOFRANGE
        DEBUGCAST Event_CAST_OUTOFRANGE \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_RECOVER
| ----------------------------------------------------------------------------
    Sub Event_CAST_RECOVER
        DEBUGCAST Event_CAST_RECOVER \awEnter
        /varset castReturn CAST_RECOVER
        DEBUGCAST Event_CAST_RECOVER \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_RESISTED
| ----------------------------------------------------------------------------
    Sub Event_CAST_RESISTED(line,name)
        DEBUGCAST Event_CAST_RESISTED \awEnter
        DEBUGCAST Event_CAST_RESISTED \aw${name.Length} ${CheckResisted}
        /if (!${CheckResisted}) /return
        /echo \aw${name} was Resisted \ag
        /if (${castReturn.NotEqual[CAST_IMMUNE]}) /varset castReturn CAST_RESISTED
        /varset LastResisted ${name}
        DEBUGCAST Event_CAST_RESISTED \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_RESISTEDYOU
| ----------------------------------------------------------------------------
    Sub Event_CAST_RESISTEDYOU(line,name)
        DEBUGCAST Event_CAST_RESISTEDYOU \awEnter
        DEBUGCAST Event_CAST_RESISTEDYOU \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_STANDING
| ----------------------------------------------------------------------------
    Sub Event_CAST_STANDING
        DEBUGCAST Event_CAST_STANDING \awEnter
        /stand
        /varset castReturn CAST_RESTART
        DEBUGCAST Event_CAST_STANDING \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_STUNNED
| ----------------------------------------------------------------------------
    Sub Event_CAST_STUNNED
        DEBUGCAST Event_CAST_STUNNED \awEnter
        /if (${Me.Stunned}) {
            /delay 3s !${Me.Stunned}
        } else {
            /delay 7
        }
        /varset castReturn CAST_STUNNED
        DEBUGCAST Event_CAST_STUNNED \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_Success1
| ----------------------------------------------------------------------------
    Sub Event_CAST_SUCCESS1(line)
        DEBUGCAST Event_CAST_SUCCESS1 \awEnter
        /varset castReturn CAST_SUCCESS
        DEBUGCAST Event_CAST_SUCCESS1 \awLeave
    /return
| ----------------------------------------------------------------------------
| SUB: Event_CAST_TAKEHOLD
| ----------------------------------------------------------------------------
    Sub Event_CAST_TAKEHOLD
        DEBUGCAST Event_CAST_TAKEHOLD \awEnter
        /varset castReturn CAST_TAKEHOLD
        DEBUGCAST Event_CAST_TAKEHOLD \awLeave
    /return
