//
//
// This file has been excerpted from Quisk 3.5.11 by James C. Ahlstrom
// http://james.ahlstrom.name/quisk/
//
// In the original file there is no license.
//
// This module provides access to the SDR-IQ by RfSpace.  It is the source
// for the Python extension module sdriq.  It can be used as a model for an
// extension module for other hardware.  Read the end of this file for more
// information.  This module was written by James Ahlstrom, N2ADR.
//
//

#ifdef MS_WINDOWS
#include <windows.h>
#include "ftd2xx.h"
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include <complex.h>

//#include "quisk.h"
#include "sdriq.h"

void QuiskSleepMicrosec(int usec)
{
#ifdef MS_WINDOWS
        int msec = (usec + 500) / 1000;         // convert to milliseconds
        if (msec < 1)
                msec = 1;
        Sleep(msec);
#else
        struct timespec tspec;
        tspec.tv_sec = usec / 1000000;
        tspec.tv_nsec = (usec - tspec.tv_sec * 1000000) * 1000;
        nanosleep(&tspec, NULL);
#endif
}
 double QuiskTimeSec(void)
{  // return time in seconds as a double
#ifdef MS_WINDOWS
        FILETIME ft;
        ULARGE_INTEGER ll;

        GetSystemTimeAsFileTime(&ft);
        ll.LowPart  = ft.dwLowDateTime;
        ll.HighPart = ft.dwHighDateTime;
        return (double)ll.QuadPart * 1.e-7;
#else
        struct timeval tv;

        gettimeofday(&tv, NULL);
        return (double)tv.tv_sec + tv.tv_usec * 1e-6;
#endif
}

QUISK_SOUND_STATE quisk_sound_state = { 0,0, 6000 };

#include "sdriq.h"

struct ad6620 dec360 = {
	4, 18, 5,		// decimation factors
	4, 13, 6,		// scale factors
	{131, -230, -38, -304, -235, -346, -237, -181, 12, 149, 310, 349, 320, 154, -60,
-310, -480, -540, -423, -169, 187, 523, 749, 762, 543, 117, -394, -851, -1093, -1025,
-621, 22, 737, 1300, 1522, 1288, 625, -309, -1245, -1893, -2013, -1515, -489, 793, 1957,
 2623, 2533, 1640, 149, -1533, -2893, -3475, -3023, -1584, 480, 2582, 4063, 4405, 3401, 1246,
-1484, -3986, -5455, -5345, -3557, -509, 2951, 5776, 7030, 6193, 3355, -760, -4970, -7969, -8722,
-6815, -2628, 2712, 7632, 10563, 10431, 7033, 1169, -5529, -11037, -13543, -12021, -6623, 1287, 9443,
 15320, 16896, 13319, 5269, -5122, -14811, -20711, -20642, -14088, -2504, 10961, 22272, 27682, 24909, 13986,
-2524, -20051, -33214, -37378, -30153, -12380, 11742, 35506, 51387, 53179, 38008, 7662, -31208, -68176, -91255,
-89756, -57102, 7096, 96306, 197916, 295555, 372388, 414662, 414662, 372388, 295555, 197916, 96306, 7096, -57102,
-89756, -91255, -68176, -31208, 7662, 38008, 53179, 51387, 35506, 11742, -12380, -30153, -37378, -33214, -20051,
-2524, 13986, 24909, 27682, 22272, 10961, -2504, -14088, -20642, -20711, -14811, -5122, 5269, 13319, 16896,
 15320, 9443, 1287, -6623, -12021, -13543, -11037, -5529, 1169, 7033, 10431, 10563, 7632, 2712, -2628,
-6815, -8722, -7969, -4970, -760, 3355, 6193, 7030, 5776, 2951, -509, -3557, -5345, -5455, -3986,
-1484, 1246, 3401, 4405, 4063, 2582, 480, -1584, -3023, -3475, -2893, -1533, 149, 1640, 2533,
 2623, 1957, 793, -489, -1515, -2013, -1893, -1245, -309, 625, 1288, 1522, 1300, 737, 22,
-621, -1025, -1093, -851, -394, 117, 543, 762, 749, 523, 187, -169, -423, -540, -480,
-310, -60, 154, 320, 349, 310, 149, 12, -181, -237, -346, -235, -304, -38, -230, 131}};

struct ad6620 dec500 = {
	4, 25, 5,
	4, 16, 5,
	{-197, 356, -153, 176, -101, 34, -125, -46, -106, -7, 12, 115, 129,
 157, 86, 12, -116, -197, -251, -203, -97, 80, 242, 364, 367,
 259, 33, -228, -461, -565, -504, -255, 106, 488, 756, 813, 604,
 172, -377, -868, -1139, -1066, -639, 53, 807, 1390, 1584, 1288, 537,
-470, -1439, -2046, -2060, -1406, -232, 1143, 2290, 2820, 2496, 1339, -366,
-2120, -3369, -3659, -2808, -976, 1340, 3448, 4652, 4486, 2873, 198, -2785,
-5152, -6095, -5184, -2546, 1137, 4785, 7240, 7613, 5604, 1641, -3190, -7438,
-9701, -9091, -5546, 69, 6163, 10849, 12519, 10373, 4745, -2905, -10342, -15198,
-15692, -11253, -2807, 7368, 16229, 20838, 19296, 11436, -946, -14436, -24891, -28637,
-23657, -10406, 8025, 26518, 39215, 41181, 30008, 6896, -23122, -51997, -70364, -69788,
-44995, 4465, 73600, 152608, 228689, 288639, 321648, 321648, 288639, 228689, 152608, 73600,
 4465, -44995, -69788, -70364, -51997, -23122, 6896, 30008, 41181, 39215, 26518, 8025,
-10406, -23657, -28637, -24891, -14436, -946, 11436, 19296, 20838, 16229, 7368, -2807,
-11253, -15692, -15198, -10342, -2905, 4745, 10373, 12519, 10849, 6163, 69, -5546,
-9091, -9701, -7438, -3190, 1641, 5604, 7613, 7240, 4785, 1137, -2546, -5184,
-6095, -5152, -2785, 198, 2873, 4486, 4652, 3448, 1340, -976, -2808, -3659,
-3369, -2120, -366, 1339, 2496, 2820, 2290, 1143, -232, -1406, -2060, -2046,
-1439, -470, 537, 1288, 1584, 1390, 807, 53, -639, -1066, -1139, -868,
-377, 172, 604, 813, 756, 488, 106, -255, -504, -565, -461, -228,
 33, 259, 367, 364, 242, 80, -97, -203, -251, -197, -116, 12,
 86, 157, 129, 115, 12, -7, -106, -46, -125, 34, -101, 176, -153, 356, -197}};

