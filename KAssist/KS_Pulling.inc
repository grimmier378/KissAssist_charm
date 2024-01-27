| -------------------------------------------------------------------------------------
| SUB: PullVars
| -------------------------------------------------------------------------------------
    Sub PullVars(int pRange, pType, int pRange2)
        DEBUGPULL PullVars Enter
        /varset PullRange ${pRange}
        /varset PullWith ${pType}
        /if (${PullPathWpCount} && ${PullMoveUse.Equal[advpath]}) /varcalc MaxRadius ${TempMaxRadius}+${PullRange}
        DEBUGPULL MaxRadius is now ${MaxRadius}
        DEBUGPULL PullVars Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Bind AddToIgnore
| -------------------------------------------------------------------------------------
    Sub Bind_AddToIgnore(MTIgnore, int byID)
        | Take the targeted mob as a parameter for mob to ignore.
        /if (!${Defined[MTIgnore]}) /declare MTIgnore string local null
        /if (${MTIgnore.Equal[null]} || ${MTIgnore.Length}==0) /varset MTIgnore ${Target.CleanName}
        /if (!${byID}) {
            /if (${MTIgnore.Equal[null]} || ${Spawn[${MTIgnore}].Type.NotEqual[NPC]} || ${Spawn[${MTIgnore}].CleanName.Equal[null]}) {
                /echo No NPCs named (${MTIgnore}) detected. Nothing added to list. ${byID}
                /return
            }
            | Assign temp var   list
            /declare IgnoreAdd string local ${MobsToIgnore}
            /declare NameToUse string local
            /declare s_SpawnName string local ${Spawn[${MTIgnore}].CleanName}
            /if (${Spawn[${MTIgnore}].Name.Find[#]}) {
                /varset NameToUse #${s_SpawnName}
            } else /if (${s_SpawnName.Right[-10].Find[corpse]}) {
                /varset NameToUse ${s_SpawnName.Right[-8]}
            } else {
                /varset NameToUse ${s_SpawnName}
            }
            | If mob is on the list no need to add it again.
            /if (${MobsToIgnore.Find[${NameToUse}]}) {
                /echo >> ${NameToUse} << already on Ignore List.
                /return
            }
            | If MobsToIgnore default text with the word null in it assign var spawn clean name
            /if (${IgnoreAdd.Find[null]}) {
                /varset IgnoreAdd ${NameToUse}
            } else {
                /varset IgnoreAdd ${IgnoreAdd},${NameToUse}
            }
            | Remove's corpse if closest match is a mob corpse
            /ini "${InfoFileName}" "${ZoneName}" "MobsToIgnore" "${IgnoreAdd}"
            /echo AddToIgnore -> ${NameToUse} <- Adding to Ignore list.
            | Reassign MobsToIgnore var the new list
            /varset MobsToIgnore ${IgnoreAdd}
        } else {
            /if (${MTIgnore.Equal[null]} || ${Spawn[id ${MTIgnore}].Type.NotEqual[NPC]} || ${Spawn[id ${MTIgnore}].CleanName.Equal[null]}) {
                /echo No NPCs with ID: (${MTIgnore}) detected. Nothing added to list. ${byID}
                /return
            }
            /if (${Int[${MTIgnore}]}>0 && !${MobsToIgnoreByID.Find[${MTIgnore}|]}) {
                /varset MobsToIgnoreByID ${MTIgnore}|${MobsToIgnoreByID}
            }
        }
    /return
| -------------------------------------------------------------------------------------
| SUB: Bind AddToPull
| -------------------------------------------------------------------------------------
    Sub Bind_AddToPull(MTPull)
        /if (!${MTPull.Length} || ${MTPull.Find[null]} || ${Spawn[${MTPull}].ID}==${Me.ID}) {
            /echo No NPCs detected. Nothing added to list.
            /return
        }
        | Assign temp var MobsToPull list
        /declare PullAdd string local ${MobsToPull}
        /declare NameToUse string local
        /declare s_SpawnName string local ${Spawn[${MTPull}].CleanName}
        /if (${s_SpawnName.Equal[null]}) {
            /echo ERROR: No Mob with ${MTPull} in Name detected.
            /return
        }
        | If MobsToPull default text with the word null in it assign var spawn clean name
        /if (${Spawn[${MTPull}].Name.Find[#]}) {
            /varset NameToUse #${s_SpawnName}
        } else /if (${s_SpawnName.Right[-10].Find[corpse]}) {
            /varset NameToUse ${s_SpawnName.Right[-8]}
        } else {
            /varset NameToUse ${s_SpawnName}
        }
        | If mob is on the list no need to add it again.
        /if (${MobsToPull.Find[${NameToUse}]}) {
            /echo >> ${NameToUse} << already on Pull List.
            /return
        }
        /if (${PullAdd.Find[all]}) {
            /varset PullAdd ${NameToUse}
        } else {
            /varset PullAdd ${PullAdd},${NameToUse}
        }
        /ini "${InfoFileName}" "${ZoneName}" "MobsToPull" "${PullAdd}"
        /echo AddToPull-> ${NameToUse} <- Adding to Pull list.
        | Reassign MobsToPull var the new list
        /varset MobsToPull ${PullAdd}
    /return

	
| -----------------------------------------------------------------------------------------------------------
| SUB: FindMobToPull(int ReadyToPullFlag) (NEW VERSION) ReadyToPullFlag 1 - Find mob and pull  0 - Check to see if there is a mob
| -----------------------------------------------------------------------------------------------------------
    Sub FindMobToPull(int ReadyToPullFlag,int Piterations, int UseCampLoc)
        DEBUGPULL FindMobToPull PreCheck ${DMZ} ${Me.InInstance} ${ReadyToPullFlag} Pulled: ${Pulled} ${AggroTargetID} ${ChainPull} Invis: ${Me.Invis} ${PullHold} ${DPSPaused} ${Me.Buff[Resurrection Sickness].ID} ${Me.Buff[Revival Sickness].ID}
        /if ((${DMZ} && ${Me.InInstance}==FALSE) || !${Select[${Role},puller,pullertank,hunter,hunterpettank,pullerpettank]} || ${Pulled} || (${AggroTargetID} && !${ChainPull})) /return 0
        /if (${Me.Invis} || ${PullHold} || ${DPSPaused} || ${Me.Buff[Resurrection Sickness].ID} || ${Me.Buff[Revival Sickness].ID}) /return 0
        /if (${CheckOnReturn} && ${ReadyToPullFlag}) /varset CheckOnReturn 0
        /call MobRadar pull 0 FindMobToPull
        /if (${ChainPull} && (${MobCount}>1 || ${Me.XTarget[${XTSlot2}].ID})) /return 0
        /if (${ReadyToPullFlag}) {
            /if (${ChainPull}) {
                /if (${Target.ID}==${Me.ID}) {
                    /squelch /target clear
                    /delay 10
                }
                /if (${Me.XTarget[${XTSlot}].ID} && ((!${MyTargetID} && !${Target.ID}) || ${Target.PctHPs}>=${ChainPullHP} || (${MyTargetID} && ${Spawn[${MyTargetID}].PctHPs}>=${ChainPullHP}))) /return 0
                /if (${Math.Distance[${CampYLoc},${CampXLoc}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}>75) /return 0
            }
        }
        /if (${ReadyToPullFlag}) {
            /if (!${Select[${Role},puller,pullertank,hunter,hunterpettank,pullerpettank]} || ${Me.Buff[Resurrection Sickness].ID} || ${Me.Buff[Revival Sickness].ID} || ${IAmDead}) /return 0
            /if (${PetRampPullWait} && !${Me.CombatState.Equal[COMBAT]} && ${Select[${Role},pullerpettank]} && ${Select[${Me.Class.ShortName},MAG,NEC,BST]}) /call CheckRampPets
        }
        DEBUGPULL FindMobToPull Enter ${ReadyToPullFlag}
        /call PullRangeSet
        /while (1) {
            /varset EventFlag 0
            /doevents
            /if (!${EventFlag}) /break
        }
        /declare BeginSearchX    int    local 0
        /declare MobsNearCamp    int    local
        /declare MobsToPullList  int    local
        /declare ModsReturnCheck int    local 0
        /declare PullAttempts    int    local 3
        /declare PullCount       int    local
        /declare LastCount       int    local 1
        /declare Pindex          int    local 1
        /declare PTempRadius     int    local 0
        /declare vstStr1         string local
        /varset Pulling 0
        /call GroupWatch
        /if (${PullHold}) /return 0
        /if (!${Piterations}) /varset Piterations 1
        /if (${ReadyToPullFlag}) {
            /if (!${FailCounter}) /echo Looking for Close Range Mobs
        } else {
            /if (!${SpamTimer}) {
                /echo Checking for Close Range Mobs
                /varset SpamTimer 50
            }
        }
        | Clear alert list 1, add mobs to ignore alert list1, set timer to keep alert list manageable for pulling no alert 1
        /if (!${PullAlertTimer}) {
            /varset PullIgnore1 null
            | Add Ignore Mob list to alert list
            /call AlertAddToList 1 "${MobsToIgnore}" "null"
            /varset PullAlertTimer 5m
        } else {
            /call PullIgnoreCheck 1 c
        }
        | Advpath searches for mobs along the path using Pullwith radius not Maxradius
        | FindAdvPath Routine searches and returns first valid mob along path.
        | Does not require additional code for searching.
        /if (${PullMoveUse.Equal[advpath]}) {
            /call FindMobAdvPath ${ReadyToPullFlag}
            /return ${Macro.Return}
        }
        /if (${Select[${Role},hunter,hunterpettank]}) {
            /if (${PullMoveUse.Equal[nav]}) {
                /varset vstStr1 npc
            } else {
                /varset vstStr1 npc los
            }
            /if (${UseCampLoc}) /varset vstStr1 ${vstStr1} loc ${CampXLoc} ${CampYLoc}
        } else {
            /if (${PullMoveUse.Equal[nav]}) {
                /varset vstStr1 npc loc ${CampXLoc} ${CampYLoc}
            } else {
                /varset vstStr1 npc los loc ${CampXLoc} ${CampYLoc}
            }
        }
        |To keep from taking to long to find the next mob. Have the search break the area into smaller sizes.
        |PullAttempts=3 All mobs in MaxRange. PullAttempts=1 Use range that will find 25 mobs at a time.
        /if (${SpawnCount[${vstStr1} radius ${MaxRadius} zradius ${MaxZRange} targetable ${SearchType}]}>${PullMaxCount}) /varset PullAttempts 1
        /while (1) {
            /varset PullCount 0
            /varset Pindex 1
            /varset Piterations 1
            /if (${PullAttempts}<3) {
                /varcalc ModsReturnCheck ${PullAttempts}*25
            } else {
                /varset ModsReturnCheck 10000
            }
            /while (!${PullCount} && (${Pindex}<=${Piterations})) {
                /if (${Pindex}==${Piterations}) {
                    /varset PTempRadius ${MaxRadius}
                    /varcalc Pindex ${Piterations}+1
                } else {
                    /varcalc PTempRadius (${MaxRadius}/${Piterations})*${Pindex}
                    /varcalc Pindex ${Pindex}+1
                }
                /varset PullCount ${SpawnCount[${vstStr1} radius ${PTempRadius} zradius ${MaxZRange} targetable ${SearchType}]}
                /if (${PullCount}>${ModsReturnCheck}) {
                    /varcalc Piterations ${Piterations}+1
                    /varcalc Pindex ${Pindex}-1
                    /varset PullCount 0
                    /continue
                }
                /varset MobsNearCamp ${SpawnCount[${vstStr1} radius ${MeleeDistance} zradius ${MaxZRange} targetable ${SearchType}]}
                DEBUGPULL FindMobToPull: Checking to temp radius \at${PTempRadius}\ax and found \at${PullCount}\ax mobs \at${Pindex} - ${Piterations}\ax
            }
            /if (${Bool[${Plugin[MQ2Map]}]}) /squelch /mapfilter SpellRadius ${PTempRadius}
            DEBUGPULL FindMobToPull ReadyToPullFlag:${ReadyToPullFlag} Pullcount: ${LastCount} : ${PullCount} MobsNearCamp: ${MobsNearCamp} Pull How: ${PullMoveUse} Attempt: ${PullAttempts} Search Type: ${SearchType} vstStr1: ${vstStr1}
            /if (!${ReadyToPullFlag}) {
                | Are we Chain Pulling and Last Mob Pulled still out of melee range?
                /if (${PullCount}>0 && ${Spawn[${LastMobPullID}].Distance}>=${MeleeDistance}) /return 0
                | We are chain Pulling so recalc PullCount based on PullCount - Mobs in MelleDistance.
                /varcalc PullCount ${PullCount}-${MobsNearCamp}
            }
            | If PullCount is NOT zero then lets find a valid mob to pull.
            /if (${PullCount}) {
                /if (!${ReadyToPullFlag} && ${MobsNearCamp}) {
                    /if (${LastCount}<2) {
                        /varset BeginSearchX 2
                    } else {
                        /varset BeginSearchX ${LastCount}
                    }
                } else {
                    /varset BeginSearchX ${LastCount}
                }
                /if (${PullMoveUse.Equal[nav]}) {
                    /call FindMobNAV ${ReadyToPullFlag} ${BeginSearchX} ${PullCount} "${vstStr1}"
                } else /if (${PullMoveUse.Equal[los]}) {
                    /call FindMobLOS ${ReadyToPullFlag} ${BeginSearchX} ${PullCount} "${vstStr1}"
                }
                /varset LastCount ${PullCount}
                /if (!${PullMob}) /varset ChainPullTemp 0
            } else {
                | There were no mobs to pull based on PullCount.
                /varset ChainPullTemp 0
                /varset PullMob 0
                /break
            }
            /if (${PullAttempts}==3 || ${PullMob}) /break
            /varcalc PullAttempts ${PullAttempts}+1
        }
        DEBUGPULL FindMobToPull: ReadyToPullFlag: ${ReadyToPullFlag} Name: ${Spawn[${PullMob}].CleanName} MobID: ${PullMob} PullCount: ${PullCount}
        DEBUGPULL FindMobToPull Leave
    /return ${PullMob}
| -----------------------------------------------------------------------------------------------------------
| SUB: Find Mob Using Advanced Path.
| -----------------------------------------------------------------------------------------------------------
    Sub FindMobAdvPath(int PFlag)
        /declare i int local
        /declare k int local
        /declare l int local
        /declare PullMobName string local 0
        /declare PullMobDistance float local 0
        /declare PullMobValid int local 0
        /declare DistanceCheck int local
        /declare WPMobCount int local
        | Advpath searches for mobs along the path using Pullwith radius not Maxradius
        DEBUGPULL FindMobAvdPath Enter ${PFlag}
        /if (${Target.ID}) /squelch /target clear
        /varset PullMobValid 0
        | loop through pathwaypoints and check for mobs
        /for k 1 to ${PullPathWpCount}
            /varset WPMobCount 0
            /varset PullMob 0
            /if (${UseWayPointZ}) {
                /varset WPMobCount ${SpawnCount[npc loc ${PullPathArrayX[${k}]} ${PullPathArrayY[${k}]} ${PullPathArrayZ[${k}]} radius ${PullRange} zradius ${MaxZRange} targetable ${SearchType}]}
            } else {
                /varset WPMobCount ${SpawnCount[npc loc ${PullPathArrayX[${k}]} ${PullPathArrayY[${k}]} radius ${PullRange} zradius ${MaxZRange} targetable ${SearchType}]}
            }
            /if (${WPMobCount}) {
                /varset i 0
                /for l 1 to ${WPMobCount}
                    /if (${UseWayPointZ}) {
                        /varset PullMob ${NearestSpawn[${l}, npc loc ${PullPathArrayX[${k}]} ${PullPathArrayY[${k}]} ${PullPathArrayZ[${k}]} radius ${PullRange} zradius ${MaxZRange} targetable ${SearchType}].ID}
                    } else {
                        /varset PullMob ${NearestSpawn[${l}, npc loc ${PullPathArrayX[${k}]} ${PullPathArrayY[${k}]} radius ${PullRange} zradius ${MaxZRange} targetable ${SearchType}].ID}
                    }
                    DEBUGPULL FindMobAdvPath 1: WP: ${k} MobID ${PullMob} LOS ${LineOfSight[${PullPathArrayY[${k}]},${PullPathArrayX[${k}]},${PullPathArrayZ[${k}]}:${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X},${Spawn[id ${PullMob}].Z}]} YXZ ${PullPathArrayY[${k}]},${PullPathArrayX[${k}]},${PullPathArrayZ[${k}]}:${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X},${Spawn[id ${PullMob}].Z}
                    DEBUGPULL FindMobAdvPath 1.0: WP: ${k} MobID ${PullMob} UseWaypointZ: ${UseWayPointZ} WP Z: ${PullPathArrayZ[${k}]} Spawn Z: ${Spawn[id ${PullMob}].Z} Distance From Z: ${Math.Distance[${PullPathArrayZ[${k}]}:${Spawn[id ${PullMob}].Z}]}
                    /if (${DebugPull}) /delay 1
                    /if (${PullMob} && ${LineOfSight[${PullPathArrayY[${k}]},${PullPathArrayX[${k}]},${PullPathArrayZ[${k}]}:${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X},${Spawn[id ${PullMob}].Z}]}) {
                        /varset DistanceCheck ${Math.Distance[${PullPathArrayY[${k}]},${PullPathArrayX[${k}]},${PullPathArrayZ[${k}]}:${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X},${Spawn[id ${PullMob}].Z}]}
                        /if (${k}<${PullPathWpCount}) {
                            /varcalc i ${k}+1
                            /while (${i}<=${PullPathWpCount} && ${DistanceCheck}>${Math.Distance[${PullPathArrayY[${i}]},${PullPathArrayX[${i}]},${PullPathArrayZ[${i}]}:${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X},${Spawn[id ${PullMob}].Z}]}) {
                                /varset DistanceCheck ${Math.Distance[${PullPathArrayY[${i}]},${PullPathArrayX[${i}]},${PullPathArrayZ[${i}]}:${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X},${Spawn[id ${PullMob}].Z}]}
                                /varset k ${i}
                                /varcalc i ${i}+1
                            }
                        }
                        DEBUGPULL FindMobAdvPath: WP ${k} MobCount ${l} ${WPMobCount}
                        /varset PullMobName ${Spawn[id ${PullMob}].CleanName}
                        /varset PullMobDistance ${Math.Distance[${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X}:${PullPathArrayY[${k}]},${PullPathArrayX[${k}]}]}
                        /varcalc AdvpathPointNum ${k}
                        /varset AdvpathPointX ${PullPathArrayX[${k}]}
                        /varset AdvpathPointY ${PullPathArrayY[${k}]}
                        /varset AdvpathPointZ ${PullPathArrayZ[${k}]}
                        DEBUGPULL FindMobAdvPath 2: WP: ${k} Pullmob ${PullMob} Waypoint: ${AdvpathPointNum} XY: ${PullPathArrayX[${k}]} ${PullPathArrayY[${k}]} ${Math.Distance[${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X}:${AdvpathPointY},${AdvpathPointX}]}<${PullRange}
                        DEBUGPULL FindMobAdvPath 2: AdvPull: Pullmob: ${PullMobName} ID ${PullMob} Waypoint: ${PullPathWpCount} XY: ${PullPathArrayX[${k}]} ${PullPathArrayY[${k}]}
                        /call PullValidate ${PullMob} ${PFlag}
                        /varset PullMobValid ${Macro.Return}
                        /if (${PullMobValid}) /return ${PullMob}
                    }
                /next l
            }
        /next k
        /varset PullMob 0
        DEBUGPULL FindMobAdvPath: Mob Not Found. AdvPull: ReadyToPullFlag: ${PFlag} Role: ${Role} ID: ${PullMob} PullRange: ${PullRange}
        DEBUGPULL FindMobAdvPath: ReadyToPullFlag: ${PFlag} Leave
        | Check for mobs in max pull radius for normal,MQ2nav pulling
    /return 0
| -----------------------------------------------------------------------------------------------------------
| SUB: Find Mob Using Navigation.
| -----------------------------------------------------------------------------------------------------------
    Sub FindMobNAV(int PFlag, int BCount, int MobsInRadius, string SpawnFilter)
        /declare p int local
        /declare PullMobName string local 0
        /declare PullMobValid int local 0
        /declare MobID          int     local   0
        /declare PathLength     float   local   0
        /declare MobIDDistance  float   local   0
        /declare PathCount      int     local   1
        /declare PathShortest   float   local   0
        /if (!${BCount}) /varset BCount 1
        DEBUGPULL FindMobNAV Enter ${PFlag} ${MobID}
        /for p ${BCount} to ${MobsInRadius}
            DEBUGPULL FindMobNAV: ${SpawnFilter} ${Spawn[${MobID}].Distance} ${PathShortest} ${PathCount}
            /varset MobID ${NearestSpawn[${p}, ${SpawnFilter} radius ${MaxRadius} zradius ${MaxZRange} targetable ${SearchType}].ID}
            /if (${Spawn[${MobID}].Distance}>${PathShortest} && ${PathCount}>1) {
                /continue
            }

            DEBUGPULL FindMobNAV: Call PullValidate ${MobID} ${PFlag}
            /call PullValidate ${MobID} ${PFlag}
            DEBUGPULL FindMobNAV: ${p}.${MobID} Macro.Return ${Macro.Return}
            /if (!${Macro.Return}) {
                DEBUGPULL FindMobNAV: ${MobID} not valid pull ${Macro.Return}
                /continue
            }

            /varset PathLength ${Navigation.PathLength[id ${MobID}]}
            DEBUGPULL FindMobNAV: Sorting Mobs by nav pathlength
            /if (${PathLength} > 0) {
                /varset ChainPullTemp ${MobID}
                DEBUGPULL FindMobNAV: ${MobID} Valid pull - ${PathLength}
                /if (${PathCount}==1) {
                    /varset PullMob ${MobID}
                    /varset PathShortest ${PathLength}
                } else /if (${PathLength} < ${PathShortest}) {
                    /varset PullMob ${MobID}
                    /varset PathShortest ${PathLength}
                }
                /varcalc PathCount ${PathCount}+1
            } else {
                DEBUGPULL FindMobNAV: ${MobID} not valid pull - ${PathLength}
            }
        /next p
        /if (${PullMob}) {
            /varset PullMobName ${Spawn[id ${PullMob}].CleanName}
        } else {
            /varset PullMob 0
            /varset PullMobName
        }
        DEBUGPULL FindMobNAV: ${PullMob} PullMobName ${PullMobName} ReadyToPullFlag: ${PFlag} Leave
    /return ${PullMob}
| -----------------------------------------------------------------------------------------------------------
| SUB: Find Mob Using LOS.
| -----------------------------------------------------------------------------------------------------------
    Sub FindMobLOS(int PFlag, int BCount, int ECount, SpawnFilter)
        /declare i int local
        /declare p int local
        /declare PullMobName string local 0
        /declare PullMobValid int local 0
        DEBUGPULL FindMobLOS Enter ${PFlag}
        /for i ${BCount} to ${ECount}
            DEBUGPULL FindMobLOS 1.0:  ReadyToPullFlag: ${PFlag} Pullcount: ${ECount}
            /varset PullMob ${NearestSpawn[${i}, ${SpawnFilter} radius ${MaxRadius} zradius ${MaxZRange} targetable noalert 1].ID}
            /if (${PullMob}) /varset PullMobName ${Spawn[id ${PullMob}].CleanName}
            DEBUGPULL FindMobLOS: ReadyToPullFlag: ${PFlag} Normal Name: ${PullMobName} ID: ${PullMob} LOS: ${LineOfSight[${CampYLoc},${CampXLoc},${CampZLoc}:${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X},${Spawn[id ${PullMob}].Z}]}
            /varset PullMobValid 0
            /if (${PullMob}) {
                /call PullValidate ${PullMob} ${PFlag}
                DEBUGPULL FindMobLOS: ${i}.${PullMob} Macro.Return ${Macro.Return}
                /if (${Macro.Return}) {
                    /varset PullMobValid 1
                    /varset ChainPullTemp ${PullMob}
                }
            }
            /if (${PullMobValid}) /return ${PullMob}
        /next i
        /varset PullMob 0
        DEBUGPULL FindMobLOS: ReadyToPullFlag: ${PFlag} Leave
    /return ${PullMob}
| -----------------------------------------------------------------------------------------------------------
| SUB: PullCheck PullMob is Mob ID of mob to pull.
| -----------------------------------------------------------------------------------------------------------
    Sub PullCheck
        DEBUGPULL PullCheck Enter ${PullMob}
        DEBUGPULL PullCheck: 1.0: Name: ${Spawn[${PullMob}].CleanName} MobID: ${PullMob} ${PullMoveUse}
        /if (${ChainPull}==2) /varset ChainPull 1
        /if (${PullOnReturn} && ${CheckOnReturn}) /varset CheckOnReturn 0
        /if (${PullHold} || ${DPSPaused} || ${Me.Buff[Resurrection Sickness].ID} || ${Me.Buff[Revival Sickness].ID} || ${CampZone}!=${Zone.ID} || ${Me.Invis}) /return 0
        /if (${PullMob} && ${Select[${PullMoveUse},los,nav]}>=1 && (${Spawn[${PullMob}].Distance3D}<=360 || ${Spawn[${PullMob}].LineOfSight})) {
            /target id ${PullMob}
            /delay 20 ${Target.ID}==${PullMob}
        }
        /if (${PullMob} && ${Spawn[${PullMob}].ID}) {
            /varset Pulling 1
            | Advpath skill validate mob
            /if (${PullMoveUse.Equal[advpath]}) {
                /varset MyTargetID ${PullMob}
                /varset MyTargetName ${Spawn[${PullMob}].CleanName}
                /target id ${PullMob}
                /delay 20 ${Target.ID}==${PullMob}
                | Is mob really in range. check for spawn lag
                /if (${Math.Distance[${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X}:${AdvpathPointY},${AdvpathPointX}]}>${PullRange}) {
                    /call PullIgnoreCheck ${PullMob} a
                    /echo Mob is OOR after targeting. Trying Again. ${PullMob} ${Math.Distance[${Spawn[id ${PullMob}].Y},${Spawn[id ${PullMob}].X}:${AdvpathPointY},${AdvpathPointX}]} ${PullRange} - ${AdvpathPointY} ${AdvpathPointX}
                    /squelch /target clear
                    /varset Pulling 0
                    /return 0
                }
            }
            /call ValidateTarget ${PullMob}
            DEBUGPULL PullCheck: Target: ${Spawn[${PullMob}].CleanName} Valid: ${ValidTarget} MacReturn ${Macro.Return}
            /if (${ValidTarget}==0 || (${Target.ID} && ${Select[${PullMoveUse},los,nav]}>=1 && ${Math.Distance[${Target.Y},${Target.X}:${CampYLoc},${CampXLoc}]}>${MaxRadius})) {
                /call PullIgnoreCheck ${PullMob} a "PullChecknav${Macro.Return}"
                /squelch /target clear
                /varset Pulling 0
                /return 0
            }
            /varcalc ColorIdx ${Select[${Spawn[${PullMob}].ConColor.Replace[ ,_]},grey,green,light_blue,blue,white,yellow,red]}+1
            /varset ConColor ${ColorList.Mid[${ColorIdx},1]}
            /if (${BroadCastSay.Equal[bc]}) {
                /call BroadCast t "PULLING-> [+${ConColor}+]${Spawn[${PullMob}].CleanName}[+t+] <- ID:${PullMob} at ${Int[${Math.Distance[${Spawn[${PullMob}].Y},${Spawn[${PullMob}].X}:${CampYLoc},${CampXLoc}]}]} feet."
            } else {
                /if (${ConColor.EqualCS[W]}) /varset ConColor -${ConColor.Lower}
                /call BroadCast t "PULLING-> \a${ConColor}${Spawn[${PullMob}].CleanName}\at <- ID:${PullMob} at ${Int[${Math.Distance[${Spawn[${PullMob}].Y},${Spawn[${PullMob}].X}:${CampYLoc},${CampXLoc}]}]} feet."
            }
            /varset MyTargetID ${PullMob}
            /varset MyTargetName ${Spawn[${PullMob}].CleanName}
            /if (${Pulling} && ${MyTargetID}) {
                /while (1) {
                    /if (${PullWith.Equal[Pet]} && ${Me.Pet.ID}) {
                        /if (${Me.Pet.Combat}) /pet back off
                        /if (${Me.Pet.Stance.NotEqual[FOLLOW]}) /pet follow
                    } else /if (${PullWith.Equal[melee]}) {
                        /varcalc PullRange ${Spawn[id ${MyTargetID}].MaxRangeTo}*.90
                    }
                    /if (${Target.ID} && ${Target.ID}!=${PullMob}) {
                        /if (${Me.Combat}) /squelch /attack off
                        /squelch /target clear
                    }
                    /call Pull
                    /varset Pulled 0
                    /if (${PullPause.Arg[1,|].NotEqual[0]}) /varset PullWaitTimer2 ${Math.Calc[${PullPause.Arg[2,"|"]}+1]}m
                    /if (${Macro.Return.Equal[oor]}) {
                        /varset CheckOnReturn 1
                        /return oor
                    } else /if (${Macro.Return.Equal[lma]}) {
                        /echo I got here2: ${MyTargetID} ${PullMob} ${Pulling} ${Pulled}
                        /varset Pulling 1
                        /continue
                    } else /if (${Macro.Return.Equal[dcbtc]} || ${Macro.Return.Equal[micd]}) {
                        /varset Pulling 0
                        /if (${ReturnToCamp}) /call DoWeMove 1 pullcheck
                    } else /if (${Macro.Return.Equal[iad]}) {
                        /while (1) {
                            /varset EventFlag 0
                            /doevents
                            /if (!${EventFlag}) /break
                        }
                        /return IAD
                    } else {
                        /return 0
                    }
                    /break
                }
            }
        } else /if (!${PullMob}) {
            /varcalc FailCounter ${FailCounter}+1
            DEBUGPULL PullCheck: No Valid Target in Range ${MaxRadius} - ${FailCounter} Time(s)
            /if (${FailCounter}>=${FailMax}) {
                /call AlertClearList 1 PullCheck
                /varset FailCounter 0
                /varset PullAlertTimer 0
                /if (${Select[${Role},hunter,hunterpettank]} && ${Math.Distance[${CampYLoc},${CampXLoc}]} > 15) {
                    DEBUGPULL PullCheck Returning hunter to camp
                    /echo ${Role}: There are no mobs within ${MaxRadius}.
                    /if (${StayPut}) {
                        /echo ===> Waiting here for respawn.
                    } else {
                        /echo ===> Returning to camp.
                        /call DoWeMove 1 pullcheck
                    }
                }
                /if (${PullWait} && !${AggroTargetID}) {
                    /call BroadCast t "PULLING-> Waiting ${PullWait} seconds for mobs to respawn."
                    /call PullDelay ${PullWait}
                }
                /return 0
            }
        }
        DEBUGPULL PullCheck:  Leave
    /return 0
| -------------------------------------------------------------------------------------
| SUB: PullDelay
| -------------------------------------------------------------------------------------
    Sub PullDelay(int TimerAmount)
        DEBUGPULL PullDelay ${TimerAmount} Enter
        /declare PDTimer timer local ${TimerAmount}s
        /varset Pulling 0
        /varset Pulled 0
        /if (${MedOn} && !${Me.Mount.ID} && ${Me.Standing} && !${Me.Moving}) /sit
        /while (${PDTimer} && !${AggroTargetID}) {
            /doevents
            /delay 2
            DOPARSE
        }
        /if (${AggroTargetID}) {
            /if (${DPSOn} || ${MeleeOn}) {
                /call CheckForCombat 0 PullDelay1 0
            } else {
                /call CheckForCombat 1 PullDelay2 0
            }
        }
        DEBUGPULL PullDelay Leave
    /return
| ----------------------------------------------------------------------------
| SUB: PullValidate
| ----------------------------------------------------------------------------
    Sub PullValidate(PVPullMob, int PFlag)
        /declare j int local
        /declare skipFlag int    local 0
        /declare mobCheck string local
        /declare IsNamed1 bool   local False
        /if (${MobsToPull.NotEqual[all]}) {
            /for j 1 to 25
                /if (!${MobsToPull.Arg[${j},,].Length}) /break
                /varset mobCheck ${MobsToPull.Arg[${j},,]}
                DEBUGPULL PullValidate: PFlag: ${PFlag} ${j} ${PVPullMob} PullList ${Spawn[id ${PVPullMob}].CleanName}/${MobsToPull.Arg[${j},,]} ${Spawn[id ${PVPullMob}].CleanName.Equal[${MobsToPull.Arg[${j},,]}]}
                /if (${mobCheck.Find[*]}) {
                    /varset mobCheck ${mobCheck.Replace[*,]}
                    /if (${Spawn[id ${PVPullMob}].CleanName.Find[${mobCheck}]}) /varset skipFlag 1
                } else {
                    /if (${mobCheck.Find[#]}) /varset mobCheck ${mobCheck.Replace[#,]}
                    /if (${Spawn[id ${PVPullMob}].CleanName.Equal[${mobCheck}]}) /varset skipFlag 1
                }
                /if (${skipFlag}) /break
            /next j
            /if (!${skipFlag}) {
                DEBUGPULL PullValidate ${Spawn[id ${PVPullMob}].CleanName} NOT on MobsToPull List
                /return 0
            }
        }
        | If mob found on ignore list
        /if (${MobsToIgnore.Find[${Spawn[${PVPullMob}].CleanName}]}) /return 0
        /if (${MobsToIgnoreByID.Find[${PVPullMob}|]}) /return 0
        /if (${PullIgnore1.Find[${PVPullMob}|]}) /return 0
        /if (${PullLocsOn}) {
            /for j 1 to ${PullLocs.Size}
                /if (!${Bool[${PullLocs[${j}]}]}) /break
                /if (${Math.Distance[${Spawn[id ${PVPullMob}].Y},${Spawn[${PVPullMob}].X}:${PullLocs[${j}].Arg[1,|]}]}<=${PullLocs[${j}].Arg[2,|]}) {
                    DEBUGPULL PullValidate ${Spawn[id ${PVPullMob}].CleanName} is in Invalid PullLocs Location.
                    /return 0
                }
            /next j
        }
        DEBUGPULL PullValidate Pulling: ${Spawn[id ${PVPullMob}].CleanName}
        | Check spawn for out of range - macro start point as epicEnter
        /if (${Select[${PullMoveUse},los,nav]}>=1 && ${Math.Distance[${Spawn[id ${PVPullMob}].Y},${Spawn[${PVPullMob}].X}:${CampYLoc},${CampXLoc}]}>${MaxRadius}) {
            DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].CleanName} invalid target Reason: Out of Range
            /return 0
        }
        | Check spawn for eye of zomm/tallon ${SpawnCount[pc ${PVPullMob.Right[${Math.Calc[${PVPullMob.Length}-7]}]}]}
        /if (${Spawn[${PVPullMob}].CleanName.Find[Eye of]} && ${SpawnCount[pc ${Spawn[${PVPullMob}].CleanName.Right[${Math.Calc[${Spawn[${PVPullMob}].CleanName.Length}-7]}]}]}) {
            DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].CleanName} invalid target Reason: Eye of Zomm/Tallon
                /return 0
        }
        | Check spawn for line of sight - no mq2nav
        /if (${Select[${Role},puller,pullertank,pullerpettank]} && !${Spawn[${PVPullMob}].LineOfSight} && ${PullMoveUse.Equal[los]}) {
            DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].CleanName} invalid target Reason: No line of sight
                /return 0
        }
        | Check spawn for Level Range
        /if (${Spawn[${PVPullMob}].Level}<${PullMin} || ${Spawn[${PVPullMob}].Level}>${PullMax}) {
            DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].Level} invalid target Reason: Invalid NPC Level
            /return 0
        }
        | Check if any PC/toon is near the mob
        /if (${SpawnCount[notid ${Me.ID} loc ${Spawn[${PVPullMob}].X} ${Spawn[${PVPullMob}].Y} radius 30 pc nogroup]}>0 && ${Pulling}) {
        |/if (${SpawnCount[loc ${Spawn[${PVPullMob}].X} ${Spawn[${PVPullMob}].Y} radius 15 pc]}>=1 && ${Pulling} && ${Math.Distance[${Spawn[${PVPullMob}].Y},${Spawn[${PVPullMob}].X}:${Me.Y},${Me.X}]}>=16) {
            DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].CleanName} invalid target Reason: PCs near mob
            /return 0
        }
        /if (${PullArcWidth}>0) {
            /call FigureMobAngle ${Int[${PVPullMob}]}
            /if (${Macro.Return.Equal[0]}) {
                DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].CleanName} ${PullLSide} ${Spawn[id ${PVPullMob}].HeadingTo[${CampYLoc},${CampXLoc}].Degrees} ${PullRSide} invalid target Reason: Mob not in Pull Area
                /return 0
            }
        }
        | Check spawn for 100% health
        /if (${Spawn[${PVPullMob}].PctHPs}<=99 && ${Spawn[${PVPullMob}].Distance}>=${MeleeDistance}) {
            DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].CleanName} invalid target Reason: Mob not 100% health ${Spawn[${PVPullMob}].PctHPs}
            | If chain pulling then target mob to update server.
            /if (${PFlag} && ${Spawn[${PVPullMob}].Distance}<=360 && ${Target.ID}!=${PVPullMob}) {
                /Echo Mob not at 100% HPs Double checking for server lag PFlag: ${PFlag} PVPullMob: ${PVPullMob} - ${Spawn[${PVPullMob}].CleanName} MobCount: ${MobCount}
                /target id ${Spawn[${PVPullMob}].ID}
                /delay 20 ${Target.ID}==${PVPullMob} && ${Target.BuffsPopulated}
                /if (${Target.PctHPs}>99) {
                    /return 1
                } else {
                    /call PullIgnoreCheck ${PVPullMob} a pullvalidate
                }
            }
            /return 0
        }
        /call IsSpawnNamed ${PVPullMob}
        /varset IsNamed1 ${Macro.Return}
        |/if (${Spawn[${PVPullMob}].Named} && ((${PFlag} && ${MobCount} && ${Me.XTarget[${XTSlot}].ID}) || !${PFlag})) {
        /if (${IsNamed1} && ((${PFlag} && ${MobCount} && ${Me.XTarget[${XTSlot}].ID}) || !${PFlag})) {
            DEBUGPULL PullValidate: PFlag: ${PFlag} ${Spawn[${PVPullMob}].CleanName} invalid target Reason: No Pull Names with Mobs in Camp
            /return 0
        }
    /return 1

| -------------------------------------------------------------------------------------
| SUB: Pull
| -------------------------------------------------------------------------------------
    Sub Pull
        /if (${DMZ} && ${Me.InInstance}==FALSE) /return
        /if (${Select[${Role},puller,pullertank,hunter,hunterpettank,pullerpettank]}==0) /return
        /if (!${Pulling}) /return
        /if (${DPSPaused} || ${Me.Invis}) /return
        /if (${PullHoldCond}) {
            /if (!${SpamTimer1}) {
                /echo Holding Pulls Due to Pull Hold Condition.
                /varset SpamTimer1 50
            }
            /return 0
        }
        DEBUGPULL Pull Enter
        /declare PullAttempts       int    local 0
        /declare StuckCount         int    local 0
        /declare PullDist           float  local ${PullRange}
        /declare AdvpathPaused      int    local 0
        /declare WPCurrent          int    local 0
        /declare AdvpathDirection   string local
        /declare X1                 int    local
        /declare Y1                 int    local
        /declare X2                 int    local
        /declare Y2                 int    local
        /declare AdjustHeadingTimer timer  local 0
        /declare WasInRange         int    local 0
        /declare PullStatusFlag     int    local 0
        /declare ReturnStat         string local 0
        /declare TryPetFirst        int    local 0
        /varset Pulled 0
        /varset PullTooFar 0
        /varset PullTimer 50
        /varset CantHit 0
        /varset ToClose 0
        /if (${Role.Equal[hunterpettank]}) /varset TryPetFirst 1
        | PullStatusFlag: 1-Normal Pull, 2-You are OOR Mob may be pullable, 3-Mob is OOR and may be pullable, 4-Don't even try pulling the mob, 5-I am OOR and I need to go back to camp.
        /if (${GroupWatchOn}) /call GroupWatch
        DEBUGPULL Pull: PullWith: ${PullWith} PullDist: ${PullDist} PullUsing: ${PullMoveUse}
        DEBUGPULL Check Ability: ${AggroTargetID} ${PullAggroTargetID} ${ChainPull} ${Select[${PullWith},Melee,Pet,Ranged]} ${Me.SpellReady[${PullWith}]} ${Me.AltAbilityReady[${PullWith}]} ${Me.CombatAbilityReady[${PullWith}]} ${Me.ItemReady[${PullWith}]} ${PullWith} ${PullWith.Equal[Ranged]} ${Me.RangedReady}
        | Set autofire setting off during pulls if not using ranged item to pull
        /if (${Me.AutoFire}) /autofire
        /if (${IAmABard}) {
          /call CastBardCheck 0
        }
        /if (!${Me.Mount.ID} && (${Me.Sitting} || ${Me.Feigning})) /stand
        /if (${Select[${Role},pullerpettank]} && ${PullRoleToggle} && ${Group.Puller.ID}!=${Me.ID}) {
            /if (${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X},${Spawn[${MyTargetID}].Z}:${CampYLoc},${CampXLoc},${CampZLoc}]}>${CampRadius}) /call PullModeToggle TurnOn
        }
        /varset BeginMobID ${Me.XTarget[${XTSlot}].ID}
        /while (1) {
            /varset PullStatusFlag 1
            /varset ReturnStat 0
            /while (1) {
                /varset EventFlag 0
                /doevents
                /if (!${EventFlag}) /break
            }
            /if (${CorpseRecoveryOn}) {
                /if (!${DragCorpse} && ${SpawnCount[pccorpse ${Me} radius ${MaxRadius}]}) {
                    /call GrabCorpse 1
                    /if (${DragCorpse}) {
                        /return dcbtc
                    }
                }
            }
            /if (${DPSPaused}) /return
            | vars used to determine if we are stuck
            /varset X1 ${Int[${Me.X}]}
            /varset Y1 ${Int[${Me.Y}]}
            /if (${PullAggroTargetID}) {
                DEBUGPULL Pulling 1.1
                /varset Pulled 1
                /varset MyTargetID ${AggroTargetID}
                /varset MyTargetName ${Spawn[${AggroTargetID}].CleanName}
                /call StopMoving
                DEBUGPULL Pull Aggro detected
                /break
            }
            /if (((${AggroTargetID} && !${ChainPull}) || (${Me.XTarget[${XTSlot2}].ID} && ${ChainPull})) && ${Math.Distance[${CampYLoc},${CampXLoc}]} < ${CampRadius}) {
                /echo Looks like mobs in camp aborting pull.
                /call PullReset
                DEBUGPULL Pull Mobs in camp detected
                /return micd
            }
            | Exit pull and reset if timed out or wandered too far from camp
            DEBUGPULL Pulling 1.2 PullTimer: ${PullTimer} Distance: ${Math.Distance[${CampYLoc},${CampXLoc}]} Distance3D: ${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]} MaxRadius: ${Math.Calc[${MaxRadius}*.90]} PullAttempts: ${PullAttempts} MyTargetID: ${Spawn[${MyTargetID}].ID}  Target LOS: ${Spawn[${MyTargetID}].LineOfSight}
            /if (${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X},${Spawn[${MyTargetID}].Z}:${CampYLoc},${CampXLoc},${CampZLoc}]}>${Math.Calc[${MaxRadius}+${PullRange}]}) {
                /varset PullStatusFlag 4
            } else /if (${PullTimer}==0 || (${PullAttempts}>=7 && !${Spawn[${MyTargetID}].LineOfSight}) || !${Spawn[${MyTargetID}].ID}) {
                /varset PullStatusFlag 4
            } else /if (${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]} >= ${Math.Calc[${MaxRadius}*.95]}) {
                /varset PullStatusFlag 5
            }
            /if (${PullStatusFlag}!=1) {
                /varset ReturnStat btcr
            }
            /if (${PullWith.Equal[Ranged]}) {
                /call PullWithRanged ${PullDist} 0
            }
            | - Filter to prevent pulling until AA/Disc/Spell/Item is ready.
            DEBUGPULL Pull Starting ${PullDist} ${PullMoveUse} ${PullStatusFlag} ${ReturnStat}
            /varset PullAttempts 0
            | Set group role puller to adjust for merc running up while pulling if soloing
            /if (${Group}==1 && !${Group.Puller.Name.Equal[${Me}]} && ${Select[${Role},puller]} && ${Spawn[=${MainAssist}].Type.Equal[Mercenary]}) /call AssignGroupRole set "${Me.CleanName}" 3
            /while (1) {
                /if (${PullStatusFlag}!=1) /break
|------------------------------------------------------------------------Pull Begin Move-------------------------------------------------------------------------------------------|
                /if ((${Spawn[${MyTargetID}].Distance3D}>${PullDist} || !${Spawn[${MyTargetID}].LineOfSight}) && ${Math.Distance[${Me.Y},${Me.X},${Me.Z}:${CampYLoc},${CampXLoc},${CampZLoc}]}<${MaxRadius}) {
|------------------------------------------------------------------------MQ2AdvPath-------------------------------------------------------------------------------------------|
                    | Advpath pull does not use max pull radius. It uses pullwith radius
                    /if (${PullMoveUse.Equal[advpath]}) {
                        /call PullUsingAdvPath ${MyTargetID} ${PullDist}
                        /if (${Macro.Return.Equal[btcr]}) {
                            /varset ReturnStat btcr
                            /varset PullStatusFlag 2
                            /break
                        }
                        /if (!${MyTargetID}) /return 0
|------------------------------------------------------------------------MQ2Nav-------------------------------------------------------------------------------------------|
                    } else /if (${PullMoveUse.Equal[nav]}) {
                        /call PullUsingNav ${MyTargetID} ${PullDist}
                        /if (${Macro.Return.Equal[huntpr]}) {
                            /varset ReturnStat btcr
                            /varset PullStatusFlag 5
                            /break
                        } else /if (${Macro.Return.Equal[btcr]}) {
                            /varset ReturnStat btcr
                            /varset PullStatusFlag 2
                            /break
                        } else /if (${Macro.Return.Equal[dcbtc]}) {
                            /return dcbtc
                        }
|------------------------------------------------------------------------LOS-------------------------------------------------------------------------------------------|
                    | We are pulling by LOS.
                    } else /if (${Spawn[${MyTargetID}].LineOfSight}) {
                        /if (${Me.FeetWet}) {
                            /if (${X2}==0) /varcalc PullDist ${PullDist}-(${Spawn[${MyTargetID}].Distance3D}-${Spawn[${MyTargetID}].Distance})
                            /moveto id ${MyTargetID} mdist ${PullDist} uw
                        } else {
                            /moveto id ${MyTargetID} mdist ${PullDist}
                        }
                        /delay 10
                        /varset X2 ${Int[${Me.X}]}
                        /varset Y2 ${Int[${Me.Y}]}
                        DEBUGPULL Pull LOS ${Spawn[${MyTargetID}].LineOfSight} ${PullDist}
|------------------------------------------------------------------------Mob OOR Return to Camp-------------------------------------------------------------------------------------------|
                    } else {
                        /if (${X2}!=0) {
                            /if (${Math.Distance[${Me.Y},${Me.X}:${Y2},${X2}]}>200 || ${PullTimer}==0) {
                                /echo Mob is no Longer in LOS. Returning to Camp.
                                /if (${MoveTo.Moving}) /moveto off
                                /varset ReturnStat btcr
                                /varset PullStatusFlag 4
                                /break
                            } else /if ((!${Me.Moving} || !${MoveTo.Moving}) && ${PullDist}<${Spawn[${MyTargetID}].Distance}) {
                                /if (${Me.FeetWet}) {
                                    /moveto id ${MyTargetID} mdist ${PullDist} uw
                                } else {
                                    /moveto id ${MyTargetID} mdist ${PullDist}
                                }
                                /delay 10
                            }
                        }
                        DEBUGPULL Pull NO-LOS ${Spawn[${MyTargetID}].LineOfSight} ${Spawn[${MyTargetID}].Distance3D} ${PullDist}
                    }
                } else /if (${Spawn[${MyTargetID}].Distance3D}<=${PullDist} && ${Spawn[${MyTargetID}].LineOfSight}) {
                    /varset PullStatusFlag 2
                    /break
                } else /if (${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X},${Spawn[${MyTargetID}].Z}:${CampYLoc},${CampXLoc},${CampZLoc}]}>${Math.Calc[${MaxRadius}+${PullRange}]}) {
                    /varset PullStatusFlag 4
                    /varset ReturnStat btcr
                    /break
                }
