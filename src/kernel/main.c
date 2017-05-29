
int i = 10;

static char c[10];

int main1()
{
  c[0] = 10;
  return c[2];
}

int main()
{
  unsigned short* ptr = (unsigned short*)0xbffb8000;
  for (int i = 0; i < 80*25 - 1; i++)
    *ptr++ = 0x7145;

  for(;;);
}