struct ad6620 dec600 = {
	5, 30, 4,		// decimation factors
	5, 17, 5,		// scale factors
	{ 436, -1759, 99, -1281, 0, -280, 619, 409, 553, -71, -344, -753, -537, -203,
 453, 782, 838, 325, -326, -949, -1037, -628, 230, 991, 1330, 923, 10, -1032,
-1569, -1324, -299, 956, 1822, 1739, 716, -809, -2000, -2212, -1212, 520, 2123, 2678,
 1823, -111, -2124, -3143, -2509, -463, 2002, 3548, 3279, 1188, -1699, -3877, -4088,
-2087, 1206, 4069, 4920, 3137, -478, -4094, -5720, -4343, -493, 3887, 6454, 5669, 1741,
-3412, -7052, -7096, -3266, 2607, 7462, 8573, 5084, -1425, -7602, -10058, -7187, -193,
 7400, 11481, 9579, 2301, -6756, -12777, -12244, -4971, 5569, 13854, 15181, 8285, -3699,
-14613, -18387, -12369, 966, 14920, 21888, 17412, 2905, -14598, -25744, -23754, -8362,
 13363, 30114, 32035, 16259, -10708, -35362, -43638, -28445, 5493, 42387, 62053, 49891, 5603, -53825,
-99044, -99811, -38467, 80479, 229234, 365232, 446270, 446270, 365232, 229234, 80479, -38467,
-99811, -99044, -53825, 5603, 49891, 62053, 42387, 5493, -28445, -43638, -35362, -10708, 16259,
 32035, 30114, 13363, -8362, -23754, -25744, -14598, 2905, 17412, 21888, 14920, 966, -12369,
-18387, -14613, -3699, 8285, 15181, 13854, 5569, -4971, -12244, -12777, -6756, 2301, 9579,
 11481, 7400, -193, -7187, -10058, -7602, -1425, 5084, 8573, 7462, 2607, -3266, -7096, -7052, -3412,
 1741, 5669, 6454, 3887, -493, -4343, -5720, -4094, -478, 3137, 4920, 4069, 1206, -2087, -4088,
-3877, -1699, 1188, 3279, 3548, 2002, -463, -2509, -3143, -2124, -111, 1823, 2678, 2123, 520, -1212,
-2212, -2000, -809, 716, 1739, 1822, 956, -299, -1324, -1569, -1032, 10, 923, 1330, 991, 230, -628,
-1037, -949, -326, 325, 838, 782, 453, -203, -537, -753, -344, -71, 553, 409, 619, -280, 0, -1281,
 99, -1759, 436}};

struct ad6620 dec1250 = {
	10, 25, 5,		// decimation factors
	7, 15, 6,		// scale factors
	{-378, 13756, -14444, 8014, -7852, 3556, -3779, 2733, -909, 2861, 208, 1827, -755, -243, -2134, -1267, -1705,
 20, 492, 2034, 1885, 1993, 535, -459, -2052, -2387, -2454, -1112, 246, 2053, 2832, 3019, 1774, 133, -1973,
-3220, -3654, -2546, -683, 1769, 3531, 4330, 3431, 1417, -1400, -3730, -5013, -4428, -2350, 831, 3780, 5669,
 5520, 3489, -23, -3635, -6252, -6689, -4839, -1057, 3245, 6715, 7904, 6403, 2443, -2555, -6998, -9129, -8175,
-4172, 1504, 7033, 10318, 10147, 6281, -23, -6747, -11415, -12315, -8815, -1972, 6041, 12354, 14669, 11830, 4593,
-4800, -13060, -17207, -15419, -7992, 2861, 13425, 19944, 19729, 12404, 21, -13318, -22930, -25017, -18239, -4245,
 12519, 26289, 31789, 26259, 10571, -10635, -30306, -41114, -38121, -20661, 6795, 35686, 55688, 58124, 39093, 1561,
-44548, -84372, -101901, -84500, -26969, 66196, 180937, 296484, 390044, 442339, 442339, 390044, 296484, 180937,
 66196, -26969, -84500, -101901, -84372, -44548, 1561, 39093, 58124, 55688, 35686, 6795, -20661, -38121, -41114,
-30306, -10635, 10571, 26259, 31789, 26289, 12519, -4245, -18239, -25017, -22930, -13318, 21, 12404, 19729, 19944,
 13425, 2861, -7992, -15419, -17207, -13060, -4800, 4593, 11830, 14669, 12354, 6041, -1972, -8815, -12315, -11415,
-6747, -23, 6281, 10147, 10318, 7033, 1504, -4172, -8175, -9129, -6998, -2555, 2443, 6403, 7904, 6715, 3245, -1057,
-4839, -6689, -6252, -3635, -23, 3489, 5520, 5669, 3780, 831, -2350, -4428, -5013, -3730, -1400, 1417, 3431, 4330,
 3531, 1769, -683, -2546, -3654, -3220, -1973, 133, 1774, 3019, 2832, 2053, 246, -1112, -2454, -2387, -2052, -459,
 535, 1993, 1885, 2034, 492, 20, -1705, -1267, -2134, -243, -755, 1827, 208, 2861, -909, 2733, -3779, 3556, -7852,
 8014, -14444, 13756, -378 }};


// This module provides access to the SDR-IQ by RfSpace.  It is the source
// for the Python extension module sdriq.  It can be used as a model for an
// extension module for other hardware.  Read the end of this file for more
// information.  This module was written by James Ahlstrom, N2ADR.

// Start of SDR-IQ specific code:
//

#define SDRIQ_BLOCK		8194
#define SDRIQ_BUF_SIZE	131072
#define INC_IREAD		if (++iread >= SDRIQ_BUF_SIZE) iread = 0;

// Number of milliseconds to wait for SDR-IQ data on each read
#define SDRIQ_MSEC	4
// Timeout for SDR-IQ as a multiple of SDRIQ_MSEC
#define SDRIQ_TIMEOUT	50

// Type field for the message block header; upper 3 bits of byte
#define TYPE_HOST_SET	0
#define TYPE_HOST_GET	(1 << 5)
#define NAME_SIZE		16

static double sdriq_clock;

static int	sdr_ack;	// got ACK
static int	sdr_nak;	// got NAK
static char sdr_name[NAME_SIZE];		// control item 1
static char sdr_serial[NAME_SIZE];		// item 2
static int  sdr_interface;				// item 3
static int  sdr_firmware;				// item 4
static int  sdr_bootcode;				// item 4
static int  sdr_status;					// item 5
static char sdr_product_id[4];          // item 9
static int	sdr_idle;					// item 0x18, 1==idle, 2==run
static int	sdriq_freq=7050000;					// set SDR-IQ to this frequency
static int	sdriq_gstate=2, sdriq_gain=64;		// set SDR-IQ gain to this value
static int  sdriq_decimation = 1250;			// set SDR-IQ decimation to this value
static int	cur_freq, cur_gstate, cur_gain;		// current value of frequency and gain
static int  cur_decimation;						// current value of decimation

#ifdef MS_WINDOWS
static FT_HANDLE quisk_sdriq_fd = INVALID_HANDLE_VALUE;

static int Read(void * buf, int bufsize)
{
	DWORD bytes, rx_bytes;

	if (quisk_sdriq_fd == INVALID_HANDLE_VALUE)
		return 0;

	if (FT_GetQueueStatus(quisk_sdriq_fd, &rx_bytes) != FT_OK) {
		quisk_sound_state.read_error++;
		return 0;
	}
	if (rx_bytes > bufsize)
		rx_bytes = bufsize;
	if (rx_bytes == 0) {
		return 0;
	}
	else if (FT_Read(quisk_sdriq_fd, buf, rx_bytes, &bytes) == FT_OK) {
		return bytes;
	}
	else {
		quisk_sound_state.read_error++;
		return 0;	
	}
}

