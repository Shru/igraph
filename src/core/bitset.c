/* -*- mode: C -*-  */
/* vim:set ts=4 sw=4 sts=4 et: */
/*
   IGraph library.
   Copyright (C) 2024  The igraph development team <igraph@igraph.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "igraph_bitset.h"
#include "igraph_memory.h"

igraph_error_t igraph_bitset_init(igraph_bitset_t *bitset, igraph_integer_t size) {
    igraph_integer_t alloc_size = IGRAPH_BITNSLOTS(size);
    bitset->stor_begin = IGRAPH_CALLOC(alloc_size, igraph_integer_t);
    IGRAPH_CHECK_OOM(bitset->stor_begin, "Cannot initialize bitset");
    bitset->size = size;
    bitset->stor_end = bitset->stor_begin + alloc_size;
    return IGRAPH_SUCCESS;
}

void igraph_bitset_destroy(igraph_bitset_t *bitset) {
    IGRAPH_ASSERT(bitset != NULL);
    if (bitset->stor_begin != NULL) {
        IGRAPH_FREE(bitset->stor_begin);
        bitset->size = 0;
        bitset->stor_begin = NULL;
    }
}

igraph_integer_t igraph_bitset_popcount(igraph_bitset_t *bitset)
{
    const igraph_integer_t final_block_size = bitset->size % IGRAPH_INTEGER_SIZE ? bitset->size % IGRAPH_INTEGER_SIZE : IGRAPH_INTEGER_SIZE;
    const igraph_integer_t padding = IGRAPH_INTEGER_SIZE - final_block_size;
    const igraph_integer_t slots = IGRAPH_BITNSLOTS(bitset->size);
    const igraph_integer_t mask = final_block_size == IGRAPH_INTEGER_SIZE ? ~0 : ((1 << final_block_size) - 1);
    igraph_integer_t count = 0;
    for (igraph_integer_t i = 0; i + 1 < slots; ++i)
    {
        count += IGRAPH_POPCOUNT(VECTOR(*bitset)[i]);
    }
    if (bitset->size) {
        count += IGRAPH_POPCOUNT(mask & VECTOR(*bitset)[slots - 1]);
    }
    return count;
}

igraph_integer_t igraph_bitset_countl_zero(igraph_bitset_t *bitset)
{
    const igraph_integer_t final_block_size = bitset->size % IGRAPH_INTEGER_SIZE ? bitset->size % IGRAPH_INTEGER_SIZE : IGRAPH_INTEGER_SIZE;
    const igraph_integer_t padding = IGRAPH_INTEGER_SIZE - final_block_size;
    const igraph_integer_t slots = IGRAPH_BITNSLOTS(bitset->size);
    const igraph_integer_t one = 1, zero = 0;
    const igraph_integer_t mask = final_block_size == IGRAPH_INTEGER_SIZE ? ~zero : ((one << final_block_size) - one);
    if (bitset->size && (mask & VECTOR(*bitset)[slots - 1]) != 0) {
        return IGRAPH_CLZ(mask & VECTOR(*bitset)[slots - 1]) - padding;
    }
    for (igraph_integer_t i = 1; i < slots; ++i) {
        if (VECTOR(*bitset)[slots - i - 1] != 0) {
            const igraph_integer_t result = IGRAPH_INTEGER_SIZE * i + IGRAPH_CLZ(VECTOR(*bitset)[slots - i - 1]);
            return result - padding;
        }
    }
    return bitset->size;
}

igraph_integer_t igraph_bitset_countl_one(igraph_bitset_t *bitset)
{
    const igraph_integer_t final_block_size = bitset->size % IGRAPH_INTEGER_SIZE ? bitset->size % IGRAPH_INTEGER_SIZE : IGRAPH_INTEGER_SIZE;
    const igraph_integer_t padding = IGRAPH_INTEGER_SIZE - final_block_size;
    const igraph_integer_t slots = IGRAPH_BITNSLOTS(bitset->size);
    const igraph_integer_t one = 1, zero = 0;
    const igraph_integer_t mask = final_block_size == IGRAPH_INTEGER_SIZE ? zero : ~((one << final_block_size) - one);
    if (bitset->size && (mask | VECTOR(*bitset)[slots - 1]) != ~zero) {
        const igraph_integer_t result = IGRAPH_CLO(mask | VECTOR(*bitset)[slots - 1]);
        return IGRAPH_CLO(mask | VECTOR(*bitset)[slots - 1]) - padding;
    }
    for (igraph_integer_t i = 1; i < slots; ++i) {
        if (VECTOR(*bitset)[slots - i - 1] != ~zero) {
            const igraph_integer_t result = IGRAPH_INTEGER_SIZE * i + IGRAPH_CLO(VECTOR(*bitset)[slots - i - 1]);
            return result - padding;
        }
    }
    return bitset->size;
}

igraph_integer_t igraph_bitset_countr_zero(igraph_bitset_t *bitset)
{
    const igraph_integer_t final_block_size = bitset->size % IGRAPH_INTEGER_SIZE ? bitset->size % IGRAPH_INTEGER_SIZE : IGRAPH_INTEGER_SIZE;
    const igraph_integer_t slots = IGRAPH_BITNSLOTS(bitset->size);
    const igraph_integer_t one = 1, zero = 0;
    const igraph_integer_t mask = final_block_size == IGRAPH_INTEGER_SIZE ? ~zero : ((one << final_block_size) - one);
    for (igraph_integer_t i = 0; i + 1 < slots; ++i) {
        if (VECTOR(*bitset)[i] != zero) {
            const igraph_integer_t result = IGRAPH_INTEGER_SIZE * i + IGRAPH_CTZ(VECTOR(*bitset)[i]);
            return result;
        }
    }
    if (bitset->size && (mask & VECTOR(*bitset)[slots - 1]) != zero) {
        return IGRAPH_INTEGER_SIZE * (slots - 1) + IGRAPH_CTZ(mask & VECTOR(*bitset)[slots - 1]);
    }
    return bitset->size;
}

igraph_integer_t igraph_bitset_countr_one(igraph_bitset_t *bitset)
{
    const igraph_integer_t final_block_size = bitset->size % IGRAPH_INTEGER_SIZE ? bitset->size % IGRAPH_INTEGER_SIZE : IGRAPH_INTEGER_SIZE;
    const igraph_integer_t slots = IGRAPH_BITNSLOTS(bitset->size);
    const igraph_integer_t one = 1, zero = 0;
    const igraph_integer_t mask = final_block_size == IGRAPH_INTEGER_SIZE ? zero : ~((one << final_block_size) - one);
    for (igraph_integer_t i = 0; i + 1 < slots; ++i) {
        if (VECTOR(*bitset)[i] != ~zero) {
            const igraph_integer_t result = IGRAPH_INTEGER_SIZE * i + IGRAPH_CTO(VECTOR(*bitset)[i]);
            return result;
        }
    }
    if (bitset->size && (mask | VECTOR(*bitset)[slots - 1]) != ~zero) {
        return IGRAPH_INTEGER_SIZE * (slots - 1) + IGRAPH_CTO(mask | VECTOR(*bitset)[slots - 1]);
    }
    return bitset->size;
}

void igraph_bitset_or(igraph_bitset_t *dest, igraph_bitset_t *src1, igraph_bitset_t *src2) {
    for (igraph_integer_t i = 0; i < IGRAPH_BITNSLOTS(dest->size); ++i)
    {
        VECTOR(*dest)[i] = VECTOR(*src1)[i] | VECTOR(*src2)[i];
    }
}

void igraph_bitset_and(igraph_bitset_t *dest, igraph_bitset_t *src1, igraph_bitset_t *src2) {
    for (igraph_integer_t i = 0; i < IGRAPH_BITNSLOTS(dest->size); ++i)
    {
        VECTOR(*dest)[i] = VECTOR(*src1)[i] & VECTOR(*src2)[i];
    }
}

void igraph_bitset_xor(igraph_bitset_t *dest, igraph_bitset_t *src1, igraph_bitset_t *src2) {
    for (igraph_integer_t i = 0; i < IGRAPH_BITNSLOTS(dest->size); ++i)
    {
        VECTOR(*dest)[i] = VECTOR(*src1)[i] ^ VECTOR(*src2)[i];
    }
}

void igraph_bitset_not(igraph_bitset_t *dest, igraph_bitset_t *src) {
    for (igraph_integer_t i = 0; i < IGRAPH_BITNSLOTS(dest->size); ++i)
    {
        VECTOR(*dest)[i] = ~VECTOR(*src)[i];
    }
}
