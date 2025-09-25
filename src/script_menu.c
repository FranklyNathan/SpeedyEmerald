#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon.h"
#include "event_data.h"
#include "field_effect.h"
#include "field_screen_effect.h"
#include "field_specials.h"
#include "pokemon.h"
#include "field_weather.h"
#include "graphics.h"
#include "item.h"
#include "menu.h"
#include "palette.h"
#include "script.h"
#include "random.h"
#include "script_menu.h"
#include "battle_gfx_sfx_util.h"
#include "overworld.h"
#include "sound.h"
#include "pokemon_summary_screen.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "list_menu.h"
#include "malloc.h"
#include "util.h"
#include "decompress.h"
#include "item_icon.h"
#include "pokemon_icon.h"
#include "constants/field_specials.h"
#include "constants/items.h"
#include "constants/script_menu.h"
#include "constants/pokemon_icon.h"
#include "constants/songs.h"
#include "script_pokemon_util.h"
#include "trainer_pokemon_sprites.h"
#include "list_menu.h"

#define TAG_GIFT_MON_PIC_TILES 5510
#define TAG_GIFT_MON_PIC_PALETTE 5511

#define MAX_GIFT_MON 10
#define GIFT_POKEMON_COUNT 100

#include "data/script_menu.h"

struct DynamicListMenuEventArgs
{
    struct ListMenuTemplate *list;
    u16 selectedItem;
    u8 windowId;
};

typedef void (*DynamicListCallback)(struct DynamicListMenuEventArgs *eventArgs);

struct DynamicListMenuEventCollection
{
    DynamicListCallback OnInit;
    DynamicListCallback OnSelectionChanged;
    DynamicListCallback OnDestroy;
};

struct GiftMonMenu
{
    u8 monSpriteId;
    u8 windowId;
    u8 bottomWindowId;
    u16 monSpriteIds[4];
    u16 selectedMonSpriteId;
};

static EWRAM_DATA struct GiftMonMenu sGiftMonMenuData = {0};

static EWRAM_DATA u8 sProcessInputDelay = 0;
static EWRAM_DATA u8 sGiftSpriteIds[GIFT_POKEMON_COUNT];
static EWRAM_DATA u8 sDynamicMenuEventId = 0;
static EWRAM_DATA struct DynamicMultichoiceStack *sDynamicMultiChoiceStack = NULL;
static EWRAM_DATA u16 *sDynamicMenuEventScratchPad = NULL;
static EWRAM_DATA bool8 sGiftMonIsTaken[SPECIES_EGG + 1] = {0};
static EWRAM_DATA bool8 sIsGiftMonMenu = FALSE; // TODO: Make this a part of the dynamic list menu args
static u8 sLilycoveSSTidalSelections[SSTIDAL_SELECTION_COUNT] = {0}; // TODO: Make this a part of the dynamic list menu args

static const u16 sGiftEggPool[] =
{
    SPECIES_PICHU,
    SPECIES_CLEFFA,
    SPECIES_IGGLYBUFF,
    SPECIES_TOGEPI,
    SPECIES_TYROGUE,
    SPECIES_SMOOCHUM,
    SPECIES_ELEKID,
    SPECIES_MAGBY,
    SPECIES_AZURILL,
    SPECIES_WYNAUT,
    SPECIES_BUDEW,
    SPECIES_CHINGLING,
    SPECIES_BONSLY,
    SPECIES_MIME_JR,
    SPECIES_HAPPINY,
    SPECIES_MUNCHLAX,
    SPECIES_RIOLU,
    SPECIES_MANTYKE,
    SPECIES_TOXEL,
};

static const u8 sGiftMenuTextColors[][3] =
{
    [0] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY}, // Normal
    [1] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GREEN, TEXT_COLOR_GREEN}, // Chosen
};

static void FreeListMenuItems(struct ListMenuItem *items, u32 count);
static void Task_HandleScrollingMultichoiceInput(u8 taskId);
static void Task_HandleMultichoiceInput(u8 taskId);
static void Task_HandleYesNoInput(u8 taskId);
static void Task_HandleMultichoiceGridInput(u8 taskId);
static void DrawMultichoiceMenuDynamic(u8 left, u8 top, u8 argc, struct ListMenuItem *items, bool8 ignoreBPress, u32 initialRow, u8 maxBeforeScroll, u32 callbackSet);
void ListMenuSetLRBtnWrap(u8 listTaskId, bool8 lrWrap);
u16 ListMenu_GetSelectedRowIndex(u8 listTaskId);
static u8 CountTakenGiftMons(void);
static void GiftMonMenu_DestroyMonIcons(void);
static void DrawMultichoiceMenu(u8 left, u8 top, u8 multichoiceId, bool8 ignoreBPress, u8 cursorPos);
static void InitMultichoiceCheckWrap(bool8 ignoreBPress, u8 count, u8 windowId, u8 multichoiceId);
static void DrawLinkServicesMultichoiceMenu(u8 multichoiceId);
static void CreateLilycoveSSTidalMultichoice(void);
static void CreateStartMenuForPokenavTutorial(void);
static void InitMultichoiceNoWrap(bool8 ignoreBPress, u8 unusedCount, u8 windowId, u8 multichoiceId);
static void MultichoiceDynamicEventShowItem_OnInit(struct DynamicListMenuEventArgs *eventArgs);
static void MultichoiceDynamicEventShowItem_OnSelectionChanged(struct DynamicListMenuEventArgs *eventArgs);
static void MultichoiceDynamicEventShowItem_OnDestroy(struct DynamicListMenuEventArgs *eventArgs);
static void Task_PokemonPicWindow(u8 taskId);
static void CreatePCMultichoice(void);
static void Task_ExitGiftMenu(u8 taskId);
static const struct OamData sMonPicOamData = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 0,
};
static void CreateGiftMonSpritesAtPos(u16 selectedMon);


const struct SpriteTemplate gMonPicSpriteTemplate = {
    .tileTag = TAG_NONE,
    .paletteTag = TAG_NONE,
    .oam = &sMonPicOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy};

static const struct DynamicListMenuEventCollection sDynamicListMenuEventCollections[] =
{
    [DYN_MULTICHOICE_CB_SHOW_ITEM] =
    {
        .OnInit = MultichoiceDynamicEventShowItem_OnInit,
        .OnSelectionChanged = MultichoiceDynamicEventShowItem_OnSelectionChanged,
        .OnDestroy = MultichoiceDynamicEventShowItem_OnDestroy
    }
};

