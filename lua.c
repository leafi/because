#include <lua.h>
#include <lapi.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>

extern void printk(char* s);
extern void printki(long n);
extern void printkid(long n);

extern void* kmalloc(long len);

extern unsigned char rdiskb(unsigned long offset);
extern unsigned short rdiskw(unsigned long offset);
extern unsigned int rdiski(unsigned long offset);
extern unsigned long rdiskl(unsigned long offset);

extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern unsigned int ind(unsigned short port);
extern void outb(unsigned short port, unsigned char val);
extern void outw(unsigned short port, unsigned short val);
extern void outd(unsigned short port, unsigned int val);

extern void irq_mask(unsigned char irq, unsigned char mask);

static int lkmalloc(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) kmalloc(lua_tounsigned(L, 1)));
	return 1;
}

static int lpeekb(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) *((unsigned char*)lua_tounsigned(L, 1)));
	return 1;
}

static int lpeekw(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) *((unsigned short*)lua_tounsigned(L, 1)));
	return 1;
}

static int lpeeki(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) *((unsigned int*)lua_tounsigned(L, 1)));
	return 1;
}

static int lpeekl(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) *((unsigned long*)lua_tounsigned(L, 1)));
	return 1;
}

static int lpokeb(lua_State *L)
{
	*((unsigned char*)lua_tounsigned(L, 1)) = (unsigned char) lua_tounsigned(L, 2);
	return 0;
}

static int lpokew(lua_State *L)
{
	*((unsigned short*)lua_tounsigned(L, 1)) = (unsigned short) lua_tounsigned(L, 2);
	return 0;
}

static int lpokei(lua_State *L)
{
	*((unsigned int*)lua_tounsigned(L, 1)) = (unsigned int) lua_tounsigned(L, 2);
	return 0;
}

static int lpokel(lua_State *L)
{
	*((unsigned long*)lua_tounsigned(L, 1)) = (unsigned long) lua_tounsigned(L, 2);
	return 0;
}

static int linb(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) inb((unsigned short) lua_tounsigned(L, 1)));
	return 1;
}

static int linw(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) inw((unsigned short) lua_tounsigned(L, 1)));
	return 1;
}

static int lini(lua_State *L)
{
	lua_pushunsigned(L, (lua_Unsigned) ind((unsigned short) lua_tounsigned(L, 1)));
	return 1;
}

static int loutb(lua_State *L)
{
	outb((unsigned short) lua_tounsigned(L, 1), (unsigned char) lua_tounsigned(L, 2));
	return 0;
}

static int loutw(lua_State *L)
{
	outw((unsigned short) lua_tounsigned(L, 1), (unsigned short) lua_tounsigned(L, 2));
	return 0;
}

static int louti(lua_State *L)
{
	outd((unsigned short) lua_tounsigned(L, 1), (unsigned int) lua_tounsigned(L, 2));
	return 0;
}

static int lrdiskb(lua_State *L)
{
	int n = lua_gettop(L);
	if (lua_gettop(L) < 1 || !lua_isinteger(L, n) || !lua_isnumber(L, n)) {
		lua_pushstring(L, "lrdiskb incorrect argument");
		lua_error(L);
	}
	lua_pushunsigned(L, (lua_Unsigned) rdiskb(lua_tounsigned(L, n)));
	return 1;
}

static int lrdiskw(lua_State *L)
{
	int n = lua_gettop(L);
	if (lua_gettop(L) < 1 || !lua_isinteger(L, n) || !lua_isnumber(L, n)) {
		lua_pushstring(L, "lrdiskw incorrect argument");
		lua_error(L);
	}
	lua_pushunsigned(L, (lua_Unsigned) rdiskw(lua_tounsigned(L, n)));
	return 1;
}

static int lrdiski(lua_State *L)
{
	int n = lua_gettop(L);
	if (lua_gettop(L) < 1 || !lua_isinteger(L, n) || !lua_isnumber(L, n)) {
		lua_pushstring(L, "lrdiski incorrect argument");
		lua_error(L);
	}
	lua_pushunsigned(L, (lua_Unsigned) rdiski(lua_tounsigned(L, n)));
	return 1;
}

