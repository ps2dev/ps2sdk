float inf = 1.0f/0.0f;
float nnn = 1.0f/0.0f - 1.0f/0.0f;
float fin = 1.0f;
float neg = -1.0f;
float nn2;

int isnanf(), isfinitef(), signbitf();

void pvalue (char *str, float x)
{
union
  {
    float f;
    unsigned int i;
  }u;

printf("%s ", str);
u.f = x;
printf("%08x\n", u.i);
}


int
main()
{

if (!isnanf(nnn))
  abort();
pvalue("nnn", nnn);
pvalue("inf", inf);
nn2 = inf - inf;
pvalue("inf - inf", nn2);
if (isnanf(fin))
  abort();
if (isnanf(inf))
  abort();
if (!isfinitef(fin))
  abort();
if (isfinitef(nnn))
  abort();
if (isfinitef(inf))
  abort();
if (!signbitf(neg))
  abort();
if (signbitf(fin))
  abort();
if (signbitf(inf))
  abort();
/*
if (signbitf(nnn))
  abort();
  */
exit (0);
}