static void GiftMonMenu_CreateFrontSprite(u16 species)
{
    u8 spriteId;

    // Destroy the old sprite if it exists
    if (sGiftMonMenuData.selectedMonSpriteId != 0xFFFF)
    {
        // The species of the previously displayed mon is not tracked, so we can't reliably free its specific palette tag.
        // Instead, we'll free the generic tag and rely on the new unique tag to load the correct palette.
        FreeSpritePaletteByTag(TAG_GIFT_MON_PIC_PALETTE);
        FreeAndDestroyMonPicSprite(sGiftMonMenuData.selectedMonSpriteId);
        sGiftMonMenuData.selectedMonSpriteId = 0xFFFF;
    }

    if (species != SPECIES_EGG && species != 999)
    {
        // Use the species ID as the palette tag to ensure each Pokémon gets its own unique palette.
        spriteId = CreateMonPicSprite(species, FALSE, 0, TRUE, 176, 50, 15, species);
        gSprites[spriteId].oam.priority = 0; // Ensure it's drawn on top of the window
        sGiftMonMenuData.selectedMonSpriteId = spriteId;
    }
}

static void GiftMonMenu_CreateChosenMonIcon(u16 species)
{
    u8 i;
    u8 spriteId;
    u8 row, col;
    u16 x, y;

    // Display up to 10 Pokémon icons in a 2x5 grid in the bottom right window.
    for (i = 0; i < GIFT_POKEMON_COUNT; i++)
    {
        if (sGiftSpriteIds[i] == 0xFF)
        {
            // Found an empty slot.
            row = i / 5;
            col = i % 5;

            // Position the 2x5 grid on the center-right of the screen.
            x = 120 + col * 24;
            y = 112 + row *24;

            spriteId = CreateMonIcon(species, SpriteCallbackDummy, x, y, 4, 0);
            if (spriteId != MAX_SPRITES)
            {
                // The window is on BG0 (priority 0). The icons also need to have priority 0 to be drawn on top.
                // Sprites with the same priority as a BG are drawn on top of it.
                gSprites[spriteId].oam.priority = 0;
                sGiftSpriteIds[i] = spriteId;
            }
            break;
        }
    }
}

static void GiftMonMenu_DestroyMonIcons(void)
{
    u8 i;

    for (i = 0; i < GIFT_POKEMON_COUNT; i++)
    {
        if (sGiftSpriteIds[i] != 0xFF)
            FreeAndDestroyMonIconSprite(&gSprites[sGiftSpriteIds[i]]);
    }
    if (sGiftMonMenuData.selectedMonSpriteId != 0xFFFF)
    {
        FreeAndDestroyMonPicSprite(sGiftMonMenuData.selectedMonSpriteId);
        FreeSpritePaletteByTag(TAG_GIFT_MON_PIC_PALETTE);
        sGiftMonMenuData.selectedMonSpriteId = 0xFFFF;
    }
}

void ListMenuSetLRBtnWrap(u8 listTaskId, bool8 lrWrap)
{
    struct ListMenu *list = (void *)gTasks[listTaskId].data;
}

static u8 CountTakenGiftMons(void)
{
    u8 i;
    u8 count = 0;

    for (i = 0; i < GIFT_POKEMON_COUNT; i++)
    {
        if (sGiftSpriteIds[i] != 0xFF)
            count++;
    }
    return count;
}

static void GiftMonMenu_ItemPrintFunc(u8 windowId, u32 speciesId, u8 y)
{
    struct ListMenuItem *items;
    u16 *data = (u16 *)&gTasks[FindTaskIdByFunc(Task_HandleScrollingMultichoiceInput)].data[3];
    u32 i;
    u32 count = gTasks[FindTaskIdByFunc(Task_HandleScrollingMultichoiceInput)].data[5];
    const u8 *name = NULL;
    const u8 *colors;

    LoadWordFromTwoHalfwords(data, (u32 *)&items);

    // Find the item in the list to get its name
    for (i = 0; i < count; i++)
    {
        if ((u16)items[i].id == speciesId)
        {
            name = items[i].name;
            break;
        }
    }

    if (sGiftMonIsTaken[speciesId])
        colors = sGiftMenuTextColors[1];
    else
        colors = sGiftMenuTextColors[0];
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 8, y, colors, TEXT_SKIP_DRAW, name);

}

void MultichoiceDynamic_MoveCursor(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    u8 taskId;
    if (!onInit && !sIsGiftMonMenu) // In gift menu, scrolling sound is handled elsewhere
        PlaySE(SE_SELECT);

    taskId = FindTaskIdByFunc(Task_HandleScrollingMultichoiceInput);
    if (taskId != TASK_NONE)
    {
        ListMenuGetScrollAndRow(gTasks[taskId].data[0], &gScrollableMultichoice_ScrollOffset, (u16 *)&itemIndex);
        if (sIsGiftMonMenu) // This is a custom callback for the gift mon menu
        {
            if (onInit || gScrollableMultichoice_ScrollOffset + itemIndex != sGiftMonMenuData.selectedMonSpriteId)
                CreateGiftMonSpritesAtPos(onInit ? itemIndex : gScrollableMultichoice_ScrollOffset + itemIndex);
        }
        else if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnSelectionChanged && !onInit) // Otherwise, use the generic dynamic list menu callbacks
        {
            struct DynamicListMenuEventArgs eventArgs = {.selectedItem = itemIndex, .windowId = list->template.windowId, .list = &list->template};
            sDynamicListMenuEventCollections[sDynamicMenuEventId].OnSelectionChanged(&eventArgs);
        }
    }
}

static const struct ListMenuTemplate sScriptableListMenuTemplate =
{
    .item_X = 8,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemPrintFunc = NULL,
    .moveCursorFunc = MultichoiceDynamic_MoveCursor,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NORMAL,
};

void Task_ExitGiftMenu(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        struct ListMenuItem *items;
        s16 *taskData = gTasks[taskId].data;

        // Cleanup logic
        u32 i;
        for (i = 0; i < ARRAY_COUNT(sGiftMonMenuData.monSpriteIds); i++)
        {
            if (sGiftMonMenuData.monSpriteIds[i] != 0xFFFF)
                FreeAndDestroyMonPicSprite(sGiftMonMenuData.monSpriteIds[i]);
        }

        GiftMonMenu_DestroyMonIcons();
        ClearStdWindowAndFrameToTransparent(sGiftMonMenuData.windowId, TRUE);
        RemoveWindow(sGiftMonMenuData.windowId);
        if (sGiftMonMenuData.bottomWindowId != WINDOW_NONE)
        {
            ClearStdWindowAndFrameToTransparent(sGiftMonMenuData.bottomWindowId, TRUE);
            RemoveWindow(sGiftMonMenuData.bottomWindowId);
        }
        sIsGiftMonMenu = FALSE;

        if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnDestroy)
        {
            struct DynamicListMenuEventArgs eventArgs = {.selectedItem = gSpecialVar_Result, .windowId = taskData[2], .list = NULL};
            sDynamicListMenuEventCollections[sDynamicMenuEventId].OnDestroy(&eventArgs);
        }
        sDynamicMenuEventId = DYN_MULTICHOICE_CB_NONE;

        if (taskData[5] > taskData[7])
            RemoveScrollIndicatorArrowPair(taskData[6]);

        LoadWordFromTwoHalfwords((u16 *)&taskData[3], (u32 *)&items);
        FreeListMenuItems(items, taskData[5]);
        TRY_FREE_AND_SET_NULL(sDynamicMenuEventScratchPad);
        DestroyListMenuTask(taskData[0], NULL, NULL);
        ClearStdWindowAndFrame(taskData[2], TRUE);
        RemoveWindow(taskData[2]);

        // Set up return to field
        gFieldCallback = FieldCB_ContinueScriptHandleMusic;
        SetMainCallback2(CB2_ReturnToField);
        DestroyTask(taskId);
    }
}

