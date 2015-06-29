/*
    bug 1938300
*/

#include <testfwk.h>

#if (defined PORT_HOST) || defined (__SDCC_STACK_AUTO)

/*****************************************************************************
* Product; Jongle Reconfigurable USB hardware
* Last Updated for Version;
* Date of the Last Update;
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information;
* Quantum Leaps Web site;  http;//www.quantum-leaps.com
* e-mail;                  info@quantum-leaps.com
*****************************************************************************/
#ifndef qpn_port_h
#define qpn_port_h

/* #include "ezusbfx2.h" */

#define NEAR  __near
#define IDATA __idata
#define XDATA __xdata
#define CODE  __code
#define SFR_T __sfr
#define BIT_T __bit

#define QF_ISR_NEST
/* #define Q_ROM_VAR               CODE */
/* #define Q_ROM_PTR(X)            (X) */
#define Q_REENTRANT             /* __reentrant */
#define Q_ROM                   CODE
#define Q_PARAM_SIZE            0
#define QF_TIMEEVT_CTR_SIZE     2
#define Q_NFSM
/*#define QF_FSM_ACTIVE*/
#define QF_MAX_ACTIVE           4
#define QF_TIMEEVT_CTR_SIZE     2

                                /* interrupt locking policy for task level */
#define QF_INT_LOCK()           EA = 0
#define QF_INT_UNLOCK()         EA = 1

#define __disable_interrupt()   EA = 0
#define __enable_interrupt()    EA = 1

                            /* interrupt locking policy for interrupt level */
/* #define QF_ISR_NEST */                    /* nesting of ISRs not allowed */

/*#include <intrinsics.h>  * contains prototypes for the intrinsic functions */
#include <stdint.h>    /* Exact-width integer types. WG14/N843 C99 Standard */

#endif                                                        /* qpn_port_h */



/*****************************************************************************
* Product; QEP-nano public interface
* Last Updated for Version; 3.4.01
* Date of the Last Update;  Sep 24, 2007
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information;
* Quantum Leaps Web site;  http;//www.quantum-leaps.com
* e-mail;                  info@quantum-leaps.com
*****************************************************************************/
#ifndef qepn_h
#define qepn_h

/** \ingroup qepn qfn qkn
* \file qepn.h
* \brief Public QEP-nano interface.
*
* This header file must be included in all modules that use QP-nano.
* Typically, this header file is included indirectly through the
* header file qpn_port.h.
*/

#ifndef Q_ROM             /* if NOT defined, provide the default definition */

    /** \brief Macro to specify compiler-specific directive for placing a
    * constant object in ROM.
    *
    * Many compilers for 8-bit Harvard-architecture MCUs provide non-stanard
    * extensions to support placement of objects in different memories.
    * In order to conserve the precious RAM, QP-nano uses the Q_ROM macro for
    * all constant objects that can be allocated in ROM.
    *
    * To override the following empty definition, you need to define the
    * Q_ROM macro in the qpn_port.h header file. Some examples of valid
    * Q_ROM macro definitions are; __code (IAR 8051 compiler), code (Keil
    * 8051 compiler), PROGMEM (gcc for AVR), __flash (IAR for AVR).
    */
    #define Q_ROM
#endif
#ifndef Q_ROM_VAR         /* if NOT defined, provide the default definition */

    /** \brief Macro to specify compiler-specific directive for accessing a
    * constant object in ROM.
    *
    * Many compilers for 8-bit MCUs provide different size pointers for
    * accessing objects in various memories. Constant objects allocated
    * in ROM (see #Q_ROM macro) often mandate the use of specific-size
    * pointers (e.g., far pointers) to get access to ROM objects. The
    * macro Q_ROM_VAR specifies the kind of the pointer to be used to access
    * the ROM objects.
    *
    * To override the following empty definition, you need to define the
    * Q_ROM_VAR macro in the qpn_port.h header file. An example of valid
    * Q_ROM_VAR macro definition is; __far (Freescale HC(S)08 compiler).
    */
    #define Q_ROM_VAR
#endif
#ifndef Q_REENTRANT       /* if NOT defined, provide the default definition */

    /** \brief Macro to specify compiler-specific directive for generating
    * reentrant function.
    *
    * Some compilers for 8-bit MCUs provide, most notably the Keil C51
    * compiler for 8051, don't generate ANSI-C compliant reentrant functions
    * by default, due to the limited hardware architecture. These compilers
    * allow to dedicate specific functions to be reentrant with a special
    * extended keyword (such as "reentrant" for Keil C51). The macro
    * Q_REENTRANT is defined to nothing by default, to work with ANSI-C
    * compiliant compilers, but can be defined to "reentrant" to work with
    * Keil C51 and perhpas other compilers.
    */
    #define Q_REENTRANT
#endif

/****************************************************************************/
/** helper macro to calculate static dimension of a 1-dim array \a array_ */
#define Q_DIM(array_) (sizeof(array_) / sizeof(array_[0]))

