/**
 * @file pmatch.cc
 *
 * @brief implements pmatch routines.
 */

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "pmatch_utils.h"
#include "HfstTransducer.h"

using std::string;
using std::map;

extern char* pmatchtext;
extern int pmatchparse();
extern int pmatchnerrs;

int
pmatcherror(const char *msg)
{
#ifndef NDEBUG
    fprintf(stderr, "*** pmatch parsing failed: %s\n", msg);
    if (strlen(hfst::pmatch::data) < 60)
    {
        fprintf(stderr, "***    parsing %s [near %s]\n", hfst::pmatch::data,
                pmatchtext);
    }
    else
    {
        fprintf(stderr, "***    parsing %60s [near %s]...\n", 
                hfst::pmatch::data, pmatchtext);
    }
#endif
    return 0;
}

namespace hfst 
{ 
namespace pmatch 
{

char* data;
std::map<std::string,hfst::HfstTransducer*> definitions;
char* startptr;
hfst::HfstTransducer* last_compiled;
hfst::ImplementationType format;
size_t len;

std::map<std::string, hfst::HfstTransducer> named_transducers;


int*
get_n_to_k(const char* s)
{
    int* rv = static_cast<int*>(malloc(sizeof(int)*2));
    char *endptr;
    char *finalptr;
    if (*(s + 1) == '{')
      {
        rv[0] = strtol(s + 2, &endptr, 10);
        rv[1] = strtol(endptr + 1, &finalptr, 10);
        assert(*finalptr == '}');
      }
    else
      {
        rv[0] = strtol(s + 1, &endptr, 10);
        rv[1] = strtol(endptr + 1, &finalptr, 10);
        assert(*finalptr == '\0');
      }
    return rv;
}



int
getinput(char *buf, int maxlen)
{
    int retval = 0;
    if ( maxlen > len ) {
        maxlen = len;
    }
    memcpy(buf, data, maxlen);
    data += maxlen;
    len -= maxlen;
    retval = maxlen;
    return retval;
}

char*
strip_percents(const char *s)
{
    const char *c = s;
    char *stripped = (char*)calloc(sizeof(char),strlen(s)+1);
    size_t i = 0;
    while (*c != '\0')
    {
        if (*c == '%')
        {
            if (*(c + 1) == '\0')
            {
                break;
            }
            else
            {
                stripped[i] = *(c + 1);
                i++;
                c += 2;
            }
        }
        else
        {
            stripped[i] = *c;
            i++;
            c++;
        }
    }
    stripped[i] = '\0';
    return stripped;
}

char*
add_percents(const char *s)
  {
    char* ns = static_cast<char*>(malloc(sizeof(char)*strlen(s)*2+1));
    char* p = ns;
    while (*s != '\0')
      {
        if ((*s == '@') || (*s == '-') || (*s == ' ') || (*s == '|') ||
            (*s == '!') || (*s == ':') || (*s == ';') || (*s == '0') ||
            (*s == '\\') || (*s == '&') || (*s == '?') || (*s == '$') ||
            (*s == '+') || (*s == '*') || (*s == '/') || (*s == '/') ||
            (*s == '_') || (*s == '(') || (*s == ')') || (*s == '{') ||
            (*s == '}') || (*s == '[') || (*s == ']'))
          {
            *p = '%';
            p++;
          }
        *p = *s;
        p++;
        s++;
      }
    *p = '\0';
    return ns;
  }

char *
get_Ins_transition(const char *s)
{
    char* rv = static_cast<char*>(malloc(sizeof(char)*(strlen(s) + 4 + 1)));
    rv = strcpy(rv, "@I.");
    rv = strcat(rv, s);
    rv = strcat(rv, "@");
    return rv;
}

char *
get_RC_transition(const char *s)
{
    char* rv = static_cast<char*>(malloc(sizeof(char)*(strlen(s) + 5 + 1)));
    rv = strcpy(rv, "@RC.");
    rv = strcat(rv, s);
    rv = strcat(rv, "@");
    return rv;
}

char *
get_LC_transition(const char *s)
{
    char* rv = static_cast<char*>(malloc(sizeof(char)*(strlen(s) + 5 + 1)));
    rv = strcpy(rv, "@LC.");
    rv = strcat(rv, s);
    rv = strcat(rv, "@");
    return rv;
}

HfstTransducer * add_pmatch_delimiters(HfstTransducer * regex)
{
    HfstTransducer * delimited_regex = new HfstTransducer(hfst::internal_epsilon,
                                                          "@_PMATCH_ENTRY_@",
                                                          regex->get_type());
    delimited_regex->concatenate(*regex);
    delimited_regex->concatenate(HfstTransducer(hfst::internal_epsilon,
                                                "@_PMATCH_EXIT_@",
                                                regex->get_type()));
    delete regex;
    delimited_regex->minimize();
    return delimited_regex;
}

void add_end_tag(HfstTransducer * regex, std::string tag)
{
    HfstTransducer end_tag(hfst::internal_epsilon,
                           "@_PMATCH_ENDTAG_" + tag + "_@",
                           regex->get_type());
    regex->concatenate(end_tag);
}

char *
get_quoted(const char *s)
{
    const char *qstart = strchr((char*) s, '"') + 1;
    const char *qend = strrchr((char*) s, '"');
    char* qpart = strdup(qstart);
    *(qpart+ (size_t) (qend - qstart)) = '\0';
    return qpart;
}

char*
parse_quoted(const char *s)
{
    char* quoted = get_quoted(s);
    char* rv = static_cast<char*>(malloc(sizeof(char)*strlen(quoted)));
    char* p = quoted;
    char* r = rv;
    while (*p != '\0')
      {
        if (*p != '\\')
          {
            *r = *p;
            r++;
            p++;
          }
        else if (*p == '\\')
          {
            switch (*(p + 1))
              {
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
                fprintf(stderr, "*** PMATCH unimplemented: "
                        "parse octal escape in %s", p);
                *r = '\0';
                p = p + 5;
                break;
              case 'a':
                *r = '\a';
                r++;
                p = p + 2;
                break;
              case 'b':
                *r = '\b';
                r++;
                p = p + 2;
                break;
              case 'f':
                *r = '\f';
                r++;
                p = p + 2;
                break;
              case 'n':
                *r = '\n';
                r++;
                p = p + 2;
                break;
              case 'r':
                *r = '\r';
                r++;
                p = p + 2;
                break;
              case 't':
                *r = '\t';
                r++;
                p = p + 2;
                break;
              case 'u':
                fprintf(stderr, "Unimplemented: parse unicode escapes in %s", p);
                *r = '\0';
                r++;
                p = p + 6;
                break;
              case 'v':
                *r = '\v';
                r++;
                p = p + 2;
                break;
              case 'x':
                  {
                    char* endp;
                    int i = strtol(p + 2, &endp, 10);
                    if ( 0 < i && i <= 127)
                      {
                        *r = static_cast<char>(i);
                      }
                    else
                      {
                        fprintf(stderr, "*** PMATCH unimplemented: "
                                "parse \\x%d\n", i);
                        *r = '\0';
                      }
                    r++;
                    assert(endp != p);
                    p = endp;
                   break;
                }
              case '\0':
                fprintf(stderr, "End of line after \\ escape\n");
                *r = '\0';
                r++;
                p++;
                break;
              default:
                *r = *(p + 1);
                r++;
                p += 2;
                break;
              }
          }
      }
    *r = '\0';
    free(quoted);
    return rv;
}

double
get_weight(const char *s)
{
    double rv = -3.1415;
    const char* weightstart = s;
    while ((*weightstart != '\0') && 
           ((*weightstart == ' ') || (*weightstart == '\t') ||
            (*weightstart == ';')))
    {
        weightstart++;
    }
    char* endp;
    rv = strtod(weightstart, &endp);
    assert(endp != weightstart);
    return rv;
}

HfstTransducer*
compile(const string& pmatch, map<string,HfstTransducer*>& defs,
        ImplementationType impl)
{
    // lock here?
    data = strdup(pmatch.c_str());
    startptr = data;
    len = strlen(data);
    definitions = defs;
    format = impl;
    pmatchparse();
    free(startptr);
    data = 0;
    len = 0;
    if (pmatchnerrs == 0)
      {
        HfstTransducer* rv = new HfstTransducer(*last_compiled);
        delete last_compiled;
        return rv;
      }
    else
      {
        return new HfstTransducer(impl);
      }
}

} }
