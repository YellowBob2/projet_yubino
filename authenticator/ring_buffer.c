#include "ring_buffer.h"

void ring_buffer__init(struct ring_buffer* rb, uint8_t* buffer, uint8_t buffer_size){
    rb->buffer = buffer;
    rb->maxlen = buffer_size;
    rb->head = 0;
    rb->tail = 0;
}

void ring_buffer__push(struct ring_buffer* rb, uint8_t data){
    uint8_t next_head = rb->head + 1;
    if (next_head >= rb->maxlen)
        next_head = 0;
    // si buffer plein, on ecrase
    if (next_head == rb->tail){
        uint8_t next_tail = rb->tail + 1;
        if (next_tail >= rb->maxlen)
            next_tail = 0;
        rb->tail = next_tail;
    }
    rb->buffer[rb->head] = data;
    rb->head = next_head;
}

uint8_t ring_buffer__pop(struct ring_buffer* rb, uint8_t* data){
    if (rb->head == rb->tail) return 1; // buffer vide
    *data = rb->buffer[rb->tail];
    rb->tail++;
    if (rb->tail >= rb->maxlen) rb->tail = 0;
    return 0;
}
