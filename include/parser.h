#include <libxml/parser.h>
#include <libxml/tree.h>
#include "wfc.h"

#ifndef PARSER_H
#define PARSER_H

xmlDocPtr parse_xml(const char *, const char *);
int parser_getint(xmlDocPtr, xmlNodePtr);
struct wfc_image parse_wfc_xml(char *infile);

#endif