bool8 ScriptMenu_MultichoiceDynamic(u8 left, u8 top, u8 argc, struct ListMenuItem *items, bool8 ignoreBPress, u8 maxBeforeScroll, u32 initialRow, u32 callbackSet)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
    {
        FreeListMenuItems(items, argc);
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = 0xFF;
        DrawMultichoiceMenuDynamic(left, top, argc, items, ignoreBPress, initialRow, maxBeforeScroll, callbackSet);
        return TRUE;
    }
}

bool8 ScriptMenu_Multichoice(u8 left, u8 top, u8 multichoiceId, bool8 ignoreBPress)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
    {
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = 0xFF;
        DrawMultichoiceMenu(left, top, multichoiceId, ignoreBPress, 0);
        return TRUE;
    }
}

bool8 ScriptMenu_MultichoiceWithDefault(u8 left, u8 top, u8 multichoiceId, bool8 ignoreBPress, u8 defaultChoice)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
    {
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = 0xFF;
        DrawMultichoiceMenu(left, top, multichoiceId, ignoreBPress, defaultChoice);
        return TRUE;
    }
}

#define sAuxWindowId sDynamicMenuEventScratchPad[0]
#define sItemSpriteId sDynamicMenuEventScratchPad[1]
#define TAG_CB_ITEM_ICON 3000

static void MultichoiceDynamicEventShowItem_OnInit(struct DynamicListMenuEventArgs *eventArgs)
{
    struct WindowTemplate *template = &gWindows[eventArgs->windowId].window;
    u32 baseBlock = template->baseBlock + template->width * template->height;
    struct WindowTemplate auxTemplate = CreateWindowTemplate(0, template->tilemapLeft + template->width + 2, template->tilemapTop, 4, 4, 15, baseBlock);
    u32 auxWindowId = AddWindow(&auxTemplate);
    SetStandardWindowBorderStyle(auxWindowId, FALSE);
    FillWindowPixelBuffer(auxWindowId, 0x11);
    CopyWindowToVram(auxWindowId, COPYWIN_FULL);
    sAuxWindowId = auxWindowId;
    sItemSpriteId = MAX_SPRITES;
}

static void MultichoiceDynamicEventShowItem_OnSelectionChanged(struct DynamicListMenuEventArgs *eventArgs)
{
    struct WindowTemplate *template = &gWindows[eventArgs->windowId].window;
    u32 x = template->tilemapLeft * 8 + template->width * 8 + 36;
    u32 y = template->tilemapTop * 8 + 20;

    if (sItemSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_CB_ITEM_ICON); // This seems to be unused in your provided code, but I'll leave it.
        FreeSpritePaletteByTag(TAG_CB_ITEM_ICON);
        DestroySprite(&gSprites[sItemSpriteId]);
    }

    sItemSpriteId = AddItemIconSprite(TAG_CB_ITEM_ICON, TAG_CB_ITEM_ICON, eventArgs->selectedItem);
    gSprites[sItemSpriteId].oam.priority = 0;
    gSprites[sItemSpriteId].x = x;
    gSprites[sItemSpriteId].y = y;
}

static void MultichoiceDynamicEventShowItem_OnDestroy(struct DynamicListMenuEventArgs *eventArgs)
{
    ClearStdWindowAndFrame(sAuxWindowId, TRUE);
    RemoveWindow(sAuxWindowId);

    if (sItemSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_CB_ITEM_ICON);
        FreeSpritePaletteByTag(TAG_CB_ITEM_ICON);
        DestroySprite(&gSprites[sItemSpriteId]);
    }
}

#undef sAuxWindowId
#undef sItemSpriteId
#undef TAG_CB_ITEM_ICON

static void FreeListMenuItems(struct ListMenuItem *items, u32 count)
{
    u32 i;
    for (i = 0; i < count; ++i)
    {
        // All items were dynamically allocated, so items[i].name is not actually constant.
        Free((void *)items[i].name);
    }
    Free(items);
}

void MultichoiceDynamic_InitStack(u32 capacity)
{
    AGB_ASSERT(sDynamicMultiChoiceStack == NULL);
    sDynamicMultiChoiceStack = AllocZeroed(sizeof(*sDynamicMultiChoiceStack));
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    sDynamicMultiChoiceStack->capacity = capacity;
    sDynamicMultiChoiceStack->top = -1;
    sDynamicMultiChoiceStack->elements = AllocZeroed(capacity * sizeof(struct ListMenuItem));
}

void MultichoiceDynamic_ReallocStack(u32 newCapacity)
{
    struct ListMenuItem *newElements;
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    AGB_ASSERT(sDynamicMultiChoiceStack->capacity < newCapacity);
    newElements = AllocZeroed(newCapacity * sizeof(struct ListMenuItem));
    AGB_ASSERT(newElements != NULL);
    memcpy(newElements, sDynamicMultiChoiceStack->elements, sDynamicMultiChoiceStack->capacity * sizeof(struct ListMenuItem));
    Free(sDynamicMultiChoiceStack->elements);
    sDynamicMultiChoiceStack->elements = newElements;
    sDynamicMultiChoiceStack->capacity = newCapacity;
}

bool32 MultichoiceDynamic_StackFull(void)
{
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    return sDynamicMultiChoiceStack->top == sDynamicMultiChoiceStack->capacity - 1;
}

bool32 MultichoiceDynamic_StackEmpty(void)
{
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    return sDynamicMultiChoiceStack->top == -1;
}

u32 MultichoiceDynamic_StackSize(void)
{
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    return sDynamicMultiChoiceStack->top + 1;
}

void MultichoiceDynamic_PushElement(struct ListMenuItem item)
{
    if (sDynamicMultiChoiceStack == NULL)
        MultichoiceDynamic_InitStack(MULTICHOICE_DYNAMIC_STACK_SIZE);
    if (MultichoiceDynamic_StackFull())
        MultichoiceDynamic_ReallocStack(sDynamicMultiChoiceStack->capacity + MULTICHOICE_DYNAMIC_STACK_INC);
    sDynamicMultiChoiceStack->elements[++sDynamicMultiChoiceStack->top] = item;
}

