# KissAssist_charm
KissAssist with Charm, and Medley support


To install close the repository and put the kisscharm.mac and KAssist folder inside your MacroQuest Macros folder. 
to run /mac kisscharm
everything follows the same proceedures as KissAssist http://kissassist.com


Major Changes from Kissassist are:
  * Dropping MQ2TWIST for MQ2Medley, this causes much less headaches on a bard.
  * Adding in Charming for Druid, Enchanter, Necro, and Bards.

New Config Settings:
  * CharmOn=0 off, 1 on
  * CharmSpell=Your Charm Spell or Ability
  * CharmMinLevel=Mob Min Level to try charming
  * CharmMaxLevel=Mob Max level to try charming
  * CharmRadius=50
  * CharmKeep=0 off, 1 on "Will attempt to keep the same pet after breaks"

New Commands:
  * /charmon    ---Toggles on | off charming
  * /charmkeep  ---Toggles on | off trying to keep the same pet
  * /setcharmed ---will set your current pet as the Pet you want to keep incase you want to keep CharmKeep off unbtil you find that perfect pet =)
  * /medleyon   ---Toggles on | off Medley on the bard. 

If you have MezOn and CharmOn, it will first try and mez the mob before charming it.
If you already have a pet, routine is ignored.
Will check for Charm Immune mob in your KissAssist_Info.ini works the same as Mez Immune
If you have MezOn and have a MezDebuff spell set, the macro will attempt to debuff the mob on a failed charm attempt as well.
if CharmKeep is on, you will ONLY try to re-charm the same charmed mob "defaults to first charmed mob, you can overwrite with /setcharmed" and It will ignore the min of 2 mobs required before charming.

file is named KissCharm.mac for ease of keeping it separate but will identify as KissAssist
you load it /mac kisscharm instead of kissassist is only difference.
allows you to run both for different class toons, if you so choose.  

Discussion and previous update notes can be found
https://www.redguides.com/community/threads/kissassist-with-charming.89163/
