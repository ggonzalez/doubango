/*
* Copyright (C) 2009 Mamadou Diop.
*
* Contact: Mamadou Diop <diopmamadou@yahoo.fr>
*	
* This file is part of Open Source Doubango Framework.
*
* DOUBANGO is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*	
* DOUBANGO is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*	
* You should have received a copy of the GNU General Public License
* along with DOUBANGO.
*
*/

/**@file tmsrp.h
 * @brief MSRP API.
 *
 * @author Mamadou Diop <diopmamadou(at)yahoo.fr>
 *
 * @date Created: Sat Nov 8 16:54:58 2009 mdiop
 */
#ifndef TINYMSRP_TMSRP_H
#define TINYMSRP_TMSRP_H

#include "tinyMSRP_config.h"

#include "tinyMSRP/tmsrp_message.h"

TMSRP_BEGIN_DECLS

TINYMSRP_API tmsrp_request_t* tmsrp_create_bodiless();
TINYMSRP_API tmsrp_response_t* tmsrp_create_response(const tmsrp_request_t* request, short status, const char* comment);
TINYMSRP_API tmsrp_request_t* tmsrp_create_report(const tmsrp_request_t* request, short status, const char* reason);
TINYMSRP_API int tmsrp_isReportRequired(const tmsrp_request_t* request);

TMSRP_END_DECLS

#endif /* TINYMSRP_TMSRP_H */