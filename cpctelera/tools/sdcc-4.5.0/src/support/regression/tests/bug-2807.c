/* bug-2807.c
   Overwritten operand in code generation for division.
 */

#include <testfwk.h>

#include <stdint.h>

typedef struct
{
    uint8_t interval;
    uint8_t duration;
}
Buzzer_request;

typedef struct
{
    const Buzzer_request* request;
    uint8_t state;
}
Buzzer;

int f(const char *c)
{
  static int truecount;

  if(*c == 't')
    truecount++;

  ASSERT(truecount <= 1);

  return (0);
}

void Buzzer_tick(Buzzer* const self)
{
  const Buzzer_request* const request = self->request;

  const uint8_t state = self->state + 1U;

  self->state = state;

  if(request->duration && (state < request->duration))
  {
    f("stop\n");
  }
  else if (request->interval)
  {
    if ((state % request->interval) == 0U)
    {
      // This if statement is always true due to overwritten request->interval.
      if((state / request->interval) % 2U)
      {
        f("true\n");
      }
      else
      {
        f("false\n");
      }
    }
  }
}


void testBuzzer(void)
{
  Buzzer_request request = { 1, 0 };
  Buzzer buzzer = { &request, 0 };

  Buzzer_tick(&buzzer);
  Buzzer_tick(&buzzer);
}

