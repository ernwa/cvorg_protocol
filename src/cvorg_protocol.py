from __future__ import print_function
from serial import Serial
import struct
import sys

DEBUG_LEVEL = 3


ESC           = b'\x1b'
START         = b'~'
CHECKSUM_INIT = 0xffff

LAST_RESPONSE = b''


def set_debug_level(level):
	global DEBUG_LEVEL
	DEBUG_LEVEL = level

def debug_print(level, *objs):
#	global DEBUG_LEVEL
	if level <= DEBUG_LEVEL:
		print("DEBUG: ", *objs, file=sys.stderr)



class CVORGProtocolException(Exception):
	def __str__(self):
		docstr = self.__doc__
		value, c_name = self.enum
		return "%s (%d) -- %s" % (c_name, value, docstr)

	@property
	def number(self):
		return self.enum[0]

	@property
	def name(self):
		return self.enum[1]

class NotReadyError(CVORGProtocolException):
	'Receiver is not ready to receive'
	enum = 11, 'STATUS_RX_NOT_READY'

class ChecksumError(CVORGProtocolException):
	'Packet and computed checksum mismatch'
	enum = 12, 'STATUS_RX_BAD_CHECKSUM'

class MalformedPacketError(CVORGProtocolException):
	'Packet contained invalid character sequence'
	enum = 13, 'STATUS_RX_CMD_MALFORMED'

class TimeoutError(CVORGProtocolException):
	'Sender stopped in the middle of a packet'
	enum = 14, 'STATUS_RX_TIMEOUT'

class PacketTooLargeError(CVORGProtocolException):
	'Packet size is too large for reciever buffer'
	enum = 15, 'STATUS_RX_TOO_LONG'

class InternalFailureError(CVORGProtocolException):
	'Reciever state is invalid (memory error?)'
	enum = 19, 'STATUS_RX_INTERNAL_FAIL'

class InvalidCommandError(CVORGProtocolException):
	'Command is not recognized by receiver'
	enum = 21, 'STATUS_CMD_INVALID'

class CommandFailedError(CVORGProtocolException):
	'Command was tried but could not be completed'
	enum = 22, 'STATUS_CMD_FAILED'


ERROR_CODES = {
    11:NotReadyError, 12:ChecksumError, 13:MalformedPacketError,
	14:TimeoutError, 15:PacketTooLargeError, 19:InternalFailureError,
    21:InvalidCommandError, 22:CommandFailedError
	}


def handle_error(buf, raise_errors=True):
	code = ord(buf[:1])
	rest = buf[1:]
	if code in ERROR_CODES:
		error_obj = ERROR_CODES[code](rest)
		if raise_errors:
			raise error_obj
		else:
			return error_obj
	else:
		return buf[1:]


def str2hex(bytes):
	return ' '.join(['%2.2X' % (byte_int(x),) for x in bytes])


def hex2str(str):
	return ''.join( chr(int(x,16)) for x in str.split() )

def byte_int(c):
	return ord(c) if type(c) != int else c

def byte_chr(i):
	return bytes([i]) if type(i) == int else i


def _crc_byte(c):
	crc = 0
	c = c << 8
	for j in range(8):
		if (crc ^ c) & 0x8000:
			crc = (crc << 1) ^ 0x1021
		else:
			crc = crc << 1
		c = c << 1
	return crc

_CRC_BYTE = tuple( _crc_byte(i) for i in range(256) )


def _update_crc(crc, c):
	tmp = (crc >> 8) ^ byte_int(c)
	crc = (crc << 8) ^ _CRC_BYTE[ tmp & 0xff ]
	crc = crc & 0xffff
	return crc


def checksum(str, crc=CHECKSUM_INIT):
	for c in str:
		crc = _update_crc( crc, c )
	return crc



def escape(rawstr):
	escaped_esc = rawstr.replace( ESC, ESC+ESC )
	escaped_tildes = escaped_esc.replace( START, ESC+START)
	return escaped_tildes


def sz2str(sz):
	assert sz < 8192
	if sz < 224:
		return struct.pack('B', sz)
	else:
		return struct.pack('>H', sz | 0xe000 )




class StringIOPort(object):
	def __init__(self, startstr=b""):
		self.ibuf = b""
		self.obuf = b""

	def _nop(self, *args, **kwargs ):
		pass

	def tx( self, istring ):
		self.ibuf += istring

	def rx( self, maxbytes=None ):
		if not maxbytes: maxbytes = len(self.obuf)
		t = self.obuf[:maxbytes]
		self.obuf = self.obuf[maxbytes:]
		return t

	def read( self, maxbytes=None ):
		if not maxbytes: maxbytes = len(self.ibuf)
		t = self.ibuf[:maxbytes]
		self.ibuf = self.ibuf[maxbytes:]
		return t

	def write( self, ostr):
		self.obuf += ostr


	flush = _nop
	flushInput = _nop
	close = _nop