static int Write(void * buf, int length)
{
	DWORD bytes;

	if (quisk_sdriq_fd == INVALID_HANDLE_VALUE)
		return 0;

	if (FT_Write(quisk_sdriq_fd, buf, length, &bytes) == FT_OK) {
		return bytes;
	}
	else {
		return 0;
	}
}
#else
#define INVALID_HANDLE_VALUE	-1
static int quisk_sdriq_fd = INVALID_HANDLE_VALUE;
static int Read(void * buf, int bufsize)
{
	int res;

	if (quisk_sdriq_fd == INVALID_HANDLE_VALUE)
		return 0;
	res = read(quisk_sdriq_fd, buf, bufsize);
	if (res < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			quisk_sound_state.read_error++;
		}
		return 0;
	}
	return res;
}

static int Write(void * buf, int length)
{
	int res;

	if (quisk_sdriq_fd == INVALID_HANDLE_VALUE)
		return 0;
	res = write(quisk_sdriq_fd, buf, length);
    if (res <= 0)
		return 0;
	return res;
}
#endif

static void update_item(int item, const unsigned char * data)
{
	switch(item) {

    //  5.1.General Control Items
    //  5.1.1 Target Name
    //  Purpose: Returns an ASCII string describing the Target device.
    //  Control Item Code : 0x0001
    //  Control Item Parameter Format: The data is a NULL(zero) terminated character byte string.
    //  Example, to request the target name, the host sends:
    //  [04][20] [01][00]
    //  The Target responds with "SDR-IQ" :
    //  [0B][00] [01]00] [53][44][52][2D][31][34][00]

	case 1:
		strncpy(sdr_name, (char *)data, NAME_SIZE);
		sdr_name[NAME_SIZE - 1] = 0;
		break;

    //  5.1.2 Target Serial Number
    //  Purpose: Contains an ASCII string containing the Target device serial number.
    //  Control Item Code : 0x0002
    //  Control Item Parameter Format: The data is a NULL(zero) terminated character byte string.
    //  Example, to request the target serial number the host sends:
    //  [04][20] [02][00]
    //  The Target responds with "MT123456" or the serial number the particular device:
    //  [0C][00] [02]00] [4D][54][31][32]33][34[35[36][00]\Uffffffff
    //  

	case 2:
		strncpy(sdr_serial, (char *)data, NAME_SIZE);
		sdr_serial[NAME_SIZE - 1] = 0;
		break;

    //  5.1.3 Interface Version
    //  Purpose: Contains the version number of the Host or Targets implemented Interface. This allows the Host or Target
    //  to display or adapt to different versions of the interface.
    //
    //  Control Item Code: 0x0003
    //  Control Item Parameter Format: The data is a 2 byte 16 bit unsigned variable equal to the version times 100. For
    //  example the value 123 would be version 1.23.
    //  Example, to request the target interface version the host sends:
    //  [04][20] [03][00]
    //  The Target with an interface version of 5.29 responds with:
    //  [06][00] [03]00] [11][02]

	case 3:
		sdr_interface = (data[1] << 8) | data[0];
		break;
             
    //  5.1.4 Hardware/Firmware Versions
    //  Purpose: Contains the Firmware or Hardware version information of the Target.
    //  Control Item Code: 0x0004
    //  Control Item Parameter Format:
    //  The first parameter is a 1 byte Firmware ID specifying which firmware or hardware version to retrieve.
    //  ID=0 returns the PIC boot code version.
    //  ID=1 returns the PIC firmware version.
    //  The version data is a 2 byte 16 bit unsigned variable equal to the version times 100. For example the value 123
    //  would be version 1.23.
    //
    //  Example, to request the PIC firmware version host sends:
    //  [05][20] [04][00] [01]
    //  The Target with a PIC firmware version of 5.29 responds with:
    //  [07][00] [04][00] [01] [11][02]
    //
    //  Example, to request the PIC boot code version host sends:
    //  [05][20] [04][00] [00]
    //  The Target with a PIC firmware version of 5.29 responds with:
    //  [07][00] [04][00] [00] [11][02]    
    //
	case 4:
		if (data[0])
			sdr_firmware = (data[2] << 8) | data[1];
		else
			sdr_bootcode = (data[2] << 8) | data[1];
		break;

     // 5.1.5 Status/Error Code
     // Purpose: Contains the Error/Status code(s) of the Target. This item is used to notify the Host of any error or
     // problem using a list of code values. Once the error code(s) are obtained, the host can interrogate the Target "Error
     // String" Control Item to obtain a description string of the error(s) or status.
     // Control Item Code: 0x0005
     // Control Item Parameter Format: The data is a list of 1 byte unsigned variable equal to the error number associated
     // with a particular error. There can be multiple error codes returned by the Target.
     // 0x0B = SDR-IQ Idle
     // 0x0C = SDR-IQ Busy(capturing data)
     // 0x0D = SDR-IQ Loading AD6620 parameters
     // 0x0E = SDR-IQ Boot mode Idle
     // 0x0F = SDR-IQ Boot mode busy programming
     // 0x20 = SDR-IQ A/D overload occurred
     // 0x80 = SDR-IQ Boot mode programming error
     // Example, host request status:
     // [04][20] [05][00]
     // The idle Target responds with
     // [05][00] [05]00] [0B]
     //
	case 5:
		sdr_status = data[0];
		if (data[0] == 0x20)
			quisk_sound_state.overrange++;
#if DEBUG
		if (data[0] == 0x20)
			printf ("Got overrange (clip)\n");
		else
			printf ("Got status 0x%X\n", data[0]);
#endif
		break;
        
    // 5.1.6 Product ID (Firmware 1.04 or Greater)
    // Purpose: Returns the 4 byte product ID for the SDR-IQ used in firmware update validation.
    // Control Item Code: 0x0009
    // Control Item Parameter Format:
    // A read only value returned identifying the SDR-IQ.
    // Example, host request product ID:
    // [04][20] [09][00]
    // The Target responds with
    // [08][00] [09][00] [00][A5][FF][5A]
    case 9:
        memcpy (sdr_product_id, data, 4);
        break;

	case 0x18:
		sdr_idle = data[1];
#if DEBUG
		if (sdr_idle == 1)
			printf ("Got idle code IDLE\n");
		else if (sdr_idle == 2)
			printf ("Got idle code RUN\n");
		else
			printf ("Got idle code UNKNOWN\n");
#endif
		break;
	}
}

static void get_item(		// Host sends a request for a control item
	int item,				// the item number
	int nparams,			// the length of params
	char * params)			// byte array of parameters, or NULL iff nparams==0
{
	int length;				// length of message block
	char buf[64];			// message block header and control item and data

	length = 4 + nparams;
	if (length > 60)
		return;		// error
	buf[0] = length & 0xFF;			// length LSB
	buf[1] = TYPE_HOST_GET | ((length >> 8) & 0x1F);	// 3-bit type and 5-bit length MSB
	buf[2] = item & 0xFF;			// item LSB
	buf[3] = (item >> 8) & 0xFF;	// item MSB
	if (nparams)
		memcpy(buf + 4, params, nparams);
	if (Write(buf, length) != length) {
		quisk_sound_state.read_error++;
#if DEBUG
		printf("get_item write error\n");
#endif
	}
#if DEBUG > 1
	printf ("get_item 0x%X\n", item);
#endif
	return;
}