|------------------------------------------------------------------------End Pull Move-------------------------------------------------------------------------------------------|
                /if (${Pulled} || ${PullAggroTargetID}) /break
                /if (${Spawn[${MyTargetID}].Distance3D}<=${PullDist} && (${Target.FeetWet}==${Me.FeetWet} || ${PullWith.Equal[pet]})) /varset WasInRange 1
                /varset CantSee 0
                /while (1) {
                    /varset EventFlag 0
                    /doevents
                    /if (!${EventFlag}) /break
                }
                /varcalc PullAttempts ${PullAttempts}+1
                |Checking to see if you are far from camp to try and fix the exceed to far from camp check.
                /if (${Select[${PullMoveUse},los,nav]}>0) {
                    |Have I ran to far from camp?
                    /if (${Math.Distance[${CampYLoc},${CampXLoc},${CampZLoc}]}>=${Math.Calc[${MaxRadius}*.95]}) {
                        /if (${Spawn[${MyTargetID}].Distance3D}<=${PullDist} && (${Spawn[${MyTargetID}].LineOfSight} || ${PullWith.Equal[pet]})) {
                            /varset PullStatusFlag 2
                        } else {
                            /varset PullStatusFlag 4
                        }
                        /varset ReturnStat btcr
                        /break
                    }
                    | Extending PullTimer if moving closer or target moving && !${Select[${Role},hunter]}
                    /if (${PullMoveUse.Equal[nav]}) {
                        /if ((${Navigation.Active} || ${MoveTo.Moving} || (${Spawn[${MyTargetID}].Speed}>25 && ${Spawn[${MyTargetID}].Distance3D}>${PullRange}))) /varcalc PullTimer ${PullTimer}+50
                    } else /if (${PullMoveUse.Equal[los]} && !${Select[${Role},hunter,hunterpettank]}) {
                        /if ((${Me.Moving} || ${MoveTo.Moving} || (${Spawn[${MyTargetID}].Speed}>25 && ${Spawn[${MyTargetID}].Distance3D}>${PullRange}))) /varcalc PullTimer ${PullTimer}+50
                    }
                    | Try and pull again after 7 seconds or 3 seconds if target is moving
                    /if (${PullAttempts}>=7) {
                        | Make range smaller to creep closer to mob if not los
                        /if (${Spawn[${MyTargetID}].Distance3D}<=${PullDist} && !${Spawn[${MyTargetID}].LineOfSight} && ${PullWith.NotEqual[pet]}) /varcalc PullDist ${PullDist}*.6
                        DEBUGPULL Pull PullDist: ${PullDist} - PullLoops: ${PullAttempts} LineOfSight: ${Spawn[${MyTargetID}].LineOfSight}
                        /break
                    } else /if (${PullAttempts}>=3 && ${Spawn[${MyTargetID}].Speed}>25 && ${WasInRange} && ${Spawn[${MyTargetID}].Distance3D}>${PullDist}) {
                        | Make range smaller to creep closer to mob if Mob is Moving
                        /varcalc PullDist ${PullDist}*.6
                        /varset WasInRange 0
                        DEBUGPULL Pull PullDist: ${PullDist}*.6 - PullLoops: ${PullAttempts}
                        /break
                    }
                }
                /if (${DPSPaused}) /return
                | 1s timer used in conjunction with PullAttempts to control pulling
                /delay 10
