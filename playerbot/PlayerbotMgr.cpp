#include "../botpch.h"
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotDbStore.h"
#include "PlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "ServerFacade.h"
#include "TravelMgr.h"


class LoginQueryHolder;
class CharacterHandler;

PlayerbotHolder::PlayerbotHolder() : PlayerbotAIBase()
{
    for (uint32 spellId = 0; spellId < sServerFacade.GetSpellInfoRows(); spellId++)
        sServerFacade.LookupSpellInfo(spellId);
}

PlayerbotHolder::~PlayerbotHolder()
{
}


void PlayerbotHolder::UpdateAIInternal(uint32 elapsed, bool minimal)
{
}

void PlayerbotHolder::UpdateSessions(uint32 elapsed)
{
    for (PlayerBotMap::const_iterator itr = GetPlayerBotsBegin(); itr != GetPlayerBotsEnd(); ++itr)
    {
        Player* const bot = itr->second;
        if (bot->IsBeingTeleported())
        {
            bot->GetPlayerbotAI()->HandleTeleportAck();
        }
        else if (bot->IsInWorld())
        {
            bot->GetSession()->HandleBotPackets();
        }
    }
}

void PlayerbotHolder::LogoutAllBots()
{    
    /*
    while (true)
    {
        PlayerBotMap::const_iterator itr = GetPlayerBotsBegin();
        if (itr == GetPlayerBotsEnd()) break;
        Player* bot= itr->second;
        if (!bot->GetPlayerbotAI()->isRealPlayer())
            LogoutPlayerBot(bot->GetObjectGuid().GetRawValue());
    }
    */    
    PlayerBotMap bots = playerBots;

    for (auto& itr : bots)
    {
        Player* bot = itr.second;
        if (!bot)
            continue;

        if (!bot->GetPlayerbotAI() || bot->GetPlayerbotAI()->IsRealPlayer())
            continue;

        LogoutPlayerBot(bot->GetObjectGuid().GetRawValue());
    }
}

void PlayerbotHolder::LogoutPlayerBot(uint64 guid)
{
    Player* bot = GetPlayerBot(guid);
    if (bot)
    {
        bot->GetPlayerbotAI()->TellMaster("Goodbye!");
        Group *group = bot->GetGroup();
        if (group && !bot->InBattleGround() && !bot->InBattleGroundQueue() && bot->GetPlayerbotAI()->HasActivePlayerMaster())
        {
            sPlayerbotDbStore.Save(bot->GetPlayerbotAI());
        }
        sLog.outDebug("Bot %s logged out", bot->GetName());
        bot->SaveToDB();

        if (bot->GetPlayerbotAI()->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            TravelTarget* target = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
            if (target)
                delete target;
        }

        WorldSession * botWorldSessionPtr = bot->GetSession();
        playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap
#ifdef CMANGOS
        botWorldSessionPtr->LogoutPlayer(); // this will delete the bot Player object and PlayerbotAI object
#endif
#ifdef MANGOS
        botWorldSessionPtr->LogoutPlayer(true); // this will delete the bot Player object and PlayerbotAI object
#endif
        delete botWorldSessionPtr;  // finally delete the bot's WorldSession
    }
}

void PlayerbotHolder::DisablePlayerBot(uint64 guid)
{
    Player* bot = GetPlayerBot(guid);
    if (bot)
    {
        bot->GetPlayerbotAI()->TellMaster("Goodbye!");
        bot->StopMoving();
        MotionMaster& mm = *bot->GetMotionMaster();
        mm.Clear();

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleGround() && !bot->InBattleGroundQueue() && bot->GetPlayerbotAI()->HasActivePlayerMaster())
        {
            sPlayerbotDbStore.Save(bot->GetPlayerbotAI());
        }
        sLog.outDebug("Bot %s logged out", bot->GetName());
        bot->SaveToDB();

        if (bot->GetPlayerbotAI()->GetAiObjectContext()) //Maybe some day re-write to delate all pointer values.
        {
            TravelTarget* target = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
            if (target)
                delete target;
        }

        WorldSession* botWorldSessionPtr = bot->GetSession();
        playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap

        if (bot->GetPlayerbotAI()) {
            {
                delete bot->GetPlayerbotAI();
            }
            bot->SetPlayerbotAI(0);
        }
    }
}

