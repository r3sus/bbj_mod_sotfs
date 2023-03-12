state("DarkSoulsII")
{
}

init {
print("\n [ASL] ds2s nbj setup started \n");
var page = modules.First();
var scanner = new SignatureScanner(game, page.BaseAddress, page.ModuleMemorySize);

var AoBstr = "FF 0F 28 08 48 8B 47 10 ";

IntPtr addr = scanner.Scan(
new SigScanTarget(0, AoBstr) 
);

if (addr == (IntPtr)0) {
	print("not found: "+AoBstr);
}
else 
	print(AoBstr+addr.ToString()); 

var padr = addr+1+0xB4-0x95;

var AoB1 = new byte [] { 0x4C, 0x8B, 0x87, 0x18, 0xDB, 0xFF, 0xFF, 0xF3, 0x41, 0x0F, 0x10, 0x80, 0xE8, 0x00, 0x00, 0x00, 0x0F, 0x51, 0xC9};

int blen = AoB1.Length;	

MemPageProtect oldProtect;
game.VirtualProtect(addr, blen, MemPageProtect.PAGE_EXECUTE_READWRITE, out oldProtect);
memory.WriteBytes(padr,AoB1);
game.VirtualProtect(addr, blen, oldProtect);

print("\n [ASL] ds2s nbj setup finished \n");

}

// todo: add options: restore, mid jmp len, etc in the ui