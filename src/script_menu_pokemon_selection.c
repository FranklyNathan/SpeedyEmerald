#include "global.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "strings.h"
#include "pokemon.h"
#include "constants/species.h"
#include "constants/flags.h"
#include "event_data.h"

static const u16 sGiftPokemonList[] = {
    SPECIES_APPLIN, 
    SPECIES_ARON, 
    SPECIES_BAGON, 
    SPECIES_BUIZEL,
    SPECIES_BUNEARY, 
    SPECIES_BUNNELBY, 
    SPECIES_CACNEA, 
    SPECIES_CARVANHA, 
    SPECIES_CHESPIN,
    SPECIES_CHINCHOU, 
    SPECIES_CORPHISH, 
    SPECIES_CUBONE, 
    SPECIES_CUTIEFLY, 
    SPECIES_DEWPIDER,
    SPECIES_DIGLETT, 
    SPECIES_DRILBUR, 
    SPECIES_ELECTRIKE, 
    SPECIES_EXEGGCUTE, 
    SPECIES_FENNEKIN,
    SPECIES_FLABEBE, 
    SPECIES_FLETCHLING, 
    SPECIES_FROAKIE, 
    SPECIES_FUECOCO, 
    SPECIES_GEODUDE,
    SPECIES_GIBLE, 
    SPECIES_GLIGAR, 
    SPECIES_GOOMY, 
    SPECIES_GREAVARD, 
    SPECIES_GROWLITHE,
    SPECIES_GRUBBIN, 
    SPECIES_HELIOPTILE, 
    SPECIES_HONEDGE, 
    SPECIES_HORSEA,
    SPECIES_HOUNDOUR, 
    SPECIES_KUBFU, 
    SPECIES_LITTEN, 
    SPECIES_LITWICK, 
    SPECIES_MAGNEMITE,
    SPECIES_MAKUHITA, 
    SPECIES_MANKEY, 
    SPECIES_MAREEP, 
    SPECIES_MAWILE, 
    SPECIES_MINCCINO,
    SPECIES_MINIOR_METEOR_RED, 
    SPECIES_MORELULL, 
    SPECIES_MUDBRAY, 
    SPECIES_MUDKIP, 
    SPECIES_NINCADA,
    SPECIES_NOIBAT, 
    SPECIES_NUMEL, 
    SPECIES_ODDISH, 
    SPECIES_ORICORIO, 
    SPECIES_PANCHAM,
    SPECIES_PIKIPEK, 
    SPECIES_POLIWAG, 
    SPECIES_POOCHYENA, 
    SPECIES_POPPLIO, 
    SPECIES_PORYGON,
    SPECIES_RALTS, 
    SPECIES_ROCKRUFF, 
    SPECIES_ROTOM, 
    SPECIES_ROWLET, 
    SPECIES_SABLEYE,
    SPECIES_SALANDIT, 
    SPECIES_SANDILE, 
    SPECIES_SCATTERBUG, 
    SPECIES_SCYTHER, 
    SPECIES_SHELLDER,
    SPECIES_SHINX, 
    SPECIES_SHROOMISH,
    SPECIES_SHUPPET, 
    SPECIES_SKIDDO, 
    SPECIES_SKRELP,
    SPECIES_SLOWPOKE_GALAR, 
    SPECIES_SNORUNT, 
    SPECIES_SOLOSIS, 
    SPECIES_SPHEAL, 
    SPECIES_SPOINK,
    SPECIES_SPRIGATITO, 
    SPECIES_STARYU, 
    SPECIES_SURSKIT, 
    SPECIES_TEDDIURSA, 
    SPECIES_TENTACOOL,
    SPECIES_TINKATINK, 
    SPECIES_TORCHIC, 
    SPECIES_TRAPINCH, 
    SPECIES_TREECKO, 
    SPECIES_TRUBBISH,
    SPECIES_TURTWIG, 
    SPECIES_VENIPEDE, 
    SPECIES_VULPIX_ALOLA, 
    SPECIES_WINGULL, 
    SPECIES_WISHIWASHI,
    SPECIES_WOOPER_PALDEA, 
    SPECIES_ZIGZAGOON_GALAR, 
    SPECIES_ZUBAT
};

// This function builds the list of Pokémon for the multichoice menu.
void BuildGiftPokemonMenu(void)
{
    u32 i;

    ScriptMenu_ClearDynMultichoice();

    for (i = 0; i < ARRAY_COUNT(sGiftPokemonList); i++)
        // This will add all Pokémon from the list to the menu every time.
        // This allows the player to receive the same Pokémon multiple times.
        ScriptMenu_AddDynmultichoice(gSpeciesInfo[sGiftPokemonList[i]].speciesName, sGiftPokemonList[i], 0);
    ScriptMenu_AddDynmultichoice(gText_Finished, 999, -1);
}