Player* PlayerbotHolder::GetPlayerBot(uint64 playerGuid) const
{
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

class ViewPointAccess
{
public:
    friend class Camera;

    typedef std::list<Camera*> CameraList;

    CameraList m_cameras;
    GridType* m_grid;

    void Attach(Camera* c) { m_cameras.push_back(c); }
    void Detach(Camera* c) { m_cameras.remove(c); }
};

void PlayerbotHolder::OnBotLogin(Player * const bot)
{
	PlayerbotAI* ai = new PlayerbotAI(bot);
	bot->SetPlayerbotAI(ai);
	OnBotLoginInternal(bot);

    playerBots[bot->GetObjectGuid().GetRawValue()] = bot;


    Player* master = ai->GetMaster();

    if (master != bot)
    {
        ViewPoint* view = &bot->GetViewPoint();
        ViewPointAccess* viewAccess = reinterpret_cast<ViewPointAccess*>(view);

        viewAccess->m_cameras.clear();
    }

    if (master)
    {
        ObjectGuid masterGuid = master->GetObjectGuid();
        if (master->GetGroup() && !master->GetGroup()->IsLeader(masterGuid))
            master->GetGroup()->ChangeLeader(masterGuid);
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        bool groupValid = false;
        Group::MemberSlotList const& slots = group->GetMemberSlots();
        for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
        {
            ObjectGuid member = i->guid;
            
            if (master)
            {
                if (master->GetObjectGuid() == member)
                {
                    groupValid = true;
                    break;
                }
            }
            else
            {
                uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(member);
                if (!sPlayerbotAIConfig.IsInRandomAccountList(account))
                {
                    groupValid = true;
                    break;
                }
            }
        }

        if (!groupValid)
        {
            WorldPacket p;
            string member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
            bot->GetSession()->HandleGroupDisbandOpcode(p);
        }
    }

    group = bot->GetGroup();
    if (group)
    {
        ai->ResetStrategies();
    }
    else
    {
        ai->ResetStrategies(!sRandomPlayerbotMgr.IsRandomBot(bot->GetGUIDLow()));
    }

    if (master && !master->IsTaxiFlying())
    {
        bot->GetMotionMaster()->MovementExpired();
#ifdef MANGOS
        bot->m_taxi.ClearTaxiDestinations();
#endif
    }

    // check activity
    ai->AllowActivity(ALL_ACTIVITY, true);
    // set delay on login
    ai->SetNextCheckDelay(urand(1000, 2000));

    ai->TellMaster("Hello!");
}

string PlayerbotHolder::ProcessBotCommand(string cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId)
{
    if (!sPlayerbotAIConfig.enabled || guid.IsEmpty())
        return "bot system is disabled";

    uint32 botAccount = sObjectMgr.GetPlayerAccountIdByGUID(guid);
    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(guid);
    bool isRandomAccount = sPlayerbotAIConfig.IsInRandomAccountList(botAccount);
    bool isMasterAccount = (masterAccountId == botAccount);

    if (!isRandomAccount && !isMasterAccount && !admin && masterguid)
    {
        Player* master = sObjectMgr.GetPlayer(masterguid);
        if (master && (!sPlayerbotAIConfig.allowGuildBots || !masterGuildId || (masterGuildId && master->GetGuildIdFromDB(guid) != masterGuildId)))
            return "not in your guild or account";
    }

    if (cmd == "add" || cmd == "login")
    {
        if (sObjectMgr.GetPlayer(guid))
            return "player already logged in";

        AddPlayerBot(guid.GetRawValue(), masterAccountId);
        return "ok";
    }
    else if (cmd == "remove" || cmd == "logout" || cmd == "rm")
    {
        if (!sObjectMgr.GetPlayer(guid))
            return "player is offline";

        if (!GetPlayerBot(guid.GetRawValue()))
            return "not your bot";

        LogoutPlayerBot(guid.GetRawValue());
        return "ok";
    }

    if (admin)
    {
        Player* bot = GetPlayerBot(guid.GetRawValue());
        if (!bot) bot = sRandomPlayerbotMgr.GetPlayerBot(guid.GetRawValue());
        if (!bot)
            return "bot not found";

        Player* master = bot->GetPlayerbotAI()->GetMaster();
        if (master)
        {
            if (cmd == "init=white" || cmd == "init=common")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_NORMAL);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=green" || cmd == "init=uncommon")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_UNCOMMON);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=blue" || cmd == "init=rare")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=epic" || cmd == "init=purple")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false);
                return "ok";
            }
            else if (cmd == "init=legendary" || cmd == "init=yellow")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false);
                return "ok";
            }
        }

        if (cmd == "levelup" || cmd == "level")
        {
            PlayerbotFactory factory(bot, bot->getLevel());
            factory.Randomize(true);
            return "ok";
        }
        else if (cmd == "refresh")
        {
            PlayerbotFactory factory(bot, bot->getLevel());
            factory.Refresh();
            return "ok";
        }
        else if (cmd == "random")
        {
            sRandomPlayerbotMgr.Randomize(bot);
            return "ok";
        }
    }

    return "unknown command";
}

