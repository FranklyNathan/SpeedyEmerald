FortreeCity_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FortreeCity_Gym_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, FortreeCity_Gym_OnWarp
	.byte 0

FortreeCity_Gym_OnTransition:
	special RotatingGate_InitPuzzle
	setflag FLAG_TEMP_HIDE_FOLLOWER @ Hide follower bc of rotating gates
	end

FortreeCity_Gym_OnWarp:
	map_script_2 VAR_TEMP_0, VAR_TEMP_0, FortreeCity_Gym_EventScript_InitRotatingGates
	.2byte 0

@ NOTE: This rotating gate puzzle makes use of VAR_TEMP_0 - VAR_TEMP_3
FortreeCity_Gym_EventScript_InitRotatingGates::
	special RotatingGate_InitPuzzleAndGraphics
	end

FortreeCity_Gym_EventScript_Winona::
    setvar VAR_FORCEANIM, 1
	call_if_eq VAR_BADGE_COUNT, 0, FortreeCity_Gym_EventScript_Winona_Trainer1
	call_if_eq VAR_BADGE_COUNT, 1, FortreeCity_Gym_EventScript_Winona_Trainer2
	call_if_eq VAR_BADGE_COUNT, 2, FortreeCity_Gym_EventScript_Winona_Trainer3
	call_if_eq VAR_BADGE_COUNT, 3, FortreeCity_Gym_EventScript_Winona_Trainer4
	call_if_eq VAR_BADGE_COUNT, 4, FortreeCity_Gym_EventScript_Winona_Trainer5
	call_if_eq VAR_BADGE_COUNT, 5, FortreeCity_Gym_EventScript_Winona_Trainer6
	call_if_eq VAR_BADGE_COUNT, 6, FortreeCity_Gym_EventScript_Winona_Trainer7
	call_if_eq VAR_BADGE_COUNT, 7, FortreeCity_Gym_EventScript_Winona_Trainer8

FortreeCity_Gym_EventScript_Winona_Trainer1::
	trainerbattle_single TRAINER_WINONA_1, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_Winona_Trainer2::
	trainerbattle_single TRAINER_WINONA_2, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_Winona_Trainer3::
	trainerbattle_single TRAINER_WINONA_3, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_Winona_Trainer4::
	trainerbattle_single TRAINER_WINONA_4, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_Winona_Trainer5::
	trainerbattle_single TRAINER_WINONA_5, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_Winona_Trainer6::
	trainerbattle_single TRAINER_WINONA_6, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_Winona_Trainer7::
	trainerbattle_single TRAINER_WINONA_7, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_Winona_Trainer8::
	trainerbattle_single TRAINER_WINONA_8, FortreeCity_Gym_Text_WinonaIntro, FortreeCity_Gym_Text_WinonaDefeat, FortreeCity_Gym_EventScript_WinonaDefeated, NO_MUSIC
	goto FortreeCity_Gym_EventScript_PostBattle

FortreeCity_Gym_EventScript_PostBattle::
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, FortreeCity_Gym_EventScript_WinonaRematch
	goto_if_unset FLAG_RECEIVED_TM_AERIAL_ACE, FortreeCity_Gym_EventScript_GiveAerialAce2
	msgbox FortreeCity_Gym_Text_WinonaPostBattle, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_WinonaDefeated::
	message FortreeCity_Gym_Text_ReceivedFeatherBadge
	waitmessage
	msgbox FortreeCity_Gym_Text_ExplainFeatherBadgeTakeThis, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_FORTREE_GYM
	setflag FLAG_BADGE06_GET
	setvar VAR_0x8008, 6
	setvar VAR_FORCEANIM, 0
	call Common_EventScript_SetGymTrainers
	call FortreeCity_Gym_EventScript_GiveAerialAce
	closemessage
	delay 30
	//playfanfare MUS_REGISTER_MATCH_CALL
	//msgbox FortreeCity_Gym_Text_RegisteredWinona, MSGBOX_DEFAULT
	//waitfanfare
	//closemessage
	//delay 30
	//setflag FLAG_ENABLE_WINONA_MATCH_CALL
	//setvar VAR_SCOTT_FORTREE_CALL_STEP_COUNTER, 0
	//setflag FLAG_SCOTT_CALL_FORTREE_GYM
	switch VAR_RETURN_MAP
        case 0, EventScript_WarpHomeRustboro
        case 1, EventScript_WarpHomeDewford
        case 2, EventScript_WarpHomeMauville
        case 3, EventScript_WarpHomeLavaridge
        case 4, EventScript_WarpHomePetalburg
        case 5, EventScript_WarpHomeFortree
        case 6, EventScript_WarpHomeMossdeep
        case 7, EventScript_WarpHomeSootopolis
	release
	end

