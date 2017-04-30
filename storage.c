/* INCLUDE FILES
 */

#include <stdlib.h>
#include <stdio.h>
#include "storage.h"

/* TYPEDEFINITIES
 */

typedef struct TempBlok *TempBlokPtr;
struct TempBlok
{
    unsigned int grootte;
    unsigned int vrij;
    char *top;
    TempBlokPtr volgende;
};

/* LOCALE PROTOTYPES
 */

static TempBlokPtr NieuwTempBlok(void);

/* CONSTANTEN
 */

#define MIN_BLOKGROOTTE 32768

/* GLOBALE VARIABELEN
 */

static char *blok = NULL;
static long int ruimte_over = 0;
static long int laatste_allocatiegrootte = 131072;
long int gebruikt_geheugen = 0;
static TempBlokPtr temp_blok = NULL;
static TempBlokPtr vrije_blokken = NULL;
static char *stackTop = NULL;
static TempBlokPtr stackBlok = NULL;

#if !defined ALIGNMENTSIZE
#if defined __alpha
#define ALIGNMENTSIZE 8
#else
#define ALIGNMENTSIZE 4
#endif
#endif

/* NIEUW_TEMPBLOK
 */
static TempBlokPtr NieuwTempBlok(void)
{
    TempBlokPtr nieuw_blok;
    unsigned int blokruimte;

    if (vrije_blokken != NULL)
    {
	nieuw_blok = vrije_blokken;
	vrije_blokken = vrije_blokken->volgende;
    }
    else
    {
	if (0 < ruimte_over && ruimte_over < 32768)
	{
	    blokruimte = ruimte_over;
	}
	else
	{
	    blokruimte = 32768;
	}
	nieuw_blok = (TempBlokPtr) mmalloc(blokruimte);
	nieuw_blok->grootte = blokruimte;
    }
    nieuw_blok->vrij = nieuw_blok->grootte - sizeof(struct TempBlok);
    nieuw_blok->top = (char *) nieuw_blok + sizeof(struct TempBlok);
    if ((long int) nieuw_blok->top % ALIGNMENTSIZE != 0)
    {
	int v = (long int) nieuw_blok->top % ALIGNMENTSIZE;
	nieuw_blok->vrij -= ALIGNMENTSIZE - v;
	nieuw_blok->top += ALIGNMENTSIZE - v;
    }
    return (nieuw_blok);
}

/* GEEXPORTEERDE FUNCTIEDEFINITIES
 */

/* MMALLOC
   [extern] char *mmalloc(unsigned int grootte);
   Geeft een pointer naar een blok van (minstens) ``grootte'' aantal bytes, of
   NULL als de gewenste hoeveelheid niet meer beschikbaar is, terug.
 */
char *mmalloc(grootte)
unsigned int grootte;
{
     char *w;

     if (grootte > ruimte_over)
     {
          blok = NULL;
          while (laatste_allocatiegrootte >= 8 + grootte &&
                 laatste_allocatiegrootte >= MIN_BLOKGROOTTE && blok == NULL)
          {
               blok = malloc(laatste_allocatiegrootte - 8);
               if (blok == NULL)
               {
                    laatste_allocatiegrootte /= 2;
               }
          }
          if (blok == NULL || grootte > laatste_allocatiegrootte - 8)
          {
               return (NULL);
          }
          gebruikt_geheugen += laatste_allocatiegrootte;
          ruimte_over = laatste_allocatiegrootte - 8;
     }
     w = blok;
     if (grootte % ALIGNMENTSIZE != 0)
     {
	 grootte += ALIGNMENTSIZE - grootte % ALIGNMENTSIZE;
     }
     blok += grootte;
     ruimte_over -= grootte;
     return (w);
}

/* TEMPORARY MEMORY ALLOCATION
   temp_malloc() alloceert geheugen a la mmalloc() met dien verstande dat al het geheugen
   dat via temp_malloc() is gealloceerd na een push_tempmalloc() wordt vrijgegeven bij de
   overeenkomstige pop_tempmalloc(). temp_malloc() dient ook altijd voorafgegaan te worden
   door een push_tempmalloc()!
 */

/* PUSH_TEMPMALLOC
 */
void push_tempmalloc()
{
    TempBlokPtr nieuw_blok;

    if (temp_blok == NULL || temp_blok->vrij < 64)
    {
	if ((nieuw_blok = NieuwTempBlok()) == NULL)
	{
	    (void) fprintf(stderr, "Out of temp memory!\n");
	    exit(1);
	}
	else
	{
	    nieuw_blok->volgende = temp_blok;
	    temp_blok = nieuw_blok;
	}
    }
    *((TempBlokPtr *) temp_blok->top) = stackBlok;
    temp_blok->top += sizeof(TempBlokPtr);
    temp_blok->vrij -= sizeof(TempBlokPtr);
    *((char **) temp_blok->top) = stackTop;
    temp_blok->top += sizeof(char *);
    temp_blok->vrij -= sizeof(char *);
    *((unsigned int *) temp_blok->top) = temp_blok->vrij;
    temp_blok->top += sizeof(unsigned int);
    temp_blok->vrij -= sizeof(unsigned int);
    if ((long int) temp_blok->top % ALIGNMENTSIZE != 0)
    {
	int v = (long int) temp_blok->top % ALIGNMENTSIZE;
	temp_blok->vrij -= ALIGNMENTSIZE - v;
	temp_blok->top += ALIGNMENTSIZE - v;
    }
    stackTop = temp_blok->top;
    stackBlok = temp_blok;
}

/* TEMP_MALLOC
   temp_malloc() is niet geschikt voor blokken van meer dan 32752 bytes,
   maar wie gebruikt die nou?
 */
char *temp_malloc(grootte)
unsigned int grootte;
{
    char *w;
    TempBlokPtr nieuw_blok;

    while (grootte > temp_blok->vrij)
    {
	if ((nieuw_blok = NieuwTempBlok()) == NULL)
	{
	    (void) fprintf(stderr, "Out of temp memory!\n");
	    exit(1);
	}
	else
	{
	    nieuw_blok->volgende = temp_blok;
	    temp_blok = nieuw_blok;
	}
    }
    w = temp_blok->top;
    if (grootte % ALIGNMENTSIZE != 0)
    {
	grootte += ALIGNMENTSIZE - grootte % ALIGNMENTSIZE;
    }
    temp_blok->top += grootte;
    temp_blok->vrij -= grootte;
    return (w);
}

void pop_tempmalloc()
{
    TempBlokPtr tbp;

    while (temp_blok != stackBlok)
    {
	tbp = temp_blok->volgende;
	temp_blok->volgende = vrije_blokken;
	vrije_blokken = temp_blok;
	temp_blok = tbp;
    }
    temp_blok->top = stackTop;
    temp_blok->vrij += sizeof(unsigned int);
    temp_blok->top -= sizeof(unsigned int);
    if ((long int) temp_blok->top % ALIGNMENTSIZE != 0)
    {
	int v = (long int) temp_blok->top % ALIGNMENTSIZE;
	temp_blok->vrij += v;
	temp_blok->top += v;
    }
    temp_blok->vrij = *((unsigned int *) temp_blok->top);
    temp_blok->vrij += sizeof(char *);
    temp_blok->top -= sizeof(char *);
    stackTop = *((char **) temp_blok->top);
    temp_blok->vrij += sizeof(TempBlokPtr);
    temp_blok->top -= sizeof(TempBlokPtr);
    stackBlok = *((TempBlokPtr *) temp_blok->top);
}