bool PlayerbotMgr::HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
{
	if (!sPlayerbotAIConfig.enabled)
	{
		handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
        return false;
	}

    WorldSession *m_session = handler->GetSession();

    if (!m_session)
    {
        handler->PSendSysMessage("You may only add bots from an active session");
        return false;
    }

    Player* player = m_session->GetPlayer();
    PlayerbotMgr* mgr = player->GetPlayerbotMgr();
    if (!mgr)
    {
        handler->PSendSysMessage("you cannot control bots yet");
        return false;
    }

    list<string> messages = mgr->HandlePlayerbotCommand(args, player);
    if (messages.empty())
        return true;

    for (list<string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage("%s",i->c_str());
    }

    return true;
}

list<string> PlayerbotHolder::HandlePlayerbotCommand(char const* args, Player* master)
{
    list<string> messages;

    if (!*args)
    {
        messages.push_back("usage: list/reload or add/init/remove PLAYERNAME");
        return messages;
    }

    char *cmd = strtok ((char*)args, " ");
    char *charname = strtok (NULL, " ");
    if (!cmd)
    {
        messages.push_back("usage: list/reload or add/init/remove PLAYERNAME");
        return messages;
    }

    if (!strcmp(cmd, "list"))
    {
        messages.push_back(ListBots(master));
        return messages;
    }

    if (!strcmp(cmd, "reload"))
    {
        messages.push_back("Reloading config");
        sPlayerbotAIConfig.Initialize();
        return messages;
    }   

    if (!strcmp(cmd, "tweak"))
    {
        sPlayerbotAIConfig.tweakValue = sPlayerbotAIConfig.tweakValue++;
        if (sPlayerbotAIConfig.tweakValue > 2)
            sPlayerbotAIConfig.tweakValue = 0;

        messages.push_back("Set tweakvalue to " + to_string(sPlayerbotAIConfig.tweakValue));
        return messages;
    }

    if (!strcmp(cmd, "self"))
    {
        if (master->GetPlayerbotAI())
        {
            messages.push_back("Disable player ai");
            DisablePlayerBot(master->GetObjectGuid().GetRawValue());
        }
        else if (sPlayerbotAIConfig.selfBotLevel == 0)
            messages.push_back("Self-bot is disabled");
        else if (sPlayerbotAIConfig.selfBotLevel == 1 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            messages.push_back("You do not have permission to enable player ai");
        else
        {
            messages.push_back("Enable player ai");
            OnBotLogin(master);
        }
       return messages;
     }

    if (!charname)
    {
        messages.push_back("usage: list or add/init/remove PLAYERNAME");
        return messages;
    }

    std::string cmdStr = cmd;
    std::string charnameStr = charname;

    set<string> bots;
    if (charnameStr == "*" && master)
    {
        Group* group = master->GetGroup();
        if (!group)
        {
            messages.push_back("you must be in group");
            return messages;
        }

        Group::MemberSlotList slots = group->GetMemberSlots();
        for (Group::member_citerator i = slots.begin(); i != slots.end(); i++)
        {
			ObjectGuid member = i->guid;

			if (member.GetRawValue() == master->GetObjectGuid().GetRawValue())
				continue;

			string bot;
			if (sObjectMgr.GetPlayerNameByGUID(member, bot))
			    bots.insert(bot);
        }
    }

    if (charnameStr == "!" && master && master->GetSession()->GetSecurity() > SEC_GAMEMASTER)
    {
        for (PlayerBotMap::const_iterator i = GetPlayerBotsBegin(); i != GetPlayerBotsEnd(); ++i)
        {
            Player* bot = i->second;
            if (bot && bot->IsInWorld())
                bots.insert(bot->GetName());
        }
    }

    vector<string> chars = split(charnameStr, ',');
    for (vector<string>::iterator i = chars.begin(); i != chars.end(); i++)
    {
        string s = *i;

        uint32 accountId = GetAccountId(s);
        if (!accountId)
        {
            bots.insert(s);
            continue;
        }

        QueryResult* results = CharacterDatabase.PQuery(
            "SELECT name FROM characters WHERE account = '%u'",
            accountId);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                string charName = fields[0].GetString();
                bots.insert(charName);
            } while (results->NextRow());

			delete results;
        }
	}

    for (set<string>::iterator i = bots.begin(); i != bots.end(); ++i)
    {
        string bot = *i;
        ostringstream out;
        out << cmdStr << ": " << bot << " - ";

        ObjectGuid member = sObjectMgr.GetPlayerGuidByName(bot);
        if (!member)
        {
            out << "character not found";
        }
        else if (master && member.GetRawValue() != master->GetObjectGuid().GetRawValue())
        {
            out << ProcessBotCommand(cmdStr, member,
                    master->GetObjectGuid(),
                    master->GetSession()->GetSecurity() >= SEC_GAMEMASTER,
                    master->GetSession()->GetAccountId(),
                    master->GetGuildId());
        }
        else if (!master)
        {
            out << ProcessBotCommand(cmdStr, member, ObjectGuid(), true, -1, -1);
        }

        messages.push_back(out.str());
    }

    return messages;
}

