#include "botpch.h"
#include "../../playerbot.h"
#include "KarazhanDungeonTriggers.h"
#include "GenericTriggers.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

using namespace ai;

bool NetherspiteBeamsCheatNeedRefreshTrigger::IsActive()
{
    //Checking that is portal phase
    list<Unit*> creatures;
    MaNGOS::AllCreaturesOfEntryInRangeCheck u_check(bot, 17369, 100);
    MaNGOS::UnitListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatures, u_check);
    Cell::VisitAllObjects(bot, searcher, 100);

    if (creatures.empty())
        return false;

    //Checking that is Netherspite target
    return AI_VALUE2(bool, "has aggro", "current target");
}