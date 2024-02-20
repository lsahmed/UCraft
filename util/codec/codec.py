#!/usr/bin/env python3
from nbt.nbt import *
import gzip

nbtfile = NBTFile(TAG_END)
dimension_type = TAG_Compound(name="dimension_type")
dimension_type.tags.append(TAG_String(name="type", value="dimension_type"))
dimensions = TAG_List(name="value", type=TAG_Compound)

dimension = TAG_Compound()
dimension.tags.append(TAG_String(name="name", value="overworld"))
dimension.tags.append(TAG_Int(name="id", value=0))
dimension_attr = TAG_Compound(name="element")
dimension_attr.tags.append(TAG_Byte(name="piglin_safe", value=0))
dimension_attr.tags.append(TAG_Int(name="monster_spawn_light_level", value=15))
dimension_attr.tags.append(TAG_Int(name="monster_spawn_block_light_limit", value=15))
dimension_attr.tags.append(TAG_Byte(name="natural", value=1))
dimension_attr.tags.append(TAG_Float(name="ambient_light", value=0.0))
dimension_attr.tags.append(TAG_String(name="infiniburn", value="#infiniburn_overworld"))
dimension_attr.tags.append(TAG_Byte(name="respawn_anchor_works", value=0))
dimension_attr.tags.append(TAG_Byte(name="has_skylight", value=1))
dimension_attr.tags.append(TAG_Byte(name="bed_works", value=1))
dimension_attr.tags.append(TAG_String(name="effects", value="overworld"))
dimension_attr.tags.append(TAG_Byte(name="has_raids", value=1))
dimension_attr.tags.append(TAG_Int(name="logical_height", value=384))
dimension_attr.tags.append(TAG_Float(name="coordinate_scale", value=1.0))
dimension_attr.tags.append(TAG_Int(name="min_y", value=-64))
dimension_attr.tags.append(TAG_Byte(name="ultrawarm", value=0))
dimension_attr.tags.append(TAG_Byte(name="has_ceiling", value=0))
dimension_attr.tags.append(TAG_Int(name="height", value=384))

biome = TAG_Compound(name="worldgen/biome")
biome.tags.append(TAG_String(name="type", value="worldgen/biome"))
biome_types = TAG_List(name="value", type=TAG_Compound)

plains_biome = TAG_Compound()
plains_biome.tags.append(TAG_String(name="name", value="plains"))
plains_biome.tags.append(TAG_Int(name="id", value=0))
plains_attr = TAG_Compound(name="element")
# plains_attr.tags.append(TAG_String(name="precipitation",value= "rain"))
plains_attr.tags.append(TAG_Byte(name="has_precipitation", value=1))
plains_attr.tags.append(TAG_Float(name="depth", value=0.125))
plains_attr.tags.append(TAG_Float(name="temperature", value=0.800000011920929))
plains_attr.tags.append(TAG_Float(name="scale", value=0.05000000074505806))
plains_attr.tags.append(TAG_Float(name="downfall", value=0.4000000059604645))
plains_attr.tags.append(TAG_String(name="category", value="plains"))
plains_attr_effects = TAG_Compound(name="effects")
plains_attr_effects.tags.append(TAG_Int(name="sky_color", value=7907327))
plains_attr_effects.tags.append(TAG_Int(name="water_fog_color", value=329011))
plains_attr_effects.tags.append(TAG_Int(name="water_color", value=0))
plains_attr_effects.tags.append(TAG_Int(name="fog_color", value=12638463))
plains_attr_effects_mood = TAG_Compound(name="mood_sound")
plains_attr_effects_mood.tags.append(TAG_Double(name="offset", value=2.0))
plains_attr_effects_mood.tags.append(TAG_Int(name="tick_delay", value=6000))
plains_attr_effects_mood.tags.append(TAG_String(name="sound", value="ambient.cave"))
plains_attr_effects_mood.tags.append(TAG_Int(name="block_search_extent", value=8))
plains_attr_effects.tags.append(plains_attr_effects_mood)
plains_attr.tags.append(plains_attr_effects)

