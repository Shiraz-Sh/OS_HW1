#ifndef RW_LOCK_H
#define RW_LOCK_H

/**
 * Readers Writers (RW) lock
 */

// initialize the lock
void readers_writers_init();

// reader lock and unlock
void reader_lock();
void reader_unlock();

// writer lock and unlock
void writer_lock();
void writer_unlock();

// destructor of the lock
void reader_writer_destroy();

#endif