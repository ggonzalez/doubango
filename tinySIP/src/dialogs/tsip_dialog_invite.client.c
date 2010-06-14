/*
* Copyright (C) 2009 Mamadou Diop.
*
* Contact: Mamadou Diop <diopmamadou(at)doubango.org>
*	
* This file is part of Open Source Doubango Framework.
*
* DOUBANGO is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as publishd by
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

/**@file tsip_dialog_invite.client.c
 * @brief SIP dialog INVITE as per RFC 3261.
 * The SOA machine is designed as per RFC 3264 and draft-ietf-sipping-sip-offeranswer-12.
 * MMTel services implementation follow 3GPP TS 24.173.
 *
 * @author Mamadou Diop <diopmamadou(at)doubango.org>
 *
 * @date Created: Sat Nov 8 16:54:58 2009 mdiop
 */
#include "tinysip/dialogs/tsip_dialog_invite.h"

#include "tinysip/dialogs/tsip_dialog_invite.common.h"

#include "tinysip/headers/tsip_header_Session_Expires.h"

#include "tinysdp/parsers/tsdp_parser_message.h"

#include "tnet_transport.h"

#include "tsk_debug.h"

extern int send_INVITEorUPDATE(tsip_dialog_invite_t *self, tsk_bool_t is_INVITE);
extern int send_ACK(tsip_dialog_invite_t *self, const tsip_response_t* r2xxINVITE);
extern int tsip_dialog_invite_process_ro(tsip_dialog_invite_t *self, const tsip_message_t* message);
extern int tsip_dialog_invite_stimers_handle(tsip_dialog_invite_t* self, const tsip_message_t* message);

/* Started -> (oINVITE) -> Outgoing
*/
int c0000_Started_2_Outgoing_X_oINVITE(va_list *app)
{
	int ret;
	tsip_dialog_invite_t *self = va_arg(*app, tsip_dialog_invite_t *);
	
	/* This is the first transaction when you try to make an audio/video/msrp call */
	if(!self->msession_mgr){		
		self->msession_mgr = tmedia_session_mgr_create((tmedia_audio | tmedia_video | tmedia_msrp | tmedia_t38), /* FIXME */
		TSIP_DIALOG_GET_STACK(self)->network.local_ip, tsk_false, tsk_true);
	}

	/* We are the client */
	self->is_client = tsk_true;

	/* Default: enable session timers
		RFC 4028 - 7.1. Generating an Initial Session Refresh Request

		A UAC MAY include a Session-Expires header field in an initial
		session refresh request if it wants a session timer applied to the
		session.  The value of this header field indicates the session
		interval desired by the UAC.  If a Min-SE header is included in the
		initial session refresh request, the value of the Session-Expires
		MUST be greater than or equal to the value in Min-SE.

		The UAC MAY include the refresher parameter with value 'uac' if it
		wants to perform the refreshes.  However, it is RECOMMENDED that the
		parameter be omitted so that it can be selected by the negotiation
		mechanisms described below.
	*/
	self->stimers.timer.timeout = 90/* FIXME: (TSIP_SESSION_EXPIRES_DEFAULT_VALUE) */;
	tsk_strupdate(&self->stimers.refresher, "uac");
	
	/* send the request */
	ret = send_INVITE(self);

	/* alert the user */
	TSIP_DIALOG_SIGNAL(self, tsip_event_code_dialog_connecting, "Dialog connecting");


	return ret;
}

/* Outgoing -> (i2xx INVITE) -> Connected
*/
int c0001_Outgoing_2_Connected_X_i2xxINVITE(va_list *app)
{
	int ret;

	tsip_dialog_invite_t *self = va_arg(*app, tsip_dialog_invite_t *);
	const tsip_response_t *r2xxINVITE = va_arg(*app, const tsip_response_t *);

	/* Update the dialog state */
	if((ret = tsip_dialog_update(TSIP_DIALOG(self), r2xxINVITE))){
		return ret;
	}
	
	/* Process remote offer */
	if((ret = tsip_dialog_invite_process_ro(self, r2xxINVITE))){
		/* Send error */
		return ret;
	}

	/* send ack */
	if(ret == 0){
		ret = send_ACK(self, r2xxINVITE);
	}
	else{
		/* send error */
	}

	/* Determine whether the remote party support UPDATE 
	* FIXME: do the same in server side */
	self->support_update = tsip_message_allowed(r2xxINVITE, "UPDATE");

	/* Session Timers */
	if(self->stimers.timer.timeout){
		tsip_dialog_invite_stimers_handle(self, r2xxINVITE);
	}

	/* alert the user */
	TSIP_DIALOG_INVITE_SIGNAL(self, tsip_ao_invite, 
		TSIP_RESPONSE_CODE(r2xxINVITE), TSIP_RESPONSE_PHRASE(r2xxINVITE), r2xxINVITE);

	return ret;
}

/* Outgoing -> (i300-i699 INVITE) -> Terminated
*/
int c0002_Outgoing_2_Terminated_X_i300_to_i699INVITE(va_list *app)
{
	tsip_dialog_invite_t *self = va_arg(*app, tsip_dialog_invite_t *);
	const tsip_response_t *response = va_arg(*app, const tsip_response_t *);

	/* alert the user */
	TSIP_DIALOG_INVITE_SIGNAL(self, tsip_ao_invite, 
		TSIP_RESPONSE_CODE(response), TSIP_RESPONSE_PHRASE(response), response);

	return 0;
}

/* Outgoing -> (oCANCEL ) -> Terminated
*/
int c0003_Outgoing_2_Terminated_X_oCANCEL(va_list *app)
{
	/* alert the user */
	return 0;
}
