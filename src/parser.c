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
#include <ctype.h>
#include "tile.h" /* Included only for tiledefs */
#include "message.h"
#include "parser.h"

void remove_whitespace(unsigned char *);

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
 * @brief Parses an XML file and returns a document pointer to it.
 * 
 * @param filename The name of the file to be parsed.
 * @param root The root node of the XML.
 * @return xmlDocPtr Pointer to the parsed document.
 */
xmlDocPtr parse_xml(const char *filename, const char *root) {
    xmlDocPtr doc;
    xmlNodePtr cur;

    LIBXML_TEST_VERSION

    doc = xmlParseFile(filename);

    if (doc == NULL) {
        logm("Could not parse xml document: %s.", filename);
        return NULL;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) {
        logm("Empty xml document: %s.", filename);
        xmlFreeDoc(doc);
        return NULL;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) root)) {
        logm("Improper xml root node: Expected %s, received %s.", root, cur->name);
        xmlFreeDoc(doc);
        return NULL;
    }

    return doc;
}

/**
 * @brief Fetches the entry in the current node and returns it as an integer
 using atoi.
 * 
 * @param doc Pointer to the document being parsed.
 * @param cur Pointer to the current XML node.
 * @return int The integer that was parsed.
 */
int parser_getint(xmlDocPtr doc, xmlNodePtr cur) {
    int ret;
    xmlChar *item;

    item = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    ret = atoi((char *) item);
    free(item);
    return ret;
}