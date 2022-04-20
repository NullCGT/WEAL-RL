#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tile.h" /* Included only for tiledefs */
#include "message.h"
#include "parser.h"

void remove_whitespace(unsigned char *);
void parser_tiles(const char *infile);

/* Strips whitespace characters from a string. */
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

/* Parses an XML file and returns a pointer to it. */
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

/* Fetches the entry in the current node and returns it as an integer. */
int parser_getint(xmlDocPtr doc, xmlNodePtr cur) {
    int ret;
    xmlChar *item;

    item = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    ret = atoi((char *) item);
    free(item);
    return ret;
}

/* Parses a file with wfc data and returns a wfc_image. */
/* IMPORTANT: This function does not actually free the memory allocated to
   tempdata. This MUST be freed after use. */
/* TODO: Return a pointer instead of a struct? */
struct wfc_image parse_wfc_xml(const char *infile) {
    int width = 0;
    int height = 0;
    unsigned char *tempdata;

    xmlDocPtr doc = parse_xml(infile, "wfc");
    xmlNodePtr cur = xmlDocGetRootElement(doc)->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"width"))) {
            width = parser_getint(doc, cur);
        } else if ((!xmlStrcmp(cur->name, (const xmlChar *)"height"))) {
            height = parser_getint(doc, cur);
        } else if ((!xmlStrcmp(cur->name, (const xmlChar *)"map"))) {
            tempdata = (unsigned char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            remove_whitespace(tempdata);
        }
        cur = cur->next;
    }
    xmlFreeDoc(doc);
    free(cur);
    xmlCleanupParser();

    struct wfc_image image = {
        .data = tempdata,
        .component_cnt = 1,
        .width = width,
        .height = height
    };

    return image;
}