|------------------------------------------------------------------------Are We Stuck-------------------------------------------------------------------------------------------|
                | - Check to see if we are stuck
                /if ((${Int[${Me.X}]}==${X1}) && (${Int[${Me.Y}]}==${Y1})) {
                    /varcalc StuckCount (${StuckCount})+1
                    /if (${StuckCount}>=2) {
                        /if (${IAmDead} || ${Me.Hovering}) {
                            /call Stopmoving
                            /return
                        } else {
                            /call Stuck pull
                        }
                    }
                    /if (${StuckCount}>=7 && !${PullAggroTargetID}) {
                        /echo I am stuck aborting pull
                        /call StopMoving
                        /varset ReturnStat btcr
                        /varset PullStatusFlag 4
                        /break
                    }
                }
|------------------------------------------------------------------------Are We Stuck End-------------------------------------------------------------------------------------------|
                DEBUGPULL Pull Loop Count: ${PullAttempts} ${Me.Moving} ${MoveTo.Moving} ${Me.Speed} ${Spawn[${MyTargetID}].Distance3D} ${PullRange} ${PullDist}
                | Not using advpath
                /if (${Select[${PullMoveUse},los,nav]}>0) {
                    | Distance loop check until mob in range to pull
                    /if ((${Spawn[${MyTargetID}].Distance3D}>${PullRange} || !${Spawn[${MyTargetID}].LineOfSight} || ${Target.FeetWet}!=${Me.FeetWet}) && !${PullAggroTargetID}) {
                        /if (${PullWith.NotEqual[pet]} || ${Spawn[${MyTargetID}].Distance3D}>${PullRange}) {
                        /if (!${Me.Moving} || ${WasInRange}) /varcalc PullDist ${PullDist}*.8
                            DEBUGPULL Pull Decrease pull distance to ${PullDist}
                            /continue
                        }
                    } else /if (${PullAggroTargetID}) {
                        /break
                    }
                }
                /varset PullStatusFlag 2
                /break
            | end of Inner /while loop.
            }
            /if (${Pulled} || ${PullAggroTargetID}) /break
            | Back To Camp Reset was flaged, but if target is not LOS, there is no reason to try and pull it.
            /if (${ReturnStat.Equal[btcr]}) {
                /if (${PullStatusFlag}==4) /break
                /if (${PullWith.NotEqual[pet]}) {
                    /if (!${Spawn[${MyTargetID}].LineOfSight}) /break
                } else {
                    /if (${Spawn[${MyTargetID}].Distance3D}>${PullRange}) /break
                }
                /if (${PullStatusFlag}!=5) /varset ReturnStat 0
            }
            | We are not close enough to the mob to pull
            /if (${PullStatusFlag}==1) /continue
            | We are in pull range and have LOS.
            /if (${Spawn[${MyTargetID}].ID} && (${Target.FeetWet}==${Me.FeetWet} || ${PullWith.Equal[pet]}) && !${PullAggroTargetID} && (${Spawn[${MyTargetID}].Distance3D}<${PullRange} || (${PullWith.Equal[Melee]} && ${Spawn[${MyTargetID}].Distance3D}<${Math.Calc[${PullRange}*2]}))) {
                /if (${PullMoveUse.Equal[advpath]}) /varset WasInRange 1
                /call StopMoving
                | Target mob before Aggroing
                /target id ${MyTargetID}
                /delay 20 ${Target.ID}==${MyTargetID}
                | Validate target one more time before pulling
                /call ValidateTarget
                /if (${ValidTarget}==0) {
                    /echo Aborting Pull! Target invalid now! Reason:${Macro.Return}
                    /varset ReturnStat btcr
                    /break
                }
                |/if (${CalmOn}) {
                |    /call PullCalmCheck ${Target.ID} ${CalmRadius}
                |}
                /if (${PullWith.Equal[Ranged]} && ${Spawn[${MyTargetID}].Distance3D}<30) /varset ToClose 1
|------------------------------------------------------------------------Send in Pet-------------------------------------------------------------------------------------------|
                /if (${TryPetFirst} && ${Pet.ID} && ${Spawn[${MyTargetID}].Distance3D}<${MeleeDistance}) {
                    /if (${PullWith.NotEqual[Pet]}) /echo ====>> Sending in Pet. Ignoring PullWith.
                    |/echo 1 - ${Pet.Combat}
                    /call CombatPet
                    |/echo 2 - ${Pet.Combat}
                    /if (!${Pet.Combat}) {
                        /if (${PullWith.NotEqual[Pet]}) /echo ====>> Sending in Pet Failed. Trying normal PullWith Option.
                        /varset TryPetFirst 0
                        /continue
                    }
                    /if (${PullAggroTargetID} || ${Pet.Combat}) /varset Pulled 1
|------------------------------------------------------------------------Pull with Melee-------------------------------------------------------------------------------------------|
                | Handle pulling with Melee setting
                } else /if (${PullWith.Equal[Melee]} || (${PullWithAlt.Equal[Melee]} && ${ToClose} && !${Select[${Role},hunter,hunterpettank]})) {
                    /call PullWithMelee
|------------------------------------------------------------------------Pull with Ranged-------------------------------------------------------------------------------------------|
                | Pull with ranged
                } else /if (${PullWith.Equal[Ranged]} && !${ToClose}) {
                    /call PullWithRanged ${PullDist} 1
                    /if (${Macro.Return}>=1) /varset PullDist ${Int[${Macro.Return}]}
|------------------------------------------------------------------------Pull with Pet-------------------------------------------------------------------------------------------|
                | Pull with pet
                } else /if (${PullWith.Equal[Pet]}) {
                    /call PullWithPet
|------------------------------------------------------------------------Pull with Cast-------------------------------------------------------------------------------------------|
                | Pull with cast
                } else {
                    /call PullWithCast ${PullDist}
                    /if (${Macro.Return}>=1) {
                        /varset PullDist ${Int[${Macro.Return}]}
                        |/continue
                    }
                }
|------------------------------------------------------------------------Pull with End-------------------------------------------------------------------------------------------|
                |- Toggle puller mode off if option enabled.
                /if (${Select[${Role},pullerpettank]} && ${PullRoleToggle} && ${Group.Puller.ID}==${Me.ID}) /call PullModeToggle TurnOff
            } else /if (${PullAggroTargetID}) {
                | The else /if fixes when puller stalls because puller grabs aggro with out getting to pull.
                /varset Pulled 1
            } else /if (${PullStatusFlag}==3 || ${PullStatusFlag}==5) {
                /break
            }
            | If pull failed start over while timer > 0
            /if (${Select[${PullMoveUse},los,nav]}>0 && !${Pulled}) /continue
            /break
        | end of outer /while loop
        }
        | reset mq2moveutils dist back to 10 from pull distance to ensure correct movement
        /moveto dist 10
        /if (!${Pulled} && ${PullAggroTargetID}) {
            /varset Pulled 1
            /varset ReturnStat 0
        }
        /if (${PullWith.Equal[Ranged]}) {
            /call PullWithRanged ${PullDist} 2
        }
        /varset Pulling 0
        /if (${ReturnStat.Equal[btcr]}) {
            /if (${MyTargetID}) {
                /call PullIgnoreCheck ${MyTargetID} a pull2
                /varset MyTargetID 0
            } else /if (${Target.ID}) {
                /call PullIgnoreCheck ${Target.ID} a pull3
                /squelch /target clear
            }
            |/echo I Got Here: ----------> ${PullOnReturn}
            /if (!${PullOnReturn}) {
                /call BackToCampReset
                /if (${Macro.Return}) /return 0
            } else {
                /return oor
            }
        }
        DEBUGPULL Pull Done Pulling ${ReturnToCamp} ${Pulled} ${PullAggroTargetID}
        /if (${ReturnToCamp}) {
            /if (${Pulled}) {
                /call DoWeMove 1 pull
                /if (${Macro.Return.Equal[iad]}) /return IAD
                /if (!${Me.Running}) /nomodkey /keypress RUN_WALK
                /if (${Macro.Return.Equal[lma]}) {
                    /varset Pulled 0
                    /echo Returned To Pull: lma
                    /return lma
                }
                /call WaitForMob
                /if (${Target.ID} && ${FaceMobOn}==3) /face fast nolook
                /varset Pulled 0
            } else {
                /call DoWeMove 1 pull
            }
        }
        DEBUGPULL Pull Leave Mob ID:${Spawn[${MyTargetID}].ID}
    /return