/****************************************************************************/
/** \brief get the current QP version number string
*
* \return version of the QP as a constant 6-character string of the form
* x.y.zz, where x is a 1-digit major version number, y is a 1-digit minor
* version number, and zz is a 2-digit release number.
*/
char const Q_ROM * Q_ROM_VAR QP_getVersion(void);

/** \brief Scalar type describing the signal of an event.
*/
typedef uint8_t QSignal;

/****************************************************************************/
#ifndef Q_PARAM_SIZE
    /** \brief macro to define the size of event parameter.
    * Valid values 0, 1, 2, or 4; default 0
    */
    #define Q_PARAM_SIZE 0
#endif
#if (Q_PARAM_SIZE == 0)
#elif (Q_PARAM_SIZE == 1)

    /** \brief type of the event parameter.
    *
    * This typedef is configurable via the preprocessor switch #Q_PARAM_SIZE.
    * The other possible values of this type are as follows; \n
    * none when (Q_PARAM_SIZE == 0); \n
    * uint8_t when (Q_PARAM_SIZE == 1); \n
    * uint16_t when (Q_PARAM_SIZE == 2); and \n
    * uint32_t when (Q_PARAM_SIZE == 4).
    */
    typedef uint8_t QParam;
#elif (Q_PARAM_SIZE == 2)
    typedef uint16_t QParam;
#elif (Q_PARAM_SIZE == 4)
    typedef uint32_t QParam;
#else
    #error "Q_PARAM_SIZE defined incorrectly, expected 0, 1, 2, or 4"
#endif

/** \brief Event structure.
*
* QEvent represents events, optionally with a single scalar parameter.
* \sa Q_PARAM_SIZE
* \sa ;;QParam
*/
typedef struct QEventTag {
    QSignal sig;                                   /**< signal of the event */

#if (Q_PARAM_SIZE != 0)
    QParam par;                          /**< scalar parameter of the event */
#endif
} QEvent;

/****************************************************************************/
/** \brief QP reserved signals */
enum QReservedSignals {
    Q_ENTRY_SIG = 1,                   /**< signal for coding entry actions */
    Q_EXIT_SIG,                         /**< signal for coding exit actions */
    Q_INIT_SIG,           /**< signal for coding nested initial transitions */
    Q_TIMEOUT_SIG,                          /**< signal used by time events */
    Q_USER_SIG      /**< first signal that can be used in user applications */
};

/****************************************************************************/
struct QFsmTag;                                      /* forward declaration */

        /** \brief the signature of non-hierarchical state handler function */
typedef void (*QState)(NEAR struct QFsmTag *me);

/** \brief Finite State Machine.
*
* QFsm represents a traditional non-hierarchical Finite State Machine (FSM)
* without state hierarchy, but with entry/exit actions.
*
* \note QFsm is not intended to be instantiated directly, but rather serves
* as the base structure for derivation of state machines in the application
* code.
*
* The following example illustrates how to derive a state machine structure
* from QFsm. Please note that the QFsm member super_ is defined as the FIRST
* member of the derived struct.
* \include qepn_qfsm.c
*
* \sa \ref derivation
*/
typedef struct QFsmTag {
    QState state;            /**< current active state of the FSM (private) */
    QEvent evt;       /**< currently processed event in the FSM (protected) */
} QFsm;

/** \brief macro to access the signal of the current event of a state machine
*
* \sa ;;QFsm ;;QHsm
*/
#define Q_SIG(me_)  (((QFsm *)(me_))->evt.sig)

#if (Q_PARAM_SIZE != 0)
/** \brief macro to access the parameter of the current event of
* a state machine
*
* \sa ;;QFsm ;;QHsm Q_PARAM_SIZE
*/
#define Q_PAR(me_)  (((QFsm *)(me_))->evt.par)
#endif

#ifndef Q_NFSM

/** \brief State machine constructor.
*
* \param me_ pointer the state machine structure derived from ;;QHsm.
* \param initial_ is the pointer to the initial state of the state machine.
* \note Must be called only ONCE before taking the initial transition
* with QFsm_init() and dispatching any events via QFsm_dispatch().
*/
#define QFsm_ctor(me_, initial_) do { \
    ((QFsm *)me_)->state = (QState)(initial_); \
    Q_SIG(me_) = (QSignal)Q_INIT_SIG; \
} while (0)

/** \brief Initializes a FSM
*
* Takes the top-most initial transition in a FSM.
* \param me is the pointer the state machine structure derived from ;;FHsm.
*
* \note Must be called only ONCE after QFsm_ctor() and before any calls
* to QFsm_dispatch().
*/
void QFsm_init(NEAR QFsm *me);

/** \brief Dispatches an event to a FSM
*
* Processes one event at a time in Run-to-Completion fashion. The argument
* \a me is the pointer the state machine structure derived from ;;QFsm.
*
* \note Must be called after QFsm_init().
*/
void QFsm_dispatch(NEAR QFsm *me) Q_REENTRANT;

/* protected methods */

