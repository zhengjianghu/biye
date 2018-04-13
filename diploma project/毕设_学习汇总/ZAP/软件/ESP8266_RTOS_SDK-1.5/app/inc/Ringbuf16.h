/*
 *
 * \brief       common library -- Ring buffer
 * \Copyright (c) 2017 by inSona Corporation. All rights reserved.
 * \author      zhouyong
 * \file        ringbuf16.h
 *
 * General crc16-ccitt function.
 *
 */

/** \addtogroup lib
 * @{ */

/**
 * \defgroup ringbuf Ring buffer library
 * @{
 *
 * The ring buffer library implements ring (circular) buffer where
 * bytes can be read and written independently. A ring buffer is
 * particularly useful in device drivers where data can come in
 * through interrupts.
 *
 */

#ifndef RINGBUF16_H_
#define RINGBUF16_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief      Structure that holds the state of a ring buffer.
 *
 *             This structure holds the state of a ring buffer. The
 *             actual buffer needs to be defined separately. This
 *             struct is an opaque structure with no user-visible
 *             elements.
 *
 */
struct ringbuf
{
    unsigned char *data;
    unsigned short mask;

    /* XXX these must be 8-bit quantities to avoid race conditions. */
    unsigned short put_ptr, get_ptr;
};

typedef struct ringbuf Ringbuf16_ST;

/**
 * \brief      Initialize a ring buffer
 * \param r    A pointer to a Ringbuf16_ST to hold the state of the ring buffer
 * \param a    A pointer to an array to hold the data in the buffer
 * \param size_power_of_two The size of the ring buffer, which must be a power of two
 *
 *             This function initiates a ring buffer. The data in the
 *             buffer is stored in an external array, to which a
 *             pointer must be supplied. The size of the ring buffer
 *             must be a power of two and cannot be larger than 128
 *             bytes.
 *
 */
void    Ringbuf16_Init(Ringbuf16_ST *r, unsigned char *a,
                     unsigned short size_power_of_two);

/**
 * \brief      Insert a byte into the ring buffer
 * \param r    A pointer to a Ringbuf16_ST to hold the state of the ring buffer
 * \param c    The byte to be written to the buffer
 * \return     Non-zero if there data could be written, or zero if the buffer was full.
 *
 *             This function inserts a byte into the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int     Ringbuf16_Put(Ringbuf16_ST *r, unsigned char c);


/**
 * \brief      Get a byte from the ring buffer
 * \param r    A pointer to a Ringbuf16_ST to hold the state of the ring buffer
 * \return     The data from the buffer, or -1 if the buffer was empty
 *
 *             This function removes a byte from the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int     Ringbuf16_Get(Ringbuf16_ST *r);

/**
 * \brief      Insert a byte into the ring buffer
 * \param r    A pointer to a Ringbuf16_ST to hold the state of the ring buffer
 * \param c    The byte to be written to the buffer
 * \return     Non-zero if there data could be written, or zero if the buffer was full.
 *
 *             This function inserts a byte into the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int
Ringbuf16_PutArray(struct ringbuf *r, const unsigned char array[], int size);

/**
 * \brief      Get a byte from the ring buffer
 * \param r    A pointer to a Ringbuf16_ST to hold the state of the ring buffer
 * \return     The data from the buffer, or -1 if the buffer was empty
 *
 *             This function removes a byte from the ring buffer. It
 *             is safe to call this function from an interrupt
 *             handler.
 *
 */
int
Ringbuf16_GetArray(struct ringbuf *r, unsigned char array[], int length);

/**
 * \brief      Get the size of a ring buffer
 * \param r    A pointer to a Ringbuf16_ST to hold the state of the ring buffer
 * \return     The size of the buffer.
 */
int     Ringbuf16_Size(Ringbuf16_ST *r);

/**
 * \brief      Get the number of elements currently in the ring buffer
 * \param r    A pointer to a Ringbuf16_ST to hold the state of the ring buffer
 * \return     The number of elements in the buffer.
 */
int     Ringbuf16_Elements(Ringbuf16_ST *r);


unsigned char * Ringbuf16_GetPutPtr(struct ringbuf *r);

int Ringbuf16_GetPutLength(struct ringbuf *r, int isContinuous);

void Ringbuf16_PutPtrInc(struct ringbuf *r, int size);

unsigned char * Ringbuf16_GetGetPtr(struct ringbuf *r);

int Ringbuf16_GetGetLength(struct ringbuf *r, int isContinuous);

void Ringbuf16_GetPtrInc(struct ringbuf *r, int size);

#ifdef __cplusplus
}
#endif

#endif /* RINGBUF16_H_ */

/** @}*/
/** @}*/