static void set_item(	// host command to set a control item
	int item,			// the item number
	int nparams,		// the length of params
	char * params)		// byte array of parameters, or NULL iff nparams==0
{
	int length;
	char buf[64];			// message block header and control item and data

	length = 4 + nparams;		// total length 
	if (length > 60)
		return;		// error
	buf[0] = length & 0xFF;				// length LSB
	buf[1] = TYPE_HOST_SET | ((length >> 8) & 0x1F);	// 3-bit type and 5-bit length MSB
	buf[2] = item & 0xFF;			// item LSB
	buf[3] = (item >> 8) & 0xFF;	// item MSB
	if (nparams)
		memcpy(buf + 4, params, nparams);
	if (Write(buf, length) != length) {
		quisk_sound_state.read_error++;
#if DEBUG
		printf("set_item write error\n");
#endif
	}
#if DEBUG > 1
		printf ("set_item 0x%X\n", item);
#endif
}

// The ft245 driver does not have a circular buffer for input; bytes are just appended
// to the buffer.  When all bytes are read and the buffer goes empty, the pointers are reset to zero.
// Be sure to empty out the ft245 frequently so its buffer does not overflow.
static int sdr_recv(complex * samp, int sampsize)
{		// Read all data from the SDR-IQ and process it.
		// Return the number >= 0 of I/Q data samples that are available in samp.
	int k, res, item, navail, nSamples;
    short ii, qq;
	unsigned char buf128[128];
	static unsigned char buf[SDRIQ_BUF_SIZE];
	static int iread=0;
	static int iwrite=0;
	static int state=0;
	static int length;
	static int type;
	static int sample_count;

	nSamples = 0;		// number of samples added to samp
	// first read all characters from the ft245 driver into our large buffer
	if (iread == 0) {
		k = SDRIQ_BUF_SIZE - iwrite - 1;
		if (k > 65536)		// maximum read for ft245
			k = 65536;
		if (k > 0) {
			res = Read(buf + iwrite, k);
			iwrite += res;
		}
	}
	else if (iread <= iwrite) {
		k = SDRIQ_BUF_SIZE - iwrite;
		if (k > 65536)		// maximum read for ft245
			k = 65536;
		res = Read(buf + iwrite, k);
		if (res == k)
			iwrite = 0;
		else if (res > 0)
			iwrite += res;
	}
	if (iread > iwrite) {
		k = iread - iwrite - 1;
		if (k > 65536)		// maximum read for ft245
			k = 65536;
		if (k > 0) {
			res = Read(buf + iwrite, k);
			iwrite += res;
		}
	}

	// Now process the data we have in buf
start_here:
	if (iread > iwrite)		// calculate number of available bytes: navail
		navail = SDRIQ_BUF_SIZE - iread + iwrite;
	else
		navail = iwrite - iread;
	if (state == 0) {		// starting state; we need to read the first two bytes for length and type
		if (navail < 2)
			return nSamples;		// no more data available
		// we have the first two bytes
		length = buf[iread];
		INC_IREAD
		type = (buf[iread] >> 5) & 0x7;			// 3-bit type
		length |= (buf[iread] & 0x1F) << 8;		// length including header
		INC_IREAD
#if DEBUG > 1
		if (length != 0 && !(type == 3 && length == 3))
			printf("Got message type %d length %d\n", type, length);
#endif
		if (type > 3 && length == 0)			// data block with zero length
			length = 8194;	// special data length
		length -= 2;		// we read two bytes; length is the remaining bytes
		if (length < 0) {
			state = 9;		// bad length; attempt resync
		}
		else if (length == 0) {			// NAK
			sdr_nak = 1;
#if DEBUG
			printf("Got NAK\n");
#endif
			// state remains at zero
		}
		else if (samp && length > 50 && length < 8192) {	// No such message; we are out of sync
			state = 9;
		}
		else if (samp && type == 4 && length == 8192) {		// ADC samples data block
			state = 5;
			sample_count = 2048;
		}
		else if (navail >= length) {
			state = 3;
		}
		else {
			state = 2;
		}
		goto start_here;		// process the next state
	}
	else if (state == 2) {		// waiting for all "length" bytes to be read
		if (navail < length)
			return nSamples;	// partially read block
		state = 3;
		goto start_here;		// process the next state
	}
	else if (state == 3) {		// we have all the bytes of the record available
		if (length == 1 && type == 3) {	// ACK
			sdr_ack = buf[iread];
			INC_IREAD
#if DEBUG > 1
			printf("Got ACK for 0x%X\n", sdr_ack);
#endif
		}
		else if ((type == 0 || type == 1) && length >= 2) {		// control item
			item = buf[iread];
			INC_IREAD
			item |= buf[iread] << 8;		// control item number
			INC_IREAD
			length -= 2;
			for (k = 0; k < length; k++) {
				if (k < 128)
					buf128[k] = buf[iread];
				INC_IREAD
			}
			update_item(item, buf128);
		}
		else {
			iread += length;	// discard block
			if (iread >= SDRIQ_BUF_SIZE)
				iread -= SDRIQ_BUF_SIZE;
		}
		state = 0;
		goto start_here;		// we read a whole block
	}
	else if (state == 5) {		// read available samples into samp
		//ptimer(4096);
		while (navail >= 4 && sample_count && nSamples < sampsize) {			// samples are 16-bit little-endian
			ii = buf[iread];	// assumes a short is two bytes
			INC_IREAD
			ii |= buf[iread] << 8;
			INC_IREAD
			qq = buf[iread];
			INC_IREAD
			qq |= buf[iread] << 8;
			INC_IREAD
			navail -= 4;	// we read four bytes
			// convert 16-bit samples to 32-bit samples
			samp[nSamples++] = 65536.0 * ii + 65536.0 * qq * I;	// return sample as complex number
			sample_count--;		// we added one sample
		}
//printf("State %d navail %d sample_count %d nSamples %d\n", state, navail, sample_count, nSamples);
		if (sample_count > 0)		// no more samples available
			return nSamples;		// return the available samples
		state = 0;				// this block was completely read
		goto start_here;		// process the next state
	}
	else if (state == 9) {		// try to re-synchronize
		quisk_sound_state.read_error++;
#if DEBUG
		printf ("Lost sync: type %d  length %d\n", type, length);
#endif
		while (1) {		// empty the buffer
			if (Read(buf, 1024) == 0)
				break;
		}
#if DEBUG > 2
			printf("Buffer is empty\n");
#endif
		while (1) {		// look for the start of data blocks "\x00\x80"
			res = Read(buf, 1);
			if (res != 1) {
				QuiskSleepMicrosec(SDRIQ_MSEC * 1000);
			}
			else if (state == 9) {		// look for 0x00
				if (buf[0] == 0x00)
					state = 10;
			}
			else {		// state 10: look for 0x80
				if (buf[0] == 0x80) {
					state = 5;
					iread = iwrite = 0;
					sample_count = 2048;
#if DEBUG
					printf("Regained sync\n");
#endif
					break;	// we probably have a data block start
				}
				else if (buf[0] != 0x00) {
					state = 9;
				}
			}
		}
		goto start_here;		// process the next state
	}
	return nSamples;		// should not happen
}