struct ListMenuItem *MultichoiceDynamic_PopElement(void)
{
    if (sDynamicMultiChoiceStack == NULL)
        return NULL;
    if (MultichoiceDynamic_StackEmpty())
        return NULL;
    return &sDynamicMultiChoiceStack->elements[sDynamicMultiChoiceStack->top--];
}

struct ListMenuItem *MultichoiceDynamic_PeekElement(void)
{
    if (sDynamicMultiChoiceStack == NULL)
        return NULL;
    if (MultichoiceDynamic_StackEmpty())
        return NULL;
    return &sDynamicMultiChoiceStack->elements[sDynamicMultiChoiceStack->top];
}

struct ListMenuItem *MultichoiceDynamic_PeekElementAt(u32 index)
{
    if (sDynamicMultiChoiceStack == NULL)
        return NULL;
    if (sDynamicMultiChoiceStack->top < index)
        return NULL;
    return &sDynamicMultiChoiceStack->elements[index];
}

static void DrawMultichoiceMenuDynamic(u8 left, u8 top, u8 argc, struct ListMenuItem *items, bool8 ignoreBPress, u32 initialRow, u8 maxBeforeScroll, u32 callbackSet)
{
    u32 i;
    s32 width = 0;
    u8 newWidth;
    u8 taskId;
    u32 windowHeight;
    u8 windowId;

for (i = 0; i < argc; ++i)
    {
        width = DisplayTextAndGetWidth(items[i].name, width);
    }

    if (sIsGiftMonMenu)
    {
        sGiftMonMenuData.monSpriteId = MAX_SPRITES;
        sGiftMonMenuData.bottomWindowId = WINDOW_NONE;
        memset(sGiftMonIsTaken, 0, sizeof(sGiftMonIsTaken));
        sGiftMonMenuData.selectedMonSpriteId = 0xFFFF;
        LoadMonIconPalettes();
        memset(sGiftSpriteIds, 0xFF, sizeof(sGiftSpriteIds));
        sGiftMonMenuData.monSpriteId = MAX_SPRITES;
        
        u32 j;
        for (j = 0; j < ARRAY_COUNT(sGiftMonMenuData.monSpriteIds); j++)
            sGiftMonMenuData.monSpriteIds[j] = 0xFFFF;

        struct WindowTemplate template;
        template.bg = 0;
        template.tilemapLeft = 18;
        template.tilemapTop = 2;
        template.width = 8;
        template.height = 8;
        template.paletteNum = 15;
        template.baseBlock = 300;
        sGiftMonMenuData.windowId = AddWindow(&template);
        FillWindowPixelBuffer(sGiftMonMenuData.windowId, PIXEL_FILL(1));
        PutWindowTilemap(sGiftMonMenuData.windowId);
        SetStandardWindowBorderStyle(sGiftMonMenuData.windowId, TRUE);

        template.bg = 0;
        template.tilemapLeft = 14;
        template.tilemapTop = 13;
        template.width = 14;
        template.height = 6;
        // template.paletteNum = 14;
        template.baseBlock = 0x250;
        sGiftMonMenuData.bottomWindowId = AddWindow(&template);
        FillWindowPixelBuffer(sGiftMonMenuData.bottomWindowId, PIXEL_FILL(1));
        // Create the single Pokémon sprite. It will be invisible until the first scroll.
        DrawStdFrameWithCustomTileAndPalette(sGiftMonMenuData.bottomWindowId, TRUE, 0x214, 14);
    }

    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    windowHeight = (argc < maxBeforeScroll) ? argc * 2 : maxBeforeScroll * 2;
    newWidth = ConvertPixelWidthToTileWidth(width);
    left = ScriptMenu_AdjustLeftCoordFromWidth(left, newWidth);
    windowId = CreateWindowFromRect(left, top, newWidth, windowHeight);
    SetStandardWindowBorderStyle(windowId, FALSE);
    CopyWindowToVram(windowId, COPYWIN_FULL);

    sDynamicMenuEventId = callbackSet;
    sDynamicMenuEventScratchPad = AllocZeroed(100 * sizeof(u16));
    if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnInit)
    { // This is never used in the provided code, but it's good practice to keep it safe.
        struct DynamicListMenuEventArgs eventArgs = {.selectedItem = initialRow, .windowId = windowId, .list = NULL};
        sDynamicListMenuEventCollections[sDynamicMenuEventId].OnInit(&eventArgs);
    }

    gMultiuseListMenuTemplate = sScriptableListMenuTemplate;
    gMultiuseListMenuTemplate.windowId = windowId;
    gMultiuseListMenuTemplate.items = items;
    gMultiuseListMenuTemplate.totalItems = argc;
    gMultiuseListMenuTemplate.maxShowed = maxBeforeScroll;
    if (sIsGiftMonMenu)
    {
        gMultiuseListMenuTemplate.itemPrintFunc = GiftMonMenu_ItemPrintFunc;
        gMultiuseListMenuTemplate.scrollMultiple = LIST_MULTIPLE_SCROLL_GIFT;
    }
    else
    {
        gMultiuseListMenuTemplate.itemPrintFunc = NULL;
        gMultiuseListMenuTemplate.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    }

    taskId = CreateTask(Task_HandleScrollingMultichoiceInput, 80);
    gTasks[taskId].data[1] = ignoreBPress;
    gTasks[taskId].data[2] = windowId;
    gTasks[taskId].data[5] = argc;
    gTasks[taskId].data[7] = maxBeforeScroll;
    StoreWordInTwoHalfwords((u16*) &gTasks[taskId].data[3], (u32) items);
    gTasks[taskId].data[0] = ListMenuInit(&gMultiuseListMenuTemplate, 0, initialRow);
    if (argc > maxBeforeScroll)
    {
    // Create Scrolling Arrows
        struct ScrollArrowsTemplate template;
        template.firstX = (newWidth / 2) * 8 + 12 + (left) * 8;
        template.firstY = top * 8 + 5;
        template.secondX = template.firstX;
        template.secondY = top * 8 + windowHeight * 8 + 12;
        template.fullyUpThreshold = 0;
        template.fullyDownThreshold = argc - maxBeforeScroll;
        template.firstArrowType = SCROLL_ARROW_UP;
        template.secondArrowType = SCROLL_ARROW_DOWN;
        template.tileTag = 2000,
        template.palTag = 1,
        template.palNum = 0;

        gTasks[taskId].data[6] = AddScrollIndicatorArrowPair(&template, &gScrollableMultichoice_ScrollOffset);
    }
    ListMenuGetScrollAndRow(gTasks[taskId].data[0], &gScrollableMultichoice_ScrollOffset, NULL);

    if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnSelectionChanged)
    {
        CreateGiftMonSpritesAtPos(initialRow);
    }

    if (sIsGiftMonMenu)
    {
        ListMenuSetLRBtnWrap(gTasks[taskId].data[0], TRUE); // Enables page up/down with L/R
    }
}

