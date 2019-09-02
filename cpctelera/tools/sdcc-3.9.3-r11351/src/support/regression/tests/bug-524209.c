/* Problem with casts and warnings.
 */
#include <testfwk.h>
#include <string.h>

typedef unsigned short UINT16;

typedef struct _Class Class;

typedef struct _Instance
{
  Class *pClass;
} Instance;

typedef struct _StringBuffer
{
  Instance header;
  UINT16 uLength;
  UINT16 uMaxLength;
} StringBuffer;


void _scan(StringBuffer *pSB)
{
  UNUSED(pSB);
}

void checkCast(void *pIn)
{
  Instance *p = (Instance *)pIn;
  _scan((StringBuffer *)p);
}

void testBug(void)
{
}