static void set_ad6620(	// host command to set an AD6620 register
	int address,		// the register address
	int value)			// the value; up to 4 bytes
{
	char buf[12];

	buf[0] = '\x09';
	buf[1] = '\xA0';
	buf[2] = address & 0xFF;			// low byte
	buf[3] = (address >> 8) & 0xFF;		// high byte
	buf[4] = value & 0xFF;				// low byte
	value = value >> 8;
	buf[5] = value & 0xFF;
	value = value >> 8;
	buf[6] = value & 0xFF;
	value = value >> 8;
	buf[7] = value & 0xFF;
	buf[8] = 0;
	if (Write(buf, 9) != 9) {
		quisk_sound_state.read_error++;
#if DEBUG
		printf ("set_ad6620 write error\n");
#endif
	}
#if DEBUG > 1
	printf ("set_ad6620 address 0x%X\n", address);
#endif
}

static void wset_ad6620(int address, int value)
{	// Set AD6620 register and wait for ACK
	int i;

	sdr_ack = -1;
	set_ad6620(address, value);
	for (i = 0; i < SDRIQ_TIMEOUT; i++) {
		sdr_recv(NULL, 0);
		if (sdr_ack != -1)
			break;
		QuiskSleepMicrosec(SDRIQ_MSEC * 1000);
	}
#if DEBUG
	if (sdr_ack != 1)
		printf ("Failed to get ACK for AD6620 address 0x%X\n", address);
#endif
}

static void set_freq_sdriq(void)		// Set SDR-IQ frequency
{
	char buf[8];
	int freq;

	freq = sdriq_freq;
	buf[0] = 0;
	buf[1] = freq & 0xFF;				// low byte
	freq = freq >> 8;
	buf[2] = freq & 0xFF;
	freq = freq >> 8;
	buf[3] = freq & 0xFF;
	freq = freq >> 8;
	buf[4] = freq & 0xFF;
	buf[5] = 1;
	set_item(0x0020, 6, buf);
	cur_freq = sdriq_freq;
}

static void set_gain_sdriq(void)
{
	char buf[2];

	switch (sdriq_gstate) {
	case 0:
		buf[0] = 0;
		buf[1] = sdriq_gain & 0xFF;
		break;
	case 1:
		buf[0] = 1;
		buf[1] = sdriq_gain & 0x7F;
		buf[1] |= 0x80;
		break;
	case 2:
		buf[0] = 1;
		buf[1] = sdriq_gain & 0x7F;
		break;
	}
	set_item(0x0038, 2, buf);
	cur_gstate = sdriq_gstate;
	cur_gain = sdriq_gain;
}

static void program_ad6620(void)		// Set registers
{
	int i;
	struct ad6620 *pt;

	switch (sdriq_decimation) {
		case 360:
			pt = &dec360;
			break;
		case 500:
			pt = &dec500;
			break;
		case 600:
			pt = &dec600;
			break;
		case 1250:
			pt = &dec1250;
			break;
		default:
			pt = &dec1250;
			break;
	}
	wset_ad6620(0x300, 1);		// soft reset
	for (i = 0; i < 256; i++)
		wset_ad6620(i, pt->coef[i]);
	wset_ad6620(0x301, 0);
	wset_ad6620(0x302, -1);
	wset_ad6620(0x303, 0);
	wset_ad6620(0x304, 0);
	wset_ad6620(0x305, pt->Scic2);
	wset_ad6620(0x306, pt->Mcic2 - 1);
	wset_ad6620(0x307, pt->Scic5);
	wset_ad6620(0x308, pt->Mcic5 - 1);
	wset_ad6620(0x309, pt->Sout);
	wset_ad6620(0x30A, pt->Mrcf - 1);
	wset_ad6620(0x30B, 0);
	wset_ad6620(0x30C, 255);
	wset_ad6620(0x30D, 0);
	set_freq_sdriq();
	set_gain_sdriq();
	wset_ad6620(0x300, 0);
	cur_decimation = sdriq_decimation;
}


#ifdef MS_WINDOWS
static void quisk_open_sdriq_dev(const char * name, char * buf, int bufsize)
{
#if DEBUG
	FT_STATUS ftStatus;
	FT_DEVICE_LIST_INFO_NODE *devInfo;
	DWORD numDevs;
	int i;

	// create the device information list
	ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (ftStatus == FT_OK) {
		printf("Number of devices is %d\n", (int)numDevs);
	}
	else {
		printf("Number of devices failed\n");
		numDevs = 0;
	 }
	if (numDevs > 0) {
		// allocate storage for list based on numDevs
		devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
		// get the device information list
		ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
		if (ftStatus == FT_OK) {
			for (i = 0; i < numDevs; i++) {
				printf("Dev %d:\n",i);
				printf(" Flags=0x%x\n", (unsigned int)devInfo[i].Flags);
				printf(" Type=0x%x\n", (unsigned int)devInfo[i].Type);
				printf(" ID=0x%x\n", (unsigned int)devInfo[i].ID);
				printf(" LocId=0x%x\n", (unsigned int)devInfo[i].LocId);
				printf(" SerialNumber=%s\n", devInfo[i].SerialNumber);
				printf(" Description=%s\n", devInfo[i].Description);
				printf(" ftHandle=0x%x\n", (unsigned int)devInfo[i].ftHandle);
			}
		}
		free(devInfo);
	}
#endif		// DEBUG

	if (FT_OpenEx ("SDR-IQ", FT_OPEN_BY_DESCRIPTION, &quisk_sdriq_fd) != FT_OK) {
		strncpy(buf, "Open SDR-IQ failed", bufsize);
		quisk_sdriq_fd = INVALID_HANDLE_VALUE;
		return;
	}
	if (FT_SetTimeouts(quisk_sdriq_fd, 2, 100) != FT_OK) {
		strncpy(buf, "Set Timeouts failed", bufsize);
		return;
	}
}
#else
static void quisk_open_sdriq_dev(const char * name, char * buf, int bufsize)
{
    struct termios newtio;

	if (!strncmp(name, "/dev/ttyUSB", 11)) {	// use ftdi_sio driver
		quisk_sdriq_fd = open(name, O_RDWR | O_NOCTTY);
		if (quisk_sdriq_fd < 0) {
			strncpy(buf, "Open SDR-IQ : ", bufsize);
			strncat(buf, strerror(errno), bufsize - strlen(buf) - 1);
			quisk_sdriq_fd = INVALID_HANDLE_VALUE;
			return;
		}
		bzero(&newtio, sizeof(newtio));
		newtio.c_cflag = CS8 | CLOCAL | CREAD;
		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;
		cfsetispeed(&newtio, B230400);
		cfsetospeed(&newtio, B230400);
		newtio.c_lflag = 0;
		newtio.c_cc[VTIME]    = 0;	// specify non-blocking read 
		newtio.c_cc[VMIN]     = 0;
		tcflush(quisk_sdriq_fd, TCIFLUSH);
		tcsetattr(quisk_sdriq_fd, TCSANOW, &newtio);
	}
	else {		// use ft245 or similar driver
		quisk_sdriq_fd = open(name, O_RDWR | O_NONBLOCK); 
		if (quisk_sdriq_fd < 0) {
			strncpy(buf, "Open SDR-IQ: ", bufsize);
			strncat(buf, strerror(errno), bufsize - strlen(buf) - 1);
			quisk_sdriq_fd = INVALID_HANDLE_VALUE;
			return;
		}
	}
	return;
}
#endif