void DrawMultichoiceMenuInternal(u8 left, u8 top, u8 multichoiceId, bool8 ignoreBPress, u8 cursorPos, const struct MenuAction *actions, int count)
{
    int i;
    u8 windowId;
    int width = 0;
    u8 newWidth;

    for (i = 0; i < count; i++)
    {
        width = DisplayTextAndGetWidth(actions[i].text, width);
    }

    newWidth = ConvertPixelWidthToTileWidth(width);
    left = ScriptMenu_AdjustLeftCoordFromWidth(left, newWidth);
    windowId = CreateWindowFromRect(left, top, newWidth, count * 2);
    SetStandardWindowBorderStyle(windowId, FALSE);
    PrintMenuTable(windowId, count, actions);
    InitMenuInUpperLeftCornerNormal(windowId, count, cursorPos);
    ScheduleBgCopyTilemapToVram(0);
    InitMultichoiceCheckWrap(ignoreBPress, count, windowId, multichoiceId);
}

void DrawMultichoiceMenu(u8 left, u8 top, u8 multichoiceId, bool8 ignoreBPress, u8 cursorPos)
{
    DrawMultichoiceMenuInternal(left, top, multichoiceId, ignoreBPress, cursorPos, sMultichoiceLists[multichoiceId].list, sMultichoiceLists[multichoiceId].count);
}

#define tLeft           data[0]
#define tTop            data[1]
#define tRight          data[2]
#define tBottom         data[3]
#define tIgnoreBPress   data[4]
#define tDoWrap         data[5]
#define tWindowId       data[6]
#define tMultichoiceId  data[7]

void InitMultichoiceCheckWrap(bool8 ignoreBPress, u8 count, u8 windowId, u8 multichoiceId)
{
    u8 i;
    u8 taskId;
    sProcessInputDelay = 2;

    for (i = 0; i < ARRAY_COUNT(sLinkServicesMultichoiceIds); i++)
    {
        if (sLinkServicesMultichoiceIds[i] == multichoiceId)
        {
            sProcessInputDelay = 12;
        }
    }

    taskId = CreateTask(Task_HandleMultichoiceInput, 80);

    gTasks[taskId].tIgnoreBPress = ignoreBPress;

    if (count > 3)
        gTasks[taskId].tDoWrap = TRUE;
    else
        gTasks[taskId].tDoWrap = FALSE;

    gTasks[taskId].tWindowId = windowId;
    gTasks[taskId].tMultichoiceId = multichoiceId;

    DrawLinkServicesMultichoiceMenu(multichoiceId);
}

static void Task_HandleScrollingMultichoiceInput(u8 taskId)
{
    bool32 done = FALSE;
    s32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);
    switch (input)
    {
    case LIST_HEADER:
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        if (gTasks[taskId].data[1] == 0)
        {
            gSpecialVar_Result = MULTI_B_PRESSED;
            done = TRUE;
        }
        break;


    case LIST_PAGE_UP:
        ListMenuChangeSelection((struct ListMenu *)gTasks[gTasks[taskId].data[0]].data, TRUE, 8, FALSE);
        if (sIsGiftMonMenu)
        {
            PlaySE(SE_DEX_PAGE);
        }
        break;
    case LIST_PAGE_DOWN:
        ListMenuChangeSelection((struct ListMenu *)gTasks[gTasks[taskId].data[0]].data, TRUE, 8, TRUE);
        if (sIsGiftMonMenu)
        {
            PlaySE(SE_DEX_PAGE);
        }
        break;

        break;
    default:
        if (sIsGiftMonMenu && input != 999) // 999 is the "Finished" option
        {
            if ((u16)input == SPECIES_EGG)
            {
                if (CountTakenGiftMons() >= GIFT_POKEMON_COUNT)
                {
                    PlaySE(SE_FAILURE);
                }
                else
                {
                    u16 species = sGiftEggPool[Random() % ARRAY_COUNT(sGiftEggPool)];
                    u8 result = ScriptGiveEgg(species);

                    if (result != MON_CANT_GIVE)
                    {
                        PlaySE(SE_SUCCESS);
                        sGiftMonIsTaken[SPECIES_EGG] = TRUE;
                GiftMonMenu_CreateChosenMonIcon(SPECIES_EGG);
                        RedrawListMenu(gTasks[taskId].data[0]);
                    }
                }
            }
            else if (sGiftMonIsTaken[(u16)input] || CountTakenGiftMons() >= GIFT_POKEMON_COUNT)
            {
                PlaySE(SE_FAILURE);
            }
            else
            {
                u8 result = ScriptGiveMon(input, 15, ITEM_NONE);
                if (result != MON_CANT_GIVE)
                {
                    PlaySE(SE_SUCCESS);
                    sGiftMonIsTaken[(u16)input] = TRUE;
                GiftMonMenu_CreateChosenMonIcon(input);
                    RedrawListMenu(gTasks[taskId].data[0]);
                }
            }
        }
        else
        {
            gSpecialVar_Result = input;
            done = TRUE;
        }
        break;
    }

    if (done)
    {
        PlaySE(SE_SELECT);
        if (sIsGiftMonMenu)
        {
            FadeScreen(FADE_TO_BLACK, 0);
            gTasks[taskId].func = Task_ExitGiftMenu;
        }
        else
        {
            struct ListMenuItem *items;
            if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnDestroy)
            {
                struct DynamicListMenuEventArgs eventArgs = {.selectedItem = gSpecialVar_Result, .windowId = gTasks[taskId].data[2], .list = NULL};
                sDynamicListMenuEventCollections[sDynamicMenuEventId].OnDestroy(&eventArgs);
            }
            sDynamicMenuEventId = DYN_MULTICHOICE_CB_NONE;
            if (gTasks[taskId].data[5] > gTasks[taskId].data[7])
                RemoveScrollIndicatorArrowPair(gTasks[taskId].data[6]);
            LoadWordFromTwoHalfwords((u16*) &gTasks[taskId].data[3], (u32* )(&items));
            FreeListMenuItems(items, gTasks[taskId].data[5]);
            TRY_FREE_AND_SET_NULL(sDynamicMenuEventScratchPad);
            DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
            ClearStdWindowAndFrame(gTasks[taskId].data[2], TRUE);
            RemoveWindow(gTasks[taskId].data[2]);
            ScriptContext_Enable();
            DestroyTask(taskId);
        }
    }
}

