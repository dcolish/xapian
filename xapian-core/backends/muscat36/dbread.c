/* dbread.c: Code to read an old muscat DB file
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <stdio.h>   /* stderr etc */
#include <stdlib.h>  /* malloc, exit etc */
#include <string.h>  /* memmove etc */
#include "3point6.h"
#include "dbread.h"
#include "dbdefs.h"

#define true   1
#define false  0
#define PWIDTH (ILEN + 1)

static void readda(struct DB_file * DB, int n, byte * b)
{
    if (X_point(DB->locator, DB->block_size, n) >= 0)
       if (X_read(DB->locator, b, DB->block_size) == DB->block_size) return;
    fprintf(stderr, "Can't read block %d of DB file\n", n); exit(1);
}

static struct DB_pool * find_block(struct DB_file * DB, int n)
{
    struct DB_pool * P = DB->pool;
    int pool_size = DB->pool_size;
    long int oldest = -1; /* for the oldest read block */
    int oldest_j = 0;     /* - and its index in P */
    int j;
    for (j = 0; j < pool_size; j++) {
    if (P[j].n == n) {
        return P + j;
    }
    if (P[j].p == NULL) {
        byte * b = malloc(DB->block_size);
        readda(DB, n, b);
        P[j].p = b + HEADER;
        P[j].n = n;
        return P + j;
    }
    {
        long int age = DB->clock - P[j].clock;
        if (age > oldest) {
        oldest = age;
        oldest_j = j;
        }
    }
    }
    readda(DB, n, P[oldest_j].p - HEADER);
    P[oldest_j].n = n;
    return P + oldest_j;
}

/* C_block(DB, j) pulls into the cursor structure the block appropriate to
   level j.

   Returns the data address for the block.
*/

static byte * C_block(struct DB_file * DB, struct DB_cursor * C, int j)
{
    struct DB_pool * P = C[j].pool;
    int n = C[j].n;
    if (P == NULL || n != P->n)  /* W(P->p, BLOCKNUMBER) */
    {   P = find_block(DB, n);
        C[j].pool = P;
        C[j].version = W(P->p, BLOCK_VERSION);
        if (j == 0)
        {   if (W(P->p, LEVEL) != DB->levels) goto DB_exit;   }
        else
        {   if (C[j-1].version < C[j].version) goto DB_exit;
            if (W(P->p, LEVEL) != DB->levels - j)
            {   fprintf(stderr, "DB will fail on DBcheck\n");
                exit(1);
            }
        }
    }
    P->clock = DB->clock++;
    return P->p;

DB_exit:

   fprintf(stderr, "Panic point 1\n"); exit(0);
}

static int findin(const byte * p, int keylength, const byte * q, int i)
{   int j = W(p, DLEN);
    while (j-i > 2)
    {   int k = i + (j - i)/4*2;
        int o = L2(p, k);
        int t = M_compare_bytes(keylength, q, 1, p[o + 2] - 1, p, o + 3);
        if (t < 0) j = k; else i = k;
    }
    return i;
}

static void set_DB_positions(struct DB_file * DB, const byte * p, int o)
{   int item_size = L2(p, o);
    DB->p = p;
    DB->c = o; o += 2;
    DB->key = p + o;
    DB->tag = p + o + p[o];
    DB->tag_size = item_size - p[o] - 2;
}

static int DB_move_forward_at_level(struct DB_file * DB, struct DB_cursor * C, int j)
{   byte * p = C_block(DB, C, j);
    int c = C[j].c;
    c += 2;
    if (c == W(p, DLEN))
    {   if (j == 0) return false;
        if (! DB_move_forward_at_level(DB, C, j - 1)) return false;
        p = C_block(DB, C, j);
        c = 0;
    }
    C[j].c = c;
    {   int o = L2(p,c);
        if (j < DB->levels)
        {   o += L2(p,o) - ILEN;
            C[j+1].n = I(p,o) + DB->block_offset;
        }
        else set_DB_positions(DB, p, o);
    }
    return true;
}

