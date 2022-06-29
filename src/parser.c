/**
 * @file parser.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functionality for parsing xml files.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <cjson/cJSON.h>
#include "message.h"
#include "parser.h"
#include "spawn.h"
#include "actor.h"
#include "ai.h"
#include "register.h"
#include "invent.h"

void remove_whitespace(unsigned char *);
struct cJSON* json_from_file(const char *);
struct ai *ai_from_json(struct ai *, cJSON *);
struct item *item_from_json(struct item *, cJSON *);
struct actor *actor_primitives_from_json(struct actor *, cJSON *);
struct actor *attacks_from_json(struct actor *, cJSON *);
void dtypes_from_json(unsigned long *, cJSON *);
void tags_from_json(unsigned long *, cJSON *);
void color_from_json(unsigned char *, cJSON *);

/**
 * @brief Strips the whitespace characters from a string, including
 internal whitespace characters. Also used in message.c.
 * 
 * @param str The string to be mutated.
 */
void remove_whitespace(unsigned char *str) {
    int len = strlen((const char *) str);
    int j = 0;
    for (int i = 0; i < len; i++) {
        char ch = str[i];
        if (!isspace(ch)) {
            str[j++] = ch;
        }
    }
    str[j] = '\0';
}

/**
 * @brief Read JSON from a file.
 * 
 * @param fname The name of the file to be read.
 * @return struct cJSON* A pointer to the cJSON struct. Returns NULL if 
 parsing was not possible.
 */
struct cJSON* json_from_file(const char *fname) {
    long len;
    FILE *fp;
    char *buf = NULL;
    cJSON *json = NULL;
    /* Read the file. */
    fp = fopen(fname, "rb");
    if (!fp) {
        logma(MAGENTA, "Error: Could not find file: %s", fname);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buf = (char *) malloc(len + 1);
    fread(buf, 1, len, fp);
    buf[len] = '\0';
    fclose(fp);
    /* Parse the file into JSON. */
    json = cJSON_Parse(buf);
    if (!json) {
        logma(MAGENTA, "Error: Could not parse JSON in file: %s", fname);
        return NULL;
    }
    free(buf);
    return json;
}

/**
 * @brief Assembles an actor struct from a JSON file. When done,
 frees any cJSON in use.
 * 
 * @param fname The name of the file.
 * @return struct actor* The actor struct created.
 */
struct actor *actor_from_file(const char *fname) {
    cJSON *actor_json = NULL;
    cJSON *field = NULL;
    struct actor *actor;
    
    /* Initialize Actor */
    actor_json = json_from_file(fname);
    if (!actor_json)
        return NULL;
    actor = (struct actor *) malloc(sizeof(struct actor));
    if (!actor)
        return NULL;
    *actor = (struct actor) { 0 };
    
    /* Parse Fields */
    actor_primitives_from_json(actor, actor_json);
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "attacks");
    attacks_from_json(actor, field);
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "color");
    color_from_json(&(actor->color), field);
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "weak");
    dtypes_from_json(&(actor->weak), field);
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "resist");
    dtypes_from_json(&(actor->resist), field);
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "tags");
    tags_from_json(&(actor->tags), field);

    /* Parse Components */
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "ai");
    if (field) {
        init_ai(actor);
        ai_from_json(actor->ai, field);
    }
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "item");
    if (field) {
        init_item(actor);
        item_from_json(actor->item, field);
    }
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "equip");
    if (field) {
        init_equip(actor);
    }

    
    /* Free memory and return the actor. */
    cJSON_Delete(actor_json);
    return actor;
}

/**
 * @brief Parses the dungeon struct from a file.
 * 
 * @param fname The name of the file.
 */
void dungeon_from_file(const char *fname) {
    cJSON *dgn_json = NULL;
    cJSON *field = NULL;
    
    /* Initialize Actor */
    dgn_json = json_from_file(fname);
    if (!dgn_json)
        return;
    memset(dgn.filename, 0, sizeof(dgn.filename));
    snprintf(dgn.filename, sizeof(dgn.filename), fname);
    
    field = cJSON_GetObjectItemCaseSensitive(dgn_json, "name");
    memset(dgn.name, 0, sizeof(dgn.name));
    snprintf(dgn.name, sizeof(dgn.name), field->valuestring);

    field = cJSON_GetObjectItemCaseSensitive(dgn_json, "randomness");
    dgn.randomness = field->valueint;

    field = cJSON_GetObjectItemCaseSensitive(dgn_json, "wall_color");
    color_from_json(&(dgn.wall_color), field);

    field = cJSON_GetObjectItemCaseSensitive(dgn_json, "forbidden_tags");
    tags_from_json(&(dgn.forbidden_tags), field);
    field = cJSON_GetObjectItemCaseSensitive(dgn_json, "required_tags");
    tags_from_json(&(dgn.required_tags), field);
    field = cJSON_GetObjectItemCaseSensitive(dgn_json, "preferred_tags");
    tags_from_json(&(dgn.preferred_tags), field);
}

