#################################################################
#								#
#	Copyright 2007 Fidelity Information Services, Inc	#
#								#
#	This source code contains the intellectual property	#
#	of its copyright holder(s), and is made available	#
#	under a license.  If you do not know the terms of	#
#	the license, please stop and do not read further.	#
#								#
#################################################################

#	PAGE	,132
	.title	opp_indincr.s

#	.386
#	.MODEL	FLAT, C

.include "linkage.si"
	.INCLUDE	"g_msf.si"

	.sbttl	opp_indincr
#	PAGE	+
	.DATA
.extern	frame_pointer 	# /* :DWORD */

	.text
.extern	op_indincr

# PUBLIC	opp_indincr
ENTRY opp_indincr  	# /* PROC */
	putframe
	addq 	$8,REG_SP   # burn return PC
	call	op_indincr
	getframe
	ret
# opp_indincr ENDP

# END