/** \brief Returns current active state of a FSM.
*
* \note this is a protected function to be used only inside state handler
* functions.
*/
#define QFsm_getState(me_)  ((QState const)((QFsm *)(me_))->state)

#endif                                                            /* Q_NFSM */

/** \brief Designates a target for an initial or regular transition.
*
* Q_TRAN() can be used both in the FSMs and HSMs;
*
* \include qepn_qtran.c
*/
#define Q_TRAN(target_) do { \
    ((QFsm *)me)->state = (QState)(target_); \
    ((QFsm *)me)->evt.sig = (QSignal)0; \
} while (0)

/****************************************************************************/
#ifndef Q_NHSM

struct QHsmTag;                                      /* forward declaration */

                 /** \brief the signature of state handler function for HSM */
typedef QState (*QHsmState)(NEAR struct QHsmTag *me);

  /** \brief a name for the return type from the HSM state handler function */
typedef QState QSTATE;

/** \brief a Hierarchical State Machine.
*
* QHsm represents a Hierarchical Finite State Machine (HSM). QHsm
* extends the capabilities of a basic FSM with state hierarchy.
*
* \note QHsm is not intended to be instantiated directly, but rather serves
* as the base structure for derivation of state machines in the application
* code.
*
* The following example illustrates how to derive a state machine structure
* from QHsm. Please note that the QHsm member super_ is defined as the FIRST
* member of the derived struct.
* \include qepn_qhsm.c
*
* \sa \ref derivation
*/
typedef struct QHsmTag {
    QHsmState state;         /**< current active state of the HSM (private) */
    QEvent evt;       /**< currently processed event in the HSM (protected) */
} QHsm;

/* public methods */
/** \brief State machine constructor.
*
* \param me_ pointer the state machine structure derived from ;;QHsm.
* \param initial_ is the pointer to the initial state of the state machine.
* \note Must be called only ONCE before taking the initial transition
* with QHsm_init() and dispatching any events via QHsm_dispatch().
*/
#define QHsm_ctor(me_, initial_) do { \
    ((QHsm *)me_)->state  = (QHsmState)(initial_); \
    Q_SIG(me_) = (QSignal)Q_INIT_SIG; \
} while (0)

/** \brief Initializes a HSM.
*
* Takes the top-most initial transition in a HSM.
* \param me is the pointer the state machine structure derived from ;;QHsm.
*
* \note Must be called only ONCE after QHsm_ctor() and before any calls
* to QHsm_dispatch().
*/
void QHsm_init(NEAR QHsm *me);

/** \brief Dispatches an event to a HSM
*
* Processes one event at a time in Run-to-Completion fashion.
* \param me is the pointer the state machine structure derived from ;;QHsm.
*
* \note Must be called repetitively for each event after QHsm_init().
*/
void QHsm_dispatch(NEAR QHsm *me) Q_REENTRANT;

/* protected methods... */

/** \brief The top-state.
*
* QHsm_top() is the ultimate root of state hierarchy in all HSMs derived
* from ;;QHsm. This state handler always returns (QSTATE)0, which means
* that it "handles" all events.
*
* \sa Example of the QCalc_on() state handler for Q_INIT().
*/
QSTATE QHsm_top(NEAR QHsm *me) Q_REENTRANT;

/** \return the current active state of the \a me_ state machine  */
#define QHsm_getState(me_)  ((QHsmState const)((QHsm *)(me_))->state)

#endif                                                            /* Q_NHSM */


/****************************************************************************/
/*  DEPRECATED  DEPRECATED  DEPRECATED  DEPRECATED  DEPRECATED  DEPRECATED  */

/* If QPN compatibility level not defined or the level is lower than v3.4    */
#if (!defined(QPN_COMP_LEVEL) || (QPN_COMP_LEVEL < 34))

/** signal for coding the top-most initial transition (deprecated) */
#define Q_TOP_INIT_SIG      Q_INIT_SIG

/** \brief Designates a target for an initial transition.
*
* \note DEPRECATED
* \sa Q_TRAN()
*/
#define Q_INIT(target_)     Q_TRAN(target_)

/** \brief Returns current active state of a FSM.
*
* \note DEPRECATED
* \sa QFsm_getState()
*/
#define QFsm_getState_(me_) QFsm_getState(me_)

/** \brief Returns current active state of a HSM.
*
* \note DEPRECATED
* \sa QHsm_getState()
*/
#define QHsm_getState_(me_) QHsm_getState(me_)

#endif                                               /* QPN_COMP_LEVEL < 34 */


#endif                                                            /* qepn_h */



/*****************************************************************************
* Product; QF-nano public interface
* Last Updated for Version; 3.4.01
* Date of the Last Update;  Sep 18, 2007
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information;
* Quantum Leaps Web site;  http;//www.quantum-leaps.com
* e-mail;                  info@quantum-leaps.com
*****************************************************************************/
#ifndef qfn_h
#define qfn_h

/** \ingroup qepn qfn qkn
* \file qfn.h
* \brief Public QF-nano interface.
*
* This header file must be included in all modules that use QP-nano.
* Typically, this header file is included indirectly through the
* header file qpn_port.h.
*/

