from __future__ import print_function

import sys
sys.path += ['../src', '../include']

from cvorg_protocol import *
from cffi import FFI



def replace_byte(s, i, c):
	return s[:i] + c + s[i+1:]


STATUS_STR = {
	0:"RX_AWAITING_START",
	1:"RX_AWAITING_RESYNC",
	2:"RX_GETTING_BYTES"
	}

ffi = FFI()
ffi.cdef(open('lib_cvorg_protocol.i.h','r').read())
lib = ffi.dlopen('lib_cvorg_protocol.so')

f = StringIOPort()
tester = CVORGProtocolMaster(f)

def unpack_frame(frame):
	f.tx(frame)
	return tester.rx_frame()

def test_rx_packet(RxBytes, RxStatus=None):

	rxbuf = ffi.new('RX_BUF_T*')

	rxbytes = []
#	rxstatus = []
	txbytes = []
	link = None

	@ffi.callback("TX_BYTE_FUNC_T")
	def tx_byte(c):
		txbytes.append(c)

	@ffi.callback("SWAP_BUF_FUNC_T")
	def handle_frame(inbuf):
		sz = inbuf.sz_payload
		payload = ffi.unpack(inbuf.payload, sz)
#		print( ' got_frame(): [%d bytes]: %-50s \t %s' %( sz, str2hex(payload), repr(payload) ))
#		print( 'got bytes: %s' % str2hex(inbuf.bytes[0:255]))
		rxbytes.append( payload )
		return inbuf

#	link = ffi.new('LINK_T*', ( tx_byte, handle_command, ffi.NULL, rxbuf,  '\0'))

	link = ffi.new('LINK_T*', (
		tx_byte,				# p_tx_byte_func
		handle_frame,			# p_handle_command_func
		rxbuf,					# pbuf
		0,						# ibuf
		0,						# sz_payload
		0,						# CRC
		b'\0',					# PrevChar
		1,						# rx_watchdog
		lib.RX_AWAITING_START	# status
		) )

	print( ' master sent:  ', str2hex(RxBytes))
	bytearr = bytearray(RxBytes)
	for c in RxBytes:
		lib.rx_packet_byte(link, byte_chr(c))
	#	print hex(g_RxCRC.value), ':', g_iRxBuf.value,  str2hex(g_RxBuf[0:g_iRxBuf.value])#[:g_iRxBuf.value])
#	print
	assert link.rx_watchdog == 0, 'link->rx_watchdog == %d' % link.rx_watchdog

	for _ in range(101):
		lib.inc_rx_watchdog(link)

	txstr = b''.join(txbytes)
	print( '  slave sent:  ', str2hex(txstr))
#	buf = link.pbuf
	print( ' crc=%.4X  ibuf=%d  watchdog=%d  status=%s' % (
			link.crc, link.ibuf, link.rx_watchdog, STATUS_STR[link.status] ) )
	print()

	# print str2hex(txstr)
	# unpack_frame( txstr)

	return int(link.status), rxbytes, txstr


