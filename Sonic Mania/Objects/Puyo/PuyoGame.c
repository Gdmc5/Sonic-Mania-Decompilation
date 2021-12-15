// ---------------------------------------------------------------------
// RSDK Project: Sonic Mania
// Object Description: PuyoGame Object
// Object Author: Christian Whitehead/Simon Thomley/Hunter Bridges
// Decompiled by: Rubberduckycooly & RMGRich
// ---------------------------------------------------------------------

#include "SonicMania.h"
#include <time.h>

ObjectPuyoGame *PuyoGame;

void PuyoGame_Update(void)
{
    RSDK_THIS(PuyoGame);
    if (!self->started) {
        PuyoGame_SetupStartingEntities();
        self->started = true;
    }

    StateMachine_Run(self->state);
    RSDK.ProcessAnimation(&self->animator);

    if ((ControllerInfo->keyStart.press || Unknown_pausePress) && !RSDK_GET_ENTITY(SLOT_PAUSEMENU, PauseMenu)->objectID) {
        RSDK.ResetEntitySlot(SLOT_PAUSEMENU, PauseMenu->objectID, NULL);
        RSDK_GET_ENTITY(SLOT_PAUSEMENU, PauseMenu)->triggerPlayer = 1;
        RSDK.PlaySfx(PauseMenu->sfxAccept, false, 255);
        RSDK.SetGameMode(ENGINESTATE_FROZEN);
    }
}

void PuyoGame_LateUpdate(void) {}

void PuyoGame_StaticUpdate(void) {}

void PuyoGame_Draw(void) {}

void PuyoGame_Create(void *data)
{
    RSDK_THIS(PuyoGame);
    self->active        = ACTIVE_NORMAL;
    self->drawOrder     = Zone->drawOrderLow;
    self->startPos.x    = self->position.x;
    self->startPos.y    = self->position.y;
    self->visible       = false;
    self->drawFX        = FX_FLIP;
    self->updateRange.x = 0x800000;
    self->updateRange.y = 0x800000;
    if (!SceneInfo->inEditor)
        RSDK.AddCamera(&self->position, ScreenInfo->centerX << 16, ScreenInfo->centerY << 16, true);
    self->state = PuyoGame_Unknown6;
}

void PuyoGame_StageLoad(void)
{
    PuyoGame->aniFrames = RSDK.LoadSpriteAnimation("Puyo/PuyoBeans.bin", SCOPE_STAGE);
    foreach_all(FXFade, fxFade) { PuyoGame->fxFade = fxFade; }

    TextInfo buffer;
    Localization_GetString(&buffer, STR_RPC_PLAYING);
    API_SetRichPresence(0, &buffer);
    RSDK.ResetEntitySlot(SLOT_ZONE, TYPE_BLANK, NULL);
}

void PuyoGame_SetupStartingEntities(void)
{
    RSDK_THIS(PuyoGame);
    EntityMenuParam *param            = (EntityMenuParam *)globals->menuParam;
    EntityCompetitionSession *session = (EntityCompetitionSession *)globals->competitionSession;

    ScreenInfo->position.x = (self->position.x >> 16) - ScreenInfo->centerX;
    ScreenInfo->position.y = (self->position.y >> 16) - ScreenInfo->centerY;

    foreach_all(PuyoScore, score)
    {
        int32 playerID = score->playerID;
        if (score->counter) {
            PuyoGame->score2[playerID] = (Entity *)score;
            if (param->selectionFlag == 3)
                score->score = session->wins[playerID];
        }
        else {
            PuyoGame->score1[playerID] = (Entity *)score;
        }
    }

    foreach_all(PuyoLevelSelect, levelSel) { PuyoGame->levelSel[levelSel->playerID] = (Entity *)levelSel; }

    if (param->selectionFlag == 3) {
        self->selectedLevels[0] = 1;
        self->selectedLevels[1] = 1;
        self->state             = PuyoGame_Unknown9;
    }
    else {
        self->state = PuyoGame_Unknown8;
    }
}

void PuyoGame_SetupPlayer(uint8 player)
{
    RSDK_THIS(PuyoGame);
    EntityPuyoMatch *manager           = (EntityPuyoMatch *)PuyoGame->managers[player];
    EntityCollapsingPlatform *platform = (EntityCollapsingPlatform *)PuyoGame->platforms[player];

    int32 foe             = player ^ 1;
    self->field_9C      = 1;
    self->field_A0      = player ^ 1;
    platform->playerPos.x = manager->position.x;
    manager->timer        = 7;
    PuyoAI->value3[0]   = false;
    PuyoAI->value3[1]   = false;

    EntityPuyoMatch *foeManager = (EntityPuyoMatch *)PuyoGame->managers[foe];
    EntityPuyoScore *foeScore   = (EntityPuyoScore *)PuyoGame->score2[foe];
    ++foeScore->score;
    foeScore->flag    = true;
    foeManager->state = StateMachine_None;

    PuyoBean_DestroyPuyoBeans();
    PuyoGame_Unknown5();
    self->timer = 0;
    if (!self->field_98)
        self->state = PuyoGame_Unknown13;
    else
        self->state = PuyoGame_Unknown12;
}