/**
 * @brief Parse an ai struct from JSON.
 * 
 * @param ai A pointer to the ai struct.
 * @param ai_json A pointer to the JSON to be parsed.
 * @return struct ai* A pointer to the ai struct.
 */
struct ai *ai_from_json(struct ai *ai, cJSON *ai_json) {
    cJSON* field = NULL;
    field = cJSON_GetObjectItemCaseSensitive(ai_json, "seekdef");
    ai->seekdef = field->valueint;
    return ai;
}

struct item *item_from_json(struct item *item, cJSON *item_json) {
    cJSON* field = NULL;
    field = cJSON_GetObjectItemCaseSensitive(item_json, "pref_slot");
    if (field) {
        for (int i = 0; i < MAX_SLOTS; i++) {
            if (!strcmp(field->valuestring, slot_types[i].slot_name)) {
                item->pref_slot = slot_types[i].id;
            }
        }
    }
    return item;
}

/**
 * @brief Parse the primitives necessary for an actor from JSON.
 * 
 * @param actor A pointer to the actor struct to be modified
 * @param actor_json A pointer to the JSON to be parsed.
 * @return struct actor* A pointer to the modified actor struct.
 */
struct actor *actor_primitives_from_json(struct actor *actor, cJSON *actor_json) {
    cJSON* field = NULL;
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "name");
    init_permname(actor, field->valuestring);
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "chr");
    actor->chr = field->valuestring[0];
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "tile");
    actor->tile_offset = field->valueint;
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "hp");
    actor->hp = field->valueint;
    actor->hpmax = actor->hp;
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "weight");
    actor->weight = field->valueint;
    field = cJSON_GetObjectItemCaseSensitive(actor_json, "speed");
    actor->speed = field->valueint;
    return actor;
}

/**
 * @brief Parse an array of attacks from JSON and assign them to an actor.
 * 
 * @param actor A pointer to the actor to be modified.
 * @param attacks_json A pointer to the JSON.
 * @return struct actor* A pointer to the modified actor.
 */
struct actor *attacks_from_json(struct actor *actor, cJSON *attacks_json) {
    cJSON* attack_json = NULL;
    cJSON* field = NULL;
    int i = 0;

    cJSON_ArrayForEach(attack_json, attacks_json) {
        if (i > MAX_ATTK) break;
        field = cJSON_GetObjectItemCaseSensitive(attack_json, "dice");
        actor->attacks[i].dam_n = field->valueint;
        field = cJSON_GetObjectItemCaseSensitive(attack_json, "sides");
        actor->attacks[i].dam_d = field->valueint;
        field = cJSON_GetObjectItemCaseSensitive(attack_json, "knockback");
        actor->attacks[i].kb = field->valueint;
        field = cJSON_GetObjectItemCaseSensitive(attack_json, "accuracy");
        actor->attacks[i].accuracy = field->valueint;
        field = cJSON_GetObjectItemCaseSensitive(attack_json, "types");
        dtypes_from_json(&(actor->attacks[i].dtype), field);
        i++;
    }
    while (i < MAX_ATTK) {
        actor->attacks[i] = (struct attack) { 0 };
        i++;
    }
    return actor;
}

/**
 * @brief Parse damage type bitflags from JSON.
 * 
 * @param field A pointer to the field to be modified.
 * @param types_json A pointer to the JSON.
 */
void dtypes_from_json(unsigned long *field, cJSON *types_json) {
    cJSON* damage_json;

    cJSON_ArrayForEach(damage_json, types_json) {
        for (int j = 0; j < MAX_DTYPE; j++) {
            if (!strcmp(dtypes_arr[j].str, damage_json->valuestring)) {
                *field |= dtypes_arr[j].val;
            }
        }
    }
}

/**
 * @brief Parse tags bitflags from JSON.
 * 
 * @param field A pointer to the field to be modified.
 * @param types_json A pointer to the JSON.
 */
void tags_from_json(unsigned long *field, cJSON *types_json) {
    cJSON* tags_json;

    cJSON_ArrayForEach(tags_json, types_json) {
        for (int j = 0; j < MAX_DTYPE; j++) {
            if (!strcmp(tags_arr[j].str, tags_json->valuestring)) {
                *field |= tags_arr[j].val;
            }
        }
    }
}


/**
 * @brief Parse color information from JSON.
 * 
 * @param color A pointer to the color to be modified.
 * @param color_json A pointer to the JSON.
 */
void color_from_json(unsigned char *color, cJSON *color_json) {
    for (int i = 0; i < MAX_COLOR; i++) {
        if (!strcmp(w_colors[i].str, color_json->valuestring)) {
            *color = w_colors[i].cnum;
        }
    }
}