static int quisk_open_sdriq(const char * name, char * buf, int bufsize)
{
	char buf1024[1024];
	int i, freq;

    fprintf (stderr, "********** %s\n", __FUNCTION__);
	quisk_open_sdriq_dev(name, buf, bufsize);
	if (quisk_sdriq_fd == INVALID_HANDLE_VALUE)
		return -1;		// error

	sdr_name[0] = 0;
	sdr_serial[0] = 0;
	sdr_idle = -1;			// unknown state
	set_item(0x0018, 4, "\x81\x01\x00\x00");
	QuiskSleepMicrosec(1000000);
	while (1) {		// read and discard any available output
		if (Read(buf1024, 1024) == 0)
			break;
	}
	set_item(0x0018, 4, "\x81\x01\x00\x00");
    get_item(0x0002, 0, NULL);		// request serial number
    get_item(0x0003, 0, NULL);		// request interface Version
    get_item(0x0004, 0, NULL);		// request harwdware/firmware version 
    //get_item(0x0004, 1, "0x00");	// request harwdware/firmware version 
    //get_item(0x0004, 1, "0x01");	// request harwdware/firmware version 
    get_item(0x0005, 0, NULL);		// request status
    get_item(0x0009, 0, NULL);		// request product ID
	// set sample rate
	freq = sdriq_clock;
	buf1024[0] = 0;
	buf1024[1] = freq & 0xFF;				// low byte
	freq = freq >> 8;
	buf1024[2] = freq & 0xFF;
	freq = freq >> 8;
	buf1024[3] = freq & 0xFF;
	freq = freq >> 8;
	buf1024[4] = freq & 0xFF;
	set_item(0x00B0, 5, buf1024);	// set actual clock speed
    get_item(0x0001, 0, NULL);		// request name
	// loop for input
	for (i = 0; i < SDRIQ_TIMEOUT; i++) {
		sdr_recv(NULL, 0);
		if (sdr_name[0] != 0)
			break;
		QuiskSleepMicrosec(SDRIQ_MSEC * 1000);
	}
	if (sdr_name[0]) {		// we got a response
		snprintf(buf, bufsize, "Capture from %s serial %s.",
			sdr_name, sdr_serial);
		program_ad6620();
	}
	else {
		snprintf(buf, bufsize, "No response from SDR-IQ");
	}
#if DEBUG
	printf ("%s\n", buf);
#endif
    return 0;
}

static void WaitForPoll(void)
{
    #if 1
	static double time0 = 0;	// time in seconds
	double timer;				// time remaining from last poll usec

	timer = quisk_sound_state.data_poll_usec - (QuiskTimeSec() - time0) * 1e6;
	if (timer > 1000.0)	// see if enough time has elapsed
		QuiskSleepMicrosec((int)timer);		// wait for the remainder of the poll interval
	time0 = QuiskTimeSec();		// reset starting time value
    #endif
}

// End of most SDR-IQ specific code.


#if 0
///////////////////////////////////////////////////////////////////////////
// The API requires at least two Python functions for Open and Close, plus
// additional Python functions as needed.  And it requires exactly three
// C funcions for Start, Stop and Read samples.  Quisk runs in two threads,
// a GUI thread and a sound thread.  You must not call the GUI or any Python
// code from the sound thread.  You must return promptly from functions called
// by the sound thread.
//
// The calling sequence is Open, Start, then repeated calls to Read, then
// Stop, then Close.

// Start of Application Programming Interface (API) code:

// Start sample capture; called from the sound thread.
static void quisk_start_sdriq(void)
{
	if (sdr_idle != 2)
		set_item(0x0018, 4, "\x81\x02\x00\x01");
}

// Stop sample capture; called from the sound thread.
static void quisk_stop_sdriq(void)
{
	int msec;
	complex samples[2048];

	for (msec = 0; msec < 1001; msec++) {
		if (msec % 100 == 0)
			set_item(0x0018, 4, "\x81\x01\x00\x00");
		sdr_recv(samples, 2048);
		if (sdr_idle == 1)
			break;
		QuiskSleepMicrosec(1000);
	}
#if DEBUG
	if (msec < 1001)
		printf("quisk_stop_sdriq succeeded\n");
	else
		printf("quisk_stop_sdriq timed out\n");
#endif
}

// Called in a loop to read samples; called from the sound thread.
static int quisk_read_sdriq (complex * cSamples)
{
	int length;

	WaitForPoll();
	if (quisk_sdriq_fd == INVALID_HANDLE_VALUE)
		return -1;		// sdriq is closed
	length = sdr_recv(cSamples, SAMP_BUFFER_SIZE);	// get all available samples
	if (cur_freq != sdriq_freq)		// check frequency
		set_freq_sdriq();
	if (cur_gstate != sdriq_gstate || cur_gain != sdriq_gain)	// check gain
		set_gain_sdriq();
	if (cur_decimation != sdriq_decimation) {		// check decimation
		quisk_stop_sdriq();
		program_ad6620();
		quisk_start_sdriq();
	}
	return length;	// return number of samples
}

// Called to close the sample source; called from the GUI thread.
static PyObject * close_samples(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple (args, ""))
		return NULL;
	if (quisk_sdriq_fd != INVALID_HANDLE_VALUE) {
		sdr_idle = -1;		// unknown state
#ifdef MS_WINDOWS
		FT_Close(quisk_sdriq_fd);
#else
		close(quisk_sdriq_fd);
#endif
		quisk_sdriq_fd = INVALID_HANDLE_VALUE;
	}
	Py_INCREF (Py_None);
	return Py_None;
}

// Called to open the sample source; called from the GUI thread.
static PyObject * open_samples(PyObject * self, PyObject * args)
{
	const char * name;
	char buf[128];

	if (!PyArg_ParseTuple (args, ""))
		return NULL;

	name = QuiskGetConfigString("sdriq_name", "NoName");
	sdriq_clock = QuiskGetConfigDouble("sdriq_clock", 66666667.0);

// Record our C-language Start/Stop/Read functions for use by sound.c.
	pt_sample_start = &quisk_start_sdriq;
	pt_sample_stop = &quisk_stop_sdriq;
	pt_sample_read = &quisk_read_sdriq;
//////////////
	quisk_open_sdriq(name, buf, 128);		// SDR-IQ specific
	return PyString_FromString(buf);		// return a string message
}

// Miscellaneous functions needed by the SDR-IQ; called from the GUI thread as
// a result of button presses.