static int DB_move_back_at_level(struct DB_file * DB, struct DB_cursor * C, int j)
{   byte * p = C_block(DB, C, j);
    int c = C[j].c;
    if (c == 0)
    {   if (j == 0) return false;
        if (! DB_move_back_at_level(DB, C, j - 1)) return false;
        p = C_block(DB, C, j);
        c = W(p, DLEN);
    }
    c -= 2;
    C[j].c = c;
    {   int o = L2(p,c);
        if (j < DB->levels)
        {   o += L2(p,o) - ILEN;
            C[j+1].n = I(p,o) + DB->block_offset;
        }
        else set_DB_positions(DB, p, o);
    }
    return true;
}

static int DB_move_forward(struct DB_file * DB, struct DB_cursor * C)
{   return DB_move_forward_at_level(DB, C, DB->levels);   }

static int DB_move_back(struct DB_file * DB, struct DB_cursor * C)
{   return DB_move_back_at_level(DB, C, DB->levels);   }

/* DB_find(DB, C, q) searches for the key q in the B-tree of DB with
   C as cursor. Result is true if found, false otherwise.
   The cursor accesses the last item <= the key q.
*/

static int DB_find(struct DB_file * DB, struct DB_cursor * C, const byte * q)
{
    byte * p; int c;
    int keylength = q[0] - 1;
    int j;
    int levels = DB->levels;
    for (j = 0; j < levels; j++)
    {   byte * p = C_block(DB, C, j);
        c = findin(p, keylength, q, 0); /* c is an offset in the block's directory */
        C[j].c = c;
        c = L2(p, c);            /* c is an offset to an item */
        c = c+L2(p, c) - ILEN;   /* c is an offset to a block number */
        C[j+1].n = I(p, c) + DB->block_offset;
    }
    p = C_block(DB, C, levels);
    c = findin(p, keylength, q, -2);
    C[levels].c = c;

    if (c < 0)
    {   C[levels].c = 0;
        DB_move_back(DB, C);
        c = C[levels].c;
    }

    set_DB_positions(DB, p, L2(p, c));
    return M_compare_bytes(keylength, q, 1, keylength, DB->key, 1) == 0;
}

#define BUFFER_INC 80  /* used in two places below */

static void copy_tag(struct DB_file * DB, struct DB_postings * q, int extra_bit)
{   int size = DB->tag_size + extra_bit;
    if (size > q->buffer_size)
    {   free(q->buffer);
        q->buffer_size = size + BUFFER_INC;
        q->buffer = (byte *) malloc(q->buffer_size);
    }
    memmove(q->buffer, DB->tag - extra_bit, size);
    q->lim = size;
}

static struct DB_cursor * DB_make_cursor(struct DB_file * DB)
{
    int n = 20; /* The number of levels in the B-tree can never approach 20 */

    struct DB_cursor * C = (struct DB_cursor *) calloc(1, n * sizeof(struct DB_cursor));
    int i;
    for (i = 0; i < n; i++) C[i].pool = NULL;
    C[0].n = DB->root;
    return C;
}

extern struct DB_postings *
DB_open_postings(struct DB_term_info * t, struct DB_file * DB)
{
    struct DB_postings * q =
        (struct DB_postings *) calloc(1, sizeof(struct DB_postings));
    {
    q->DB = DB;
    q->cursor = DB_make_cursor(DB);
    q->buffer_size = 0;
    q->buffer = NULL;
    if (DB_find(DB, q->cursor, t->key))
    {   q->key = (byte *) malloc(t->key[0] + ILEN);
        memmove(q->key, t->key, t->key[0]);
        q->key[0] += ILEN;
        copy_tag(DB, q, 0);
        q->i = PWIDTH;
        q->freq = -I(q->buffer, 0);
        if (q->i == q->lim)            /* the famous bug [41] fix */
        {   DB_move_forward(DB, q->cursor);
            copy_tag(DB, q, ILEN);
            q->i = 0;
        }
    }
    else
    {   q->buffer = (byte *) malloc(PWIDTH);
        q->i = 0;
        q->freq = 0;
        M_put_I(q->buffer, 0, MAXINT);
    }
    q->Doc = 0;
    q->E = 0;
    }
    return q;
}