FortreeCity_Gym_EventScript_GiveAerialAce2::
	giveitem ITEM_TM_AERIAL_ACE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	msgbox FortreeCity_Gym_Text_ExplainAerialAce, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_AERIAL_ACE
	release
	end

FortreeCity_Gym_EventScript_GiveAerialAce::
	giveitem ITEM_TM_AERIAL_ACE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_BagIsFull
	msgbox FortreeCity_Gym_Text_ExplainAerialAce, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_TM_AERIAL_ACE
	return

FortreeCity_Gym_EventScript_WinonaRematch::
	trainerbattle_rematch_double TRAINER_WINONA_1, FortreeCity_Gym_Text_WinonaPreRematch, FortreeCity_Gym_Text_WinonaRematchDefeat, FortreeCity_Gym_Text_WinonaRematchNeedTwoMons
	msgbox FortreeCity_Gym_Text_WinonaPostRematch, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Jared::
	trainerbattle_single TRAINER_JARED, FortreeCity_Gym_Text_JaredIntro, FortreeCity_Gym_Text_JaredDefeat
	msgbox FortreeCity_Gym_Text_JaredPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Edwardo::
	trainerbattle_single TRAINER_EDWARDO, FortreeCity_Gym_Text_EdwardoIntro, FortreeCity_Gym_Text_EdwardoDefeat
	msgbox FortreeCity_Gym_Text_EdwardoPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Flint::
	trainerbattle_single TRAINER_FLINT, FortreeCity_Gym_Text_FlintIntro, FortreeCity_Gym_Text_FlintDefeat
	msgbox FortreeCity_Gym_Text_FlintPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Ashley::
	trainerbattle_single TRAINER_ASHLEY, FortreeCity_Gym_Text_AshleyIntro, FortreeCity_Gym_Text_AshleyDefeat
	msgbox FortreeCity_Gym_Text_AshleyPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Humberto::
	trainerbattle_single TRAINER_HUMBERTO, FortreeCity_Gym_Text_HumbertoIntro, FortreeCity_Gym_Text_HumbertoDefeat
	msgbox FortreeCity_Gym_Text_HumbertoPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_Darius::
	trainerbattle_single TRAINER_DARIUS, FortreeCity_Gym_Text_DariusIntro, FortreeCity_Gym_Text_DariusDefeat
	msgbox FortreeCity_Gym_Text_DariusPostBattle, MSGBOX_AUTOCLOSE
	end

FortreeCity_Gym_EventScript_GymGuide::
	lock
	faceplayer
    special HealPlayerParty
	goto_if_set FLAG_DEFEATED_FORTREE_GYM, FortreeCity_Gym_EventScript_GymGuidePostVictory
	msgbox FortreeCity_Gym_Text_GymGuideAdvice, MSGBOX_DEFAULT
	warp MAP_FORTREE_CITY_GYM, 15, 24
	release
	end

FortreeCity_Gym_EventScript_GymGuidePostVictory::
	msgbox FortreeCity_Gym_Text_GymGuidePostVictory, MSGBOX_DEFAULT
	release
	end

FortreeCity_Gym_EventScript_LeftGymStatue::
	lockall
	goto_if_set FLAG_BADGE06_GET, FortreeCity_Gym_EventScript_GymStatueCertified
	goto FortreeCity_Gym_EventScript_GymStatue
	end

