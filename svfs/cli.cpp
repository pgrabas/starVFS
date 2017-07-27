/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include "cli.h"
#include "luasupport.h"
#include <signal.h>
#include <LuaBridge/LuaBridge.h>

//#define LUA_USE_READLINE
//#define USE_READLINE_STATIC

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LUA_MAXINPUT		512

#if !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

#if !defined(lua_stdin_is_tty)	/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#include <io.h>
#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#else				/* }{ */

/* ISO C definition */
#define lua_stdin_is_tty()	1  /* assume stdin is a tty */

#endif				/* } */

#endif				/* } */

/*
** lua_readline defines how to show a prompt and then read a line from
** the standard input.
** lua_saveline defines how to "save" a read line in a "history".
** lua_freeline defines how to free a line read by lua_readline.
*/
#if !defined(lua_readline)	/* { */

#if defined(LUA_USE_READLINE)	/* { */

#include <readline/readline.h>
#include <readline/history.h>
#define lua_readline(L,b,p)	((void)L, ((b)=readline(p)) != NULL)
#define lua_saveline(L,line)	((void)L, add_history(line))
#define lua_freeline(L,b)	((void)L, free(b))

#else				/* }{ */

#define lua_readline(L,b,p) \
        ((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
        fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */
#define lua_saveline(L,line)	{ (void)L; (void)line; }
#define lua_freeline(L,b)	{ (void)L; (void)b; }

#endif				/* } */

#endif				/* } */

static lua_State *globalL = NULL;

/*
** Hook set by signal function to stop the interpreter.
*/
static void lstop(lua_State *L, lua_Debug *ar) {
	(void)ar;  /* unused arg. */
	lua_sethook(L, NULL, 0, 0);  /* reset hook */
	luaL_error(L, "interrupted!");
}

/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
static void laction(int i) {
	signal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
	lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void l_message(const char *pname, const char *msg) {
	if (pname) lua_writestringerror("%s: ", pname);
	lua_writestringerror("%s\n", msg);
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int report(lua_State *L, int status) {
	if (status != LUA_OK) {
		const char *msg = lua_tostring(L, -1);
		l_message("?", msg);
		lua_pop(L, 1);  /* remove message */
	}
	return status;
}

/*
** Message handler used to run all chunks
*/
static int msghandler(lua_State *L) {
	const char *msg = lua_tostring(L, 1);
	if (msg == NULL) {  /* is error object not a string? */
		if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
			lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
			return 1;  /* that is the message */
		else
			msg = lua_pushfstring(L, "(error object is a %s value)",
								  luaL_typename(L, 1));
	}
	luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
	return 1;  /* return the traceback */
}

/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static int docall(lua_State *L, int narg, int nres) {
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, msghandler);  /* push message handler */
	lua_insert(L, base);  /* put it under function and args */
	globalL = L;  /* to be available to 'laction' */
	signal(SIGINT, laction);  /* set C-signal handler */
	status = lua_pcall(L, narg, nres, base);
	signal(SIGINT, SIG_DFL); /* reset C-signal handler */
	lua_remove(L, base);  /* remove message handler from the stack */
	return status;
}

static void get_prompt(lua_State *L, int firstline) {
	if(firstline) {
		lua_getglobal(L, "prompt");
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			lua_pushstring(L, "# ");
			return;
		}
		if (lua_isstring(L, -1))
			return;

		if (lua_isfunction(L, -1) || lua_iscfunction(L, -1)) {
			lua_call(L, 0, 1);
			return;
		}
	}
	lua_pushstring(L, "> ");
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)

/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
static int incomplete(lua_State *L, int status) {
	if (status == LUA_ERRSYNTAX) {
		size_t lmsg;
		const char *msg = lua_tolstring(L, -1, &lmsg);
		if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0) {
			lua_pop(L, 1);
			return 1;
		}
	}
	return 0;  /* else... */
}

/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
int CLI::pushline(lua_State *L, int firstline) {
	char buffer[LUA_MAXINPUT];
	char *b = buffer;
	size_t l;
	get_prompt(L, firstline);
	int readstatus = lua_readline(L, b, lua_tostring(L, -1));
	lua_pop(L, 1); /* remove prompt */
	if (readstatus == 0)
		return 0;  /* no input (prompt will be popped by caller) */

	l = strlen(b);
	if (l > 0 && b[l - 1] == '\n')  /* line ends with newline? */
		b[--l] = '\0';  /* remove it */

	if (firstline && b[0] == '=')  /* for compatibility with 5.2, ... */
		lua_pushfstring(L, "return %s", b + 1);  /* change '=' to 'return' */
	else {
		if (firstline && m_BashMode) {
			lua_getglobal(L, b);
			if (lua_isfunction(L, -1) || lua_iscfunction(L, -1)) {
				strcat(b, "()");
				l += 2;
			}
			lua_pop(L, 1);
		}
		lua_pushlstring(L, b, l);
	}
	lua_freeline(L, b);
	return 1;
}

/*
** Try to compile line on the stack as 'return <line>'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int addreturn(lua_State *L) {
	int status;
	size_t len; const char *line;
	lua_pushliteral(L, "return ");
	lua_pushvalue(L, -2);  /* duplicate line */
	lua_concat(L, 2);  /* new line is "return ..." */
	line = lua_tolstring(L, -1, &len);
	if ((status = luaL_loadbuffer(L, line, len, "=stdin")) == LUA_OK) {
		lua_remove(L, -3);  /* remove original line */
		line += sizeof("return") / sizeof(char);  /* remove 'return' for history */
		if (line[0] != '\0')  /* non empty? */
			lua_saveline(L, line);  /* keep history */
	} else
		lua_pop(L, 2);  /* remove result from 'luaL_loadbuffer' and new line */
	return status;
}

