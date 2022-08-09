#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ItemStack.hpp>
#include <LLAPI.h>
#include <RegCommandAPI.h>
#include <MC/ServerPlayer.hpp>
#include <vector>
#include <MC/BlockPos.hpp>

Logger logger("FishingFestival");
bool fishing = false;

class FishingCommand : public Command {
    enum FishingCommandOP : int
    {
        start = 1,
        stop = 2,
        help = 3

    } op;
    std::string message;
public:
    void execute(CommandOrigin const& ori, CommandOutput& output) const override {
        switch (op)
        {
        case FishingCommand::start:
            fishing = true;
            for (Player* pl : Level::getAllPlayers())
            {
                pl->sendBossEventPacket(BossEvent::Show, "§b钓鱼节", 1, BossEventColour::Blue, 0);
            }
            Level::broadcastText("§e钓鱼节开始了！", TextType::RAW);
            break;
        case FishingCommand::stop:
            fishing = false;
            for (Player* pl : Level::getAllPlayers())
            {
                pl->sendBossEventPacket(BossEvent::Hide, "§b钓鱼节", 1, BossEventColour::Blue, 0);
            }
            Level::broadcastText("§e钓鱼节结束了！", TextType::RAW);
            break;
        case FishingCommand::help:
        {
			output.success("/fishing start - 开始钓鱼节");
			output.success("/fishing stop - 结束钓鱼节");
        }
        default:
            break;
        }
        return;
    }
    static void setup(CommandRegistry* registry) {
        using RegisterCommandHelper::makeMandatory;
        using RegisterCommandHelper::makeOptional;
        registry->registerCommand("fishing", "钓鱼节", CommandPermissionLevel::GameMasters, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
        registry->addEnum<FishingCommandOP>("fishingOP", { {"off", FishingCommandOP::stop} ,{"on", FishingCommandOP::start}, {"help", FishingCommandOP::help} });
        registry->registerOverload<FishingCommand>("fishing", makeMandatory<CommandParameterDataType::ENUM>(&FishingCommand::op, "optional", "fishingOP"));
    }
};

void PluginInit()
{
    LL::registerPlugin("FishingFestival", "钓鱼节", LL::Version(1, 0, 0));
    Event::ServerStartedEvent::subscribe([](const Event::ServerStartedEvent& ev) -> bool {
        return true;
        });
    Event::RegCmdEvent::subscribe([](Event::RegCmdEvent ev) { //register command
        FishingCommand::setup(ev.mCommandRegistry);
        return true;
        });

}

#include <MC/FishingHook.hpp>
#include <random>
#include <MC/ItemActor.hpp>
#include <MC/CompoundItem.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/IntTag.hpp>
#include <MC/Tag.hpp>
using namespace std;
TInstanceHook(void, "?_pullCloser@FishingHook@@IEAAXAEAVActor@@M@Z",
    FishingHook, Actor* a2, float a3) {
    string type = a2->getTypeName();
    if (a2->isItemActor() && fishing)
    {
        Player* pl = this->getPlayerOwner();
        default_random_engine e;
        e.seed(time(0));
        uniform_int_distribution<unsigned> u(1, 100);
        ItemActor* itac = (ItemActor*)a2;
        ItemStack* it = ItemStack::create("minecraft:fish", 1);
        vector<string>a;
        int weight = u(e);
        a.push_back("§r§9重量: " + to_string(weight));
        auto snbt = "{\"Count\":1b, \"Damage\" : 0s, \"Name\" : \"minecraft:cod\", \"WasPickedUp\" : 0b, \"tag\":{\"weight\":" + to_string(weight) + "}}";
        CompoundTag* nbt;
        it->setNbt(nbt->fromSNBT(snbt).get());
        it->setLore(a);
        itac->getItemStack()->setItem(it);
        Level::broadcastText("玩家: " + pl->getRealName() + " 钓上了鱼, 重: " + to_string(weight), TextType::JUKEBOX_POPUP);
        return original(this, itac, a3);
    }
    return original(this, a2, a3);
}