uint32 PlayerbotHolder::GetAccountId(string name)
{
    uint32 accountId = 0;

    QueryResult* results = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", name.c_str());
    if(results)
    {
        Field* fields = results->Fetch();
        accountId = fields[0].GetUInt32();
		delete results;
    }

    return accountId;
}

string PlayerbotHolder::ListBots(Player* master)
{
    set<string> bots;
    map<uint8,string> classNames;
    classNames[CLASS_DRUID] = "Druid";
    classNames[CLASS_HUNTER] = "Hunter";
    classNames[CLASS_MAGE] = "Mage";
    classNames[CLASS_PALADIN] = "Paladin";
    classNames[CLASS_PRIEST] = "Priest";
    classNames[CLASS_ROGUE] = "Rogue";
    classNames[CLASS_SHAMAN] = "Shaman";
    classNames[CLASS_WARLOCK] = "Warlock";
    classNames[CLASS_WARRIOR] = "Warrior";
#ifdef MANGOSBOT_TWO
    classNames[CLASS_DEATH_KNIGHT] = "DeathKnight";
#endif

    map<string, string> online;
    list<string> names;
    map<string, string> classes;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        string name = bot->GetName();
        bots.insert(name);

        names.push_back(name);
        online[name] = "+";
        classes[name] = classNames[bot->getClass()];
    }

    if (master)
    {
        QueryResult* results = CharacterDatabase.PQuery("SELECT class,name FROM characters where account = '%u'",
                master->GetSession()->GetAccountId());
        if (results != NULL)
        {
            do
            {
                Field* fields = results->Fetch();
                uint8 cls = fields[0].GetUInt8();
                string name = fields[1].GetString();
                if (bots.find(name) == bots.end() && name != master->GetSession()->GetPlayerName())
                {
                    names.push_back(name);
                    online[name] = "-";
                    classes[name] = classNames[cls];
                }
            } while (results->NextRow());
			delete results;
        }
    }

    names.sort();

    Group* group = master->GetGroup();
    if (group)
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player *member = sObjectMgr.GetPlayer(itr->guid);
            if (member && sRandomPlayerbotMgr.IsRandomBot(member))
            {
                string name = member->GetName();

                names.push_back(name);
                online[name] = "+";
                classes[name] = classNames[member->getClass()];
            }
        }
    }

    ostringstream out;
    bool first = true;
    out << "Bot roster: ";
    for (list<string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        if (first) first = false; else out << ", ";
        string name = *i;
        out << online[name] << name << " " << classes[name];
    }

    return out.str();
}


