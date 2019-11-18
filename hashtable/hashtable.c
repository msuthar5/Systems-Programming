#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include <stdio.h>

/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size);
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  /* FIXME: the current implementation doesn't update existing entries */
  unsigned int idx = hash(key) % ht -> size;
  int found = 0;

  bucket_t *b = ht -> buckets[idx];

  while (b){
    if (strcmp(b -> key, key) == 0) {

      free(b -> key);
      free(b -> val);
      b -> key = key;
      b -> val = val;
      found = 1;

    }
    b = b ->next;

  }

  if (!found) {

    bucket_t *new_bucket = malloc(sizeof(bucket_t));
    new_bucket -> key = key;
    new_bucket -> val = val;
    new_bucket -> next = ht -> buckets[idx];
    ht -> buckets[idx] = new_bucket;

  }
}

void *ht_get(hashtable_t *ht, char *key) {

  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  while (b) {
    if (strcmp(b->key, key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) {
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      if (!f(b->key, b->val)) {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht) {

  int i = 0;
  while (i < ht -> size){

    bucket_t *b = ht -> buckets[i];

    while(b){

      free(b -> key);
      free(b -> val);
      bucket_t *cur = b;
      b = b -> next;
      free(cur);

    }

    i++;

  }

  free(ht ->buckets);
  free(ht); // FIXME: must free all substructures!
}

/* TODO */
void  ht_del(hashtable_t *ht, char *key) {

  unsigned int idx = hash(key) % ht->size;

  bucket_t *head = ht -> buckets[idx];
  int done = 0;

  if (strcmp(head->key,key) == 0){

    bucket_t *new_head = head -> next;
    free(head->key);
    free(head->val);
    free(head);
    ht -> buckets[idx] = new_head;
    done = 1;

  }

  while (head && !done){

    bucket_t *next = head -> next;
    if (strcmp(next -> key,key) == 0){

      head -> next = next -> next;
      free(next->key);
      free(next->val);
      free(next);
      done = 1;

    }
    head = head -> next;
}

}

void  ht_rehash(hashtable_t *ht, unsigned long newsize) {

  bucket_t **old_buckets = ht -> buckets;

  ht -> buckets = calloc(sizeof(bucket_t *), newsize);

  unsigned long oldsize = ht -> size;
  ht -> size = newsize;

  unsigned long i = 0;

  while (i < oldsize) {

    bucket_t *b = old_buckets[i];

    while (b){

      ht_put(ht, b->key, b->val);
      bucket_t *current = b;
      b = b -> next;
      free(current);

    }

    i++;
  }
  free(old_buckets);

  }
