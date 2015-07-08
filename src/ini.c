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

#include <iniparser.h>
#include <unistd.h>
#include <getopt.h>

#define OPTLIST "ae:k:p:s"
#define ACTION_STACK 1

enum {
  EXIT_NOFILE = 1,
  EXIT_NOKEY = 2,
  EXIT_OK = 0
};

static const char *options = OPTLIST;
static const struct option options_long[] = {
  { "list-sections",  no_argument,        NULL, 's' },
  { "list-keys",      required_argument,  NULL, 'k' },
  { "list-all-keys",  no_argument,        NULL, 'a' },
  { "exists",         required_argument,  NULL, 'e' },
  { "print",          required_argument,  NULL, 'p' },
  { NULL,             NULL,               NULL, NULL}};

static void usage(void);
static void list_sections(dictionary*);
static void list_keys(dictionary*, const char*);
static void list_all(dictionary*);

void usage(void) {
  puts("Invocation forms:\n"
      "  ini -h\n"
      "  ini [" OPTLIST "] INI-FILE\n"
      "Where:\n"
      "  -h                   Print this message and exit\n"
      "  -s, --list-sections  List INI sections\n"
      "  -k, --list-keys $SEC List keys in section $SEC\n"
      "  -a, --list-all-keys  List all keys\n"
      "  -e, --exists $KEY    Test if the value at $KEY exists, return 0\n"
      "                       if it does, otherwise return 2\n"
      "  -p, --print $KEY     Print the value associated with $KEY and\n"
      "                       return 0, otherwise print nothing and return 2\n"
      "\n"
      "In the case that the INI-FILE doesn't exist, return 1. A $KEY is a\n"
      "string of the format ${section}:${key}, completely lowercased. Colons\n"
      "in $section and $key must be escaped with a backslash.");
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

int main(int argc, char **argv) {
  int opt;
  const char *file = NULL;
  dictionary *dic = NULL;
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
        if(iniparser_find_entry(dic, optarg) == 1)
          puts(iniparser_getstring(dic, optarg, NULL));
        else
          ret = EXIT_NOKEY;
        goto end;
      case 'e':
        if(iniparser_find_entry(dic, optarg) == 0)
          ret = EXIT_NOKEY;
        goto end;
    }

end:
  iniparser_freedict(dic);
  return ret;
}