#ifndef QF_TIMEEVT_CTR_SIZE
    /** \brief macro to override the default QTimeEvtCtr size.
    * Valid values 0, 1, 2, or 4; default 0
    */
    #define QF_TIMEEVT_CTR_SIZE 0
#endif
#if (QF_TIMEEVT_CTR_SIZE == 0)
#elif (QF_TIMEEVT_CTR_SIZE == 1)
    typedef uint8_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 2)
    /** \brief type of the Time Event counter, which determines the dynamic
    * range of the time delays measured in clock ticks.
    *
    * This typedef is configurable via the preprocessor switch
    * #QF_TIMEEVT_CTR_SIZE. The other possible values of this type are
    * as follows; \n
    * none when (QF_TIMEEVT_CTR_SIZE not defined or == 0), \n
    * uint8_t when (QF_TIMEEVT_CTR_SIZE == 1); \n
    * uint16_t when (QF_TIMEEVT_CTR_SIZE == 2); and \n
    * uint32_t when (QF_TIMEEVT_CTR_SIZE == 4).
    */
    typedef uint16_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 4)
    typedef uint32_t QTimeEvtCtr;
#else
    #error "QF_TIMER_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

/** \brief Active Object struct
*
* QActive is the base structure for derivation of active objects. Active
* objects in QF-nano are encapsulated tasks (each embedding a state machine
* and an event queue) that communicate with one another asynchronously by
* sending and receiving events. Within an active object, events are
* processed sequentially in a run-to-completion (RTC) fashion, while QF
* encapsulates all the details of thread-safe event exchange and queuing.
*
* \note ;;QActive is not intended to be instantiated directly, but rather
* serves as the base structure for derivation of active objects in the
* application code.
*
* The following example illustrates how to derive an active object from
* QActive. Please note that the QActive member super_ is defined as the
* FIRST member of the derived struct.
* \include qfn_qactive.c
*
* \sa ;;QActiveTag for the description of the data members \n \ref derivation
*/
typedef struct QActiveTag {

#if (!defined(QF_FSM_ACTIVE) && !defined(Q_NHSM))
    QHsm super;                 /**< derives from the ;;QHsm base structure */
#else
    QFsm super;                 /**< derives from the ;;QFsm base structure */
#endif

    /** \brief offset to where next event will be inserted into the buffer
    */
    uint8_t head;

    /** \brief offset of where next event will be extracted from the buffer
    */
    uint8_t tail;

    /** \brief number of events currently present in the ring buffer
    */
    uint8_t nUsed;

#if (QF_TIMEEVT_CTR_SIZE != 0)
    /** \brief Time Event tick counter for the active object
    */
    QTimeEvtCtr tickCtr;
#endif
} QActive;

#if (!defined(QF_FSM_ACTIVE) && !defined(Q_NHSM))
    /** \brief Active object constructor.
    *
    * \param me_ pointer the active object structure derived from ;;QActive.
    * \param initial_ is the pointer to the initial state of the active
    * object.
    * \note Must be called exactly ONCE for each active object
    * in the application before calling QF_run().
    */
    #define QActive_ctor(me_, initial_) do { \
        QHsm_ctor(me_, initial_); \
        ((QActive *)(me_))->nUsed = (uint8_t)0; \
    } while (0)
#else
    #define QActive_ctor(me_, initial_) do { \
        QFsm_ctor(me_, initial_); \
        ((QActive *)(me_))->nUsed = (uint8_t)0; \
    } while (0)
#endif


#if (Q_PARAM_SIZE != 0)
    /** \brief Posts an event \a e directly to the event queue of the acitve
    * object \a prio using the First-In-First-Out (FIFO) policy. This
    * function briefly locks and unlocks interrupts to protect the
    * queue integrity.
    *
    * Direct event posting is the only asynchronous communication method
    * available in QF-nano. The following example illustrates how the
    * Ped active object posts directly the PED_WAITING event to the PELICAN
    * crossing active object.
    * \include qfn_post.c
    *
    * \note The producer of the event (Ped in this case) must only "know"
    * the recipient's priority (Pelican), but the specific definition of
    * the Pelican structure is not required.
    *
    * \note Direct event posting should not be confused with direct event
    * dispatching. In contrast to asynchronous event posting through event
    * queues, direct event dispatching is synchronous. Direct event
    * dispatching occurs when you call QHsm_dispatch(), or QFsm_dispatch()
    * function.
    */
    void QF_post(uint8_t prio, QSignal sig, QParam par) Q_REENTRANT;

    /** \brief Posts an event \a e directly to the event queue of the acitve
    * object \a prio using the First-In-First-Out (FIFO) policy. This
    * function does NOT lock/unlock interrupts and is intended only
    * to be used inside critical sections (such as inside ISRs that cannot
    * nest).
    *
    * \sa QF_post()
    */
    void QF_postISR(uint8_t prio, QSignal sig, QParam par) Q_REENTRANT;
