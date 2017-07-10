/* Bug 1908493
 * Bug test contains code fragments from qpnano framework,
 * Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
 * and released under the GPL
 * See www.quantum-leaps.com/downloads/index.htm#QPN
 */

#include <testfwk.h>

#define Q_REENTRANT __reentrant
#define Q_ASSERT
#define Q_SIG(me_) (((QFsm *)(me_))->evt.sig)
#define int8_t char
#define QEP_MAX_NEST_DEPTH 5

#define QEP_EMPTY_SIG 0

enum QReservedSignals {
  Q_ENTRY_SIG = 1,  /**< signal for coding entry actions */
  Q_EXIT_SIG,       /**< signal for coding exit actions */
  Q_INIT_SIG,       /**< signal for coding nested initial transitions */
  Q_TIMEOUT_SIG,    /**< signal used by time events */
  Q_USER_SIG        /**< first signal that can be used in user applications */
};

typedef int8_t QSignal;
typedef struct QEventTag {
    QSignal sig;
} QEvent;

struct QFsmTag;
struct QHsmTag;

typedef void (*QState) (struct QFsmTag *me);
typedef QState (*QHsmState) (struct QHsmTag *me);

typedef QState QSTATE;

typedef struct QFsmTag {
    QState state;
    QEvent evt;
} QFsm;

typedef struct QHsmTag {
    QHsmState state;
    QEvent evt;
} QHsm;

typedef struct QHsmDerivedTag {
    QHsm super;
    char value;
} QHsmDerived;

QHsmDerived AO_derived;

QSTATE
state_1 (QHsmDerived *me) Q_REENTRANT
{
  if (Q_SIG (me) == Q_INIT_SIG)
    me->value = 3;

  return (QSTATE)0;
}

QSTATE
state_2 (QHsmDerived *me) Q_REENTRANT
{
  if (Q_SIG (me) == Q_USER_SIG)
    return (QSTATE)state_1;

  return (QSTATE)0;
}

void
QHsm_dispatch (QHsm *me) Q_REENTRANT
{
  QHsmState path[QEP_MAX_NEST_DEPTH];
  QHsmState s;
  QHsmState t = me->state;

  path[1] = t;    /* save the current state in case a transition is taken */

  do                               /* process the event hierarchically... */
    {
      s = t;

      /********************************************************************
       *
       *    The call which fails when s is copied from the stack frame
       *
       ********************************************************************/
      t = (QHsmState)((*s) (me));               /* invoke state handler s */

    }
  while (t != (QHsmState)0);

  if (me->evt.sig == (QSignal)0)                     /* transition taken? */
    {
      QHsmState src = s;                  /* the source of the transition */
      int8_t ip = (int8_t)(-1);            /* transition entry path index */

      path[0] = me->state;                          /* save the new state */
      me->state = path[1];                   /* restore the current state */

                    /* exit current state to the transition source src... */
      for (s = path[1]; s != src; )
        {
          Q_SIG (me) = (QSignal)Q_EXIT_SIG;
          t = (QHsmState)(*s) (me);               /* find superstate of s */
          if (t != (QHsmState)0)                 /* exit action unhandled */
            {
              s = t;                            /* t points to superstate */
            }
          else                                     /* exit action handled */
            {
              Q_SIG (me) = (QSignal)QEP_EMPTY_SIG;
              s = (QHsmState)(*s) (me);           /* find superstate of s */
            }
        }

        t = path[0];                            /* target of the transition */
    }
}

void
testBug (void)
{
    AO_derived.super.state = (QHsmState)state_2;
    AO_derived.super.evt.sig = 2;

    QHsm_dispatch ((QHsm *)&AO_derived);

    ASSERT (1);     /*if we don't get here the regression test will timeout */
}
