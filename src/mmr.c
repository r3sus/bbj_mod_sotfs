
 byte asm_sz(void *asm1) {
WORD w1 = 0xc390; byte x=0;
for (byte i = 0; i < 30; i++)
if (memcmp(asm1+i,&w1,2) == 0) 
{x = i;break;} 
return x;
 }

byte usz(void *src, byte sz)
{
if (sz == 'a') sz = asm_sz(src);
if (sz == 'b') sz = sizeof(src);
return sz;
}

void wp(void *dst, void *src, byte sz)
{
if (!dst) return;
sz = usz(src, sz);
if (sz == 0) return;

DWORD op1 = 0x40;
 VirtualProtect(dst, sz, op1, &op1);
memcpy(dst,src,sz);
 VirtualProtect(dst, sz, op1, &op1);
}