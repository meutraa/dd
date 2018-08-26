/*******************************************************************************
 *
 *                              Delta Chat Core
 *                      Copyright (C) 2017 Björn Petersen
 *                   Contact: r10s@b44t.com, http://b44t.com
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see http://www.gnu.org/licenses/ .
 *
 ******************************************************************************/


#ifndef __DC_ARRAY_H__
#define __DC_ARRAY_H__
#ifdef __cplusplus
extern "C" {
#endif


/** the structure behind dc_array_t */
struct _dc_array
{
	/** @privatesection */

	uint32_t        magic;
	dc_context_t*   context;     /**< The context the array belongs to. May be NULL when NULL is given to dc_array_new(). */
	size_t          allocated;   /**< The number of allocated items. Initially ~ 200. */
	size_t          count;       /**< The number of used items. Initially 0. */
	uintptr_t*      array;       /**< The data items, can be used between data[0] and data[cnt-1]. Never NULL. */
};


void             dc_array_free_ptr            (dc_array_t*);
dc_array_t*      dc_array_duplicate           (const dc_array_t*);
void             dc_array_sort_ids            (dc_array_t*);
void             dc_array_sort_strings        (dc_array_t*);
char*            dc_array_get_string          (const dc_array_t*, const char* sep);
char*            dc_arr_to_string             (const uint32_t* arr, int cnt);


#ifdef __cplusplus
} /* /extern "C" */
#endif
#endif /* __DC_ARRAY_H__ */
