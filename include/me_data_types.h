/**
 * @file me_data_types.h
 *
 * @brief Boards specific types for config load function.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author Krzysztof Gantzke	(k.gantzke@meilhaus.de)
 */

#ifndef _ME_DATA_TYPES_H_
# define _ME_DATA_TYPES_H_

/// Boards specific types.

typedef struct //me_custom_constr
{
	char version[32];
} me_custom_constr_t;

typedef struct //me1000_config_load
{
	int subdevice_no;
} me1000_config_load_t;

typedef struct meFF00_config_load
{
	size_t size;
	unsigned int version;
} meFF00_config_load_t;

typedef struct me4600_ai_config_load
{
	unsigned int chunk_size;
	unsigned int chunks_count;
} me4600_ai_config_load_t;

# define AI_BUFFER_RESIZE	(ME_CAP_AI_BUFFER_SIZE)
typedef struct me4600_config_load
{
	unsigned int comand_id;
	me4600_ai_config_load_t config;
} me4600_config_load_t;

#endif	//_ME_DATA_TYPES_H_
