/*
 * (c) 2008-2009 Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/*
 * Parse the command-line for specified arguments and store the values into 
 * variables.
 *
 * For a more detailed documentation, see parse_cmd.h in the include dir.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <alloca.h>
#include <string.h>
#include <l4/util/getopt.h> 
#include <l4/util/parse_cmd.h>

struct parse_cmdline_struct{
    enum parse_cmd_type type;		// which type (int, switch, string)
    char		shortform;	// short symbol
    const char		*longform;	// long name
    union{
	void		*argptr;	// ptr to variable getting the value
	parse_cmd_fn_t	fn;		// function to call
	parse_cmd_fn_arg_t fn_arg;	// function to call with args
    } arg;
    union{
    	int		switch_to;	// value a switch sets
    	const char*	default_string;	// default string value
    	unsigned	default_int;	// default int value
    	int		id;		// identifier to pass to function
    }val;
    const char		*comment;	// a description for the generated help
};

#define TRASH(type, val) { type dummy __attribute__ ((unused)) = (val); }

L4_CV int parse_cmdline(int *argc, const char***argv, char arg0, ...){
    va_list va;
    int err;

    /* calculate the number of argument-descriptors */
    va_start(va, arg0);
    err = parse_cmdlinev(argc, argv, arg0, va);
    va_end(va);
    return err;
}