def run_tests():
	valid_payload = b'01234'
	valid_packet = tester.pack_frame( valid_payload )

	print( '* 2 valid packets')
	status, rx, tx = test_rx_packet(  valid_packet + valid_packet )
	assert rx == [valid_payload, valid_payload]
	assert status == lib.RX_AWAITING_START
	assert tx == b''

	print( '* bad start (X~)')
	bad_start_1_packet = replace_byte( valid_packet, 0, b'\x00' )
	status, rx, tx = test_rx_packet(  bad_start_1_packet )
	assert rx == []
	assert status == lib.RX_AWAITING_RESYNC
	assert handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_CMD_MALFORMED'

	print( '* bad start (~X)')
	bad_start_2_packet = replace_byte( valid_packet, 1, b'\x02' )
	status, rx, tx = test_rx_packet(  bad_start_2_packet )
	assert rx == []
	assert status == lib.RX_AWAITING_RESYNC
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_CMD_MALFORMED'

	print( '* size_too_small_packet')
	size_too_small_packet = replace_byte( valid_packet, 2, b'\x02' )
	status, rx, tx = test_rx_packet(  size_too_small_packet )
	assert rx == []
	assert status == lib.RX_AWAITING_RESYNC
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_BAD_CHECKSUM'

	print( '* size_too_big_packet')
	size_too_big_packet = replace_byte( valid_packet, 2, b'\x0a' )
	status, rx, tx = test_rx_packet(  size_too_big_packet )
	assert rx == []
	assert status == lib.RX_AWAITING_START
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_TIMEOUT'

	print( '* bad_crc_packet')
	bad_crc_packet = replace_byte( valid_packet, 3, b'*'    )
	status, rx, tx = test_rx_packet(  bad_crc_packet )
	assert rx == []
	assert status == lib.RX_AWAITING_RESYNC
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_BAD_CHECKSUM'

	print( '* bad_7e_packet')
	bad_7e_packet = replace_byte( valid_packet, 4, b'~' )
	status, rx, tx = test_rx_packet(  bad_7e_packet )
	assert rx == []
	assert status == lib.RX_AWAITING_RESYNC
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_CMD_MALFORMED'

	print( '* valid packet containing ~')
	valid_tilde_payload = b'~ ~~ ~~~ ~~~~'
	valid_tilde_packet = tester.pack_frame( valid_tilde_payload )
	status, rx, tx = test_rx_packet( valid_tilde_packet )
	assert rx == [valid_tilde_payload]
	assert status == lib.RX_AWAITING_START
	assert tx == b''

	print( '* bad_1b_packet')	# invalid escape. FIXME: currently doesn't throw error, can.
	bad_1b_packet = replace_byte( valid_packet, 4, b'\x1b' )
	status, rx, tx = test_rx_packet(  bad_1b_packet )
	assert rx == []
	assert status == lib.RX_AWAITING_START
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_TIMEOUT'

	print( '* valid packet containing ESC')
	valid_esc_payload = b'\x1b \x1b\x1b \x1b\x1b\x1b \x1b\x1b\x1b\x1b'
	valid_esc_packet = tester.pack_frame( valid_esc_payload )
	status, rx, tx = test_rx_packet( valid_esc_packet )
	assert rx == [valid_esc_payload]
	assert status == lib.RX_AWAITING_START
	assert tx == b''

	print( '* valid packet of length 27')
	tildes_27_payload = b'~' * 27
	tildes_27_packet = tester.pack_frame( tildes_27_payload )
	status, rx, tx = test_rx_packet( tildes_27_packet )
	assert rx == [tildes_27_payload]
	assert status == lib.RX_AWAITING_START
	assert tx == b''

	print( '* valid packet of length 126')
	escs_126_payload = b'\x1b' * 126
	escs_126_packet = tester.pack_frame( escs_126_payload )
	status, rx, tx = test_rx_packet( escs_126_packet )
	assert rx == [escs_126_payload]
	assert status == lib.RX_AWAITING_START
	assert tx == b''


	print( '* unexpected_resync')
	unexpected_resync = valid_packet[:5] + valid_packet
	status, rx, tx = test_rx_packet(  unexpected_resync )
	assert rx == [valid_payload]
	assert status == lib.RX_AWAITING_START
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_CMD_MALFORMED'


	print( '* valid, crap, valid')
	status, rx, tx = test_rx_packet(  valid_packet + bad_start_1_packet + valid_packet )
	assert rx == [valid_payload, valid_payload]
	assert status == lib.RX_AWAITING_START
	assert  handle_error( byte_chr( unpack_frame(tx)[0] ), False ).name == 'STATUS_RX_CMD_MALFORMED'


	print( '* 4 empty packets')
	status, rx, tx = test_rx_packet(  tester.pack_frame( b'' ) * 4 )
	assert rx == [b''] * 4
	assert status == lib.RX_AWAITING_START
	assert tx == b''

if __name__ == "__main__":
	run_tests()