static int lrdiskl(lua_State *L)
{
	int n = lua_gettop(L);
	if (lua_gettop(L) < 1 || !lua_isinteger(L, n) || !lua_isnumber(L, n)) {
		lua_pushstring(L, "lrdiskl incorrect argument");
		lua_error(L);
	}
	lua_pushunsigned(L, (lua_Unsigned) rdiskl(lua_tounsigned(L, n)));
	return 1;
}

static int lirqmask(lua_State *L)
{
	irq_mask((unsigned char) lua_tounsigned(L, 1), (unsigned char) lua_tounsigned(L, 2));
	return 0;
}

lua_State *globalL;

void call_lua_int(long inte)
{
	lua_State *L = globalL;

	lua_getglobal(L, "call_lua_int_2");
	lua_pushinteger(L, (lua_Integer) inte);
	lua_call(L, 1, 0);
}

void reportlua(lua_State *L)
{
	const char* c = lua_tostring(L, lua_gettop(L));

	int i;
	int j = 0;
	char prefix[256];

	for (i = 0; i < 255; i++) {
		prefix[i] = c[i];
		if (c[i] == ':') {
			j++;
			if (j == 4) {
				i++;
				break;
			}
		}
	}
	if (j == 4) {
		prefix[i] = 0;
		c += i;
		printk(prefix);
		printk("\n   ");
	}
	printk(c);
	printk("\n");
		
	//printk(lua_tostring(L, lua_gettop(L)));
}

#define LBUFFER_LIMIT	65536
char lbuffer[LBUFFER_LIMIT];
int lbufferptr = 0;

static int clearlbuffer(lua_State *L)
{
	lbufferptr = 0;
	return 0;
}

static int finishlbuffer(lua_State *L)
{
	lua_pushlstring(L, lbuffer, lua_tounsigned(L, 1));
	return 1;
}

static int addlbuffer(lua_State *L)
{
	long rloc = lua_tounsigned(L, 1);
	long sectors_per_cluster = lua_tounsigned(L, 2);
	long* lp = (long*)lbuffer;
	int i;
	for (i = 0; i < (sectors_per_cluster * 512 / 8); i++) {
		lp[lbufferptr + i] = rdiskl(rloc);
		rloc += 8;
	}
	lbufferptr += sectors_per_cluster * 512 / 8;
	if (lbufferptr >= LBUFFER_LIMIT) {
		printk("!!! Need to extend LBUFFER_LIMIT.\n");
		printk("!!! Lua file is too long; cannot continue.\n");
		while (1) {}
	}
	return 0;
}

static const luaL_Reg bclib[] = {
	{"rdiskb", lrdiskb},
	{"rdiskw", lrdiskw},
	{"rdiski", lrdiski},
	{"rdiskl", lrdiskl},
	{"clearlbuffer", clearlbuffer},
	{"addlbuffer", addlbuffer},
	{"finishlbuffer", finishlbuffer},
	{"inb", linb},
	{"inw", linw},
	{"ini", lini},
	{"outb", loutb},
	{"outw", loutw},
	{"outi", louti},
	{"peekb", lpeekb},
	{"peekw", lpeekw},
	{"peeki", lpeeki},
	{"peekl", lpeekl},
	{"pokeb", lpokeb},
	{"pokew", lpokew},
	{"pokei", lpokei},
	{"pokel", lpokel},
	{"kmalloc", lkmalloc},
	{"irqmask", lirqmask},
	{NULL,     NULL}
};

LUAMOD_API int luaopen_bc (lua_State *L) {
	luaL_newlib(L, bclib);
	return 1;
}

void open_private_libs(lua_State *L) {
	luaL_requiref(L, "bc", luaopen_bc, 1);
	lua_pop(L, 1);
}

// TEMP HACK
extern void irq1();
extern void set_int(unsigned char inte, void* funptr);

