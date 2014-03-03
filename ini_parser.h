/** 
 * @file   ini_parser.h
 * @author mackong <mackonghp@gmail.com>
 * @date   Tue Feb 18 19:42:15 2014
 * 
 * @brief  A simple INI parser based on list.
 * 
 * 
 */

#ifndef _INI_PARSER_H_
#define _INI_PARSER_H_

struct ini_context;

struct ini_context *ini_load(const char *filename);
void ini_unload(struct ini_context *ctx);
void ini_dump(struct ini_context *ctx);

char *ini_get_string(struct ini_context *ctx, const char *sec_name,
                     const char *elem_name, char *default_value);
int ini_get_int(struct ini_context *ctx, const char *sec_name,
                const char *elem_name, int default_value);
/** 1 for true, 0 for false. */
int ini_get_bool(struct ini_context *ctx, const char *sec_name,
                 const char *elem_name, int default_value);

#endif /* _INI_PARSER_H_ */