static void next_posting_set(struct DB_postings * q, int Z, int skippable)
{

#define LARGEGAP 1000
#define LARGEFREQ 3000

    if (I(q->buffer, q->lim - PWIDTH) < Z - LARGEGAP &&
        q->freq > LARGEFREQ &&
        skippable)
    {
        M_put_I(q->key, q->key[0] - ILEN, Z);
        DB_find(q->DB, q->cursor, q->key);

        /* bug [82] of Muscat3.6 fixed as follows: */

printf(">>%d %d\n", q->key[0] - ILEN , q->DB->key[0]);
        if (q->key[0] - ILEN == q->DB->key[0])
            {  DB_move_forward(q->DB, q->cursor);
printf("++%d %d\n", q->key[0] - ILEN , q->DB->key[0]);
            }
    }
    else DB_move_forward(q->DB, q->cursor);

    copy_tag(q->DB, q, ILEN);
    q->i = 0;
}

static void next_posting(struct DB_postings * q, int Z)
{   int skippable = true;

    /* The skippable feature is necessary. Without it, repeating DB_find
       in 'next_posting_set' can sometimes cause an infinite loop.
    */

    while(true)
    {   if (q->i == q->lim)
        {   next_posting_set(q, Z, skippable);
            skippable = false;
        }
        q->Doc = I(q->buffer, q->i);
        q->wdf = q->buffer[q->i + ILEN];
        if (q->Doc != MAXINT) q->i += PWIDTH;
        if (q->Doc < 0)
        {  q->Doc = - q->Doc;
           q->E = I(q->buffer, q->i);
           q->i += PWIDTH;
        }
        else q->E = q->Doc;
        if (q->E >= Z) break;
    }
}

extern void DB_read_postings(struct DB_postings * q, int style, int Z)
{
    if (q->Doc == MAXINT) return;

    if (style > 0) {
    next_posting(q, Z);
        if (q->Doc < Z) q->Doc = Z;
    } else {
    /* interpret ranges */
        q->Doc++;
        if (q->Doc > q->E || q->E < Z) next_posting(q, Z);
    }
    if (q->Doc < Z) q->Doc = Z;
    return;
}

extern void DB_close_postings(struct DB_postings * q)
{
    free(q->cursor);
    free(q->buffer);
    free(q->key);
    free(q);
}

int DB_term(const byte * k, struct DB_term_info * t, struct DB_file * DB)
{
    /* <set longjump label T1 for DB change detected>
       LABEL T0:
    */
                                         /* key for a term: */
    memmove(t->key + 1, k, k[0]);        /* characters of the term */
    t->key[1] = 'A';                     /* starting A */
    t->key[k[0] + 1] = 0;                /* zero terminator */
    t->key[0] = k[0] + 2;

    if (DB_find(DB, DB->cursor, t->key))
    {   t->freq = -I(DB->tag, 0);
        return true;
    }
    else
    {   t->freq = 0;
        return false;
    }

    /* LABEL T1:
       <reopen DB file and jump to T0>
    */
}

static int valid_base(const byte * p)
{   int v = W(p, DB_VERSION);
    if (v != W(p, DB_VERSION2)) return false;

    {   int f = W(p, DB_IMAGES);
        if (f == 0) return true;
        return v == W(p, DB_FVEC + 2 * f);
    }
}

static struct DB_pool * DB_make_pool(int n)
{
    struct DB_pool * P = (struct DB_pool *) calloc(1, n * sizeof(struct DB_pool));
    int i;
    for (i = 0; i < n; i++) { P[i].p = NULL; P[i].n = -1; P[i].clock = 0; }
    return P;
}