class CVORGProtocolMaster(object):

	def __init__( self, port, *args, **kwargs ):
		if isinstance(port, str):
			self.port = Serial(port, *args, **kwargs )
		else:
			self.port = port
		self.checksum = CHECKSUM_INIT


	def read_byte(self):
		global LAST_RESPONSE
		c = self.port.read(1)
		LAST_RESPONSE += c
		if len(c) != 1:
			raise TimeoutError

		self.checksum = _update_crc(self.checksum, c)
		return c


	def sync_receiver(self):
		b, c = b'', b''
		self.checksum = CHECKSUM_INIT
		while (b,c) != (START, START):
			b, c = c, self.read_byte()
			if c != START:
				self.checksum = CHECKSUM_INIT


	def escaped_read(self, n_bytes):
		out = b''
		while(len(out) < n_bytes):
			b = self.read_byte()		# char -> out. ESC + [ESC, ~] -> [ESC, ~]. ~ + ~ -> resync.

			if b == ESC:
				b = self.read_byte()
				if b not in (START, ESC):
					raise MalformedPacketError('received invalid escaped character') # send error packet, resync
			elif b == START:
				b = self.read_byte()
				if b != START:
					raise MalformedPacketError('received invalid sync sequence') # send error packet, resync
			out += b
		return out


	def pack_frame(self, payload):
		tx_sz_payload = len(payload)
		assert tx_sz_payload >= 0
		tx_escaped_frame = (START + START) + escape( sz2str( tx_sz_payload ) )
#		print( 'pre:', hex(checksum( tx_escaped_frame ) ))
		tx_escaped_frame += escape( payload )
		return tx_escaped_frame + escape( struct.pack('>H', checksum( tx_escaped_frame ) ))


	def rx_frame( self ):
		global LAST_RESPONSE
		LAST_RESPONSE = b''

		self.sync_receiver()
		c = self.escaped_read( 1 )
		szpkt = struct.unpack('B', c)[0]
		if szpkt > 223:
			szpkt = struct.unpack( '>H', c + self.escaped_read(1) )[0] & 0x1fff

		rx_payload = self.escaped_read(szpkt)
		rx_checksum = struct.pack('>H', self.checksum)
		frame_checksum = self.escaped_read( 2 )

		if rx_checksum != frame_checksum:
			debug_print(0, 'last response: %s' % str2hex(LAST_RESPONSE))
			raise ChecksumError('RX checksum failed %s != %s; %s' % (str2hex(rx_checksum), str2hex(frame_checksum), str2hex(LAST_RESPONSE)))

		return rx_payload


	def command( self, payload ):
		tx_bytes = self.pack_frame(payload)
		debug_print(3, '-->' + str2hex(tx_bytes) )
#		self.port.flushInput()
		self.port.write( tx_bytes )
		self.port.flush()

		rx_payload = self.rx_frame()
		debug_print(3, '<--' + str2hex(LAST_RESPONSE))
		return rx_payload


	def get_slave_info( self ):
		resp = self.command('')
		maker, device =  struct.unpack('>HH', resp[:4])
		info_str = resp[4:]# 7E 00 E1 F0
		return maker, device, info_str


class InterfaceBoard(object):
	def __init__(self, interface):
		self.interface = interface

	def read_one( self, rnum, index, unpack_as='' ):
		assert rnum < 128
		payload = handle_error( self.interface.command( struct.pack('BB', rnum, index) ) )
		if unpack_as:
			unpacked = struct.unpack('<'+unpack_as, payload)
			return unpacked[0] if len(unpack_as) == 1 else unpacked
		else:
			return payload # raw bytes

	def write_one( self, rnum, index, payload ):
		assert rnum < 128
		return handle_error( self.interface.command( struct.pack('BB', rnum | 0x80, index) + payload ) )


	def read_all( self, rnum ):
		assert rnum < 128
		return handle_error( self.interface.command( struct.pack('BB', rnum, 0xFF) ) )


	def write_all( self, rnum, payload ):
		assert rnum < 128
		return handle_error( self.interface.command( struct.pack('BB', rnum | 0x80, 0xFF) + payload ) )

	def set_lcd_text(self, text): # list or tuple of up to 4 lines of 20 chars
#		if not isinstance(text, (list, tuple)):
#			text = [text]
#		ostr = ''.join( t.ljust(20) for t in text ).ljust(80)
#		print ('"'+ostr+'"')
#		print (len(ostr))
#		assert len(ostr) == 80
		self.write_all( 0x40, text )