static void Task_HandleMultichoiceInput(u8 taskId)
{
    s8 selection;
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        if (sProcessInputDelay)
        {
            sProcessInputDelay--;
        }
        else
        {
            if (!tDoWrap)
                selection = Menu_ProcessInputNoWrap();
            else
                selection = Menu_ProcessInput();

            if (JOY_NEW(DPAD_UP | DPAD_DOWN))
            {
                DrawLinkServicesMultichoiceMenu(tMultichoiceId);
            }

            if (selection != MENU_NOTHING_CHOSEN)
            {
                if (selection == MENU_B_PRESSED)
                {
                    if (tIgnoreBPress)
                        return;
                    PlaySE(SE_SELECT);
                    gSpecialVar_Result = MULTI_B_PRESSED;
                }
                else
                {
                    gSpecialVar_Result = selection;
                }
                ClearToTransparentAndRemoveWindow(tWindowId);
                DestroyTask(taskId);
                ScriptContext_Enable();
            }
        }
    }
}

bool8 ScriptMenu_YesNo(u8 left, u8 top)
{
    if (FuncIsActiveTask(Task_HandleYesNoInput) == TRUE)
    {
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = 0xFF;
        DisplayYesNoMenuDefaultYes();
        CreateTask(Task_HandleYesNoInput, 0x50);
        return TRUE;
    }
}

// Unused
bool8 IsScriptActive(void)
{
    if (gSpecialVar_Result == 0xFF)
        return FALSE;
    else
        return TRUE;
}

static void Task_HandleYesNoInput(u8 taskId)
{
    if (gTasks[taskId].tRight < 5)
    {
        gTasks[taskId].tRight++;
        return;
    }

    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case MENU_NOTHING_CHOSEN:
        return;
    case MENU_B_PRESSED:
    case 1:
        PlaySE(SE_SELECT);
        gSpecialVar_Result = 0;
        break;
    case 0:
        gSpecialVar_Result = 1;
        break;
    }

    DestroyTask(taskId);
    ScriptContext_Enable();
}

bool8 ScriptMenu_MultichoiceGrid(u8 left, u8 top, u8 multichoiceId, bool8 ignoreBPress, u8 columnCount)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceGridInput) == TRUE)
    {
        return FALSE;
    }
    else
    {
        u8 taskId;
        u8 rowCount, newWidth;
        int i, width;

        gSpecialVar_Result = 0xFF;
        width = 0;

        for (i = 0; i < sMultichoiceLists[multichoiceId].count; i++)
        {
            width = DisplayTextAndGetWidth(sMultichoiceLists[multichoiceId].list[i].text, width);
        }

        newWidth = ConvertPixelWidthToTileWidth(width);

        left = ScriptMenu_AdjustLeftCoordFromWidth(left, columnCount * newWidth);
        rowCount = sMultichoiceLists[multichoiceId].count / columnCount;

        taskId = CreateTask(Task_HandleMultichoiceGridInput, 80);

        gTasks[taskId].tIgnoreBPress = ignoreBPress;
        gTasks[taskId].tWindowId = CreateWindowFromRect(left, top, columnCount * newWidth, rowCount * 2);
        SetStandardWindowBorderStyle(gTasks[taskId].tWindowId, FALSE);
        PrintMenuGridTable(gTasks[taskId].tWindowId, newWidth * 8, columnCount, rowCount, sMultichoiceLists[multichoiceId].list);
        InitMenuActionGrid(gTasks[taskId].tWindowId, newWidth * 8, columnCount, rowCount, 0);
        CopyWindowToVram(gTasks[taskId].tWindowId, COPYWIN_FULL);
        return TRUE;
    }
}

static void Task_HandleMultichoiceGridInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s8 selection = Menu_ProcessGridInput();

    switch (selection)
    {
    case MENU_NOTHING_CHOSEN:
        return;
    case MENU_B_PRESSED:
        if (tIgnoreBPress)
            return;
        PlaySE(SE_SELECT);
        gSpecialVar_Result = MULTI_B_PRESSED;
        break;
    default:
        gSpecialVar_Result = selection;
        break;
    }

    ClearToTransparentAndRemoveWindow(tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

#undef tWindowId

bool16 ScriptMenu_CreatePCMultichoice(void)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
    {
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = 0xFF;
        CreatePCMultichoice();
        return TRUE;
    }
}

static void CreatePCMultichoice(void)
{
    u8 x = 8;
    u32 pixelWidth = 0;
    u8 width;
    u8 numChoices;
    u8 windowId;
    int i;

    for (i = 0; i < ARRAY_COUNT(sPCNameStrings); i++)
    {
        pixelWidth = DisplayTextAndGetWidth(sPCNameStrings[i], pixelWidth);
    }

    if (FlagGet(FLAG_SYS_GAME_CLEAR))
    {
        pixelWidth = DisplayTextAndGetWidth(gText_HallOfFame, pixelWidth);
    }

    width = ConvertPixelWidthToTileWidth(pixelWidth);

    // Include Hall of Fame option if player is champion
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
    {
        numChoices = 4;
        windowId = CreateWindowFromRect(0, 0, width, 8);
        SetStandardWindowBorderStyle(windowId, FALSE);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_HallOfFame, x, 33, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_LogOff, x, 49, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        numChoices = 3;
        windowId = CreateWindowFromRect(0, 0, width, 6);
        SetStandardWindowBorderStyle(windowId, FALSE);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_LogOff, x, 33, TEXT_SKIP_DRAW, NULL);
    }

    // Change PC name if player has met Lanette
    if (FlagGet(FLAG_SYS_PC_LANETTE))
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_LanettesPC, x, 1, TEXT_SKIP_DRAW, NULL);
    else
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_SomeonesPC, x, 1, TEXT_SKIP_DRAW, NULL);

    StringExpandPlaceholders(gStringVar4, gText_PlayersPC);
    PrintPlayerNameOnWindow(windowId, gStringVar4, x, 17);
    InitMenuInUpperLeftCornerNormal(windowId, numChoices, 0);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    InitMultichoiceCheckWrap(FALSE, numChoices, windowId, MULTI_PC);
}

void ScriptMenu_DisplayPCStartupPrompt(void)
{
    LoadMessageBoxAndFrameGfx(0, TRUE);
    AddTextPrinterParameterized2(0, FONT_NORMAL, gText_WhichPCShouldBeAccessed, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
}

bool8 ScriptMenu_CreateLilycoveSSTidalMultichoice(void)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
    {
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = 0xFF;
        CreateLilycoveSSTidalMultichoice();
        return TRUE;
    }
}