// Set the receive frequency; called from the GUI thread.
static PyObject * freq_sdriq(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple (args, "i", &sdriq_freq))
		return NULL;
	Py_INCREF (Py_None);
	return Py_None;
}

// Set the preamp gain; called from the GUI thread.
static PyObject * gain_sdriq(PyObject * self, PyObject * args)	// Called from GUI thread
{	// gstate == 0:  Gain must be 0, -10, -20, or -30
	// gstate == 1:  Attenuator is on  and gain is 0 to 127 (7 bits)
	// gstate == 2:  Attenuator is off and gain is 0 to 127 (7 bits)

	if (!PyArg_ParseTuple (args, "ii", &sdriq_gstate, &sdriq_gain))
		return NULL;
	Py_INCREF (Py_None);
	return Py_None;
}

// Set the decimation; called from the GUI thread.
static PyObject * set_decimation(PyObject * self, PyObject * args)
{
	if (!PyArg_ParseTuple (args, "i", &sdriq_decimation))
		return NULL;
	Py_INCREF (Py_None);
	return Py_None;
}

// Functions callable from Python are listed here:
static PyMethodDef QuiskMethods[] = {
	{"open_samples", open_samples, METH_VARARGS, "Open the RfSpace SDR-IQ."},
	{"close_samples", close_samples, METH_VARARGS, "Close the RfSpace SDR-IQ."},
	{"freq_sdriq", freq_sdriq, METH_VARARGS, "Set the frequency of the SDR-IQ"},
	{"gain_sdriq", gain_sdriq, METH_VARARGS, "Set the gain of the SDR-IQ"},
	{"set_decimation", set_decimation, METH_VARARGS, "Set the decimation of the SDR-IQ"},
	{NULL, NULL, 0, NULL}		/* Sentinel */
};

// Initialization, and registration of public symbol "initsdriq":
PyMODINIT_FUNC initsdriq (void)
{
	Py_InitModule ("sdriq", QuiskMethods);
}
#else

#define SAMP_BUFFER_SIZE      66000 

void quisk_start_sdriq(void)
{
    fprintf (stderr, "%s\n", __FUNCTION__);
	if (sdr_idle != 2)
		set_item(0x0018, 4, "\x81\x02\x00\x01");
}

// Stop sample capture; called from the sound thread.
void quisk_stop_sdriq(void)
{
	int msec;
    complex  cc[2048];

    fprintf (stderr, "%s\n", __FUNCTION__);
	for (msec = 0; msec < 1001; msec++) {
		if (msec % 100 == 0)
			set_item(0x0018, 4, "\x81\x01\x00\x00");
		sdr_recv(cc, 2048);
		if (sdr_idle == 1)
			break;
		QuiskSleepMicrosec(1000);
	}
#if DEBUG
	if (msec < 1001)
		printf("quisk_stop_sdriq succeeded\n");
	else
		printf("quisk_stop_sdriq timed out\n");
#endif
}

// Called in a loop to read samples; called from the sound thread.
int quisk_read_sdriq(complex * cSamples)
{
	int length;

	WaitForPoll();
	if (quisk_sdriq_fd == INVALID_HANDLE_VALUE)
		return -1;		// sdriq is closed
	length = sdr_recv(cSamples, SAMP_BUFFER_SIZE);	// get all available samples
	if (cur_freq != sdriq_freq)		// check frequency
		set_freq_sdriq();
	if (cur_gstate != sdriq_gstate || cur_gain != sdriq_gain)	// check gain
		set_gain_sdriq();
	if (cur_decimation != sdriq_decimation) {		// check decimation
		quisk_stop_sdriq();
		program_ad6620();
		quisk_start_sdriq();
	}
	return length;	// return number of samples
}

// Called to close the sample source; called from the GUI thread.
void close_samples ()
{
    fprintf (stderr, "%s\n", __FUNCTION__);
	if (quisk_sdriq_fd != INVALID_HANDLE_VALUE) {
		sdr_idle = -1;		// unknown state
#ifdef MS_WINDOWS
		FT_Close(quisk_sdriq_fd);
#else
		close(quisk_sdriq_fd);
#endif
		quisk_sdriq_fd = INVALID_HANDLE_VALUE;
	}
}

int open_samples(const char *n, const char *clock, char* buf)
{
	const char * name;
	//static char buf[128];

    fprintf (stderr, "%s\n", __FUNCTION__);

    if (n) {
        name = n;
    } else {
        name = "NoName";
    }
	//name = QuiskGetConfigString("sdriq_name", "NoName");
	//sdriq_clock = QuiskGetConfigDouble("sdriq_clock", 66666667.0);

    sdriq_clock = 66666667.0;

// Record our C-language Start/Stop/Read functions for use by sound.c.
//	pt_sample_start = &quisk_start_sdriq;
//	pt_sample_stop = &quisk_stop_sdriq;
//	pt_sample_read = &quisk_read_sdriq;
//////////////
	return quisk_open_sdriq(name, buf, 128);		// SDR-IQ specific
}

// Miscellaneous functions needed by the SDR-IQ; called from the GUI thread as
// a result of button presses.

// Set the receive frequency; called from the GUI thread.
void freq_sdriq (int f)
{
	sdriq_freq = f;
}

// Set the preamp gain; called from the GUI thread.
void gain_sdriq (int gain_state, int gain)
{	// gstate == 0:  Gain must be 0, -10, -20, or -30
	// gstate == 1:  Attenuator is on  and gain is 0 to 127 (7 bits)
	// gstate == 2:  Attenuator is off and gain is 0 to 127 (7 bits)

	sdriq_gstate = gain_state;
    sdriq_gain   = gain;
}

// Set the decimation; called from the GUI thread.
void set_decimation (int decimation)
{
	sdriq_decimation = decimation;
}

void set_bandwidth (int nb)
{
    fprintf (stderr, "%s: BW: %d\n", __FUNCTION__, nb);
    switch (nb) {
    case 53333:      // 66666667.0 / 1250
        set_decimation (1250);
        break;
    case 111111:     // 66666667.0 / 600
        set_decimation (600);
        break;
    case 133333:     // 66666667.0 / 500
        set_decimation (500);
        break;
    case 185185:     // 66666667.0 / 360
        set_decimation (360);
        break;
    }
}

const char *get_serial ()
{
    return sdr_serial;
}

#endif

const QUISK_SOUND_STATE *get_state (void)
{
    return &quisk_sound_state;
}

#include <pthread.h>

struct AsynchCtxData {
    void     *pUserData;
    SDRIQ_CB  udcf ;
    int       run;
};

typedef struct _hq {
    pthread_t             thread_id;
    struct AsynchCtxData *pacd     ;
} HQ;

HQ hq = { -1, 0 };