#else
    void QF_post(uint8_t prio, QSignal sig) Q_REENTRANT;
    void QF_postISR(uint8_t prio, QSignal sig) Q_REENTRANT;
#endif

/****************************************************************************/
/** \brief QActive Control Block
*
* QActiveCB represents the constant information that the QF-nano needs
* to manage the active object. QActiveCB objects are grouped in the
* array QF_active[], which typically can be placed in ROM.
*
* The following example illustrates how to allocate and initialize the
* QActive control blocks in the array QF_active[].
* \include qfn_main.c
*/
typedef struct QActiveCBTag {
    QActive *act;        /**< \brief pointer to the active object structure */
    QEvent *queue;            /**< \brief pointer to the event queue buffer */
    uint8_t end;                  /**< \brief the length of the ring buffer */
} QActiveCB;

#if (QF_TIMEEVT_CTR_SIZE != 0)

#if (QF_TIMEEVT_CTR_SIZE > 1)

/** \brief Arm a one-shot time event for direct event posting.
*
* Arms a time event \a me to fire in \a tout clock ticks
* (one-shot time event). The timeout signal Q_TIMEOUT_SIG gets directly
* posted (using the FIFO policy) into the event queue of the active object
* \a me.
*
* After posting, the time event gets automatically disarmed and can be reused.
*
* A one-shot time event can be disarmed at any time by calling the
* QActive_disarm() function. Also, a one-shot time event can be re-armed
* to fire in a different number of clock ticks by calling QActive_arm() again.
*
* The following example shows how to arm a one-shot time event from a state
* machine of an active object;
* \include qfn_arm.c
*/
void QActive_arm(NEAR QActive *me, QTimeEvtCtr tout) Q_REENTRANT;

/** \brief Disarm a time event.
*
* The time event \a me gets disarmed and can be reused.
*/
void QActive_disarm(NEAR QActive *me) Q_REENTRANT;

#else                                   /* QF_TIMEEVT_CTR_SIZE must be == 1 */

/** \brief Arm a one-shot time event for direct event posting.
*
* Arms a time event \param me_ to fire in \param tout_ clock ticks
* (one-shot time event). The timeout signal Q_TIMEOUT_SIG gets directly
* posted (using the FIFO policy) into the event queue of the active object
* \param me_.
*
* After posting, the time event gets automatically disarmed and can be reused.
*
* A one-shot time event can be disarmed at any time by calling the
* QActive_disarm() function. Also, a one-shot time event can be re-armed
* to fire in a different number of clock ticks by calling QActive_arm() again.
*
* The following example shows how to arm a one-shot time event from a state
* machine of an active object;
* \include qfn_arm.c
*/
#define QActive_arm(me_, tout_) ((me_)->tickCtr = (QTimeEvtCtr)(tout_))

/** \brief Disarm a time event.
*
* The time event \param me_ gets disarmed and can be reused.
*/
#define QActive_disarm(me_)     ((me_)->tickCtr = (QTimeEvtCtr)0)

#endif                                           /* QF_TIMEEVT_CTR_SIZE > 1 */

/** \brief Processes all armed time events at every clock tick.
*
* This function must be called periodically from a time-tick ISR or from
* the highest-priority task so that QF can manage the timeout events.
*
* \note The QF_tick() function is not reentrant meaning that it must run to
* completion before it is called again. Also, QF_tick() assumes that it
* never will get preempted by a task, which is always the case when it is
* called from an ISR or the highest-priority task.
*
* The following example illustrates the call to QF_tick();
* \include qfn_tick.c
*/
void QF_tick(void);

#endif                                        /* (QF_TIMEEVT_CTR_SIZE != 0) */

/* protected methods ...*/

/** \brief QF initialization.
*
* This function initializes QF and must be called exactly once before any
* other QF function. In QF-nano this function is defined in the BSP.
*/
void QF_init(void);

/** \brief Starts the interrupts and initializes other critical resources
* that might interact with the QF application.
*
* QF_start() is called from QF_run(), right before starting the non-preemptive
* multitasking in the background loop.
*
* \note This function is strongly platform-dependent and is not implemented
* in the QF, but either in the QF port or in the Board Support Package (BSP)
* for the given application.
*
* \sa QF initialization example for ;;QActiveCB.
*/
void QF_start(void);

/** \brief Transfers control to QF to run the application.
*
* QF_run() implemetns the simple non-preemptive scheduler. QF_run() must be
* called from your startup code after you initialize the QF and define at
* least one active object control block in QF_active[].
*
* \note When the Quantum Kernel (QK) is used as the underlying real-time
* kernel for the QF, all platfrom dependencies are handled in the QK, so
* no porting of QF is necessary. In other words, you only need to recompile
* the QF platform-independent code with the compiler for your platform, but
* you don't need to provide any platform-specific implementation (so, no
* qf_port.c file is necessary). Moreover, QK implements the function QF_run()
* in a platform-independent way, in the modile qk.c.
*/
void QF_run(void);

