#include "global.h"
#include "item_use.h"
#include "item.h"
#include "party_menu.h"
#include "pokemon.h"
#include "strings.h"
#include "task.h"
#include "text.h"

// Function Declarations
void ItemUseCB_EvolutionStone(u8 taskId);
void CloseItemMessage(u8 taskId);

// Main Logic
void ItemUseCB_WhippedDream(u8 taskId)
{
    struct Pokemon *mon = &gPlayerParty[gPartyMenu.slotId];
    if (CanUseWhippedDream(GetMonData(mon, MON_DATA_SPECIES)))
        ItemUseCB_EvolutionStone(taskId);
    else
        DisplayItemMessage(taskId, FONT_NORMAL, gText_WontHaveEffect, CloseItemMessage);
}