extern struct DB_file * DB_open(const char * s, int pool_size)
{
    struct DB_file * DB;
    filehandle q;
    int block_size;

    q = X_findtoread(s); if (q == -1) return NULL;
    block_size = M_get_block_size(q, s);

    DB = (struct DB_file *) calloc(1, sizeof(struct DB_file));
    DB->locator = q;
    DB->block_size = block_size;

    {   byte * db0 = malloc(block_size);
        byte * db1 = malloc(block_size);
        readda(DB, 0, db0);
        if (W(db0, DB_BASE) != 0 || W(db0, DB_BASE2) != 1)
        {   fprintf(stderr, "Not a proper DB file\n"); exit(1);   }
        readda(DB, 1, db1);
        if (valid_base(db0) && W(db0, DB_VERSION) == W(db1, DB_VERSION) + 1)
        {   /* swap */
            byte * x0 = db0;
            byte * x1 = db1;
            db1 = x0; db0 = x1;
        }
        /* So db1 is now the most recent base block */
        if (! valid_base(db1))
        {   fprintf(stderr, "Invalid DB - consult expert\n"); exit(1);   }


        DB->levels = W(db1, DB_LEVELS);
        DB->block_count = W(db1, DB_DISCBLOCKS);
        DB->root = W(db1, DB_ROOT); /* no provision for frozen images */
        DB->block_offset = W(db1, DB_BLOCK_OFFSET);

        if ((W(db1, DB_FLAGS) & 0x2) == 0)
        {   fprintf(stderr, "access to non-wdf DBs not supported\n");
            exit(1);
        }

        DB->heavy_duty = W(db1, DB_KEYPART);

        free(db0); free(db1);
    }
    DB->cursor = DB_make_cursor(DB);
    if (pool_size < 8) pool_size = 8;
    DB->pool_size = pool_size;
    DB->pool = DB_make_pool(pool_size);
    DB->clock = 0;

    if (DB_find(DB, DB->cursor, (byte *) "\2" "C"))
    {
        DB->doc_count = I(DB->tag, 0);
        DB->term_count = I(DB->tag, 2 * ILEN);
    }
    else
    {
        DB->doc_count = 0;
        DB->term_count = 0;
    }

    return DB;
}

extern void DB_close(struct DB_file * DB)
{
    X_close(DB->locator);
    {   int i;
        for (i = 0; i < DB->pool_size; i++)
        {   byte * b = DB->pool[i].p;
            if (b != NULL) free(b - HEADER);
        }
    }
    free(DB->cursor);
    free(DB->pool);
    free(DB);
}

static int DB_read_unit(struct DB_file * DB, int n, int r_ot_tv, struct record * r)
{

    /* <set longjump label R1 for DB change detected>
       LABEL R0:
    */
    int x = DB->heavy_duty;

    byte key[20];                   /* key for a doc/termvec */

    key[1] = 'D';                   /* starting 'D' */
    M_put_I(key, 2, n);             /* number */
    key[ILEN + 2] = r_ot_tv;        /* r_or_tv == 0 for record, 1 for termvec */

    r->heavy_duty = x;
    if (!x)
    {   key[ILEN + 3] = 1;          /* the first part of the unit */
        key[0] = 8;                 /* 8 byte key length */
    }
    else
    {   M_put_I(key, ILEN + 3, 1);  /* held in 4 bytes in heavy-duty Muscat */
        key[0] = 11;                /* so 11 bytes for the key length */
    }
    if (! DB_find(DB, DB->cursor, key)) return false;

    {   int len = LENGTH_OF(DB->tag, 0, x);
        int d = 0;
        if (r->size < len)
        {   free(r->p);
            r->size = len + BUFFER_INC;
            r->p = (byte *) malloc(r->size);
        }

        while(true)
        {   int d_next = d + DB->tag_size;
            if (d_next > len)
            {   fprintf(stderr, "Doc lengths don't add up\n");
                exit(1);
            }
            memmove(r->p + d, DB->tag, DB->tag_size);
            d = d_next;
            if (d == len) break;
            DB_move_forward(DB, DB->cursor);
        }
    }
    return true;

    /* LABEL R1:
       <reopen DB file and jump to R0>
    */
}

extern int DB_get_record(struct DB_file * DB, int n, struct record * r)
{
    return DB_read_unit(DB, n, 0, r);
}

extern int DB_get_termvec(struct DB_file * DB, int n, struct termvec * tv)
{
    return DB_read_unit(DB, n, 1, (struct record *) tv);
}