damage_type = TAG_Compound(name="damage_type")
damage_type.tags.append(TAG_String(name="type", value="damage_type"))
damage_type_value = TAG_List(name="value", type=TAG_Compound)
damage_type_entry = TAG_Compound()
##these are required for 1.19.4
damage_types = [
    "in_fire",
    "lightning_bolt",
    "on_fire",
    "lava",
    "hot_floor",
    "in_wall",
    "cramming",
    "drown",
    "starve",
    "cactus",
    "fall",
    "fly_into_wall",
    "out_of_world",
    "generic",
    "magic",
    "wither",
    "dragon_breath",
    "dry_out",
    "sweet_berry_bush",
    "freeze",
    "stalagmite",
    "falling_block",
    "falling_anvil",
    "falling_stalactite",
    "sting",
    "mob_attack",
    "mob_attack_no_aggro",
    "player_attack",
    "arrow",
    "trident",
    "mob_projectile",
    "fireworks",
    "fireball",
    "unattributed_fireball",
    "wither_skull",
    "thrown",
    "indirect_magic",
    "thorns",
    "explosion",
    "player_explosion",
    "sonic_boom",
    "bad_reswpawn_point",
    "outside_border",
    "generic_kill",
]
for x in range(len(damage_types)):
    damage_type_entry = TAG_Compound()
    damage_type_entry.tags.append(TAG_String(name="name", value=damage_types[x]))
    damage_type_entry.tags.append(TAG_Int(name="id", value=x))
    damage_type_element = TAG_Compound(name="element")
    damage_type_element.tags.append(TAG_String(name="effects", value="burning"))
    damage_type_element.tags.append(TAG_String(name="scaling", value="never"))
    damage_type_element.tags.append(TAG_Float(name="exhaustion", value=0))
    damage_type_element.tags.append(TAG_String(name="message_id", value=""))
    damage_type_entry.tags.append(damage_type_element)
    damage_type_value.tags.append(damage_type_entry)
damage_type.tags.append(damage_type_value)

chat = TAG_Compound(name="chat_type")
chat.tags.append(TAG_String(name="type", value="chat_type"))
chat_value = TAG_List(name="value", type=TAG_Compound)
chat_attr = TAG_Compound()
chat_attr.tags.append(TAG_String(name="name", value="chat"))
chat_attr.tags.append(TAG_Int(name="id", value=0))
chat_element = TAG_Compound(name="element")
chat_element_chat = TAG_Compound(name="chat")
narration_element_chat = TAG_Compound(name="narration")
chat_element_chat_decoration = TAG_Compound(name="decoration")
chat_element_chat.tags.append(
    TAG_String(name="translation_key", value="chat.type.text")
)
narration_element_chat.tags.append(
    TAG_String(name="translation_key", value="chat.type.text.narrate")
)
chat_element_chat_decoration_parameters = TAG_List(name="parameters", type=TAG_String)
chat_element_chat_decoration_parameters.tags.append(TAG_String(value="sender"))
chat_element_chat_decoration_parameters.tags.append(TAG_String(value="content"))

chat_element_chat.tags.append(chat_element_chat_decoration_parameters)
narration_element_chat.tags.append(chat_element_chat_decoration_parameters)

# chat_element_chat.tags.append(chat_element_chat_decoration)
chat_element.tags.append(chat_element_chat)
chat_element.tags.append(narration_element_chat)

chat_attr.tags.append(chat_element)

chat_value.tags.append(chat_attr)


plains_biome.tags.append(plains_attr)

dimension.tags.append(dimension_attr)
dimensions.tags.append(dimension)
dimension_type.tags.append(dimensions)
nbtfile.tags.append(dimension_type)
nbtfile.tags.append(damage_type)


biome_types.append(plains_biome)
biome.tags.append(biome_types)
nbtfile.tags.append(biome)

chat.tags.append(chat_value)
nbtfile.tags.append(chat)

print(nbtfile.pretty_tree())
nbtfile.write_file("codec.nbt.gz")
with gzip.open("codec.nbt.gz", "rb") as f:
    with open("codec.nbt", "wb") as fx:
        file = bytearray(f.read())
        #remove the name of the root tag as this changed in the newer clients
        if(len(file) >=3):
            if(file[0] == 0xA and file[1] == 0x00 and file[2] == 0x00):
                file.pop(1)
                file.pop(1)
        fx.write(file)
