| ----------------------------------------------------------------------------
| SUB: Assign single heals - seperate single heals from group heals
| ---------------------------------------------------------------------------
    Sub FindSingleHeals
        DEBUGN Enter FindSingleHeals
        /declare i int local
        /declare j int local
        /declare SHCount int 0
        /declare HealHighPoint int  local 0
        /declare HealHighestPoint int  local 0
        /declare HealHighestPointMA int  local 0
        /declare HealPointTag string  local 0
        /declare HealSpell string  local
        /declare HealSpellRange int local 0
        /for i 1 to ${Heals.Size}
            /varset HealSpell ${Heals[${i}].Arg[1,|]}
            /varset HealPointTag ${Heals[${i}].Arg[3,|]}
            /if (${HealPointTag.Equal[MA]}) {
                /varset HealHighestPointMA ${Heals[${i}].Arg[2,|]}
            } else {
                /varset HealHighPoint ${Heals[${i}].Arg[2,|]}
            }
            /varset HealSpellRange ${Int[${Spell[${HealSpell}].Range}]}
            DEBUGN ${i} ${HealSpell} ${Spell[${HealSpell}].TargetType} ${Select[${Spell[${HealSpell}].TargetType},Single,Self]}
            /if (${HealSpell.Find[Aegis of Superior Divinity]} || ${HealSpell.Find[Harmony of the Soul]} || ${HealSpell.Find[Burst of Life]} || ${HealSpell.Find[Focused Celestial Regeneration]}) {
                /varcalc SHCount ${SHCount}+1
                /varset SingleHeal[${SHCount}] ${Heals[${i}]}
            } else /if (${Select[${Me.Class.Name},Druid,Shaman]} && (${HealSpell.Find[Intervention]} || ${HealSpell.Find[Survival]})) {
                /varcalc SHCount ${SHCount}+1
                /varset SingleHeal[${SHCount}] ${Heals[${i}]}
            } else /if (${Select[${Spell[${HealSpell}].TargetType},Single,Self]} || ${Spell[${FindItem[=${HealSpell}].Spell}].TargetType.Find[single]} || ${Spell[${FindItem[=${HealSpell}].Spell}].TargetType.Find[self]} || ${Heals[${i}].Arg[3,|].Find[tap]} || ${Heals[${i}].Arg[3,|].Find[pet]} || ${Spell[${HealSpell}].TargetType.Find[Targeted AE]} && ${Select[${Heals[${i}].Arg[3,|]},MA,ME,pet]}) {
                /varcalc SHCount ${SHCount}+1
                /varset SingleHeal[${SHCount}] ${Heals[${i}]}
            } else /if (${Spell[${HealSpell}].TargetType.Equal[Free Target]}) {
                /varcalc SHCount ${SHCount}+1
                /varset SingleHeal[${SHCount}] ${Heals[${i}]}
            }
            /if (${HealPointTag.NotEqual[MA]} && ${HealHighPoint}>${SingleHealPoint}) /varset SingleHealPoint ${HealHighPoint}
            /if (${HealPointTag.Equal[MA]} && ${HealHighestPointMA}>${SingleHealPointMA}) /varset SingleHealPointMA ${HealHighestPointMA}
            /if (${HealSpellRange}>${SingleHealPointRange}) /varset SingleHealPointRange ${HealSpellRange}
            DEBUGN ${SingleHealPoint} ${SingleHealPointRange}
        /next i
        /if (${SHCount}) /call SortArray SingleHeal 100|100 >
        /if (!${SingleHealPoint}) /varset SingleHealPoint 99
        /if (!${SingleHealPointMA}) /varset SingleHealPointMA ${SingleHealPoint}
        /if (!${SingleHealPointRange}) /varset SingleHealPointRange 200
        /if (${Debug}) {
        /for j 1 to ${SingleHeal.Size}
            DEBUGN ${j} ${SingleHeal[${j}]} ${Spell[${SingleHeal[${j}].Arg[1,|]}].CastTime.Float} ${Spell[${SingleHeal[${j}].Arg[1,|]}].Duration}
        /next j
        }
        DEBUGN Leave FindSingleHeals
    /return