|--------------------------------------------------------------------------------------
| SUB: PullCalmCheck
|--------------------------------------------------------------------------------------
    Sub PullCalmCheck(int PCCTarget, int PCCRadius)
        /if (!${PCCRadius}) /varset PCCRadius 50
    /return
|--------------------------------------------------------------------------------------
| SUB: PullIgnoreCheck
|--------------------------------------------------------------------------------------
    Sub PullIgnoreCheck(string mobID, string Action, string sentFrom)
        /declare k int     local     0
        /if (${Int[${mobID}]}<1) /return
        /if (${Action.Equal[a]}) {
            /if (!${PullIgnore1.Find[${mobID}|]}) {
                /if (${SearchType.Equal[noalert 1]}) /squelch /alert add 1 id ${mobID}
                /varset PullIgnore1 ${mobID}|${PullIgnore1}
                /echo Added ${mobID} to ignore pull list. ${PullIgnore1} ${sentFrom}
            }
        } else /if (${Action.Equal[d]}) {
            /if (!${PullIgnore1.Find[${mobID}|]}) {
                /varset PullIgnore1 ${PullIgnore1.Replace[${mobID}|,]}
                /if (${SearchType.Equal[noalert 1]}) /squelch /alert remove 1 id ${mobID}
            }
        } else /if (${Action.Equal[c]}) {
            /varset k 1
            /while (${PullIgnore1.Arg[${k},|].Length} && ${PullIgnore1.Find[|]} && ${PullIgnore1.NotEqual[null]}) {
                DEBUGPULL PullIgnore: ${PullIgnore1} Count: ${k} Arg: ${PullIgnore1.Arg[${k},|]}
                /if ((!${Spawn[id ${PullIgnore1.Arg[${k},|]}].ID} || ${Spawn[${PullIgnore1.Arg[${k},|]}].Type.Equal[Corpse]}) && ${PullIgnore1.Arg[${k},|].NotEqual[null]}) {
                    /varset PullIgnore1 ${PullIgnore1.Replace[${PullIgnore1.Arg[${k},|]}|,]}
                    /if (${SearchType.Equal[noalert 1]}) /squelch /alert remove 1 id ${PullIgnore1.Arg[${k},|]}
                } else {
                    /varcalc k ${k}+1
                }
            }
        }
    /return
