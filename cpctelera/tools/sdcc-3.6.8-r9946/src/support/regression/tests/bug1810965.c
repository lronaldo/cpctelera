/*
   bug1810965.c
*/

#include <testfwk.h>

/* bug 1810965 */
typedef union data_packet
{
	struct
	{
		enum
		{
			READ_VERSION = 0x00,
			RESET = 0xFF
		}cmd;
	}t;
} data_packet;

data_packet dataPacket;

void bug1810965(void)
{
	switch(dataPacket.t.cmd)
	{
	case READ_VERSION:		//error 20: Undefined identifier 'READ_VERSION'
					//error 62: 'case' expression not constant. Statement ignored
		break;
	}
}

/* bug 2698805 */
static void foo(void)
{
	enum foo_tag { e_foo };
	volatile unsigned char a = e_foo;
}

static void bar(void)
{
	enum foo_tag { e_foo };		//error 163: duplicate enum member 'e_foo'
					//error 0: Duplicate symbol 'e_foo', symbol IGNORED
					//error 51: typedef/enum 'foo_tag' duplicate. Previous definiton Ignored
	volatile unsigned char a = e_foo;
}

void testBug(void)
{
	ASSERT(1);
}