| ----------------------------------------------------------------------------
| SUB: Assign Group heals
| ---------------------------------------------------------------------------
    Sub FindGroupHeals
        /if (!${Select[${Me.Class.ShortName},BST,CLR,SHM,DRU,PAL]}) /return
        DEBUGN Enter FindGroupHeals
        /declare i int local
        /declare j int local
        /declare GHCount int 0
        /declare HealSpell string  local
        /for i 1 to ${Heals.Size}
            /varset HealSpell ${Heals[${i}].Arg[1,|]}
            DEBUGN ${HealSpell}
            /if (!${HealSpell.Length} || ${HealSpell.Equal[null]} || ${Select[${Spell[${HealSpell}].TargetType},Single,Self]} || ${Spell[${FindItem[=${HealSpell}].Spell}].TargetType.Find[self]} || ${HealSpell.Find[Aegis of Superior Divinity]} || ${HealSpell.Find[Harmony of the Soul]}) /continue
            /if (${HealSpell.Find[Burst of Life]} || ${HealSpell.Find[Focused Celestial Regeneration]}) /continue
            /if (${Me.Class.Name.Equal[Shaman]} && ${Spell[${HealSpell}].Name.Find[Intervention]} || ${Me.Class.Name.Equal[Druid]} && ${Spell[${HealSpell}].Name.Find[Survival]}) /continue
            /if (${Spell[${HealSpell}].TargetType.Find[group v]} || ${Spell[${FindItem[=${HealSpell}].Spell}].TargetType.Find[group v]} || ${Spell[${HealSpell}].TargetType.Find[Targeted AE]} && !${Select[${Heals[${i}].Arg[3,|]},MA,ME]}) {
                /varcalc GHCount ${GHCount}+1
                /varset GroupHeal[${GHCount}] ${Heals[${i}]}
            }
        /next i
        DEBUGN ${GHCount}
        /if (${GHCount}) {
            /call SortArray GroupHeal 100|100 >
            /for j 1 to ${GroupHeal.Size}
                DEBUGN ${j} ${GroupHeal[${j}]} ${Spell[${GroupHeal[${j}].Arg[1,|]}].CastTime.Float} ${Spell[${GroupHeal[${j}].Arg[1,|]}].Duration}
            /next j
        }
        DEBUGN Leave FindGroupHeals
    /return
| ----------------------------------------------------------------------------
| SUB: Create Timers Heals
| ----------------------------------------------------------------------------
    Sub CreateTimersHeals
        | Declare timers for Heal duration spells
        /declare j int local
        /declare k int local
        /declare q int local
        /for j 1 to ${GroupHeal.Size}
            | Group Heal duration timers
            /if (!${Defined[SpellGH${j}]}) {
                /declare SpellGH${j}    timer   outer   0
            } else {
                /varset SpellGH${j} 0
            }
        /next j
        /for j 1 to ${SingleHeal.Size}
            /for k 0 to 13
                | Self and MA duration heal timers for  out of group including Pets.
                /if (!${Defined[Spell${j}GM${k}]}) {
                    /declare Spell${j}GM${k}     timer   outer   0
                } else {
                    /varset Spell${j}GM${k} 0
                }
                DEBUGN Group heal Timers Spell${j}GM${k} ${Spell${j}GM${k}}
            /next k
        /next j
       | Declare timers for Pet heals
        /for q 1 to ${SingleHeal.Size}
            /if (!${Defined[PetHealTimer${q}]}) {
                /declare PetHealTimer${q} timer outer 0
            } else {
                /varset PetHealTimer${q} 0
            }
        /next q
        /for j 1 to ${SingleHeal.Size}
            /for k 0 to ${XSlotTotal}
                | XTarget Timers.
                /if (!${Defined[Spell${j}XT${k}]}) {
                    /declare Spell${j}XT${k}     timer   outer   0
                } else {
                    /varset Spell${j}XT${k} 0
                }
                DEBUGN XTarget heal Timers Spell${j}XT${k} ${Spell${j}XT${k}}
            /next k
        /next j

    /return
| ----------------------------------------------------------------------------
| SUB: Create Timers Rez
| ----------------------------------------------------------------------------
    Sub CreateTimersRez
        | Declare timers for Rez duration spells
        /declare m int local
        /for m 1 to 5
            /if (!${Defined[BattleRezTimer${m}]}) {
            /declare BattleRezTimer${m} timer outer 0
        } else {
            /varset BattleRezTimer${m} 0
        }
        /next m
    /return