|--------------------------------------------------------------------------------------
| SUB: BacktoCampReset
|--------------------------------------------------------------------------------------
    Sub BacktoCampReset
        DEBUGPULL BacktoCampReset: Enter
        /if (${ReturnToCamp}) {
            /call DoWeMove 0 backtocampreset
        } else /if (${Math.Distance[${CampYLoc},${CampXLoc}]} < ${CampRadius} || ${Select[${Role},hunter,hunterpettank]}) {
            /call PullReset
            /call DoWeMove 1 backtocampreset
            /return TRUE
        }
        DEBUGPULL BacktoCampReset: Leave
    /return 0
|--------------------------------------------------------------------------------------
| SUB: Stop Moving
|--------------------------------------------------------------------------------------
    Sub StopMoving
        DEBUGMOVE StopMoving: Enter
        /if (${MoveTo.Moving}) /moveto off
        /if (${Stick.Active}) /stick off
        /if (${PullMoveUse.Equal[nav]}) {
            /if (${Navigation.Active}) /nav stop
        } else /if (${PullMoveUse.Equal[advpath]}) {
            /if (${AdvPath.State}) /play off
        }
        /delay 30 !${Me.Moving}
        DEBUGMOVE StopMoving: Leave
    /return
|--------------------------------------------------------------------------------------
| SUB: Pull with Melee
|-------------------------------------------------------------------------------------
    Sub PullWithMelee
        DEBUGPULL PullWithMelee: Enter
        | Turn off mq2melee function so puller and pullertank doesn't attack mob on pull
        /if (${Select[${Role},puller,pullertank]} && ${UseMQ2Melee}) /squelch /melee melee=0
        /moveto id ${MyTargetID} mdist 15
        /delay 10
        /while (1) {
            /if (${PullAggroTargetID} || ${Target.PctHPs}<100) /break
            /if (${Target.ID}) /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
            /look 0
            /if (${Target.MaxRangeTo}>=${Target.Distance3D}) {
                /if (!${Me.Moving}) {
                    /moveto id ${MyTargetID} mdist ${If[${MoveTo.ArrivalDist}>5,${Math.Calc[${MoveTo.ArrivalDist}-5]},1]}
                    /if (${PullMeleeStick} && !${PullAggroTargetID}) /stick ${MoveTo.ArrivalDist} front
                } else {
                    /moveto mdist ${If[${MoveTo.ArrivalDist}>5,${Math.Calc[${MoveTo.ArrivalDist}-5]},1]}
                    /if (${PullMeleeStick} && !${PullAggroTargetID}) /stick ${MoveTo.ArrivalDist} front
                }
                /if (!${Stick.Active} && ${PullMeleeStick} && !${PullAggroTargetID}) /stick ${MoveTo.ArrivalDist} id ${MyTargetID} front
            } else {
                /if (!${Me.Moving}) {
                    /moveto id ${MyTargetID} mdist ${If[${MoveTo.ArrivalDist}>5,${Math.Calc[${MoveTo.ArrivalDist}-5]},1]}
                    /if (${PullMeleeStick} && !${PullAggroTargetID}) /stick ${MoveTo.ArrivalDist} front
                }
            }
            /squelch /attack on
            /delay 10
            /if (${PullAggroTargetID} || ${Target.PctHPs}<100) /break
        }
        | Turn off combat so puller returns to camp. You mean run like hell.
        /if (${Select[${Role},puller,pullertank,pullerpettank,hunterpettank]}) {
           /squelch /attack off
            /if (${Stick.Active}) /stick off
            /squelch /target clear
            | Turn on mq2melee function back on after pull
            /if (${Select[${Role},puller,pullertank]} && ${UseMQ2Melee}) /squelch /melee melee=1
        }
        /varset Pulled 1
        /varset ToClose 0
        DEBUGPULL PullWithMelee: Leave
    /return