static void *asynch_input_thread (void *p)
{
    static SAMPLE_T ii [SAMP_BUFFER_SIZE]; 
    static SAMPLE_T qq [SAMP_BUFFER_SIZE]; 
    static complex  cc [SAMP_BUFFER_SIZE]; 

    struct AsynchCtxData *pacd = (struct AsynchCtxData *)p;

    quisk_start_sdriq ();
    fprintf (stderr, "%s STARTED %p %p %p\n", __FUNCTION__, ii, qq, cc);
    while (pacd->run) {
        //unsigned char buffer [10240];
        //int buf_len = get_data (&hq, buffer, sizeof(buffer));quisk_read_sdriq_ul (ii, qq);

        int buf_len = quisk_read_sdriq (cc);

        //fprintf (stderr, "%s: buf_len: %d\n", __FUNCTION__, buf_len);
        if (buf_len > 0) {
           int x;
           for (x=0; x < buf_len; ++x) {
               ii[x] = crealf(cc[x]);
               qq[x] = cimagf(cc[x]);
           }
           //fprintf (stderr, "buf_len: %d\n", buf_len);
           #if 0
           for (j=0; buf_len > 5 && j<5; ++j) {
               fprintf(stderr, "(%f + i%f)", crealf(cc[j]), cimagf(cc[j]));
           }
           fprintf (stderr, "\n");
           for (j=0; buf_len > 5 && j<5; ++j) {
               fprintf(stderr, "(%f + i%f)", ii[j], qq[j] );
           }
           fprintf (stderr, "\n");
           #endif
           int rcb = pacd->udcf (ii, qq, buf_len, pacd->pUserData);
           if (rcb < 0) {
               fprintf (stderr, "SDR-IQ thread exiting with user callback returning: %d\n", rcb);
               pacd->run = 0;
           }
           
        } else if (buf_len < 0) {
            // user callback is asked for termination
            pacd->udcf ((void *)0, (void *)0, 0, 0);
            break;
        }
    }
    fprintf (stderr, "SDR-IQ thread exiting with RUN: %d\n", pacd->run);
    quisk_stop_sdriq ();
    return 0;
}

int sdriq_start_asynch_input (SDRIQ_CB cb, void *pud)
{
    int rc;

    fprintf (stderr, "%s\n", __FUNCTION__);
    if (hq.pacd) free(hq.pacd);

    hq.pacd = malloc( sizeof(struct AsynchCtxData));

    hq.pacd->pUserData = pud;
    hq.pacd->udcf = cb;
    hq.pacd->run = 1;

    // create the thread to receive data
    rc = pthread_create(&(hq.thread_id),NULL,asynch_input_thread,(void *)(hq.pacd));
    if(rc < 0) {
        perror("pthread_create asynch_input_thread failed");
    }
    return rc;
    
}


int sdriq_stop_asynch_input ()
{
    int rc = -1;

    fprintf (stderr, "%s\n", __FUNCTION__);
    if (hq.thread_id != 0) {
        void *pExit;

        hq.pacd->run = 0;
        rc = pthread_join (hq.thread_id, &pExit);
    }

    //hiqsdr_disconnect ();
    return rc;
}


#if defined __TEST_MODULE__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

complex cSamples  [SAMP_BUFFER_SIZE]; 

unsigned long ii [SAMP_BUFFER_SIZE]; 
unsigned long qq [SAMP_BUFFER_SIZE]; 

struct timespec diff(struct timespec start, struct timespec end)
{
        struct timespec temp;
        if ((end.tv_nsec - start.tv_nsec) < 0) {
                temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
                temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
        } else {
                temp.tv_sec  = end.tv_sec  - start.tv_sec;
                temp.tv_nsec = end.tv_nsec - start.tv_nsec;
        }
        return temp;
}

int asynch_callback (SAMPLE_T *pi, SAMPLE_T *pq, int bsize, void *data)
{    
    int *pndp = (int *) data;

    if (pi && pq && *pndp) {
        fprintf (stderr, "%s: %d: %d\n", __FUNCTION__, *pndp, bsize);
        *pndp = (*pndp) - 1;
        if (bsize > 0) {
            int j;
            for (j=0; j<5; ++j) fprintf(stderr, "%f+j%f ", *pi++, *pq++);
            fprintf (stderr, "\n");
        }
        return 0;
    } else 
        return -1;

}



void test_multiple_sr (int dec)
{
    int i;
    long int ts = 0;
    struct timespec  time_start;
    struct timespec  time_end;
    struct timespec  time_diff;
    long double diff_s ;

    set_decimation (dec);
    quisk_start_sdriq ();


    fprintf(stderr, "Listening.....\n");
    fflush (stderr);

    clock_gettime (CLOCK_REALTIME, &time_start);
    for (i=0; i<1000; ++i) {
        int j;
        //int n = quisk_read_sdriq (cSamples);
        int n = quisk_read_sdriq (cSamples);
        //fprintf (stderr, "**** samples[%d]: %d\n", i, n );
        //fflush (stderr);
        ts += n;
        #if 0
        for (j=0; n > 0 && j<20; ++j) {
            fprintf(stderr, "%f + %f ", creal(cSamples[j]), cimag(cSamples[j]));
        }
        #endif
    }
    clock_gettime(CLOCK_REALTIME, &time_end);
    time_diff = diff(time_start, time_end);
    diff_s = time_diff.tv_sec + (time_diff.tv_nsec/1E9) ;
    //fprintf(stderr, "diff: %ds::%dns %Lf seconds\n", time_diff.tv_sec, time_diff.tv_nsec, diff_s);
    fprintf (stderr, "Samples received: %lu, %.3Lf kS/s\n", ts, ((double)ts / (diff_s)/1E3) );
    fflush (stderr);
    quisk_stop_sdriq ();


}

int main ()
{
	char buf[128];
    int i, rc;

#if DEBUG
    fprintf (stderr, "**** DEBUG ACTIVE: %d\n", DEBUG);

    fprintf (stderr, "short: %d int: %d long: %d\n", sizeof(short), sizeof(int), sizeof(long) );
#endif
    
    rc = open_samples("/dev/ttyUSB0", "66666667.0", buf) ;
    fflush (stderr);

    if (rc != 0) {
        fprintf (stderr, "**** OPEN FAILED: [%s]\n", buf );
        return 255;
    } else
        fprintf (stderr, "**** OPEN SUCCESS: %s\n", buf );

    fprintf (stderr, "Name: [%s] Serial: [%s] Interface: %d Firmware: %d Bootcode: %d Product Id: %08lX\n",
             sdr_name, sdr_serial, sdr_interface, sdr_firmware, sdr_bootcode, *((unsigned long *)sdr_product_id) );

    fflush (stderr);
 
    //gain_sdriq (0, -30);
    gain_sdriq (1, 1);


    #if 1
    test_multiple_sr (1250);
    test_multiple_sr (600);
    test_multiple_sr (500);
    test_multiple_sr (360);

    #else
    set_decimation (1250);
    {
        static int ndp = 5;

        fprintf (stderr, "\nAsyhchronously listening for %d data packets.....\n", ndp);

        sdriq_start_asynch_input (asynch_callback, (void *)&ndp);

        while (ndp) {
            sleep (1);
        };

        sdriq_stop_asynch_input (); // necessary in order to avoid memory leaks

        fprintf (stderr, "...............\n");

    }
    #endif

    // Stop sample capture; called from the sound thread.
    fflush (stderr);


    close_samples ();

    fprintf (stderr, "Errors: %d overrange: %d poll: %d\n", 
             quisk_sound_state.read_error, 
             quisk_sound_state.overrange, 
             quisk_sound_state.data_poll_usec 
            );

    return 0;
}
#endif

