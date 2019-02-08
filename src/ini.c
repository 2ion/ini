/*
ini - examine INI files from the command line
  Copyright (C) 2015 Jens John < dev at 2ion dot de >

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <iniparser.h>
#include <regex.h>
#include <sys/types.h>
#include <unistd.h>

#define LERROR(status, errnum, ...) error_at_line((status), (errnum), \
        (__func__), (__LINE__), __VA_ARGS__)
#define OPTLIST "ae:g:G:k:p:sv:V:"

enum {
  EXIT_OK     = 0,
  EXIT_NOFILE = 1,
  EXIT_NOKEY  = 2
};

static const char *options = OPTLIST;
static const struct option options_long[] = {
  { "egrep",          required_argument,  NULL, 'G' },
  { "egrep-value",    required_argument,  NULL, 'V' },
  { "exists",         required_argument,  NULL, 'e' },
  { "grep",           required_argument,  NULL, 'g' },
  { "grep-value",     required_argument,  NULL, 'v' },
  { "list-all-keys",  no_argument,        NULL, 'a' },
  { "list-keys",      required_argument,  NULL, 'k' },
  { "list-sections",  no_argument,        NULL, 's' },
  { "print",          required_argument,  NULL, 'p' },
  { 0,                0,                  0,    0  }};
static const int DEFAULT_REG_FLAGS = REG_ICASE | REG_NOSUB;

static void grep_exec(const regex_t*, const char*, const char*);
static void grep_keys(dictionary*, const char*, int);
static void grep_values(dictionary*, const char*, int);
static void list_all(dictionary*);
static void list_keys(dictionary*, const char*);
static void list_sections(dictionary*);
static void print_regerror(int, const regex_t*);
static void usage(void);

void usage(void) {
  puts("Invocation forms:\n"
      "  ini -h\n"
      "  ini [" OPTLIST "] INI-FILE\n"
      "Where:\n"
      "  -a, --list-all-keys      List all keys\n"
      "  -e, --exists $KEY        Test if the value at $KEY exists, return 0\n"
      "                           if it does, otherwise return 2\n"
      "  -G, --egrep $REGEX       List all keys matching the given extended regex\n"
      "  -g, --grep $REGEX        List all keys matching the given POSIX regex\n"
      "  -h                       Print this message and exit\n"
      "  -k, --list-keys $SEC     List keys in section $SEC\n"
      "  -p, --print $KEY         Print the value associated with $KEY and\n"
      "                           return 0, otherwise print nothing and return 2\n"
      "  -s, --list-sections      List INI sections\n"
      "  -V, --egrep-value $REGEX List all keys the value of which matches the\n"
      "                           given extended regex\n"
      "  -v, --grep-value $REGEX  List all keys the value of which matches the\n"
      "                           given POSIX regex\n"
      "\n"
      "In the case that the INI-FILE doesn't exist, return 1. A $KEY is a\n"
      "string of the format ${section}:${key}, completely lowercased. Colons\n"
      "in $section and $key must be escaped with a backslash. Regexes are\n"
      "case-insensitive and don't have captures enabled.");
}

void list_sections(dictionary *dic) {
  const int n = iniparser_getnsec(dic);
  for(int sec = 0; sec < n; sec++)
    puts(iniparser_getsecname(dic, sec));
}

void list_keys(dictionary *dic, const char *sec) {
  const int n = iniparser_getsecnkeys(dic, sec);
  const char* rec[n];
  const char **l = iniparser_getseckeys(dic, sec, &rec[0]);
  if(l == NULL)
    LERROR(EXIT_FAILURE, 0, "iniparser_getseckeys() returned NULL");
  for(int i = 0; i < n; i++)
    puts(rec[i]);
}

void list_all(dictionary *dic) {
  const int nsec = iniparser_getnsec(dic);
  for(int sec = 0; sec < nsec; sec++) {
    const char *secname = iniparser_getsecname(dic, sec);
    list_keys(dic, secname);
  }
}

void print_regerror(int err, const regex_t *r) {
  size_t msglen = regerror(err, r, NULL, 0) + 1;
  char msgbuf[msglen];
  regerror(err, r, &msgbuf[0], msglen);
  fprintf(stderr, "%s\n", msgbuf);
}

void grep_exec(const regex_t *r, const char *s, const char *putstr) {
  int err;
  switch((err = regexec(r, s, 0, NULL, 0))) {
    case 0:
      puts(putstr);
      break;
    case REG_NOMATCH:
      break;
    default:
      print_regerror(err, r);
      break;
  }
}

void grep_keys(dictionary *dic, const char *regex, int eflags) {
  int err;
  int flags = DEFAULT_REG_FLAGS | eflags;
  regex_t r;

  if((err = regcomp(&r, regex, flags)) != 0) {
    print_regerror(err, &r);
    return;
  }

  const int nsec = iniparser_getnsec(dic);
  for(int sec = 0; sec < nsec; sec++) {
    const char *secname = iniparser_getsecname(dic, sec);
    const int nkeys = iniparser_getsecnkeys(dic, secname);
    const char *rec[nkeys];
    const char **l = iniparser_getseckeys(dic, secname, &rec[0]);
    if(l == NULL)
      return;
    for(int i = 0; i < nkeys; i++)
      grep_exec(&r, rec[i], rec[i]);
  }

  regfree(&r);

  return;
}

void grep_values(dictionary *dic, const char *regex, int eflags) {
  int err;
  int flags = DEFAULT_REG_FLAGS | eflags;
  regex_t r;

  if((err = regcomp(&r, regex, flags)) != 0) {
    print_regerror(err, &r);
    return;
  }

  const int nsec = iniparser_getnsec(dic);
  for(int sec = 0; sec < nsec; sec++) {
    const char *secname = iniparser_getsecname(dic, sec);
    const int nkeys = iniparser_getsecnkeys(dic, secname);
    const char *rec[nkeys];
    const char **l = iniparser_getseckeys(dic, secname, &rec[0]);
    if(l == NULL)
      return;
    for(int i = 0; i < nkeys; i++) {
      const char *s = iniparser_getstring(dic, rec[i], NULL);  
      if(s == NULL)
        continue;
      grep_exec(&r, s, rec[i]);
    }
  }

  regfree(&r);

  return;
}

int main(int argc, char **argv) {
  const char *file = NULL;
  dictionary *dic = NULL;
  int opt;
  int ret = EXIT_OK;

  if(argc < 2)
    return EXIT_NOFILE;

  if(argc == 2 && strcmp(argv[1], "-h") == 0) {
    usage();
    return EXIT_OK;
  }

  file = argv[argc-1];
  argc -= 1; /* hide the trailing file arg from getopt_long */
  if((dic = iniparser_load(file)) == NULL)
    return EXIT_NOFILE;

  while((opt = getopt_long(argc, argv, options, options_long, NULL)) != -1)
    switch(opt) {
      case 'a':
        list_all(dic);
        goto end;
      case 's':
        list_sections(dic);
        goto end;
      case 'k':
        list_keys(dic, optarg);
        goto end;
      case 'p':
        if(iniparser_find_entry(dic, optarg) == 1) {
          const char *s = iniparser_getstring(dic, optarg, NULL);
          if(s != NULL)
            puts(s);
        } else
          ret = EXIT_NOKEY;
        goto end;
      case 'e':
        if(iniparser_find_entry(dic, optarg) == 0)
          ret = EXIT_NOKEY;
        goto end;
      case 'g':
        grep_keys(dic, optarg, 0);
        goto end;
      case 'G':
        grep_keys(dic, optarg, REG_EXTENDED);
        goto end;
      case 'v':
        grep_values(dic, optarg, 0);
        goto end;
      case 'V':
        grep_values(dic, optarg, REG_EXTENDED);
        goto end;
    }

end:
  iniparser_freedict(dic);
  return ret;
}