L4_CV int parse_cmdlinev(int *argc, const char***argv, char arg0, va_list va0){
    va_list va;
    int c, count, shortform, cur_longopt;
    struct option *longopts, *longptr;
    char *optstring, *optptr;
    struct parse_cmdline_struct *pa;
    int err;

    va_copy(va, va0);
    /* calculate the number of argument-descriptors */
    shortform = arg0;
    for(count=0; shortform; count++){
	int type;
	int standard_int, *int_p;
	const char *standard_string, **string_p;
 
 	va_arg(va, const char*); /* skip long form */
	va_arg(va, const char*); /* skip comment */
  	type = va_arg(va, int);
  	switch(type){
	case PARSE_CMD_INT:
	    standard_int = va_arg(va, int);
	    int_p = va_arg(va, int*);
	    *int_p = standard_int;
	    break;
	case PARSE_CMD_SWITCH:
	    TRASH(int, va_arg(va, int));
	    TRASH(int*, va_arg(va, int*));
	    break;
	case PARSE_CMD_STRING:
	    standard_string = va_arg(va, char*);
	    string_p  = va_arg(va, const char**);
	    *string_p = standard_string;
	    break;
	case PARSE_CMD_FN:
	case PARSE_CMD_FN_ARG:
	    TRASH(int, va_arg(va, int));
	    TRASH(parse_cmd_fn_t, va_arg(va, parse_cmd_fn_t));
	    break;
	case PARSE_CMD_INC:
	    standard_int = va_arg(va, int);
	    int_p = va_arg(va, int*);
	    *int_p = standard_int;
	    break;
	case PARSE_CMD_DEC:
	    standard_int = va_arg(va, int);
	    int_p = va_arg(va, int*);
	    *int_p = standard_int;
	    break;
	default:
	    return -1;
  	}
  	shortform = va_arg(va, int);
    }

    /* consider the --help and -h */
    count++;

    /* allocate the fields for short options, long options and parse args */
    longopts = (struct option*)alloca(sizeof(struct option)*(count+1));
    if(longopts==0) return -2;
    
    optstring = (char*)alloca(count*2+1);
    if(optstring==0) return -2;
    
    pa = (struct parse_cmdline_struct*)
    	 alloca(count * sizeof(struct parse_cmdline_struct));
    if(pa==0) return -2;
    
    /* fill in the short options field, longopts and parse args */
    va_copy(va, va0);
    shortform = arg0;
    optptr    = optstring;
    longptr   = longopts;

    /* Prefill the 'help' switches. We know it is the first entry, so
       we can check for idx 0 when parsing the table. */
    *optptr++='h';
    pa->shortform = 'h';
    pa->longform = "help";
    pa->comment = "this help";
    pa->type = PARSE_CMD_SWITCH;
    longptr->name = pa->longform;
    longptr->flag = &cur_longopt;
    longptr->val = 0;
    longptr->has_arg = 0;
    longptr++;

    for(c=1;shortform; c++){
	if(shortform!=' ') *optptr++ = shortform;
	pa[c].shortform = shortform;
	pa[c].longform = va_arg(va, const char*);
	pa[c].comment = va_arg(va, const char*);
	pa[c].type = va_arg(va, int);

	/* prefill a few of the longoptions fields */
	if(pa[c].longform){
	    longptr->name = pa[c].longform;
	    longptr->flag = &cur_longopt;
	    longptr->val = c;
	}
	switch(pa[c].type){
	case PARSE_CMD_INT:
	    if(shortform!=' ') *optptr++ = ':';
	    if(pa[c].longform) longptr->has_arg = 1;

	    pa[c].val.default_int = va_arg(va, int);
	    pa[c].arg.argptr = va_arg(va, int*);
	    break;

	case PARSE_CMD_SWITCH:
	    if(pa[c].longform) longptr->has_arg = 0;

	    pa[c].val.switch_to = va_arg(va, int);
	    pa[c].arg.argptr = va_arg(va, int*);
	    break;
	case PARSE_CMD_STRING:
	    if(shortform!=' ') *optptr++ = ':';
	    if(pa[c].longform) longptr->has_arg = 1;

	    pa[c].val.default_string = va_arg(va, char*);
	    pa[c].arg.argptr = va_arg(va, char**);
	    break;
	case PARSE_CMD_FN:
	    if(pa[c].longform) longptr->has_arg = 0;

	    pa[c].val.id = va_arg(va, int);
	    pa[c].arg.fn = va_arg(va, parse_cmd_fn_t);
	    break;
	case PARSE_CMD_FN_ARG:
	    if(shortform!=' ') *optptr++ = ':';
	    if(pa[c].longform) longptr->has_arg = 1;

	    pa[c].val.id = va_arg(va, int);
	    pa[c].arg.fn_arg = va_arg(va, parse_cmd_fn_arg_t);
	    break;
	case PARSE_CMD_INC:
	case PARSE_CMD_DEC:
	    if(pa[c].longform) longptr->has_arg = 0;

	    TRASH(int, va_arg(va, int));
	    pa[c].arg.argptr = va_arg(va, int*);
	    break;
  	}

	if(pa[c].longform) longptr++;
	// next short form
  	shortform = va_arg(va, int);
    }

    // end the optstring string
    *optptr=0;

    // end the longopt field
    longptr->name=0;
    longptr->has_arg=0;
    longptr->flag=0;
    longptr->val=0;

    err = -3;

    /* now, parse the arguments */
    do{
	int val;
	int idx;

	val = getopt_long_only(*argc, (char**)*argv, optstring, longopts, &idx);
	switch(val){
	case ':':
	    printf("Option -%c requires an argument\n",optopt);
	    goto e_help;
	case '?':
	    if(opterr){
	        printf("Unrecognized option: - %c\n", optopt ? optopt : '?');
		goto e_help;
	    }
	    break;
	case -1:
	    *argc-=optind;
	    *argv+=optind;
	    optind=1;
	    return 0;
	default:
	    /* we got an option. If it is a short option (val!=0),
	       lookup the index. */
	    if(val!=0){
		for(idx = 0; idx < count; idx++){
		    if(pa[idx].shortform == val) break;
		}
	    } else {
		/* it was a long option. We are lucky, the pa-element is
		   stored in the cur_longopt variable. */
		idx = cur_longopt;
	    }
	    if(idx == 0){
		err = -4;
		goto e_help;
	    }
	    if(idx<count){
		switch(pa[idx].type){
		case PARSE_CMD_INT:
		    *((int*)pa[idx].arg.argptr) = strtol(optarg, 0, 0);
		    break;
		case PARSE_CMD_SWITCH:
		    *((int*)pa[idx].arg.argptr) = pa[idx].val.switch_to;
		    break;
		case PARSE_CMD_STRING:
		    *((const char**)pa[idx].arg.argptr) = optarg;
		    break;
		case PARSE_CMD_FN:
		    pa[idx].arg.fn(pa[idx].val.id);
		    break;
		case PARSE_CMD_FN_ARG:
		    pa[idx].arg.fn_arg(pa[idx].val.id,
				       optarg, strtol(optarg, 0, 0));
		    break;
		case PARSE_CMD_INC:
		    (*((int*)pa[idx].arg.argptr))++;
		    break;
		case PARSE_CMD_DEC:
		    (*((int*)pa[idx].arg.argptr))--;
		    break;
		break;
		}
	    }
	    break;
	} // switch val
    } while(1);

  e_help:
    printf("Usage: %s <options>. Option list:\n", *argv[0]);
    for(c=0;c<count;c++){
	int l;
	char buf[3];
	
	if(pa[c].shortform!=' '){
		buf[0]='-';buf[1]=pa[c].shortform;buf[2]=0;
	} else {
		buf[0]=0;
	}
	
	l = printf(" [ %s%s%s%s%s ]",
		   buf,
		   (buf[0] && pa[c].longform) ? " | " : "",
		   pa[c].longform ? "--" : "",
		   pa[c].longform ? pa[c].longform : "",
		   pa[c].type==PARSE_CMD_INT ? " num" :
		   pa[c].type==PARSE_CMD_STRING ? " string" : "");
	if(pa[c].comment) printf(" %*s- %s", l<25?25-l:0,
				 "", pa[c].comment);
	if(pa[c].type == PARSE_CMD_STRING)
		printf(" (\"%s\")", pa[c].val.default_string);
	if(pa[c].type == PARSE_CMD_INT)
		printf(" (%#x)", pa[c].val.default_int);
	printf("\n");
    }
  optind=1;
  return err;
}

L4_CV int parse_cmdline_extra(const char*argv0, const char*line, char delim,
			char arg0,...){
    int i, argc_=1;
    char*s, *line_=0;
    const char**argv_;
    va_list va;

    if(line && *line){
	if((line_ = alloca(strlen(line)))==0) return -2;
	strcpy(line_, line);
	argc_++;
	for(s=line_;*s;s++)if(*s==delim) argc_++;
    }
    argv_ = alloca(sizeof(char*)*argc_);
    argv_[0]=argv0;
    argc_=1;
    s=line_;
    if(line) while(*line){
	argv_[argc_]=s;
	while((*s=*line)!=0 && *s!=delim){line++; s++;}
	*s++=0;
	if(*line)line++;
	argc_++;
    }
    va_start(va, arg0);
    i = parse_cmdlinev(&argc_, &argv_, arg0, va);
    va_end(va);
    return i;
}