FortreeCity_Gym_EventScript_RightGymStatue::
	lockall
	goto_if_set FLAG_BADGE06_GET, FortreeCity_Gym_EventScript_GymStatueCertified
	goto FortreeCity_Gym_EventScript_GymStatue
	end

FortreeCity_Gym_EventScript_GymStatueCertified::
	msgbox FortreeCity_Gym_Text_GymStatueCertified, MSGBOX_DEFAULT
	releaseall
	end

FortreeCity_Gym_EventScript_GymStatue::
	msgbox FortreeCity_Gym_Text_GymStatue, MSGBOX_DEFAULT
	releaseall
	end

FortreeCity_Gym_Text_GymGuideAdvice:
	.string "Yo, how's it going, CHAMPION-\n"
	.string "bound {PLAYER}?\p"
	.string "FORTREE GYM LEADER WINONA is\n"
	.string "a master of FLYING-type POKéMON.\p"
	.string "She's waiting at the back of this GYM,\n"
	.string "behind the rotating doors.\p"
	.string "She's waiting for new challengers\n"
	.string "who are trying to take wing!\p"
	.string "After this chat, your POKéMON should\n"
	.string "be refreshed and ready to go!\p"
	.string "Oh, and before you go, let me reset\n"
	.string "the gates for you…$"

FortreeCity_Gym_Text_GymGuidePostVictory:
	.string "You did it!\n"
	.string "You've achieved liftoff!$"

FortreeCity_Gym_Text_JaredIntro:
	.string "Behold the elegant battle style of\n"
	.string "BIRD POKéMON!$"

FortreeCity_Gym_Text_JaredDefeat:
	.string "You…\n"
	.string "You're strong…$"

FortreeCity_Gym_Text_JaredPostBattle:
	.string "A TRAINER has to be smart to keep\n"
	.string "up with unexpected turns of events.\p"
	.string "Do you have the smarts to get to\n"
	.string "our LEADER?$"

FortreeCity_Gym_Text_EdwardoIntro:
	.string "The lovers of BIRD POKéMON aspire\n"
	.string "to join this GYM.\p"
	.string "As a member of the FORTREE GYM,\n"
	.string "I'm not allowed to lose!$"

FortreeCity_Gym_Text_EdwardoDefeat:
	.string "It was too much of a load for me\n"
	.string "to bear…$"

FortreeCity_Gym_Text_EdwardoPostBattle:
	.string "The world is huge, and there are\n"
	.string "countless tough TRAINERS.\p"
	.string "I'm going to keep training and make\n"
	.string "myself even stronger.$"

FortreeCity_Gym_Text_FlintIntro:
	.string "There's no need for WINONA, our GYM\n"
	.string "LEADER, to deal with you!\p"
	.string "I'm plenty good enough for you!$"

FortreeCity_Gym_Text_FlintDefeat:
	.string "WINONA, I…\n"
	.string "I lost!$"

FortreeCity_Gym_Text_FlintPostBattle:
	.string "WINONA is cute and she's strong.\n"
	.string "She's the ultimate LEADER!\p"
	.string "Blush…$"

FortreeCity_Gym_Text_AshleyIntro:
	.string "WINONA taught me personally!\n"
	.string "You can't beat me easily!$"

FortreeCity_Gym_Text_AshleyDefeat:
	.string "I was beaten…$"

FortreeCity_Gym_Text_AshleyPostBattle:
	.string "Thanks to WINONA, the people of\n"
	.string "FORTREE can live without fear.$"

FortreeCity_Gym_Text_HumbertoIntro:
	.string "When WINONA takes to battle, her face\n"
	.string "shines with beautiful determination…\p"
	.string "I'm not letting you witness that\n"
	.string "lovely sight!$"

FortreeCity_Gym_Text_HumbertoDefeat:
	.string "Urk!\n"
	.string "I couldn't stop you.$"

