/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2017 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config_manager.h"
#include "config_manager_private.h"
#include "../gui/pge_msgbox.h"
#include <common_features/graphics_funcs.h>
#include <common_features/number_limiter.h>
#include <PGE_File_Formats/file_formats.h>
#include <common_features/util.h>
#include <fmt/fmt_format.h>
#include <Utils/files.h>

#include <QFileInfo>
#include <QDir>

/*****Level NPC************/
PGE_DataArray<obj_npc>      ConfigManager::lvl_npc_indexes;
NPC_GlobalSetup             ConfigManager::g_setup_npc;
CustomDirManager            ConfigManager::Dir_NPC;
CustomDirManager            ConfigManager::Dir_NPCScript;
QList<AdvNpcAnimator >      ConfigManager::Animator_NPC;
/*****Level NPC************/

bool ConfigManager::loadLevelNPC(obj_npc &snpc,
                                 std::string section,
                                 obj_npc *merge_with,
                                 std::string iniFile,
                                 IniProcessing *setup)
{
    bool valid = true;
    bool internal = !setup;
    std::string errStr;

    if(internal)
    {
        setup = new IniProcessing(iniFile);
    }

    snpc.isInit = false;
    snpc.image = NULL;
    snpc.textureArrayId = 0;
    snpc.animator_ID = -1;
    setup->beginGroup(section);

    if(snpc.setup.parse(setup, npcPath, default_grid, merge_with ? &merge_with->setup : nullptr, &errStr))
        valid = true;
    else
    {
        addError(errStr);
        valid = false;
    }

    snpc.effect_1_def.fill("stomp", setup);
    snpc.effect_2_def.fill("kick",  setup);
    snpc.block_spawn_type  = setup->value("block-spawn-type",  merge_with ? merge_with->block_spawn_type : 0).toUInt();
    snpc.block_spawn_speed = setup->value("block-spawn-speed", merge_with ? merge_with->block_spawn_speed : 3.0).toDouble();
    snpc.block_spawn_sound = setup->value("block-spawn-sound", merge_with ? merge_with->block_spawn_sound : true).toBool();
    setup->endGroup();

    if(internal) delete setup;

    return valid;
}

bool ConfigManager::loadLevelNPC()
{
    unsigned int i;
    obj_npc snpc;
    unsigned long npc_total = 0;
    bool useDirectory = false;
    PGESTRING npc_ini = config_dirSTD + "lvl_npc.ini";
    PGESTRING nestDir = "";

    if(!Files::fileExists(npc_ini))
    {
        addError("ERROR LOADING lvl_npc.ini: file does not exist");
        PGE_MsgBox msgBox(NULL, PGESTRING("ERROR LOADING lvl_npc.ini: file does not exist"),
                          PGE_MsgBox::msg_fatal);
        msgBox.exec();
        return false;
    }

    IniProcessing npcset(npc_ini);

    lvl_npc_indexes.clear();   //Clear old
    npcset.beginGroup("npc-main");
    npc_total =                  npcset.value("total", 0).toULongLong();
    nestDir =                    npcset.value("config-dir", "").toString();

    if(!nestDir.empty())
    {
        nestDir = config_dirSTD + nestDir;
        useDirectory = true;
    }

    npcset.read("coin-in-block", g_setup_npc.coin_in_block, 10);
    npcset.read("physics-gravity-acceleration", g_setup_npc.phs_gravity_accel, 16.25);
    npcset.read("physics-max-fall-speed", g_setup_npc.phs_max_fall_speed, 8.0);
    npcset.read("effect-lava-burn", g_setup_npc.eff_lava_burn, 13);
    npcset.read("projectile-sound-id", g_setup_npc.projectile_sound_id, 0);
    g_setup_npc.projectile_effect.fill("projectile", &npcset);
    npcset.read("projectile-speed", g_setup_npc.projectile_speed, 10.0);
    npcset.read("talking-sign-image", g_setup_npc.talking_sign_img, "");
    npcset.endGroup();

    if(npc_total == 0)
    {
        PGE_MsgBox::error("ERROR LOADING lvl_npc.ini: number of items not define, or empty config");
        return false;
    }

    lvl_npc_indexes.allocateSlots(npc_total);

    for(i = 1; i <= npc_total; i++)
    {
        if(useDirectory)
        {
            if(!loadLevelNPC(snpc, "npc", nullptr, fmt::format("{0}/npc-{1}.ini", nestDir, i)))
                return false;
        }
        else
        {
            if(!loadLevelNPC(snpc, fmt::format("npc-{0}", i), nullptr, "", &npcset))
                return false;
        }

        snpc.setup.id = i;
        lvl_npc_indexes.storeElement(snpc.setup.id, snpc);
        //Process NPC.txt if possible
        loadNpcTxtConfig(i);

        if(npcset.lastError() != IniProcessing::ERR_OK)
        {
            PGE_MsgBox::fatal(fmt::format("ERROR LOADING lvl_npc.ini N:{0} (npc-{1})", npcset.lastError(), i));
            return false;
        }
    }

    if(lvl_npc_indexes.stored() < npc_total)
        PGE_MsgBox::warn(fmt::format("Not all NPCs loaded! Total: {0}, Loaded: {1})", npc_total, lvl_npc_indexes.stored()));

    return true;
}


void ConfigManager::loadNpcTxtConfig(unsigned long npcID)
{
    NPCConfigFile npcTxt = FileFormats::CreateEmpytNpcTXT();

    if(!lvl_npc_indexes.contains(npcID))
        return;

    obj_npc *npcSetup = &lvl_npc_indexes[npcID];
    PGESTRING file = Dir_NPC.getCustomFile(fmt::format("npc-{0}.txt", npcID));

    if(file.empty())
        return;

    if(!FileFormats::ReadNpcTXTFileF(file, npcTxt, true))
        return;

    PGESTRING image = npcSetup->setup.image_n;

    //Take updated image info
    if(npcTxt.en_image && (!npcTxt.image.empty()) && (npcTxt.image != npcSetup->setup.image_n))
    {
        image = Dir_NPC.getCustomFile(npcTxt.image);
        GraphicsHelps::getImageMetrics(image, &npcSetup->image_size);
    }
    else
    {
        image = Dir_NPC.getCustomFile(npcSetup->setup.image_n);
        GraphicsHelps::getImageMetrics(image, &npcSetup->image_size);
    }

    npcSetup->setup.applyNPCtxt(&npcTxt,
                                npcSetup->setup,
                                static_cast<uint32_t>(npcSetup->image_size.w()),
                                static_cast<uint32_t>(npcSetup->image_size.h()));
}