| ----------------------------------------------------------------------------
| SUB: CheckCures - Check Character first then cure list.
| ----------------------------------------------------------------------------
    Sub CheckCures(string sentFrom)
        /if (!${CuresOn}) /return
        /if (${Me.Invis} && !${AggroTargetID}) /return
        /if (${Medding} && ${MedCombat}) /return
        DEBUGHEALS CheckCures Enter ${sentFrom}
        /declare i int local
        /declare j int local
        /declare IniIDList string local
        /declare IniIDCount int local 0
        /declare IniDebuffCount string local 0
        /declare IniDebuffList string local
        /declare CuretoCast string local
        /declare CTID int local
        /declare CTName string
        /declare CondNo int local 0
        /declare C1stPart string
        /declare C2ndPart string
        /declare C3rdPart string
        /declare CureCast int local 0
        /if (${HealsOn}) {
            /if (!${LastHealCheck} || ${AggroTargetID}) {
                /call CheckHealth CheckCures
                /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
            }
        }
        |CuresOn 0=Off, 1=Everyone in zone, 2=Self Only, 3=Group Only.
        /if (${IAmMA} && ${TargetSwitchingOn} && ${Target.ID} && ${Target.ID}!=${MyTargetID} && ${CombatStart}) /call CombatTargetCheck 1
        | Check Buffs from ini
        /if (${CuresOn}==2) {
            /varset IniIDList ${Me.ID}
            /varset IniIDCount 1
        } else {
            /if (!${DanNetOn}) {
                /varset IniIDList ${Ini["KissAssist_Buffs.ini"]}
                /varcalc IniIDCount ${IniIDList.Count[|]}
            } else {
                /if (${Zone.ShortName.Find[_]}) {
                    /varset IniIDList ${DanNet.Peers[zone_${Zone.ShortName}]}
                } else {
                    /varset IniIDList ${DanNet.Peers[zone_${EverQuest.Server.Lower}_${Zone.ShortName}]}
                }
                /varset IniIDCount ${IniIDList.Count[|]}
            }
        }
        /if (!${IniIDCount}) /return
        /declare pIDList[${IniIDCount}] string local
        /for i 1 to ${IniIDCount}
            /varset pIDList[${i}] ${IniIDList.Arg[${i},|]}
        /next i

        | For each Character
        /for j 1 to ${IniIDCount}
            /if (!${DanNetOn} || ${CuresOn}==2) {
                /varset CTID ${Spawn[${pIDList[${j}]}].ID}
            } else {
                /varset CTID ${Int[${Spawn[=${pIDList[${j}]}].ID}]}
            }
            /if (!${CTID}) /continue
            /if (${Spawn[${CTID}].Type.Equal[Corpse]}) /continue
            /if (${Spawn[${CTID}].Distance}>100) /continue
            /varset CTName ${Spawn[id ${CTID}].CleanName}
            /if (${CuresOn}==3) {
                /if (${Group} && !${Group.Member[${CTName}].Index}) /continue
            }

            /varset CureCast 0
            | For each cure.
            /for i 1 to ${Cures.Size}
                /if (!${Cures[${i}].Length} || ${Cures[${i}].Equal[null]}) /continue
                /varset C1stPart ${Cures[${i}].Arg[1,|]}
                /if (${Cures[${i}].Arg[2,|].Length}==0 || ${Cures[${i}].Arg[2,|].Find[cond]}) {
                    /varset C2ndPart
                    /varset C3rdPart everyone
                } else /if (${Cures[${i}].Arg[2,|].Equal[me]}) {
                    /varset C2ndPart
                    /varset C3rdPart me
                } else {
                    /varset C2ndPart ${Cures[${i}].Arg[2,|]}
                    /if (${Cures[${i}].Arg[3,|].Length}==0 || ${Cures[${i}].Arg[3,|].Find[cond]}) {
                        /varset C3rdPart everyone
                    } else /if (${Cures[${i}].Arg[3,|].Equal[me]}) {
                        /varset C3rdPart me
                    } else {
                        /varset C3rdPart everyone
                    }
                }
                /if (${ConOn} && ${Cures[${i}].Find[|cond]}) {
                    /varset CondNo ${Cures[${i}].Mid[${Math.Calc[${Cures[${i}].Find[|cond]}+5]},3]}
                } else {
                    /varset CondNo 0
                }

                |CuresOn 1=Everyone, 3=Group Only
                /if (${C3rdPart.Equal[me]}) {
                    /if (${CTID}!=${Me.ID}) /continue
                } else /if (${C3rdPart.NotEqual[everyone]}) {
                    /continue
                }
                | If spell NOT ready then skip it.
                /if (!${Select[TRUE,${Me.SpellReady[${Spell[${C1stPart}].RankName}]},${Me.AltAbilityReady[${C1stPart}]},${Me.CombatAbilityReady[${Spell[${C1stPart}].RankName}]},${Me.ItemReady[${C1stPart}]}]}) {
                    /continue
                }
                /varset CuretoCast
                /varset IniDebuffCount 0
                /if (${CTID}!=${Me.ID}) {
                    /if (!${DanNetOn}) {
                        /varset IniDebuffCount ${Ini["KissAssist_Buffs.ini",${pIDList[${j}]},"Debuffs"]}
                        /if (!${Int[${IniDebuffCount.Arg[1,|]}]}) /break
                    } else {
                        /if (${MacroQuest.Parser}!=2) {
                            /squelch /engine parser 2 noauto
                            /while (${MacroQuest.Parser}!=2) {
                                /delay 5
                            }
                        }
                        /dquery ${pIDList[${j}]} -q ${Parse[0,"Math.Calc[${Int[${Me.Poisoned.ID}]}+${Int[${Me.Diseased.ID}]}+${Int[${Me.Cursed.ID}]}+${Int[${Me.Song[Restless Curse].ID}]}+${Int[${Me.Corrupted.ID}]}+${Int[${Me.Mezzed.ID}]}]"]} -o DNout -t ${DanNetDelay}
                        /if (${Int[${DNout}]}==0) {
                            /varset IniDebuffCount 0|0
                            /break
                        } else {
                            /varset IniDebuffCount ${Int[${DNout}]}
                            /dquery ${pIDList[${j}]} -q ${Parse[0,"Int[${Me.Poisoned.ID}]}|${Int[${Me.Diseased.ID}]}|${Math.Calc[${Int[${Me.Cursed.ID}]}+${Int[${Me.Song[Restless Curse].ID}]}].Int}|${Int[${Me.Corrupted.ID}]}|${Int[${Me.Mezzed.ID}]"]} -o DNout -t ${DanNetDelay}
                            /varset IniDebuffCount ${IniDebuffCount}|${DNout}
                        }
                    }
                } else {
                    /varset IniDebuffCount ${Math.Calc[${Int[${Me.Poisoned.ID}]}+${Int[${Me.Diseased.ID}]}+${Int[${Me.Cursed.ID}]}+${Int[${Me.Corrupted.ID}]}+${Int[${Me.Mezzed.ID}]}]}+${Int[${Me.Song[Restless Curse].ID}]}
                    /if (!${Int[${IniDebuffCount}]}) /break
                    /varset IniDebuffCount ${IniDebuffCount}|${Int[${Me.Poisoned.ID}]}|${Int[${Me.Diseased.ID}]}|${Math.Calc[${Int[${Me.Cursed.ID}]}+${Int[${Me.Song[Restless Curse].ID}]}]}|${Int[${Me.Corrupted.ID}]}|${Int[${Me.Mezzed.ID}]}
                }
                DEBUGHEALS CheckCures ${IniDebuffCount} ${C1stPart} ${C2ndPart} ${C3rdPart}
                /if (${C2ndPart.Length}>0) {
                    /if (${C2ndPart.Equal[poison]} && ${IniDebuffCount.Arg[2,|]}>0) {
                        /varset CuretoCast ${C1stPart}
                    } else /if (${C2ndPart.Equal[disease]} && ${IniDebuffCount.Arg[3,|]}>0) {
                        /varset CuretoCast ${C1stPart}
                    } else /if (${C2ndPart.Equal[curse]} && ${IniDebuffCount.Arg[4,|]}>0) {
                        /varset CuretoCast ${C1stPart}
                    } else /if (${C2ndPart.Equal[corruption]} && ${IniDebuffCount.Arg[5,|]}>0) {
                        /varset CuretoCast ${C1stPart}
                    } else /if (${C2ndPart.Equal[mezzed]} && ${IniDebuffCount.Arg[6,|]}>0) {
                        /varset CuretoCast ${C1stPart}
                    }
                    /if (!${CuretoCast.Length}) /continue
                } else {
                    /varset CuretoCast ${C1stPart}
                }
                | Check cures for group component since they don't seem to work on out of group toons
                /if (${Spell[${CuretoCast}].TargetType.Equal[group v1]} && !${Spawn[id ${CTID} group].ID}) {
                    /if (!${Defined[CureTimer${CTID}${i}]}) {
                        /declare CureTimer${CTID}${i} timer outer 30s
                        DEBUGHEALS CheckCures /echo declare CureTimer${CTID}${i}
                    } else /if (!${CureTimer${CTID}${i}}) {
                        /varset CureTimer${CTID}${i} 30s
                        DEBUGHEALS CheckCures /echo varset CureTimer${CTID}${i}
                    }
                    /if (${DebugHeals} && !${CureTimer${CTID}${i}}) /echo CURING: >> ${CTName} << is not in your group and >> ${CuretoCast} << is a group spell.
                    /continue
                }
                /call CastWhat "${CuretoCast}" ${CTID} Cure ${CondNo} 0
                /if (${Macro.Return.Equal[CAST_SUCCESS]}) {
                    /call BroadCast o "CURING: >> ${CTName} << with ${CuretoCast}"
                    /varset CureCast 1
                    /if (${HealsOn}) {
                        /call CheckHealth CheckCures
                        /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
                    }
                }
            | Bottom Each Cure
            /next i
            /if (${CTID}==${Me.ID} && ${CureCast}) {
                /if (!${DanNetOn}) {
                    /varset IniDebuffCount ${Math.Calc[${Int[${Me.Poisoned.ID}]}+${Int[${Me.Diseased.ID}]}+${Int[${Me.Cursed.ID}]}+${Int[${Me.Corrupted.ID}]}+${Int[${Me.Mezzed.ID}]}]}+${Int[${Me.Song[Restless Curse].ID}]}
                    /if (!${IniDebuffCount}) {
                        /varset NeedCuring 1
                        /call WriteDebuffs
                    }
                }
            }
            /if (${HealsOn}) {
                /if (!${LastHealCheck} || ${AggroTargetID}) {
                    /call CheckHealth CheckCures
                    /if (${HealInterval}) /varcalc LastHealCheck ${HealInterval}*10
                }
            }
        | Bottom Each Character
        /next j
        /if (${MacroQuest.Parser}==2) /squelch /engine parser 1
        DEBUGHEALS CheckCures Leave ${sentFrom}
    /return