PlayerbotMgr::PlayerbotMgr(Player* const master) : PlayerbotHolder(),  master(master), lastErrorTell(0)
{
}

PlayerbotMgr::~PlayerbotMgr()
{
}

void PlayerbotMgr::UpdateAIInternal(uint32 elapsed)
{
    SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
    CheckTellErrors(elapsed);
}

void PlayerbotMgr::HandleCommand(uint32 type, const string& text)
{
    Player *master = GetMaster();
    if (!master)
        return;

    if (text.find(sPlayerbotAIConfig.commandSeparator) != string::npos)
    {
        vector<string> commands;
        split(commands, text, sPlayerbotAIConfig.commandSeparator.c_str());
        for (vector<string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i);
        }
        return;
    }

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleCommand(type, text, *master);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == master)
            bot->GetPlayerbotAI()->HandleCommand(type, text, *master);
    }
}

void PlayerbotMgr::HandleMasterIncomingPacket(const WorldPacket& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    switch (packet.GetOpcode())
    {
        // if master is logging out, log out all bots
        case CMSG_LOGOUT_REQUEST:
        {
            LogoutAllBots();
            return;
        }
    }
}
void PlayerbotMgr::HandleMasterOutgoingPacket(const WorldPacket& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }

    if (GetMaster()->GetGroup())
    {
        Group::MemberSlotList const& groupSlot = GetMaster()->GetGroup()->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* bot = sObjectMgr.GetPlayer(itr->guid);
            if (bot && bot->GetPlayerbotAI() && bot->GetPlayerbotAI()->GetMaster() == GetMaster())
                bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
        }
    }

    /*
    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }
    */
}

void PlayerbotMgr::SaveToDB()
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->SaveToDB();
    }
    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->SaveToDB();
    }
}

void PlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    bot->GetPlayerbotAI()->SetMaster(master);
    bot->GetPlayerbotAI()->ResetStrategies();
    sLog.outDebug("Bot %s logged in", bot->GetName());
}

void PlayerbotMgr::OnPlayerLogin(Player* player)
{
    if(sPlayerbotAIConfig.selfBotLevel > 2)
        HandlePlayerbotCommand("self", player);

    if (!sPlayerbotAIConfig.botAutologin)
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    QueryResult* results = CharacterDatabase.PQuery(
        "SELECT name FROM characters WHERE account = '%u'",
        accountId);
    if (results)
    {
        ostringstream out; out << "add ";
        bool first = true;
        do
        {
            Field* fields = results->Fetch();
            if (first) first = false; else out << ",";
            out << fields[0].GetString();
        } while (results->NextRow());

        delete results;

        HandlePlayerbotCommand(out.str().c_str(), player);
    }
}

void PlayerbotMgr::TellError(string botName, string text)
{
    set<string> names = errors[text];
    if (names.find(botName) == names.end())
    {
        names.insert(botName);
    }
    errors[text] = names;
}

void PlayerbotMgr::CheckTellErrors(uint32 elapsed)
{
    time_t now = time(0);
    if ((now - lastErrorTell) < sPlayerbotAIConfig.errorDelay / 1000)
        return;

    lastErrorTell = now;

    for (PlayerBotErrorMap::iterator i = errors.begin(); i != errors.end(); ++i)
    {
        string text = i->first;
        set<string> names = i->second;

        ostringstream out;
        bool first = true;
        for (set<string>::iterator j = names.begin(); j != names.end(); ++j)
        {
            if (!first) out << ", "; else first = false;
            out << *j;
        }
        out << "|cfff00000: " << text;
        
        ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }
    errors.clear();
}
