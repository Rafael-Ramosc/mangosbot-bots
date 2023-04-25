#pragma once
#include "../generic/ClassStrategy.h"

namespace ai
{
    class RogueAoePlaceholderStrategy : public AoePlaceholderStrategy
    {
    public:
        RogueAoePlaceholderStrategy(PlayerbotAI* ai) : AoePlaceholderStrategy(ai) {}
        string getName() override { return "aoe"; }
    };

    class RogueCcPlaceholderStrategy : public CcPlaceholderStrategy
    {
    public:
        RogueCcPlaceholderStrategy(PlayerbotAI* ai) : CcPlaceholderStrategy(ai) {}
        string getName() override { return "cc"; }
    };

    class RogueStealthPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        RogueStealthPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        string getName() override { return "stealth"; }
    };

    class RogueBuffPlaceholderStrategy : public BuffPlaceholderStrategy
    {
    public:
        RogueBuffPlaceholderStrategy(PlayerbotAI* ai) : BuffPlaceholderStrategy(ai) {}
        string getName() override { return "buff"; }
    };

    class RogueStealthedStrategy : public Strategy
    {
    public:
        RogueStealthedStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        string getName() override { return "stealthed"; }

    private:
        void InitCombatMultipliers(std::list<Multiplier *>& multipliers) override;
        void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class RogueStealthedStrategyMultiplier : public Multiplier
    {
    public:
        RogueStealthedStrategyMultiplier(PlayerbotAI* ai) : Multiplier(ai, "stealthed") {}
        float GetValue(Action* action) override;
    };

    class RoguePoisonsPlaceholderStrategy : public PlaceholderStrategy
    {
    public:
        RoguePoisonsPlaceholderStrategy(PlayerbotAI* ai) : PlaceholderStrategy(ai) {}
        string getName() override { return "poisons"; }
    };

    class RogueStrategy : public ClassStrategy
    {
    public:
        RogueStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        
        virtual NextAction** GetDefaultCombatActions() override;
    };

    class RoguePvpStrategy : public ClassPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitReactionTriggers(std::list<TriggerNode*>& triggers);
        static void InitDeadTriggers(std::list<TriggerNode*>& triggers);
    };

    class RoguePveStrategy : public ClassPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitReactionTriggers(std::list<TriggerNode*>& triggers);
        static void InitDeadTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueRaidStrategy : public ClassRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitReactionTriggers(std::list<TriggerNode*>& triggers);
        static void InitDeadTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueAoeStrategy : public AoeStrategy
    {
    public:
        RogueAoeStrategy(PlayerbotAI* ai) : AoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RogueAoePvpStrategy : public AoePvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueAoePveStrategy : public AoePveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueAoeRaidStrategy : public AoeRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueBuffStrategy : public BuffStrategy
    {
    public:
        RogueBuffStrategy(PlayerbotAI* ai) : BuffStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RogueBuffPvpStrategy : public BuffPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueBuffPveStrategy : public BuffPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueBuffRaidStrategy : public BuffRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueCcStrategy : public CcStrategy
    {
    public:
        RogueCcStrategy(PlayerbotAI* ai) : CcStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RogueCcPvpStrategy : public CcPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueCcPveStrategy : public CcPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueCcRaidStrategy : public CcRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueStealthStrategy : public Strategy
    {
    public:
        RogueStealthStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RogueStealthPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueStealthPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RogueStealthRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RoguePoisonsStrategy : public AoeStrategy
    {
    public:
        RoguePoisonsStrategy(PlayerbotAI* ai) : AoeStrategy(ai) {}

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RoguePoisonsPvpStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RoguePoisonsPveStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    class RoguePoisonsRaidStrategy
    {
    public:
        static void InitCombatTriggers(std::list<TriggerNode*>& triggers);
        static void InitNonCombatTriggers(std::list<TriggerNode*>& triggers);
    };

    /*
    class RogueDpsStrategy : public RogueStrategy
    {
    public:
        RogueDpsStrategy(PlayerbotAI* ai) : RogueStrategy(ai) {}
        string getName() override { return "dps"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class AssassinationRogueStrategy : public RogueStrategy
    {
    public:
        AssassinationRogueStrategy(PlayerbotAI* ai) : RogueStrategy(ai) {}
        string getName() override { return "assassin"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class CombatRogueStrategy : public RogueStrategy
    {
    public:
        CombatRogueStrategy(PlayerbotAI* ai) : RogueStrategy(ai) {}
        string getName() override { return "combat"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SubtletyRogueStrategy : public RogueStrategy
    {
    public:
        SubtletyRogueStrategy(PlayerbotAI* ai) : RogueStrategy(ai) {}
        string getName() override { return "subtlety"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class StealthedRogueStrategy : public CombatStrategy
    {
    public:
        StealthedRogueStrategy(PlayerbotAI* ai);
        string getName() override { return "stealthed"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class StealthStrategy : public Strategy
    {
    public:
        StealthStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        string getName() override { return "stealth"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class RogueBoostStrategy : public Strategy
    {
    public:
        RogueBoostStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        string getName() override { return "boost"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class RogueCcStrategy : public Strategy
    {
    public:
        RogueCcStrategy(PlayerbotAI* ai);
        string getName() override { return "cc"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
    */
}