#ifndef QK_PREEMPTIVE
    /** \brief QF idle callback (customized in BSPs for QF)
    *
    * QF_onIdle() is called by the non-preemptive scheduler built into QF-nano
    * when the QF-nano detects that no events are available for active objects
    * (the idle condition). This callback gives the application an opportunity
    * to enter a power-saving CPU mode, or perform some other idle processing.
    *
    * \note QF_onIdle() is invoked with interrupts LOCKED because the idle
    * condition can be asynchronously changed at any time by an interrupt.
    * QF_onIdle() MUST unlock the interrupts internally, but not before
    * putting the CPU into the low-power mode. (Ideally, unlocking interrupts
    * and low-power mode should happen atomically). At the very least, the
    * function MUST unlock interrupts, otherwise interrups will be locked
    * permanently.
    *
    * \note QF_onIdle() is not used in the PREEMPTIVE configuration. When
    * QK_PREEMPTIVE macro is defined, the preemptive kernel QK-nano is used
    * instead of the non-preemptive QF-nano scheduler. QK-nano uses a
    * different idle callback \sa QK_onIdle().
    */
void QF_onIdle(void);
#endif

/** \brief Exits the QF application and returns control to the OS/Kernel.
*
* This function exits the framework. After calling this function, QF is no
* longer in control of the application. The typical use of this method is
* for exiting the QF application to return back to the operating system
* or for handling fatal errors that require resetting the system.
*
* This function is strongly platform-dependent and is not implemented in
* QF-nano, but either in the QF port or in the Board Support Package (BSP)
* for the given application. Some QF ports might not require implementing
* QF_exit() at all, because many embedded application don't have anything
* to exit to.
*/
void QF_exit(void);

                                           /** active object control blocks */
extern QActiveCB const Q_ROM Q_ROM_VAR QF_active[];

                                           /** the number of control blocks */
extern uint8_t   const Q_ROM Q_ROM_VAR QF_activeNum;

/** \brief Ready set of QF-nano.
*
* The QF-nano ready set keeps track of active objects that are ready to run.
* The ready set represents each active object as a bit, with the bits
* assigned according to priorities of the active objects. The bit is set
* if the corresponding active object is ready to run (i.e., has one or
* more events in its event queue) and zero if the event queue is empty.
* The QF-nano ready set is one byte-wide, which corresponds to 8 active
* objects maximum.
*/
extern uint8_t volatile QF_readySet_;

#endif                                                             /* qfn_h */



/*****************************************************************************
* Product; QP-nano
* Last Updated for Version; 3.4.00
* Date of the Last Update;  Aug 20, 2007
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information;
* Quantum Leaps Web site;  http;//www.quantum-leaps.com
* e-mail;                  info@quantum-leaps.com
*****************************************************************************/
#ifndef qassert_h
#define qassert_h

/** \ingroup qepn qfn
* \file qassert.h
* \brief Customizable assertions.
*
* Defines customizable and memory-efficient assertions applicable to
* embedded systems. This header file can be used in C, C++, and mixed C/C++
* programs.
*
* \note The preprocessor switch Q_NASSERT disables checking assertions.
* In particular macros \ref Q_ASSERT, \ref Q_REQUIRE, \ref Q_ENSURE,
* \ref Q_INVARIANT, and \ref Q_ERROR do NOT evaluate the test condition
* passed as the argument to these macros. One notable exception is the
* macro \ref Q_ALLEGE, that still evaluates the test condition, but does
* not report assertion failures when the switch Q_NASSERT is defined.
*/
#ifdef Q_NASSERT          /* Q_NASSERT defined--assertion checking disabled */

    #define Q_DEFINE_THIS_FILE
    #define Q_DEFINE_THIS_MODULE(name_)
    #define Q_ASSERT(ignore_)  ((void)0)
    #define Q_ALLEGE(test_)    ((void)(test_))
    #define Q_ERROR()          ((void)0)