// gSpecialVar_0x8004 is 1 if the Sailor was shown multiple event tickets at the same time
// otherwise gSpecialVar_0x8004 is 0
static void CreateLilycoveSSTidalMultichoice(void)
{
    u8 selectionCount = 0;
    u8 count;
    u32 pixelWidth;
    u8 width;
    u8 windowId;
    u8 i;
    u32 j;

    for (i = 0; i < SSTIDAL_SELECTION_COUNT; i++)
    {
        sLilycoveSSTidalSelections[i] = 0xFF;
    }

    GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_WIDTH);

    if (gSpecialVar_0x8004 == 0)
    {
        sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_SLATEPORT;
        selectionCount++;

        if (FlagGet(FLAG_MET_SCOTT_ON_SS_TIDAL) == TRUE)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_BATTLE_FRONTIER;
            selectionCount++;
        }
    }

    if (CheckBagHasItem(ITEM_EON_TICKET, 1) == TRUE && FlagGet(FLAG_ENABLE_SHIP_SOUTHERN_ISLAND) == TRUE)
    {
        if (gSpecialVar_0x8004 == 0)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_SOUTHERN_ISLAND;
            selectionCount++;
        }

        if (gSpecialVar_0x8004 == 1 && FlagGet(FLAG_SHOWN_EON_TICKET) == FALSE)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_SOUTHERN_ISLAND;
            selectionCount++;
            FlagSet(FLAG_SHOWN_EON_TICKET);
        }
    }

    if (CheckBagHasItem(ITEM_MYSTIC_TICKET, 1) == TRUE && FlagGet(FLAG_ENABLE_SHIP_NAVEL_ROCK) == TRUE)
    {
        if (gSpecialVar_0x8004 == 0)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_NAVEL_ROCK;
            selectionCount++;
        }

        if (gSpecialVar_0x8004 == 1 && FlagGet(FLAG_SHOWN_MYSTIC_TICKET) == FALSE)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_NAVEL_ROCK;
            selectionCount++;
            FlagSet(FLAG_SHOWN_MYSTIC_TICKET);
        }
    }

    if (CheckBagHasItem(ITEM_AURORA_TICKET, 1) == TRUE && FlagGet(FLAG_ENABLE_SHIP_BIRTH_ISLAND) == TRUE)
    {
        if (gSpecialVar_0x8004 == 0)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_BIRTH_ISLAND;
            selectionCount++;
        }

        if (gSpecialVar_0x8004 == 1 && FlagGet(FLAG_SHOWN_AURORA_TICKET) == FALSE)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_BIRTH_ISLAND;
            selectionCount++;
            FlagSet(FLAG_SHOWN_AURORA_TICKET);
        }
    }

    if (CheckBagHasItem(ITEM_OLD_SEA_MAP, 1) == TRUE && FlagGet(FLAG_ENABLE_SHIP_FARAWAY_ISLAND) == TRUE)
    {
        if (gSpecialVar_0x8004 == 0)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_FARAWAY_ISLAND;
            selectionCount++;
        }

        if (gSpecialVar_0x8004 == 1 && FlagGet(FLAG_SHOWN_OLD_SEA_MAP) == FALSE)
        {
            sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_FARAWAY_ISLAND;
            selectionCount++;
            FlagSet(FLAG_SHOWN_OLD_SEA_MAP);
        }
    }

    sLilycoveSSTidalSelections[selectionCount] = SSTIDAL_SELECTION_EXIT;
    selectionCount++;

    if (gSpecialVar_0x8004 == 0 && FlagGet(FLAG_MET_SCOTT_ON_SS_TIDAL) == TRUE)
    {
        count = selectionCount;
    }

    count = selectionCount;
    if (count == SSTIDAL_SELECTION_COUNT)
    {
        gSpecialVar_0x8004 = SCROLL_MULTI_SS_TIDAL_DESTINATION;
        ShowScrollableMultichoice();
    }
    else
    {
        pixelWidth = 0;

        for (j = 0; j < SSTIDAL_SELECTION_COUNT; j++)
        {
            u8 selection = sLilycoveSSTidalSelections[j];
            if (selection != 0xFF)
            {
                pixelWidth = DisplayTextAndGetWidth(sLilycoveSSTidalDestinations[selection], pixelWidth);
            }
        }

        width = ConvertPixelWidthToTileWidth(pixelWidth);
        windowId = CreateWindowFromRect(MAX_MULTICHOICE_WIDTH - width, (6 - count) * 2, width, count * 2);
        SetStandardWindowBorderStyle(windowId, FALSE);

        for (selectionCount = 0, i = 0; i < SSTIDAL_SELECTION_COUNT; i++)
        {
            if (sLilycoveSSTidalSelections[i] != 0xFF)
            {
                AddTextPrinterParameterized(windowId, FONT_NORMAL, sLilycoveSSTidalDestinations[sLilycoveSSTidalSelections[i]], 8, selectionCount * 16 + 1, TEXT_SKIP_DRAW, NULL);
                selectionCount++;
            }
        }

        InitMenuInUpperLeftCornerNormal(windowId, count, count - 1);
        CopyWindowToVram(windowId, COPYWIN_FULL);
        InitMultichoiceCheckWrap(FALSE, count, windowId, MULTI_SSTIDAL_LILYCOVE);
    }
}

void GetLilycoveSSTidalSelection(void)
{
    if (gSpecialVar_Result != MULTI_B_PRESSED)
    {
        gSpecialVar_Result = sLilycoveSSTidalSelections[gSpecialVar_Result];
    }
}

#define tState       data[0]
#define tMonSpecies  data[1]
#define tMonSpriteId data[2]
#define tWindowX     data[3]
#define tWindowY     data[4]
#define tWindowId    data[5]

static void Task_PokemonPicWindow(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case 0:
        task->tState++;
        break;
    case 1:
        // Wait until state is advanced by ScriptMenu_HidePokemonPic
        break;
    case 2:
        FreeMonSpritesGfx();
        DestroySprite(&gSprites[task->tMonSpriteId]);
        task->tState++;
        break;
    case 3:
        ClearToTransparentAndRemoveWindow(task->tWindowId);
        DestroyTask(taskId);
        break;
    }
}

static bool8 IsPicboxClosed(void)
{
    if (FindTaskIdByFunc(Task_PokemonPicWindow) == TASK_NONE)
        return TRUE;
    else
        return FALSE;
}

bool8 ScriptMenu_ShowPokemonPic(u16 species, u8 x, u8 y)
{
    u8 taskId;
    u8 spriteId;

    if (FindTaskIdByFunc(Task_PokemonPicWindow) != TASK_NONE)
    {
        return FALSE;
    }
    else
    {
        spriteId = CreateMonSprite_PicBox(species, x * 8 + 40, y * 8 + 40, 0);
        taskId = CreateTask(Task_PokemonPicWindow, 0x50);
        {
            // Create the window manually to assign a different baseBlock and avoid VRAM tilemap conflicts.
            // Using baseBlock 300, which is safely away from the multichoice window's baseBlock of 100.
            struct WindowTemplate template = CreateWindowTemplate(0, x + 1, y + 1, 8, 8, 15, 300);
            u8 windowId = AddWindow(&template);
            FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
            PutWindowTilemap(windowId);
            gTasks[taskId].tWindowId = windowId;
        }
        gTasks[taskId].tState = 0;
        gTasks[taskId].tMonSpecies = species;
        gTasks[taskId].tMonSpriteId = spriteId;
        gSprites[spriteId].callback = SpriteCallbackDummy;
        gSprites[spriteId].oam.priority = 0;
        SetStandardWindowBorderStyle(gTasks[taskId].tWindowId, TRUE);
        ScheduleBgCopyTilemapToVram(0);
        return TRUE;
    }
}