| ----------------------------------------------------------------------------
| SUB: Set Xtargets to Heal
| ----------------------------------------------------------------------------
    Sub SetXTargetHealList(string HealList)
        /if (!${XTarHeal} && ${HealList.Equal[null]}) /return
        /declare n int local 0
        /if (${HealList.Equal[all]}) {
            /for GlobalIndex 1 to ${XSlotTotal}
                /if (${Me.XTarget[${GlobalIndex}].TargetType.Equal[Auto Hater]}) {
                    /echo XTarget ${GlobalIndex} is assigned as Auto Hater. You may want to fix this when using the ALL tag.
                } else {
                    /varcalc n ${n}+1
                    /varset XTarToHeal[${n}] ${GlobalIndex}
                }
            /next GlobalIndex
            /if (${n} && ${n}<${XSlotTotal}) {
                /varcalc n ${n}+1
                /for GlobalIndex ${n} to ${XSlotTotal}
                    /varset XTarToHeal[${GlobalIndex}] 0
                /next GlobalIndex
            }
        } else /if (${HealList.Equal[auto]}) {
            /for GlobalIndex 1 to ${XSlotTotal}
                /if (${Me.XTarget[${GlobalIndex}].TargetType.Equal[Auto Hater]}) {
                    /continue
                } else {
                    /varcalc n ${n}+1
                    /varset XTarToHeal[${n}] ${GlobalIndex}
                }
            /next GlobalIndex
            /if (${n} && ${n}<${XSlotTotal}) {
                /varcalc n ${n}+1
                /for GlobalIndex ${n} to ${XSlotTotal}
                    /varset XTarToHeal[${GlobalIndex}] 0
                /next GlobalIndex
            }
        } else {
            /varset n 1
            /varset GlobalIndex 1
            /if (${HealList.Find[|]}) {
                /while (${GlobalIndex}<${XSlotTotal}) {
                    /if (${Int[${HealList.Arg[${GlobalIndex},|]}]}>0 && ${Int[${HealList.Arg[${GlobalIndex},|]}]}<=${XSlotTotal}) {
                        /varset XTarToHeal[${n}] ${Int[${HealList.Arg[${GlobalIndex},|]}]}
                        /if (${Me.XTarget[${XTarToHeal[${n}]}].TargetType.Equal[Auto Hater]}) /echo XTarget ${XTarToHeal[${n}]} is assigned as Auto Hater. You may want to fix this.
                        /varcalc n ${n}+1
                        /varcalc GlobalIndex ${GlobalIndex}+1
                    } else /if (${Int[${HealList.Arg[${GlobalIndex},|]}]}==0) {
                        /varset XTarToHeal[${n}] 0
                        /varcalc GlobalIndex ${XSlotTotal}+1
                    } else {
                        /varcalc GlobalIndex ${GlobalIndex}+1
                    }
                }
            } else {
                /varset XTarToHeal[1] ${Int[${HealList}]}
            }
        }
    /return