#else                  /* Q_NASSERT not defined--assertion checking enabled */

    #ifdef __cplusplus
        extern "C" {
    #endif

    /** callback invoked in case the condition passed to \ref Q_ASSERT,
    * \ref Q_REQUIRE, \ref Q_ENSURE, \ref Q_ERROR, or \ref Q_ALLEGE
    * evaluates to FALSE.
    *
    * \param file file name where the assertion failed
    * \param line line number at which the assertion failed
    */
    /*lint -sem(Q_assert_handler, r_no)    Q_assert_handler() never returns */
    void Q_assert_handler(char const Q_ROM * const Q_ROM_VAR file, int line);

    #ifdef __cplusplus
        }
    #endif

    /** Place this macro at the top of each C/C++ module to define the file
    * name string using __FILE__ (NOTE; __FILE__ might contain lengthy path
    * name). This file name will be used in reporting assertions in this file.
    */
    #define Q_DEFINE_THIS_FILE \
        static char const Q_ROM Q_ROM_VAR l_this_file[] = __FILE__;

    /** Place this macro at the top of each C/C++ module to define the module
    * name as the argument \a name_. This file name will be used in reporting
    * assertions in this file.
    */
    #define Q_DEFINE_THIS_MODULE(name_) \
        static char const Q_ROM Q_ROM_VAR l_this_file[] = #name_;

    /** General purpose assertion that makes sure the \a test_ argument is
    * TRUE. Calls the Q_assert_handler() callback if the \a test_ evaluates
    * to FALSE.
    * \note the \a test_ is NOT evaluated if assertions are
    * disabled with the Q_NASSERT switch.
    */
    #define Q_ASSERT(test_) \
        if (test_) { \
        } \
        else (Q_assert_handler(l_this_file, __LINE__))

    /** General purpose assertion that ALWAYS evaluates the \a test_
    * argument and calls the Q_assert_handler() callback if the \a test_
    * evaluates to FALSE.
    * \note the \a test_ argument IS always evaluated even when assertions are
    * disabled with the Q_NASSERT macro. When the Q_NASSERT macro is
    * defined, the Q_assert_handler() callback is NOT called, even if the
    * \a test_ evaluates to FALSE.
    */
    #define Q_ALLEGE(test_)    Q_ASSERT(test_)

    /** Assertion that always calls the Q_assert_handler() callback if
    * ever executed.
    * \note can be disabled with the Q_NASSERT switch.
    */
    #define Q_ERROR() \
        (Q_assert_handler(l_this_file, __LINE__))

#endif                                                           /* NASSERT */

/** Assertion that checks for a precondition. This macro is equivalent to
* \ref Q_ASSERT, except the name provides a better documentation of the
* intention of this assertion.
*/
#define Q_REQUIRE(test_)   Q_ASSERT(test_)

/** Assertion that checks for a postcondition. This macro is equivalent to
* \ref Q_ASSERT, except the name provides a better documentation of the
* intention of this assertion.
*/
#define Q_ENSURE(test_)    Q_ASSERT(test_)

/** Assertion that checks for an invariant. This macro is equivalent to
* \ref Q_ASSERT, except the name provides a better documentation of the
* intention of this assertion.
*/
#define Q_INVARIANT(test_) Q_ASSERT(test_)

/** Compile-time assertion exploits the fact that in C/C++ a dimension of
 * an array must be non-zero. The following declaration causes a compilation
 * error if the compile-time expression (\a test_) is not TRUE. The assertion
 * has no runtime side effects.
 */
#define Q_ASSERT_COMPILE(test_) \
    extern char Q_assert_compile[(test_)]

#endif                                                         /* qassert_h */



/*****************************************************************************
* Product; QEP-nano implemenation
* Last Updated for Version; 3.4.01
* Date of the Last Update;  Sep 18, 2007
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2007 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information;
* Quantum Leaps Web site;  http;//www.quantum-leaps.com
* e-mail;                  info@quantum-leaps.com
*****************************************************************************/

Q_DEFINE_THIS_MODULE(qepn)

/** \ingroup qepn qfn
* \file qepn.c
* QEP-nano implementation.
*/

/** empty signal for internal use only */
#define QEP_EMPTY_SIG        0

/** maximum depth of state nesting (including the top level), must be >= 2 */
#define QEP_MAX_NEST_DEPTH   5