void PuyoGame_DestroyPuyoBeans(void)
{
    foreach_all(PuyoBean, bean) { destroyEntity(bean); }
}

void PuyoGame_Unknown4(void)
{
    int32 fgHighRebuild = RSDK.GetSceneLayerID("FG High Rebuild");
    int32 fgHigh        = RSDK.GetSceneLayerID("FG High");
    RSDK.CopyTileLayer(fgHigh, 0, 0, fgHighRebuild, 0, 0, 32, 16);

    EntityPuyoScore *scoreP1 = (EntityPuyoScore *)PuyoGame->score2[0];
    EntityPuyoScore *scoreP2 = (EntityPuyoScore *)PuyoGame->score2[1];

    PuyoBean->field_20[0] = 0;
    PuyoBean->field_28[0] = 0;
    scoreP1->flag         = false;
    PuyoBean->field_20[1] = 0;
    PuyoBean->field_28[1] = 0;
    scoreP2->flag         = false;

    for (int i = 0; i < 0x100; ++i) {
        PuyoBean->gameState[i] = NULL;
    }

    foreach_all(PuyoMatch, match)
    {
        match->score          = 0;
        match->comboScore     = 0;
        match->comboCount     = 0;
        match->comboBeanCount = 0;
        match->field_80       = PuyoMatch->value2;
        match->field_84       = 0;
        match->field_88       = 0;
        match->timer          = false;
        match->field_90       = 0;
        match->field_94       = 0;

        EntityPuyoScore *score = (EntityPuyoScore *)PuyoGame->score1[0];
        score->score           = match->score;
    }

    foreach_all(CollapsingPlatform, platform) { platform->playerPos.x = 0; }
}

void PuyoGame_Unknown5(void)
{
    RSDK_THIS(PuyoGame);

    EntityPuyoScore *scoreP1 = (EntityPuyoScore *)PuyoGame->score2[0];
    EntityPuyoScore *scoreP2 = (EntityPuyoScore *)PuyoGame->score2[1];

    int score = scoreP1->score;
    if (score > scoreP2->score)
        score = scoreP2->score;

    if (score >= 2) {
        self->field_A1                                                           = scoreP1->score <= scoreP2->score;
        self->field_98                                                           = 1;
        ((EntityPuyoIndicator *)PuyoGame->indicators[self->field_A1])->state     = PuyoIndicator_ShowWinner;
        ((EntityPuyoIndicator *)PuyoGame->indicators[self->field_A1 ^ 1])->state = PuyoIndicator_ShowLoser;
    }
}

void PuyoGame_Unknown6(void)
{
    RSDK_THIS(PuyoGame);
    self->state = PuyoGame_Unknown7;
}

void PuyoGame_Unknown7(void) {}

void PuyoGame_Unknown8(void)
{
    RSDK_THIS(PuyoGame);
    EntityMenuParam *param = (EntityMenuParam *)globals->menuParam;

    EntityPuyoLevelSelect *levelSelP1 = (EntityPuyoLevelSelect *)PuyoGame->levelSel[0];
    EntityPuyoLevelSelect *levelSelP2 = (EntityPuyoLevelSelect *)PuyoGame->levelSel[1];

    levelSelP1->flag = true;
    if (param->selectionFlag == 1 || !param->selectionFlag)
        levelSelP2->ready = true;
    levelSelP2->flag = true;

    if (levelSelP1->ready && levelSelP2->ready) {
        levelSelP1->flag          = false;
        levelSelP2->flag          = false;
        self->selectedLevels[0] = levelSelP1->optionID;
        self->selectedLevels[1] = levelSelP2->optionID;
        self->state             = PuyoGame_Unknown9;
    }
}

void PuyoGame_Unknown9(void)
{
    RSDK_THIS(PuyoGame);

    if (self->timer >= 60) {
        self->timer = 0;
        self->state = PuyoGame_SetupEntities;
    }
    else {
        if (!self->timer) {
            PuyoGame_Unknown4();
            foreach_all(PuyoIndicator, indicator) { indicator->state = PuyoIndicator_ShowReady; }
            int32 key      = (int32)time(NULL);
            int32 matchKey = RSDK.RandSeeded(0, 512, &key);

            foreach_all(PuyoMatch, match)
            {
                RSDK.SetSpriteAnimation(0xFFFF, 0, &match->animator1, true, 0);
                RSDK.SetSpriteAnimation(0xFFFF, 0, &match->animator2, true, 0);
                match->active   = ACTIVE_NORMAL;
                match->matchKey = matchKey;
                match->field_74 = self->selectedLevels[match->playerID];
                PuyoMatch_SetupNextBeans(match);
            }
        }
        ++self->timer;
    }
}

