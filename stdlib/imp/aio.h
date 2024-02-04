#ifndef __AIO_IMP__
#define __AIO_IMP__
#include "../aio.h"
#include "../usermode.h"

int aio_read (struct aiocb *__aiocbp) __THROW
{
    userm_read(__aiocbp->aio_fildes, );
}
/* Enqueue write request for given number of bytes and the given priority.  */
int aio_write (struct aiocb *__aiocbp) __THROW
{

}