void lua_stuff()
{
	// TEMP TESTING HACK
	set_int(0x21, irq1);

	//printk("lua_stuff() starting\n");

	lua_State *l;
	l = luaL_newstate();

	// shh
	globalL = l;

	luaL_openlibs(l);
	open_private_libs(l);

	const char* lcode =
		"-- lua.c lua\n"
		"function is_fat(id)\n"
		"	 return (id == 1) or (id == 4) or (id == 6) or (id == 0xb) or (id == 0xc) or (id == 0xe) or (id == 0x11) or (id == 0x14) or (id == 0x16) or (id == 0x1b) or (id == 0x1c) or (id == 0x1e)\n"
		"end\n"
		"\n"
		"function fat_investigate(lba)\n"
		"  -- print('base lba is ' .. lba)\n"
		"  local base = lba * 512\n"
		"  local total_sectors_16 = bc.rdiskw(base + 19)\n"
		"  local bytes_per_sector = bc.rdiskw(base + 11)\n"
		"  local sectors_per_cluster = bc.rdiskb(base + 0xd)\n"
		"  local reserved_sector_count = bc.rdiskw(base + 14)\n"
		"  -- print('reserved_sector_count: ' .. reserved_sector_count)\n"
		"  local table_count = bc.rdiskb(base + 16)\n"
		"  local table_size_16 = bc.rdiskw(base + 22)\n"
		"  local root_entry_count = bc.rdiskw(base + 17)\n"
		"  -- print('root_entry_count: ' .. root_entry_count)\n"
		"  local num_fats = bc.rdiskb(base + 0x10)\n"
		"  local sectors_per_fat = bc.rdiskw(base + 0x16)\n"
		"  -- print('num_fats: ' .. num_fats)\n"
		"  -- print('sectors_per_fat: ' .. sectors_per_fat)\n"
		"	 local root_dir_sectors = ((root_entry_count * 32) + (bytes_per_sector - 1)) / bytes_per_sector\n"
		"  local first_data_sector = reserved_sector_count + (table_count * table_size_16)\n"
		"  local data_sectors = total_sectors_16 - (reserved_sector_count + (table_count * table_size_16) + root_dir_sectors)\n"
		"  local total_clusters = data_sectors / bc.rdiskb(base + 13)\n"
		"  local fat_region = lba + reserved_sector_count\n"
		"  local root_directory_region = fat_region + num_fats * sectors_per_fat\n"
		"  -- temp hack - fix when we get division back!\n"
		"  local data_region = root_directory_region + 32\n"
		"  local fat12 = false\n"
		"  local fat16 = false\n"
		"  local fat32 = false\n"
		"  if total_clusters < 4085 then\n"
		"    fat12 = true\n"
		"  elseif total_clusters < 65525 then\n"
		"    fat16 = true\n"
		"  else\n"
		"    fat32 = true\n"
		"  end\n"
		"  -- print('bytes per fat sector is ' .. bytes_per_sector)\n"
		"  -- print('sectors per cluster is ' .. sectors_per_cluster)\n"
		"  if fat12 then print('FAT12') end\n"
		"  if fat16 then print('FAT16') end\n"
		"  if fat32 then print('FAT32') end\n"
		"  if (fat12 or fat32) then\n"
		"    print('TODO: fat12/fat32. sorry! (shouldn\\'t be much work, though.)')\n"
		"  else\n"
		"    -- print('first_data_sector: ' .. first_data_sector)\n"
		"    -- local sb = 512 * (first_data_sector + (root_entry_count * 32 / bytes_per_sector))\n"
		"    -- local sb = 512 * (first_data_sector + 32)\n"
		"    local sb = 512 * root_directory_region\n"
		"    -- print(root_entry_count * 32)\n"
		"    -- print(bytes_per_sector)\n"
		"    -- print(16384.0 / 512.0)\n"
		"    -- print((root_entry_count * 32) / bytes_per_sector)\n"
		"    -- print((first_data_sector + (root_entry_count * 32 / bytes_per_sector)))\n"
		"    -- print(sb)\n"
		"    local inc = 0\n"
		"    local j = 0\n"
		"    local foundlbc = false\n"
		"    -- print('TODO: interrogate FAT in case directory is long')\n"
		"    while (inc < (root_entry_count * 32)) do\n"
		"      -- print('j is ' .. j)\n"
		"      -- print('sb is ' .. sb)\n"
		"      -- print(' and inc is ' .. inc)\n"
		"      -- print('sb+inc == ' .. (sb + inc))\n"
		"      if ((bc.rdiskb(sb + inc) == 0) or (bc.rdiskb(sb + inc) == 0xe5)) then\n"
		"        -- print('no entry; continuing')\n"
		"      else\n"
		"        local a = ''\n"
		"        local b = ''\n"
		"        local attrib = bc.rdiskb(sb + inc + 11)\n"
		"        if ((attrib ~= 0xf) and (attrib ~= 0x8)) then\n"
		"          for i = 0,10 do\n"
		"            -- print('i stuff')\n"
		"            local c = bc.rdiskb(sb + inc + i)\n"
		"            if i == 8 then\n"
		"              a = a .. '.'\n"
		"            end\n"
		"            if ((c ~= 0) and (c ~= 0x20) and (c ~= 10)) then\n"
		"              a = a .. string.char(c)\n"
		"              b = b .. c .. ' '\n"
		"            end\n"
		"          end\n"
		"          -- is this the folder we're looking for?\n"
		"          if ((a == 'LBECAUSE.') and (bit64.band(bc.rdiskb(sb + inc + 11), 0x10) > 0)) then\n"
		"						 foundlbc = true\n"
		"            -- print('found LBECAUSE folder. this is the disk!')\n"
		"            -- print('given cluster is ' .. bc.rdiskw(sb + inc + 26))\n"
		"            --local lbecause_entry_sector = 512 * (sectors_per_cluster * bc.rdiskw(sb + inc + 26))\n"
		"            local lbecause_entry_sector = data_region + (sectors_per_cluster * (bc.rdiskw(sb + inc + 26) - 2))\n"
		"						 -- temp hack\n"
		"            --sb = lbecause_entry_sector\n"
		"            --sb = 501824\n"
		"            -- print('TODO: interrogate FAT in case subdirectory \\'lbecause\\' is long')\n"
		"            local sb2 = 512 * lbecause_entry_sector\n"
		"						 local inc2 = 0\n"
		"						 local lua_files = {}\n"
		"						 while (inc2 < 512) do\n"
		"							 if (bc.rdiskb(sb2 + inc2) == 0) then\n"
		"							   break\n"
		"              else\n"
		"								 local att = bc.rdiskb(sb2 + inc2 + 11)\n"
		"								 local a = ''\n"
		"								 if (att ~= 0xf) then\n"
		"								   for i = 0,10 do\n"
		"									   local c = bc.rdiskb(sb2 + inc2 + i)\n"
		"										 if i == 8 then\n"
		"										   a = a .. '.'\n"
		"										 end\n"
		"										 if ((c ~= 0) and (c ~= 0x20) and (c ~= 10)) then\n"
		"										   a = a .. string.char(c)\n"
		"										 end\n"
		"									 end\n"
		"								   if string.sub(a, -4) == '.LUA' then\n"
		"										 local size = bc.rdiski(sb2 + inc2 + 0x1c)\n"
		"									   --print('reading lua file ' .. a .. ' (' .. size .. ' bytes)')\n"
		"										 local cont = bc.rdiskw(sb2 + inc2 + 0x1a)\n"
		"										 bc.clearlbuffer()\n"
		"										 while ((cont > 0x0002) and (cont < 0xfff8)) do\n"
		"											 local d = data_region + (sectors_per_cluster * (cont - 2))\n"
		"											 -- read file part\n"
		"											 bc.addlbuffer(512 * d, sectors_per_cluster)\n"
		"											 --print(a .. ': [' .. string.format('%x', cont) .. '] ' .. d)\n"
		"										   cont = bc.rdiskw(512 * fat_region + 2 * cont)\n"
		"										 end\n"
		"										 local e = {a, bc.finishlbuffer(size)}\n"
		"										 if a == 'RUN.LUA' then\n"
		"										   table.insert(lua_files, e)\n"
		"										 else\n"
		"										   table.insert(lua_files, 1, e)\n"
		"										 end\n"
		"										 -- print(e[2])\n"
		"									 end\n"
		"								 end\n"
		"              end\n"
		"							 inc2 = inc2 + 32\n"
		"						 end\n"
		"						 print('* starting Lua *')\n"
		"						 for k,v in pairs(lua_files) do\n"
		"							 --print(v[1])\n"
		"						   assert(loadstring('-- ' .. v[1] .. '\\n' .. v[2]))()\n"
		"						 end\n"
		"					   print('Done.')\n"
		"						 break\n"
		"          end\n"
		"        end\n"
		"        --print(a)\n"
		"        -- print(b)\n"
		"      end\n"
		"			 inc = inc + 32\n"
		"      j = j + 1\n"
		"    end\n"
		"  	 if not foundlbc then print('Well, I failed to find LBECAUSE on a fat16 part.') end\n"
		"  end\n"
		"  return\n"
		"end\n"
		"\n"
		"print('TODO: check bus 0 slave disk, bus 1 master & slave')\n"
		"print('TODO: extended partitions')\n"
		"print('TODO: verify this is in fact an mbr')\n"
		"print('TODO: floppies, cds')\n"
		"-- print('is partition 1 bootable? ' .. (bc.rdiskb(446) == 0x80 and 'yes' or 'no'))\n"
		"-- print('partition 1 start lba is ' .. bc.rdiski(446 + 8))\n"
		"-- print('partition 1 total sectors is ' .. bc.rdiski(446 + 12))\n"
		"print('partition 1 part type ' .. bc.rdiskb(446 + 4))\n"
		"print('partition 2 part type ' .. bc.rdiskb(462 + 4))\n"
		"print('partition 3 part type ' .. bc.rdiskb(478 + 4))\n"
		"print('partition 4 part type ' .. bc.rdiskb(494 + 4))\n"
		"if is_fat(bc.rdiskb(446 + 4)) then\n"
		"  print('investigating partition 1')\n"
		"  fat_investigate(bc.rdiski(446 + 8))\n"
		"end\n"
		"if is_fat(bc.rdiskb(462 + 4)) then\n"
		"  print('investigating partition 2')\n"
		"  fat_investigate(bc.rdiski(462 + 8))\n"
		"end\n"
		"if is_fat(bc.rdiskb(478 + 4)) then\n"
		"  print('investigating partition 3')\n"
		"  fat_investigate(bc.rdiski(478 + 8))\n"
		"end\n"
		"if is_fat(bc.rdiskb(494 + 4)) then\n"
		"  print('investigating partition 4')\n"
		"  fat_investigate(bc.rdiski(494 + 8))\n"
		"end\n"
		;

	int lsv = luaL_loadstring(l, lcode);
	if (lsv != LUA_OK) {
		if (lsv == LUA_ERRSYNTAX) {
			printk("LUA_ERRSYNTAX\n");
		} else if (lsv == LUA_ERRMEM) {
			printk("LUA_ERRMEM\n");
		} else if (lsv == LUA_ERRGCMM) {
			printk("LUA_ERRGCMM\n");
		} else {
			printki(lsv); printk("\n");
		}
		reportlua(l);
		return;
	}

	int pcv = lua_pcall(l, 0, LUA_MULTRET, 0);
	if (pcv != LUA_OK) {
		if (pcv == LUA_ERRRUN) {
			printk("LUA_ERRRUN\n");
			reportlua(l);
		} else if (pcv == LUA_ERRMEM) {
			printk("LUA_ERRMEM\n");
		} else if (pcv == LUA_ERRGCMM) {
			printk("LUA_ERRGCMM\n");
		} else {
			printki(pcv); printk("\n");
		}
	}

}