|-------------------------------------------------------------------------------------
| SUB: Pull With Ranged
|-------------------------------------------------------------------------------------
    Sub PullWithRanged(int PullDistRanged,int RSwitch)
        DEBUGPULL PullWithRanged: Enter
        /declare TryCount int local 0
        /declare AmmoCount int local 0
        DEBUGPULL PullWithRanged: Ranged
        | Cursor check sometimes summoned items get stuck on cursor.
        /if (${Cursor.ID}) /call CheckCursor 0
        /if (${RSwitch}==0) {
            /if (${Me.Inventory[ranged].Name.NotEqual[${PullItem}]}) {
                /exchange "${PullItem}" ranged
                /varset RangedSwitch 1
            }
            /if (${Me.Inventory[ammo].Name.NotEqual[${PullAmmo}]}) {
                /exchange "${PullAmmo}" ammo
                /if (${TempAmmo.NotEqual[${PullAmmo}]}) /varset AmmoSwitch 1
            }
        } else /if (${RSwitch}==1) {
            /while (1) {
                /varcalc TryCount ${TryCount}+1
                /while (1) {
                    /varset EventFlag 0
                    /doevents
                    /if (!${EventFlag}) /break
                }
                /if (${DPSPaused}) /return
                /if (${PullAggroTargetID}) {
                    /varset Pulled 1
                    /return
                }
                /if (${CantSee}) {
                    /if (${Target.ID}) /squelch /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
                    /delay 10
                    /varset CantSee 0
                    DEBUGPULL Could Not see Target. Trying Again.
                }
                | Mod for puller to turn back to camp after /range this saves on the puller turning AFTER mob is aggroed and turns facing camp while waiting for mob to aggro.
                /if (${Me.Combat}) {
                    /squelch /Attack off
                    /delay 20 !${Me.Combat}
                }
                /if (${Stick.Active}) /stick off
                /if (${Target.ID}) /squelch /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
                /look 0
                /varset AmmoCount ${FindItemCount[=${PullAmmo}]}
                /delay 30 ${Me.Heading.ShortName.Equal[${Target.HeadingTo}]}
                /if (${Target.ID}==${MyTargetID} && ${Target.Distance3D}>=30) {
                    | This is where we would call the Pacification code.
                    |/echo Distance From Target: ${Target.Distance3D} ${PullDistRanged}
                    /while (${AmmoCount}==${FindItemCount[=${PullAmmo}]} && !${PullAggroTargetID} && ${Target.Distance3D}>=30 && ${Target.LineOfSight} && ${Target.FeetWet}==${Me.FeetWet}) {
                        /range
                        /doevents TooClose
                        /if (${ToClose}) {
                            /varset PullDistRanged 15
                            /return ${PullDistRanged}
                        }
                        /doevents CantHit
                        /if (${CantHit}) {
                            /varset CantHit 0
                            /varcalc PullDistRanged ${PullDistRanged}*.8
                            /return ${PullDistRanged}
                        }
                        /doevents TooFar
                        /if (${PullTooFar}) {
                            /varset PullTooFar 0
                            /varcalc PullDistRanged ${PullDistRanged}*.8
                            /return ${PullDistRanged}
                        }
                    }
                }
                /if (${Math.Distance[${CampYLoc},${CampXLoc}]} > ${CampRadius}) {
                    /delay 10 ${PullAggroTargetID}
                    /if (${Target.ID}) /squelch /face ${If[${FaceMobOn}==2,nolook,fast nolook]} loc ${CampYLoc},${CampXLoc}
                }
                /if (!${PullAggroTargetID}) {
                    /delay ${Math.Calc[1+${Target.Distance}/50].Int}s ${PullAggroTargetID}
                    /varcalc PullTimer ${PullTimer}+10
                }
                DEBUGPULL Pull: ${PullTimer} !${PullAggroTargetID} ${Target.PctHPs}==100
                /if (!${PullTimer} || ${TryCount}>2 || ${PullTooFar} || ${PullAggroTargetID}) /break
            }
            /if (${PullAggroTargetID}) {
                /varset Pulled 1
                /return
            }
        } else /if (${RSwitch}==2) {
            /if (${Cursor.ID}) /call CheckCursor PullWithRanged 0
            /if (${OrigRanged.NotEqual[null]} && ${Me.Inventory[ranged].Name.NotEqual[${OrigRanged}]}) {
                /exchange "${OrigRanged}" ranged
                /varset RangedSwitch 0
            }
            /if (${TempAmmo.NotEqual[null]} && ${Me.Inventory[ammo].Name.NotEqual[${TempAmmo}]}) {
                /exchange "${TempAmmo}" ammo
                /varset AmmoSwitch 0
            }
        }
        DEBUGPULL PullWithRanged: Leave
    /return
|-------------------------------------------------------------------------------------
| SUB: Pull With Casting
|-------------------------------------------------------------------------------------
    Sub PullWithCast(int PullDistCast)
        DEBUGPULL PullWithCast Enter
            DEBUGPULL PullWithCast: Casting to pull mob
            /if (${Me.Moving}) /call StopMoving
            /if (${Target.ID}) /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
            /look 0
            /while (1) {
                /if (${PullAggroTargetID}) {
                    /varset Pulled 1
                    /return
                }
                /if (!${Spawn[${MyTargetID}].LineOfSight}) {
                    /varcalc PullDistCast ${PullDistCast}*.8
                    /return ${PullDistCast}
                }
                /if (!${Me.Moving}) /call CastWhat "${PullWith}" ${Target.ID} Pull 0 0
                /if (${Macro.Return.Equal[CAST_FIZZLE]}) /continue
                /if (${Macro.Return.Equal[CAST_SUCCESS]} || ${Macro.Return.Equal[CAST_RESISTED]} || ${PullAggroTargetID}) /varset Pulled 1
                /delay 10 ${PullAggroTargetID}
                /break
            }
        DEBUGPULL PullWithCast: Leave
    /return
|-------------------------------------------------------------------------------------
| SUB: Pull With Pet Written by TreeHuginDruid for RedGuides
|-------------------------------------------------------------------------------------
    Sub PullWithPet(int BeginMobID,float petPullDist)
        DEBUGPULL PullWithPet: Enter
        /declare int_petMoving    int local 0
        /declare int_triedSending int local 0
        /if (${Target.ID}) /face ${If[${FaceMobOn}==2,nolook,fast nolook]}
        |- Ensure we are in pull range and pet is following!
        /if (${Me.Pet.Stance.NotEqual[FOLLOW]}) /pet follow
        | - Send in pet if I don't have a mob in extended target
        /echo Pulling with PET now !
        /while (!${PullAggroTargetID}) {
            | Send in the Pet and monitor his movement.
            /if (!${int_petMoving}) /pet attack
            /delay 10 ${PullAggroTargetID}
            /if (${PullAggroTargetID}) {
                /break
            } else {
                | Is the pet moving?
                /if (${Me.Pet.Moving}) {
                    /varset int_petMoving 1
                    /varset int_triedSending 0
                } else {
                    /varset int_petMoving 0
                    /varcalc int_triedSending ${int_triedSending}+1
                }
            }
            | Fix for HunterPetTank
            /if (!${Spawn[${MyTargetID}].ID} || ${Spawn[${MyTargetID}].Type.Equal[corpse]}) /break
            /if (${Spawn[${BeginMobID}].Distance3D}>${petPullDist}) /break
            /if (${int_triedSending}>2) /break
        }
        | - If I have a extended target, flag as pulled.
        /if (${PullAggroTargetID}) {
            /varset Pulled 1
        }
        /if (${PetHoldOn}) /pet ${PetHold} on
        /pet back off
        DEBUGPULL PullWithPet: Leave
    /return