#ifndef Q_NHSM
/*..........................................................................*/
void QHsm_dispatch(NEAR QHsm *me) Q_REENTRANT {
    QHsmState path[QEP_MAX_NEST_DEPTH];
    QHsmState s;
    QHsmState t = me->state;

    path[1] = t;    /* save the current state in case a transition is taken */

    do {                             /* process the event hierarchically... */
        s = t;
        t = (QHsmState)((*s)(me));                /* invoke state handler s */
    } while (t != (QHsmState)0);

    if (me->evt.sig == (QSignal)0) {                   /* transition taken? */
        QHsmState src = s;                  /* the source of the transition */
        int8_t ip = (int8_t)(-1);            /* transition entry path index */
        int8_t iq;                    /* helper transition entry path index */

        path[0] = me->state;                          /* save the new state */
        me->state = path[1];                   /* restore the current state */

                      /* exit current state to the transition source src... */
        for (s = path[1]; s != src; ) {
            Q_SIG(me) = (QSignal)Q_EXIT_SIG;
            t = (QHsmState)(*s)(me);                /* find superstate of s */
            if (t != (QHsmState)0) {               /* exit action unhandled */
                s = t;                            /* t points to superstate */
            }
            else {                                   /* exit action handled */
                Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
                s = (QHsmState)(*s)(me);            /* find superstate of s */
            }
        }

        t = path[0];                            /* target of the transition */

        if (src == t) {    /* (a) check source==target (transition to self) */
            Q_SIG(me) = (QSignal)Q_EXIT_SIG;
            (void)(*src)(me);                            /* exit the source */
            ip = (int8_t)0;                             /* enter the target */
        }
        else {
            Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
            t = (QHsmState)(*t)(me);           /* find superstate of target */
            if (src == t) {              /* (b) check source==target->super */
                ip = (int8_t)0;                         /* enter the target */
            }
            else {
                Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
                s = (QHsmState)(*src)(me);        /* find superstate of src */
                if (s == t) {     /* (c) check source->super==target->super */
                    Q_SIG(me) = (QSignal)Q_EXIT_SIG;
                    (void)(*src)(me);                    /* exit the source */
                    ip = (int8_t)0;                     /* enter the target */
                }
                else {
                    if (s == path[0]) {  /* (d) check source->super==target */
                        Q_SIG(me) = (QSignal)Q_EXIT_SIG;
                        (void)(*src)(me);                /* exit the source */
                    }
                    else { /* (e) check rest of source==target->super->super..
                            * and store the entry path along the way
                            */
                        iq = (int8_t)0;      /* indicate that LCA not found */
                        ip = (int8_t)1;  /* enter target and its superstate */
                        path[1] = t;       /* save the superstate of target */
                        Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
                        t = (QHsmState)(*t)(me);    /* find superstate of t */
                        while (t != (QHsmState)0) {
                            path[++ip] = t;         /* store the entry path */
                            if (t == src) {            /* is it the source? */
                                iq = (int8_t)1;  /* indicate that LCA found */
                                            /* entry path must not overflow */
                                Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH);
                                --ip;            /* do not enter the source */
                                t = (QHsmState)0;     /* terminate the loop */
                            }
                            else {   /* it is not the source, keep going up */
                                Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
                                t = (QHsmState)(*t)(me); /* superstate of t */
                            }
                        }
                        if (iq == (int8_t)0) {    /* the LCA not found yet? */

                                            /* entry path must not overflow */
                            Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH);

                            Q_SIG(me) = (QSignal)Q_EXIT_SIG;
                            (void)(*src)(me);            /* exit the source */

                                /* (f) check the rest of source->super
                                 *                  == target->super->super...
                                 */
                            iq = ip;
                            do {
                                if (s == path[iq]) {    /* is this the LCA? */
                                    t = s;    /* indicate that LCA is found */
                                    ip = (int8_t)(iq - 1);/*do not enter LCA*/
                                    iq = (int8_t)(-1);/* terminate the loop */
                                }
                                else {
                                    --iq; /* try lower superstate of target */
                                }
                            } while (iq >= (int8_t)0);

                            if (t == (QHsmState)0) {  /* LCA not found yet? */
                                    /* (g) check each source->super->...
                                     * for each target->super...
                                     */
                                do {
                                    Q_SIG(me) = (QSignal)Q_EXIT_SIG;
                                    t = (QHsmState)(*s)(me);      /* exit s */
                                    if (t != (QHsmState)0) {  /* unhandled? */
                                        s = t;    /* t points to super of s */
                                    }
                                    else {           /* exit action handled */
                                        Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
                                        s = (QHsmState)(*s)(me);/*super of s*/
                                    }
                                    iq = ip;
                                    do {
                                        if (s == path[iq]) {/* is this LCA? */
                                                        /* do not enter LCA */
                                            ip = (int8_t)(iq - 1);
                                            iq = (int8_t)(-1);/*break inner */
                                            s = (QHsmState)0; /*break outer */
                                        }
                                        else {
                                            --iq;
                                        }
                                    } while (iq >= (int8_t)0);
                                } while (s != (QHsmState)0);
                            }
                        }
                    }
                }
            }
        }
                    /* retrace the entry path in reverse (desired) order... */
        for (; ip >= (int8_t)0; --ip) {
            Q_SIG(me) = (QSignal)Q_ENTRY_SIG;
            (void)(*path[ip])(me);                        /* enter path[ip] */
        }
        s = path[0];                      /* stick the target into register */
        me->state = s;                          /* update the current state */

                                      /* drill into the target hierarchy... */
        Q_SIG(me) = (QSignal)Q_INIT_SIG;
        while ((*s)(me) == (QState)0) {
            t = me->state;
            path[0] = t;
            ip = (int8_t)0;
            Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
            t = (QHsmState)(*t)(me);                /* find superstate of t */
            while (t != s) {
                ++ip;
                path[ip] = t;
                Q_SIG(me) = (QSignal)QEP_EMPTY_SIG;
                t = (QHsmState)(*t)(me);            /* find superstate of t */
            }
                                            /* entry path must not overflow */
            Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH);

            do {    /* retrace the entry path in reverse (correct) order... */
                Q_SIG(me) = (QSignal)Q_ENTRY_SIG;
                (void)(*path[ip])(me);                    /* enter path[ip] */
                --ip;
            } while (ip >= (int8_t)0);
            s = me->state;
            Q_SIG(me) = (QSignal)Q_INIT_SIG;
        }
    }
}

void Q_assert_handler(char const Q_ROM * const Q_ROM_VAR file, int line)
{
    file;
    line;
}

#endif                                                            /* Q_NHSM */

#endif //(defined PORT_HOST) || defined (SDCC_STACK_AUTO)

void
testDummy(void)
{
}