FortreeCity_Gym_Text_HumbertoPostBattle:
	.string "You'd better watch it!\n"
	.string "Don't get distracted staring at WINONA\l"
	.string "or you'll go crashing down in a heap!$"

FortreeCity_Gym_Text_DariusIntro:
	.string "You'd better know that there are all\n"
	.string "sorts of FLYING-type POKéMON.\p"
	.string "You do know that, right?$"

FortreeCity_Gym_Text_DariusDefeat:
	.string "You seem to know your stuff!$"

FortreeCity_Gym_Text_DariusPostBattle:
	.string "Sure, you beat me all right.\n"
	.string "But you'd better watch it! Our LEADER\l"
	.string "WINONA's POKéMON are all business.$"

FortreeCity_Gym_Text_WinonaIntro:
	.string "I am WINONA. I am the LEADER of\n"
	.string "the FORTREE POKéMON GYM.\p"
	.string "I have become one with BIRD POKéMON\n"
	.string "and have soared the skies…\p"
	.string "However grueling the battle, we have\n"
	.string "triumphed with grace…\p"
	.string "Witness the elegant choreography\n"
	.string "of BIRD POKéMON and I!$"

FortreeCity_Gym_Text_WinonaDefeat:
	.string "Never before have I seen a TRAINER\n"
	.string "command POKéMON with more grace\l"
	.string "than I…\p"
	.string "In recognition of your prowess,\n"
	.string "I present to you this GYM BADGE.$"

FortreeCity_Gym_Text_ReceivedFeatherBadge:
	.string "{PLAYER} received the FEATHER BADGE\n"
	.string "from WINONA.$"

FortreeCity_Gym_Text_ExplainFeatherBadgeTakeThis:
	.string "And this…\n"
	.string "This is a gift from me.$"

FortreeCity_Gym_Text_ExplainAerialAce:
	.string "TM40 contains AERIAL ACE.\p"
	.string "Its speed…\n"
	.string "No POKéMON should be able to avoid it.\p"
	.string "… … … … … …$"

FortreeCity_Gym_Text_RegisteredWinona:
	.string "Registered GYM LEADER WINONA\n"
	.string "in the POKéNAV.$"

FortreeCity_Gym_Text_WinonaPostBattle:
	.string "Though I fell to you, I will remain\n"
	.string "devoted to BIRD POKéMON.$"

FortreeCity_Gym_Text_GymStatue:
	.string "FORTREE CITY POKéMON GYM$"

FortreeCity_Gym_Text_GymStatueCertified:
	.string "FORTREE CITY POKéMON GYM\p"
	.string "WINONA'S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

FortreeCity_Gym_Text_WinonaPreRematch:
	.string "WINONA: We humans can never escape\n"
	.string "gravity's pull on the ground.\p"
	.string "But by striving for excellence,\n"
	.string "we have the power to soar!\p"
	.string "You have scaled the pinnacle as\n"
	.string "the reigning CHAMPION.\p"
	.string "Please, allow me to see your power\n"
	.string "at full flight!$"

FortreeCity_Gym_Text_WinonaRematchDefeat:
	.string "I failed to reach your height again…$"

FortreeCity_Gym_Text_WinonaPostRematch:
	.string "WINONA: Even though I have lost,\n"
	.string "the wings of my heart remain unbroken.\p"
	.string "I can rise and soar again and\n"
	.string "yet again.\p"
	.string "I am convinced of it!$"

FortreeCity_Gym_Text_WinonaRematchNeedTwoMons:
	.string "WINONA: We humans can never escape\n"
	.string "gravity's pull on the ground.\p"
	.string "But by striving for excellence,\n"
	.string "we have the power to soar!\p"
	.string "You have scaled the pinnacle as\n"
	.string "the reigning CHAMPION.\p"
	.string "Please, allow me to see your power\n"
	.string "at full flight!\p"
	.string "Oh, no… Do you have but one\n"
	.string "POKéMON with you?\p"
	.string "Please return with two POKéMON\n"
	.string "at the very least.$"
