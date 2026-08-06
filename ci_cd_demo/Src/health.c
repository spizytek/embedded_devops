/*
 * health.c
 *
 *  Created on: Aug 6, 2026
 *      Author: Agha Ikechukwu Kingsley
 */

#include <stdio.h>
#include "version.h"
#include "health.h"


/**
 * @brief Emits the firmware identity line over the debug UART
 *
 * The output is a single CSV-like line designed for both humans and later automation:
 * HEALTH, project=...,version=...,build_id=...,git_sha=...,config=...,timestamp=...
 */

void health_emit_identity_line(void){

	printf("HEALTH, project=%s,version=%s,build_id=%s,git_sha=%s,config=%s,timestamp=%s\r\n",
			EE_FW_PROJECT_NAME,
			EE_FW_VERSION_STRING,
			EE_FW_BUILD_CONFIG,
			EE_FW_BUILD_ID,
			EE_FW_GIT_SHA,
			EE_FW_BUILD_TIMESTAMP_UTC
		);

}