bool8 (*ScriptMenu_HidePokemonPic(void))(void)
{
    u8 taskId = FindTaskIdByFunc(Task_PokemonPicWindow);

    if (taskId == TASK_NONE)
        return NULL;
    gTasks[taskId].tState++;
    return IsPicboxClosed;
}

#undef tState
#undef tMonSpecies
#undef tMonSpriteId
#undef tWindowX
#undef tWindowY
#undef tWindowId

u8 CreateWindowFromRect(u8 x, u8 y, u8 width, u8 height)
{
    struct WindowTemplate template = CreateWindowTemplate(0, x + 1, y + 1, width, height, 15, 100);
    u8 windowId = AddWindow(&template);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    PutWindowTilemap(windowId);
    return windowId;
}

void ClearToTransparentAndRemoveWindow(u8 windowId)
{
    ClearStdWindowAndFrameToTransparent(windowId, TRUE);
    RemoveWindow(windowId);
}

static void DrawLinkServicesMultichoiceMenu(u8 multichoiceId)
{
    switch (multichoiceId)
    {
    case MULTI_WIRELESS_NO_BERRY:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sWirelessOptionsNoBerryCrush[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    case MULTI_CABLE_CLUB_WITH_RECORD_MIX:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sCableClubOptions_WithRecordMix[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    case MULTI_WIRELESS_NO_RECORD:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sWirelessOptions_NoRecordMix[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    case MULTI_WIRELESS_ALL_SERVICES:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sWirelessOptions_AllServices[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    case MULTI_WIRELESS_NO_RECORD_BERRY:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sWirelessOptions_NoRecordMixBerryCrush[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    case MULTI_CABLE_CLUB_NO_RECORD_MIX:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sCableClubOptions_NoRecordMix[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    }
}

bool16 ScriptMenu_CreateStartMenuForPokenavTutorial(void)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
    {
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = 0xFF;
        CreateStartMenuForPokenavTutorial();
        return TRUE;
    }
}

static void CreateStartMenuForPokenavTutorial(void)
{
    u8 windowId = CreateWindowFromRect(21, 0, 7, 18);
    SetStandardWindowBorderStyle(windowId, FALSE);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_MenuOptionPokedex, 8, 9, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_MenuOptionPokemon, 8, 25, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_MenuOptionBag, 8, 41, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_MenuOptionPokenav, 8, 57, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gSaveBlock2Ptr->playerName, 8, 73, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_MenuOptionSave, 8, 89, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_MenuOptionOption, 8, 105, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_MenuOptionExit, 8, 121, TEXT_SKIP_DRAW, NULL);
    InitMenuNormal(windowId, FONT_NORMAL, 0, 9, 16, ARRAY_COUNT(MultichoiceList_ForcedStartMenu), 0);
    InitMultichoiceNoWrap(FALSE, ARRAY_COUNT(MultichoiceList_ForcedStartMenu), windowId, MULTI_FORCED_START_MENU);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

#define tWindowId       data[6]

static void InitMultichoiceNoWrap(bool8 ignoreBPress, u8 unusedCount, u8 windowId, u8 multichoiceId)
{
    u8 taskId;
    sProcessInputDelay = 2;
    taskId = CreateTask(Task_HandleMultichoiceInput, 80);
    gTasks[taskId].tIgnoreBPress = ignoreBPress;
    gTasks[taskId].tDoWrap = 0;
    gTasks[taskId].tWindowId = windowId;
    gTasks[taskId].tMultichoiceId = multichoiceId;
}

#undef tLeft
#undef tTop
#undef tRight
#undef tBottom
#undef tIgnoreBPress
#undef tDoWrap
#undef tWindowId
#undef tMultichoiceId

static int DisplayTextAndGetWidthInternal(const u8 *str)
{
    u8 temp[64];
    StringExpandPlaceholders(temp, str);
    return GetStringWidth(FONT_NORMAL, temp, 0);
}

int DisplayTextAndGetWidth(const u8 *str, int prevWidth)
{
    int width = DisplayTextAndGetWidthInternal(str);
    if (width < prevWidth)
    {
        width = prevWidth;
    }
    return width;
}

int ConvertPixelWidthToTileWidth(int width)
{
    return (((width + 9) / 8) + 1) > MAX_MULTICHOICE_WIDTH ? MAX_MULTICHOICE_WIDTH : (((width + 9) / 8) + 1);
}

void MultichoiceDynamic_DestroyStack(void)
{
    TRY_FREE_AND_SET_NULL(sDynamicMultiChoiceStack->elements);
    TRY_FREE_AND_SET_NULL(sDynamicMultiChoiceStack);
}

void ScriptMenu_ClearDynMultichoice(void)
{
    if (sDynamicMultiChoiceStack != NULL)
        MultichoiceDynamic_DestroyStack();
    MultichoiceDynamic_InitStack(MULTICHOICE_DYNAMIC_STACK_SIZE);
}

void ScriptMenu_AddDynmultichoice(const u8 *name, s32 id, UNUSED s32 unused)
{
    struct ListMenuItem item;
    item.name = name;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

int ScriptMenu_AdjustLeftCoordFromWidth(int left, int width)
{
    int adjustedLeft = left;

    if (left + width > MAX_MULTICHOICE_WIDTH)
    {
        if (MAX_MULTICHOICE_WIDTH - width < 0)
        {
            adjustedLeft = 0;
        }
        else
        {
            adjustedLeft = MAX_MULTICHOICE_WIDTH - width;
        }
    }

    return adjustedLeft;
}

void SetIsGiftPokemonMenu(void)
{
    sIsGiftMonMenu = TRUE;
}

static void CreateGiftMonSpritesAtPos(u16 selectedMon)
{
    u16 species;
    struct ListMenuItem *items;
    u8 taskId = FindTaskIdByFunc(Task_HandleScrollingMultichoiceInput);
    
    if (taskId == TASK_NONE)
        return;

    LoadWordFromTwoHalfwords((u16 *)&gTasks[taskId].data[3], (u32 *)&items);

    species = items[selectedMon].id;
    GiftMonMenu_CreateFrontSprite(species);
}
