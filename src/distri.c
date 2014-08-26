#include "kendynet.h"
#include "lua_util.h"
#include "luasocket.h"
#include "kn_timer.h"
#include "kn_time.h"

__thread engine_t g_engine = NULL;
__thread int      g_status = 1;

static void sig_int(int sig){
	g_status = 0;
	kn_stop_engine(g_engine);
}

/*static int  cb_timer(kn_timer_t timer)
{
	luaRef_t *Schedule = (luaRef_t*)kn_timer_getud(timer);
	const char *err;
	err = LuaCallRefFunc(*Schedule,NULL);
	if(err){
		printf("lua error0:%s\n",err);
	}
	return 1;
}*/

static int  lua_stop(lua_State *L){
	kn_stop_engine(g_engine);
	return 0;
}

static int  lua_getsystick(lua_State *L){
	lua_pushnumber(L,kn_systemms());
	return 1;
}


static void start(lua_State *L,const char *start_file)
{
	printf("start\n");
	char buf[4096];
	snprintf(buf,4096,"\
		local Sche = require \"lua/sche\"\
		local main,err= loadfile(\"%s\")\
		if err then print(err) end\
		Sche.Spawn(function () main() end)\
		local ms = 5\
		while RunOnce(ms) do\
			if Sche.Schedule() > 0 then\
				ms = 0\
			else\
				ms = 5\
			end\
		end",start_file);		
		
		//Sche.Schedule() return Sche.Schedule",start_file);
	int oldtop = lua_gettop(L);
	luaL_loadstring(L,buf);
	//luaRef_t *Schedule = calloc(1,sizeof(*Schedule));
	const char *error = luacall(L,"");
	if(error){
		lua_settop(L,oldtop);
		printf("%s\n",error);
		return;
	}
	lua_settop(L,oldtop);
	//kn_reg_timer(g_engine,1,cb_timer,Schedule);	
	//kn_engine_run(g_engine);		
}

static int lua_debug(lua_State *L){
	return 0;
}

static int lua_RunOnce(lua_State *L){
	if(g_status){ 
		uint32_t ms = (uint32_t)lua_tointeger(L,1);
		kn_engine_runonce(g_engine,ms);
	}
	lua_pushboolean(L,g_status);
	return 1;
}

void reg_luahttp(lua_State *L);
int main(int argc,char **argv)
{
	if(argc < 2){
		printf("usage distrilua luafile\n");
		return 0;
	}
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
    signal(SIGINT,sig_int);
    signal(SIGPIPE,SIG_IGN);
	lua_register(L,"stop_program",lua_stop); 
	lua_register(L,"GetSysTick",lua_getsystick); 
	lua_register(L,"Debug",lua_debug); 
	lua_register(L,"RunOnce",lua_RunOnce); 	   	
	reg_luasocket(L);
	reg_luahttp(L);
	g_engine = kn_new_engine();
	start(L,argv[1]);
	return 0;
} 