"""
		registers:
00: char[]		system_ID # 1 byte for protocol, 2 bytes for system ID, then human readable string

11: float[36]	ADC_calibration
12: uint16[36]	ADC_value_raw
13: float[36]	ADC_value

20: bool[12]	supply_state

22: uint16[3] 	breaker_thres_raw
23: float[3]	breaker_thres

24: bool[1]		breaker_tripped: clear / tripped

30: bool[2]		M_pin_pullup_state
32: uint32[2]	ADC2_value_raw
33: float[2]	ADC2_value

40: char[80]	lcd_text
"""
"""

class reciever(object):
	def __init__(self):
		self.PrevChar = '\0'
		self.sz_payload = 0
		self.crc = 0



	def rx_packet_byte( RxChar ):
		if RxChar == START_CHAR:
			if PrevChar == START_CHAR: # check for resync condition always
				if self.status >= RX_GETTING_CMD:  # unexpected resync
	#                printf("A");
					NACK( SYS_RX_INVALID )
	#            else printf("B")
				self.status = RX_GETTING_CMD
				self.crc = START_CHARS_CRC # that's the CRC of just '~~'
				goto reset_recvr
#	 	    else:    # not resync sequence yet
#				printf("C")
#				goto_skip_char


		if (self.status < RX_GETTING_CMD):       # skip chars until synced
	#        printf("2")
	#        printf("skip")
			goto skip_char

		elif (self.status > RX_GETTING_CMD):       #
	#        printf("3")
			NACK( SYS_RX_OVERRUN )
#			goto reset_recvr

		# else self.status == RX_GETTING_CMD, proceed


		# at this point we should be only getting valid characters within a frame
		if (not sz_payload or (self.ibuf < sz_payload + 2)):   # payload complete
	#        printf("4")
			self.crc = crc_ccitt_update( self.crc, RxChar )


		if (PrevChar != ESC && (RxChar == ESC || RxChar == START_CHAR)): # todo: handle start_frame better?
	#        printf("5")
			goto skip_char # don't do anything further yet


		if (self.ibuf == 0):    # take care of 2-byte length codes
	#        printf("6")
			if ((uint8_t)RxChar < 223)
	#            printf("A")
				self.pbuf->bytes[0] = 0
				self.pbuf->bytes[1] = RxChar
				self.ibuf = 1

			else
	#            printf("B")
				self.pbuf->bytes[0] = RxChar & 0x1F


		else
			self.pbuf->bytes[self.ibuf] = RxChar


	#    printf(" %d ", self.ibuf)
		if (++self.ibuf == 2)   # got the length
	#        printf("7") #, %x", self.pbuf->sz_payload)
			#ifdef LITTLE_ENDIAN
			   sz_payload = (uint8_t) ((self.pbuf->sz_payload >> 8) | ( self.pbuf->sz_payload << 8 )) # this should really be a 16 bit type
			#else
			   sz_payload = (uint8_t) *((uint16_t*)&self.pbuf->sz_payload)
			#endif
	#        printf("7, %x=%x", self.pbuf->sz_payload, sz_payload)
	#        printf("%4.4x sz_payload = %d\n", *((uint16_t*) &self.pbuf->bytes), sz_payload)


		if (self.ibuf == SZ_RX_BUF || sz_payload >= (SZ_RX_BUF-4))
	#        printf("8")
			NACK(SYS_RX_INVALID)
			goto reset_recvr


		if (self.ibuf == sz_payload + 4) # got CRC, now in possession of full packet
	#        printf("9 %d == %d ", self.ibuf, sz_payload)
			if ( struct.unpack('>H', sz_payload) == self.crc:
				#ifdef LITTLE_ENDIAN
					((uint8_t)self.pbuf->bytes[ sz_payload + 2 ] == (self.crc >> 8)) &&
					((uint8_t)self.pbuf->bytes[ sz_payload + 3 ] == (self.crc & 0xFF))
				#else
					*((crc_t*) &self.pbuf->bytes[ sz_payload ]) == self.crc
				#endif
				)
	#            printf("A")
	#            printf("\ngot valid packet!\n")
				self.status = RX_PERFORMING_CMD
				g_SystemStatus = SYS_PERFORMING_CMD
				self.pbuf = handle_command(self.pbuf)

			else
	#            printf("B")
				NACK( SYS_RX_BAD_CHECKSUM )


			goto reset_recvr


	skip_char:
	#    printf(">\n")
		PrevChar = RxChar
		return

	reset_recvr:
		self.ibuf = 0
		sz_payload = 0
	#    self.pbuf->sz_payload = 0 # might be a good idea
		PrevChar = '\0'
	#    printf("|\n")
"""


if __name__ == '__main__':

	f = StringIOPort()
	tester = CVORGProtocolMaster(f)
	f.tx( tester.pack_frame(b'\x01') )
	tester.command(b'\x1b\x7e\x7e\x1b')

	print( str2hex( f.rx() ) )

#	rxtestpkt = tester.rx_frame()

#	print( str2hex( rxtestpkt ) )
