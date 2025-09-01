EventScript_ChooseGiftPokemon::
	faceplayer
	msgbox Text_ChooseAGiftPokemon, MSGBOX_DEFAULT
	closemessage
	special BuildGiftPokemonMenu
	special SetIsGiftPokemonMenu
	// This custom command from your original script displays the dynamic multichoice menu.
	// The max items shown before scrolling is set to 8 to fit on the screen.
	dynmultistack 0, 0, FALSE, 8, FALSE, 0, FALSE
EventScript_EndGiftPokemon::
	return

Text_ChooseAGiftPokemon:
	.string "{PLAYER} checked their notes…\n"
	.string "Who did you draft?$"

gText_ReceivedGiftMon::
	.string "{PLAYER} received a {STR_VAR_1}!$"