/*
** Read multiple lines until a complete Lua statement
*/
int CLI::multiline(lua_State *L) {
	for (;;) {  /* repeat until gets a complete statement */
		size_t len;
		const char *line = lua_tolstring(L, 1, &len);  /* get what it has */
		int status = luaL_loadbuffer(L, line, len, "=stdin");  /* try it */
		if (!incomplete(L, status) || !pushline(L, 0)) {
			lua_saveline(L, line);  /* keep history */
			return status;  /* cannot or should not try to add continuation line */
		}
		lua_pushliteral(L, "\n");  /* add newline... */
		lua_insert(L, -2);  /* ...between the two lines */
		lua_concat(L, 3);  /* join them */
	}
}

/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
int CLI::loadline(lua_State *L) {
	int status;
	lua_settop(L, 0);
	if (!pushline(L, 1))
		return -1;  /* no input */
	if ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
		status = multiline(L);  /* try as command, maybe with continuation lines */
	lua_remove(L, 1);  /* remove line from the stack */
	lua_assert(lua_gettop(L) == 1);
	return status;
}

/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print(lua_State *L) {
	int n = lua_gettop(L);
	if (n > 0) {  /* any result to be printed? */
		luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, n, 0, 0) != LUA_OK)
			l_message("?", lua_pushfstring(L, "error calling 'print' (%s)",
												lua_tostring(L, -1)));
	}
}

//-------------------------------------------------------------------------------------------------

void CLI::Loop() {
	auto l = m_Lua->GetState();
	int status;
	while ((status = loadline(l)) != -1) {
		if (status == LUA_OK)
			status = docall(l, 0, LUA_MULTRET);
		if (status == LUA_OK) l_print(l);
		else report(l, status);

		if (!m_CanContinue)
			break;
	}
	lua_settop(l, 0);  /* clear stack */
	lua_writeline();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

CLI::CLI(SharedLua Lua) {
	m_Lua = Lua;

	if (Lua)
		InstallApi();
}

//-------------------------------------------------------------------------------------------------

bool CLI::InstallApi() {
	luabridge::getGlobalNamespace(m_Lua->GetState())
		.beginNamespace("api")
			.beginClass<CLI>("CLI")
				.addFunction("Exit", &CLI::Exit)
			.endClass()
		.endNamespace()
		;

	luabridge::getGlobalNamespace(m_Lua->GetState())
		.beginNamespace("inst")
			//.addProperty<CLI*, CLI*>("cli", &GetgCLI)
			.addPtrVariable<CLI>("cli", this)
		.endNamespace()
		;

	return true;
}

void CLI::Exit(int ec) {
	m_CanContinue = false;
}

bool CLI::Enter(SVFS& svfs, InitEnv &env) {
	if (!m_Lua)
		return false;

	m_BashMode = env.m_BashMode;

	m_CanContinue = true;
	Loop();

	return true;
}