void PuyoGame_SetupEntities(void)
{
    RSDK_THIS(PuyoGame);
    EntityMenuParam *param = (EntityMenuParam *)globals->menuParam;

    self->field_9C = 0;
    self->field_A0 = 0;

    int matchID = 0;
    foreach_all(PuyoMatch, match)
    {
        match->timer  = true;
        match->active = ACTIVE_NORMAL;
        match->state  = PuyoMatch_State_HandleMatch;
        if (match->playerID) {
            if (param->selectionFlag == 1 || !param->selectionFlag) {
                match->stateInput                        = PuyoAI_StateInput;
                PuyoAI->value4[match->playerID]          = 14;
                PuyoAI->targetX[match->playerID]         = 0;
                PuyoAI->value6[match->playerID]          = 0;
                PuyoAI->value3[match->playerID]          = true;
                PuyoAI->controlInterval[match->playerID] = PuyoAI->controlIntervals[self->selectedLevels[0]];
                PuyoAI->controlChance[match->playerID]   = PuyoAI->controlChances[self->selectedLevels[0]];
            }
        }
        PuyoGame->managers[matchID++] = (Entity *)match;
    }

    int indicatorID = 0;
    foreach_all(PuyoIndicator, indicator)
    {
        indicator->state                    = StateMachine_None;
        PuyoGame->indicators[indicatorID++] = (Entity *)indicator;
    }

    int platformID = 0;
    foreach_all(CollapsingPlatform, platform) { PuyoGame->platforms[platformID++] = platform; }

    self->state = PuyoGame_Unknown11;
    PuyoGame_Unknown11();
}

void PuyoGame_Unknown11(void)
{
    for (int p = 0; p < 2; ++p) {
        EntityPuyoMatch *match                          = (EntityPuyoMatch *)PuyoGame->managers[p];
        ((EntityPuyoScore *)PuyoGame->score1[p])->score = match->score;
        if (match->state == PuyoMatch_State_Lose)
            PuyoGame_SetupPlayer(p);
    }
}

void PuyoGame_Unknown12(void)
{
    RSDK_THIS(PuyoGame);
    EntityMenuParam *param = (EntityMenuParam *)globals->menuParam;

    if (self->timer >= 60) {
        int count    = 2 - (param->selectionFlag < 2);
        bool32 flags = false;

        RSDKControllerState *controller = &ControllerInfo[1];
        for (int i = 0; i < count; ++i) {
            flags |= controller[i].keyA.down || controller[i].keyB.down || controller[i].keyC.down || controller[i].keyX.down
                     || controller[i].keyY.down || controller[i].keyZ.down;
        }

        if (flags) {
            PuyoGame_DestroyPuyoBeans();
            self->timer = 0;
            self->state = PuyoGame_Unknown9;
        }
    }
    else {
        self->timer++;
    }
}

void PuyoGame_Unknown13(void)
{
    RSDK_THIS(PuyoGame);
    EntityMenuParam *param = (EntityMenuParam *)globals->menuParam;

    if (self->timer >= 60) {
        int count    = 2 - (param->selectionFlag < 2);
        bool32 flags = false;

        RSDKControllerState *controller = &ControllerInfo[CONT_P1];
        for (int i = 0; i < count; ++i) {
            flags |= controller[i].keyA.down || controller[i].keyB.down || controller[i].keyC.down || controller[i].keyX.down
                     || controller[i].keyY.down || controller[i].keyZ.down;
        }

        if (flags) {
            EntityFXFade *fxFade = PuyoGame->fxFade;
            fxFade->timer        = 0;
            fxFade->speedIn      = 16;
            fxFade->drawOrder    = 15;
            fxFade->state        = FXFade_State_FadeIn;
            self->state        = PuyoGame_Unknown14;
        }
    }
    else {
        self->timer++;
    }
}

void PuyoGame_Unknown14(void)
{
    RSDK_THIS(PuyoGame);
    EntityCompetitionSession *session = (EntityCompetitionSession *)globals->competitionSession;
    EntityMenuParam *param            = (EntityMenuParam *)globals->menuParam;

    EntityFXFade *fxFade = PuyoGame->fxFade;
    if (fxFade->timer == 512) {
        if (param->selectionFlag == 3)
            session->wins[self->field_A1]++;
        destroyEntity(self);
        RSDK.SetScene("Presentation", "Menu");
        RSDK.LoadScene();
    }
}

#if RETRO_INCLUDE_EDITOR
void PuyoGame_EditorDraw(void)
{
    RSDK_THIS(PuyoGame);
    RSDK.SetSpriteAnimation(PuyoGame->aniFrames, 24, &self->animator, false, 0);
    RSDK.DrawSprite(&self->animator, NULL, false);
}

void PuyoGame_EditorLoad(void) { PuyoGame->aniFrames = RSDK.LoadSpriteAnimation("Puyo/PuyoBeans.bin", SCOPE_STAGE); }
#endif

void PuyoGame_Serialize(void) {}