|-------------------------------------------------------------------------------------
| SUB: Pull Using MQ2Nav || (${Math.Distance[${CampYLoc},${CampXLoc}]}>=${PullNavDistance}) fix spwn dist from camp not mine.
| -------------------------------------------------------------------------------------
    Sub PullUsingNav(int BeginMobNavID,float NavPullDist)
        DEBUGPULL PullUsingNav Enter Pulling ${Spawn[${BeginMobNavID}].Name} ${NavPullDist} ${Target.Distance}
        /varset PullTimer 30
        /declare PullNavTimer timer local 3m
        /varset PullNavDistance ${Navigation.PathLength[id ${BeginMobNavID}]}
        /nav id ${BeginMobNavID}
        /while (1) {
            /while (1) {
                /varset EventFlag 0
                /doevents
                /if (!${EventFlag}) /break
            }
            | Check if puller has corpse from dying on previous pull along path
            /if (${CorpseRecoveryOn}) {
                /if (!${DragCorpse} && ${SpawnCount[pccorpse ${Me} radius 89]}) {
                    /call GrabCorpse 2
                    /if (${DragCorpse}) {
                        DEBUGPULL PullUsingNav Leave found my corpse
                        /return dcbtc
                    }
                }
            }
            |Internal loop delay 1 second. No reason to loop faster than 1 second. Unless you have Bard speed on. Zooooooooooommm
            /delay 4
            | Corrects heading when mob is moving and your using Navigation.
            /if (${Spawn[${BeginMobNavID}].Speed}>25) {
                /if ((${Role.Equal[hunter]} || ${PullWith.Equal[melee]}) && ${Spawn[id ${BeginMobNavID}].LineOfSight}) {
                    /if (!${Stick.Active} && ${Target.Distance}<20) {
                        /if (${Navigation.Active}) /nav stop
                        /moveto id ${Target.ID} mdist ${NavPullDist}
                        /stick ${NavPullDist} id ${Target.ID}
                        /return 0
                    }
                }
            }
            | Check for Unexpected Aggro
            /if (!${ChainPull}) {
                /if (${AggroTargetID}) {
                    /if (${Navigation.Active}) /nav stop
                    /return 0
                }
            } else {
                /if (${Me.XTarget[${XTSlot2}].ID}) /return 0
                /if (${Me.XTarget[${XTSlot}].ID} && ${Me.XTarget[${XTSlot}].ID}!=${BeginMobID}) /return 0
            }
            | Check for being to far from camp lose MyTargetID or path
            /if (${Math.Distance[${CampYLoc},${CampXLoc}]}>=${Math.Calc[${MaxRadius}*.95]} || ${PullNavTimer}==0 || !${Spawn[${BeginMobNavID}].ID} || ${Navigation.PathLength[id ${BeginMobNavID}]}<0.01) {
                /echo We have exceeded max pulling radius or timer or lost path to mob. Returning to camp.
                /if (${Navigation.Active}) /nav stop
                /if (${Math.Distance[${Spawn[${BeginMobNavID}].Y},${Spawn[${BeginMobNavID}].X}:${CampYLoc},${CampXLoc}]}>=${Math.Calc[${MaxRadius}*.95]}) {
                    /call PullIgnoreCheck ${BeginMobNavID} a PullUsingNav
                }
                DEBUGPULL PullUsingNav Leave Max radius/no timer/no mob/nopath  (${Math.Distance[${CampYLoc},${CampXLoc}]}>=${Math.Calc[${MaxRadius}*.95]} && ${Spawn[${BeginMobNavID}].Speed}>25) || (${Math.Distance[${CampYLoc},${CampXLoc}]}>=${PullNavDistance}) || ${PullNavTimer}==0 || !${Spawn[${BeginMobNavID}].ID} || ${Navigation.PathLength[id ${BeginMobNavID}]}<0.01
                /if (${Select[${Role},hunter,hunterpettank]}) {
                    /call PullReset
                    /return huntpr
                } else {
                    /return btcr
                }
            }
            | Keep timer from main Pull sub alive
            /if (${Me.Moving} && ${Navigation.Active}) /varset PullTimer 30
            /if (${Spawn[${BeginMobNavID}].Distance3D}>${NavPullDist} || !${Spawn[${BeginMobNavID}].LineOfSight} || ${Spawn[${BeginMobNavID}].FeetWet}!=${Me.FeetWet}) {
                /if (${PullWith.NotEqual[pet]} || ${Spawn[${BeginMobNavID}].Distance3D}>${NavPullDist}) {
                    /if (!${ChainPull} && !${AggroTargetID}) /continue
                    /if (${ChainPull}) {
                        /if (!${Me.XTarget[${XTSlot2}].ID}) {
                            /if (!${Me.XTarget[${XTSlot}].ID}) /continue
                            /if (${Me.XTarget[${XTSlot}].ID}==${BeginMobID}) /continue
                            /if (${Me.XTarget[${XTSlot}].ID}!=${BeginMobNavID}) /continue
                        }
                    }
                }
            }
            /if (${Navigation.Active}) /nav stop
            /if (${PullWith.NotEqual[pet]} && !${Spawn[${BeginMobNavID}].LineOfSight} && ${Target.Distance}>20) {
                /squelch /nav id ${BeginMobNavID}
                /continue
            }
            /break
        }
        DEBUGPULL PullUsingNav Leave
    /return 0
|-------------------------------------------------------------------------------------
| SUB: Pull Using AdvPath
| -------------------------------------------------------------------------------------
    Sub PullUsingAdvPath(int BeginMobAPID,float APPullDist)
        /declare ReturnStat string local 0
        DEBUGPULL PullUsingAdvPath Enter
        /if (${MoveTo.Moving}) /moveto off
        /play ${PullPath} nodoor smart normal
        DEBUGPULL PullUsingAdvPath /play ${PullPath} nodoor smart normal
        /while (1) {
            /if (${DebugPull}) /delay 5
            DEBUGPULL PullUsingAdvPath /echo Waypoint: ${AdvPath.NextWaypoint} Target: ${Target.ID} MyTargetID: ${MyTargetID} Aggro: ${AggroTargetID} MobToWPDist: ${Math.Distance[${Spawn[id ${MyTargetID}].Y},${Spawn[id ${MyTargetID}].X}:${AdvpathPointY},${AdvpathPointX}]}<${APPullDist} MeToWPDist: ${Math.Distance[${Me.Y},${Me.X}:${AdvpathPointY},${AdvpathPointX}]}>50 WP<NWP: ${AdvPath.NextWaypoint}>=${AdvpathPointNum}
            | Do we have aggro?
            /if (${AggroTargetID}) /break
            | Has the mob move out of range from the original way point
            /if (${Math.Distance[${Spawn[id ${MyTargetID}].Y},${Spawn[id ${MyTargetID}].X}:${AdvpathPointY},${AdvpathPointX}]}>${APPullDist}) /return btcr
            | Is mob within pull range and LOS
            /if (${Math.Distance[${Spawn[id ${MyTargetID}].Y},${Spawn[id ${MyTargetID}].X}:${Me.Y},${Me.X}]}<${APPullDist} && ${Spawn[${MyTargetID}].LineOfSight} && ${Target.FeetWet}==${Me.FeetWet}) /break
            | Stop at our waypoint
            /if (${AdvPath.NextWaypoint}>=${AdvpathPointNum} && ${Math.Distance[${Me.Y},${Me.X}:${AdvpathPointY},${AdvpathPointX}]}<=5) /return btcr
        }
        /play off
        DEBUGPULL PullUsingAdvPath /play ${PullPath} nodoor smart normal
        DEBUGPULL PullUsingAdvPath Leave ${AdvPath.NextWaypoint} ${ReturnStat}
    /return ${ReturnStat}
| -------------------------------------------------------------------------------------
| SUB: Wait for Mob
| -------------------------------------------------------------------------------------
    Sub WaitForMob
        /declare MobsInCamp int local 0
        /if (${Select[${Role},hunter,hunterpettank]} || ${DPSPaused}) /return
        DEBUGPULL WaitForMob Enter
        /varset WaitTimer 45s
        /if (${Pulled}) {
            DEBUGPULL WaitForMob Waiting for mob
            /if (${ChainPull}) /varset LastMobPullID ${Target.ID}
            /if (${Me.Inventory[ranged].Name.NotEqual[${OrigRanged}]} && ${OrigRanged.NotEqual[null]}) {
                /exchange "${OrigRanged}" ranged
            }
            | When you target a mob and another mob aggros you, but your target never gets aggroed.
            /if (${PullAggroTargetID} && ${Target.ID}!=${PullAggroTargetID} && ${Target.AggroHolder.ID}==0) {
                /target id ${AggroTargetID}
            }
            /while (1) {
                /if (${FaceMobOn} && ${Target.ID} && (${Me.Standing} || ${Me.Mount.ID})) {
                    /if (${FaceMobOn}==1) {
                        /face fast nolook
                    } else /if (${FaceMobOn}==2) {
                        /face nolook
                    }
                }
                /while (1) {
                    /varset EventFlag 0
                    /doevents
                    /if (!${EventFlag}) /break
                }
                | Did the GotHit event trigger, if it did MyTargetID will be zero.
                /if (!${MyTargetID}) {
                    /call CombatTargetCheck
                    /if (!${MyTargetID}) /varset MyTargetID ${AggroTargetID}
                }
                /call MobRadar los ${CampRadius} WaitForMob
                | If NOT chain pulling and multiple mobs in camp. /return
                /if (${MobCount}>=2 && !${ChainPull}) {
                    /call PullReset
                    /return
                }
                DEBUGPULL - WaitTimer: ${WaitTimer} Target Distance: ${Target.Distance} Mob Dist from Camp: ${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X}:${CampYLoc},${CampXLoc}]}>=${CampRadius} Mob Dist from tank: ${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X}:${Me.Y},${Me.X}]}>=20
                | if target lost somehow add to ignore list and continue pulls
                /if (!${AggroTargetID} || ${WaitTimer}==0) {
                    DEBUGPULL WaitForMob return no AggroTargetID
                    /if (${Target.ID}) /call PullIgnoreCheck ${Target.ID} a waitformob
                    /call PullReset
                    /return
                }
                /delay 5
                /varset MobsInCamp ${SpawnCount[xtarhater loc ${CampXLoc} ${CampYLoc} radius ${MeleeDistance} zradius ${MaxZRange}]}
                /if (${Select[${Role},pullertank]}) {
                    | Wait in camp for mob if timer active and mob is outside of camp radius and mob is farther away from tank than 20 feet
                    /if (${WaitTimer} && !${MobsInCamp}) /continue
                } else /if (${Select[${Role},puller]}) {
                    | If I am PULLER and NOT chain pulling
                    /if (!${ChainPull}) {
                        /if (!${MobsInCamp} && !${SpawnCount[xtarhater loc ${Spawn[=${MainAssist}].X} ${Spawn[=${MainAssist}].Y} radius 20 zradius ${MaxZRange}]}) /continue
                        /if (${MercOn} && ${MercAssisting}!=${MyTargetID} && ${Mercenary.State.Equal[Active]}) /call MercsDoWhat
                    } else {
                        | If I am PULLER and chain pulling
                        | Leave if multi mobs or no mobs
                        /if (${MobsInCamp}>1 || !${MyTargetID} || (${Me.XTarget[${XTSlot}].ID} && ${Me.XTarget[${XTSlot2}].ID})) {
                            /call PullReset
                            /return
                        }
                        /if (${Math.Distance[${Spawn[${MyTargetID}].Y},${Spawn[${MyTargetID}].X}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}>20 && ${Target.ID}==${Spawn[${MyTargetID}].ID} && ${Me.TargetOfTarget.ID}==${Me.ID}) /continue
                    }
                } else /if (${Select[${Role},pullerpettank]}) {
                    | I am PULLER with PET TANK
                    |- Make sure pet is returning to camp with me.
                    /if (${Me.Pet.ID} && ${Math.Distance[${Me.Pet.Y},${Me.Pet.X}:${CampYLoc},${CampXLoc}]}>${CampRadius} && ${Math.Distance[${Me.Pet.Y},${Me.Pet.X}:${Me.Y},${Me.X}]}>20) {
                        /pet back off
                        /delay 5
                        /pet follow
                    }
                    | If the mob is within pet attack distance from camp send the pet in to attack
                    /if (!${SpawnCount[xtarhater targetable loc ${CampXLoc} ${CampYLoc} radius ${PetAttackRange} zradius ${MaxZRange}]}) {
                        /continue
                    } else {
                        /varset MyTargetID ${NearestSpawn[1,xtarhater targetable loc ${CampXLoc} ${CampYLoc} radius ${PetAttackRange} zradius ${MaxZRange}].ID}
                    }
                    |/if (!${PetAttack}) /call CombatPet
                    /if (!${Pet.Combat}) /call CombatPet
                }
                /break
            }
        }
        /if (${Group}==1 && ${Select[${Role},puller]} && ${Group.Puller.Name.Equal[${Me}]} && ${Spawn[=${MainAssist}].Type.Equal[Mercenary]} && ${Spawn[${MyTargetID}].Distance}<=${MeleeDistance}) /call AssignGroupRole unset "${Me.CleanName}" 3
        /if (${MyTargetID} && !${PullAggroTargetID}) /call PullReset
        /if (${IAmABard}) /call DoBardStuff
        /varset WaitTimer 0
        /call MercsDoWhat
        DEBUGPULL WaitForMob Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: Reset Pull variables
