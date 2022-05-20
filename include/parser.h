#include <libxml/parser.h>
#include <libxml/tree.h>

#ifndef PARSER_H
#define PARSER_H

xmlDocPtr parse_xml(const char *, const char *);
int parser_getint(xmlDocPtr, xmlNodePtr);

#endif