| -------------------------------------------------------------------------------------
    Sub PullReset
        DEBUGPULL PullReset Enter
        /moveto dist 10
        /varset Pulling 0
        /varset Pulled 0
        /varset MyTargetID 0
        /varset MyTargetName
        /varset ToClose 0
        /attack off
        /if (${MoveTo.Moving}) /moveto off
        /if (${PullMoveUse.Equal[nav]}) {
            /if (${Navigation.Active}) /nav stop
        }
        /varset WaitTimer 0
        /varset DragCorpse 0
        /squelch /target clear
        DEBUGPULL PullReset Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: PullModeToggle
| -------------------------------------------------------------------------------------
    Sub PullModeToggle(ToggleState)
        /if (!${Select[${Me},${Group.Leader}]} || !${SpawnCount[group mercenary]}) /return
        /if (${ToggleState.Equal[TurnOn]}) {
            /while (${Target.Distance}>${CampRadius} && ${Group.Puller.ID}!=${Me.ID}) {
                /if (${Group.Puller.ID}!=${Me.ID}) /grouproles set ${Me.CleanName} 3
                /delay 10 ${Group.Puller.ID}==${Me.ID}
                /if (${Group.Puller.ID}==${Me.ID}) /echo + You have been set to be group puller.
            }
        } else /if (${ToggleState.Equal[TurnOff]}) {
            /while (${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}>${CampRadius} || ${Group.Puller.ID}==${Me.ID}) {
                /if (${Group.Puller.ID}==${Me.ID} && ${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}<=${CampRadius}) {
                    DEBUGN KICKIT! (${Group.Puller.ID}==${Me.ID} && ${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}<=${CampRadius})
                    /if (${Group.Puller.ID}==${Me.ID}) /grouproles unset ${Me.CleanName} 3
                } else {
                    /if (${Group.Puller.ID}==${Me.ID} && ${Math.Distance[${Me.Y},${Me.X}:${CampYLoc},${CampXLoc}]}>${CampRadius}) /moveto loc ${CampYLoc} ${CampXLoc} mdist 10
                }
                /delay 10 ${Group.Puller.ID}!=${Me.ID}
                /if (${Group.Puller.ID}!=${Me.ID}) /echo + You are no longer group puller.
            }
        }
    /return
| -------------------------------------------------------------------------------------
| SUB: FindMobOnReturn
| -------------------------------------------------------------------------------------
    Sub FindMobOnReturn
        DEBUGPULL FindMobOnReturn Enter ${Role} - ${PullOnReturn} - ${StayPut}
        /while (1) {
            /varset PullMob 0
            /if (${Role.Find[hunter]}) {
                /if (${PullOnReturn}) /varset CheckOnReturn 1
                /call DoWeMove 1
            } else {
                /call DoWeMove 0
            }
            /if (!${PullMob}) {
                /break
            } else {
                /call PullCheck
                /if (${Math.Distance[${CampYLoc},${CampXLoc}]} < 16) /break
                /if (${Role.Find[hunter]}) /break
            }
        }
        DEBUGPULL FindMobOnReturn Leave
    /return
| -------------------------------------------------------------------------------------
| SUB: GroupWatch
| -------------------------------------------------------------------------------------
    Sub GroupWatch
        /if (!${GroupWatchOn} ) /return
        /if (${AggroTargetID} && !${ChainPull}) /return
        DEBUGN GroupWatch Enter
        /declare i              int local
        /declare GrpMemType     string local
        /declare GrpMemName     string local
        /declare GrpMemClass    string local
        /declare GrpMemStat     string local
        /declare GrpWatchCondNo int local
        /declare GrpMedOn       int local
        /if (!${Spawn[=${MainAssist}].ID} && !${Select[${Role},tank,pullertank,pettank,pullerpettank,hunter,hunterpettank]} && ${CampZone}==${Zone.ID}) {
            /echo I am not detecting Main Assist. Pausing.
            /while (1) {
                /doevents
                /if (${DPSOn} || ${MeleeOn}) {
                    /call CheckForCombat 0 GroupWatch1 0
                } else {
                    /call CheckForCombat 1 GroupWatch2 0
                }
                /call WaitSubs
                /if (${Spawn[=${MainAssist}].ID}) {
                    /echo Main Assist is back resuming action.
                    /break
                }
                /delay 10
            }
        }
        /if (${PullHold}==1) /varset PullHold 0
        /for i 1 to 5
            /if (!${Group.Member[${i}].ID} || ${Group.Member[${i}].Type.Equal[corpse]}) {
                /if (${ChainPull} && ${Group.Member[${i}].Type.Equal[corpse]}) /varset PullHold 1
                /continue
            }
            /if (${GroupWatchOn}==2 && !${Select[${Group.Member[${i}].Class.ShortName},CLR,DRU,SHM]}) /continue
            /varset GrpMemType ${Group.Member[${i}].Type}
            /varset GrpMemName ${Group.Member[${i}].CleanName}
            /varset GrpMemClass ${Group.Member[${i}].Class.ShortName}
            /if (${Select[${GrpMemClass},BER,MNK,ROG,WAR]}) {
                /varset GrpMemStat Endurance
            } else {
                /varset GrpMemStat Mana
            }
            /varset GrpMedOn ${MedOn}
            /if (!${MedOn}) /varset MedOn 1
            DEBUGN ${i} CheckStats 1 "${GrpMemName}" "${GrpMemClass}" "${GrpMemType}" ${GrpMemStat} ${GroupWatchPct} 90
            /call CheckStats 1 "${GrpMemName}" ${GrpMemClass} ${GrpMemType} ${GrpMemStat} ${GroupWatchPct} 90
            /if (${Select[${GrpMemClass},BST,PAL,RNG,SHD]}) /call CheckStats 1 "${GrpMemName}" "${GrpMemClass}" "${GrpMemType}" Endurance ${GroupWatchPct} 90
            /if (!${GrpMedOn}) /varset MedOn 0
        /next i
        /if (${GroupWatchCheck.Find[cond]}) {
            /varset GrpWatchCondNo ${GroupWatchCheck.Mid[${Math.Calc[${GroupWatchCheck.Find[cond]}+4]},3]}
        } else {
            /varset GrpWatchCondNo 0
        }
        /if (${ConOn} && ${GrpWatchCondNo} && ${If[${Cond[${GrpWatchCondNo}]},1,0]}) {
            /echo Pausing due to GroupWatchCheck..
            /while (1) {
                /if (${If[${Cond[${GrpWatchCondNo}]},0,1]}) /break
                /delay 10
                DOPARSE
                /if (!${MedCombat} && ${AggroTargetID}) /break
            }
            /echo Continuing from GroupWatchCheck condition.
        }
        DEBUGN GroupWatch Leave
    /return
|-----------------------------------------------------------------------------
| SUB: SetPullAngle
| ----------------------------------------------------------------------------
    Sub Bind_SetPullArc(float AWidth, string FDir)
        /declare DirDegree string local 0,45,90,135,180,225,270,315
        /declare FDirTemp  string local ${FDir}
        /if (!${AWidth}) {
            /if (${PullArcWidth}) {
                /echo Turning off Directional Pulling.
                /varset PullArcWidth 0
            }
            /return
        } else {
            /varset PullArcWidth ${AWidth}
        }
        /if (!${Bool[${FDir}]}) {
            /varset FDirTemp ${Me.Heading.Degrees}
        } else /if (${Float[${FDir}]}>0) {
            /varset FDirTemp ${FDir}
        } else {
            /varset FDirTemp ${DirDegree.Arg[${Select[${FDir},n,ne,e,se,s,sw,w,nw]},,]}.00
            /if (!${FDirTemp} && ${FDir.NotEqual[n]}) {
                /echo Invalid Direction. Turning off Directional Pulling.
                /varset PullArcWidth 0
                /return
            }
        }
        /call SetPullAngles ${Float[${FDirTemp}]} ${AWidth} 0
    /return
|-----------------------------------------------------------------------------
| SUB: SetPullAngles
| ----------------------------------------------------------------------------
    Sub SetPullAngles(float FDir, float AWidth, int IgnoreMessage)
        /if (!${AWidth}) /return
        /if (!${FDir}) /varset FDir 0.00
        | Figure the Degrees for the left side
        /if (${Math.Calc[${FDir}-(${AWidth}*.5)]}<0) {
            /varcalc PullLSide 360-((${AWidth}*.5)-${FDir})
        } else {
            /varcalc PullLSide ${FDir}-(${AWidth}*.5)
        }
        |Figure the Degrees for the right side
        /if (${Math.Calc[${FDir}+(${AWidth}*.5)]}>360) {
            /varcalc PullRSide ((${AWidth}*.5)+${FDir}-360)
        } else {
            /varcalc PullRSide ${FDir}+(${AWidth}*.5)
        }
        |Store your current heading
        /varset PullHeading ${FDir}
        /if (!${IgnoreMessage}) /echo Setting Pull Angles. Facing: ${FDir} Left Side: ${PullLSide} Right Side: ${PullRSide} Width: ${AWidth}
    /return
|-----------------------------------------------------------------------------
| SUB: FigureAngles
| ----------------------------------------------------------------------------
    Sub FigureMobAngle(int aMobID)
        /if (!${aMobID}) /return 0
        /declare DirectionToMob float local 0
        /varset DirectionToMob ${Spawn[${aMobID}].HeadingTo[${CampYLoc},${CampXLoc}].Degrees}
        /if (${PullLSide}>=${PullRSide}) {
            /if (${DirectionToMob}<${PullLSide} && ${DirectionToMob}>${PullRSide}) /return 0
        } else {
            /if (${DirectionToMob}<${PullLSide} || ${DirectionToMob}>${PullRSide}) /return 0
        }
    /return 1
|-----------------------------------------------------------------------------
| SUB: FeignAggroCheck
| ----------------------------------------------------------------------------
    Sub FeignAggroCheck
        /if (${AggroOffTimer}) {
            /while (${Me.Feigning} || ${Me.Invis}) {
                /doevents Timer
                /delay 5
            }
        } else {
            /doevents Timer
        }
    /return
| ----------------------------------------------------------------------------
| SUB: ChainPullNextMob
| ----------------------------------------------------------------------------
    Sub ChainPullNextMob(int EchoWaitMessage)
        /declare IsNamed1 bool local False
        /if (${Math.Distance[${CampYLoc},${CampXLoc}:${Spawn[=${MainAssist}].Y},${Spawn[=${MainAssist}].X}]}<=75) {
            /call IsSpawnNamed ${Target.ID}
            /varset IsNamed1 ${Macro.Return}
            /if (!${PullHold} && ${MobCount}<2 && !${Me.XTarget[${XTSlot2}].ID} && ${If[${IsNamed1},0,1]}==1) {
                /if (${Target.ID} && ${Target.PctHPs} < ${ChainPullHP}) {
                    /call FindMobToPull 0 1 0
                    /if (${Macro.Return}>0 && (!${PullXPCheck} || (${Spawn[${ChainPullTemp}].Distance}<${Math.Calc[${PullRange}+400]} && ${PullXPCheck})) && ${Me.TargetOfTarget.Name.NotEqual[${Me.CleanName}]} && ${Me.PctAggro}<50) {
                        /varset PullMob ${Macro.Return}
                        /if (${UseMQ2Melee}) /squelch /melee melee=0
                        /if (${Stick.Active}) /squelch /stick off
                        /squelch /attack off
                        /delay 10
                        /if (${UseMQ2Melee}) /squelch /melee melee=1
                        /varset ChainPull 2
                        /varset MyTargetID 0
                        /varset Attacking 0
                        /varset CombatStart 0
                        /return 1
                    } else {
                        DEBUGCHAINPULL Chain Pull Failed 3: ${Macro.Return} ${PullXPCheck} ${Spawn[${ChainPullTemp}].Distance}<${Math.Calc[${PullRange}+400]} ${Me.TargetOfTarget.Name} ${Me.PctAggro}
                    }
                } else {
                    DEBUGCHAINPULL Chain Pull Failed 2: ${Target.ID} ${Target.PctHPs}<${ChainPullHP}
                }
            } else {
                DEBUGCHAINPULL Chain Pull Failed 1: ${PullHold} ${MobCount}<2 ${Me.XTarget[${XTSlot2}].ID} ${IsNamed1}
            }
        } else /if (${EchoWaitMessage}) {
            /call BroadCast r "Holding Pulls. Tank to far from camp."
            /return W
        }
    /return 0
